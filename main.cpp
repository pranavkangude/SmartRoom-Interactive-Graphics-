// ============================================================
//  SmartRoom - 2D Room & Furniture Layout Planner
//  C++ / OpenGL (FreeGLUT)             *** Version 2 (M1 - M7) ***
//    M1 window + 2D coordinate system     M2 scaled room, grid, door, window
//    M3 furniture drawing                 M4 translate / rotate / scale
//    M5 mouse picking, dragging, keys     M6 snap-to-grid + boundary/collision (SAT)
//    M7 NEW: undo/redo, save/load, zoom & pan, door-swing clearance zone
//
//  Build (your setup, PowerShell):
//  g++ main.cpp -o smartroom.exe "-Ifreeglut-mingw-3.8.0/freeglut/include" "-Lfreeglut-mingw-3.8.0/freeglut/lib" -lfreeglut -lopengl32 -lglu32
// ============================================================
#include <GL/freeglut.h>   // freeglut.h (not glut.h) declares glutMouseWheelFunc
#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

const float PI = 3.14159265f;

// ---------------------- Data structures ----------------------
enum FType { BED, SOFA, TABLE, CHAIR, WARDROBE, DESK, FTYPE_COUNT };
const char* FNAMES[FTYPE_COUNT] = {"Bed", "Sofa", "Table", "Chair", "Wardrobe", "Desk"};

struct Furniture {
    int   type;
    float x, y;       // centre position (world units = feet)
    float w, h;       // width (x) and height (y) before rotation
    float angle;      // rotation in degrees
    float col[3];     // base RGB colour
};
struct Vec2 { float x, y; };

// ---------------------- Global state -------------------------
int   winW = 1000, winH = 700;
float roomW = 12.0f, roomL = 10.0f;      // room size in feet

// view (window-to-viewport mapping + zoom/pan)
float baseScale = 1, baseOffX = 0, baseOffY = 0;   // "fit room in window" view
float zoom = 1, panX = 0, panY = 0;                // user zoom and pan (pixels)
float scaleF = 1, offX = 0, offY = 0;              // final world -> screen mapping
bool  panning = false;
int   lastMX = 0, lastMY = 0;

float gridSize = 0.5f;                   // snap step in feet
bool  snapOn = true;

// door (bottom wall): hinge at DOOR_X, opening width DOOR_W, swings inside the room
const float DOOR_X = 1.5f, DOOR_W = 3.0f;
bool  doorClear = true;                  // door-swing clearance zone on/off

vector<Furniture> items;
int   sel = -1;
bool  dragging = false, dragMoved = false;
float dragDX = 0, dragDY = 0;            // offset between mouse and item centre
float backX = 0, backY = 0;              // position before drag (to revert if invalid)
string statusMsg = "Press 1-6 to add furniture. Click an item to select it.";

// undo / redo stacks (each entry = full snapshot of the layout)
vector< vector<Furniture> > undoStack, redoStack;

Furniture makeItem(int type);            // forward declaration

// ---------------------- M2/M7: view mapping ------------------
void applyView() {
    scaleF = baseScale * zoom;
    offX = baseOffX + panX;
    offY = baseOffY + panY;
}

// Window-to-viewport mapping: fit the room inside the window with a margin.
void computeView() {
    const float margin = 70.0f;
    float sx = (winW - 2 * margin) / roomW;
    float sy = (winH - 2 * margin) / roomL;
    baseScale = min(sx, sy);
    baseOffX = (winW - roomW * baseScale) / 2.0f;
    baseOffY = (winH - roomL * baseScale) / 2.0f - 25.0f;
    applyView();
}

// Screen (mouse, origin top-left) -> world (feet, origin bottom-left of room)
Vec2 toWorld(int mx, int my) {
    float sy = (float)(winH - my);           // flip Y
    return { (mx - offX) / scaleF, (sy - offY) / scaleF };
}

// Zoom keeping the world point under (mx,my) fixed on screen
void zoomAt(int mx, int my, float factor) {
    Vec2 w = toWorld(mx, my);
    zoom = min(max(zoom * factor, 0.3f), 6.0f);
    scaleF = baseScale * zoom;
    offX = mx - w.x * scaleF;
    offY = (winH - my) - w.y * scaleF;
    panX = offX - baseOffX;
    panY = offY - baseOffY;
}

// ---------------------- M4: geometry helpers -----------------
// Rotate point p about centre c by 'deg' degrees (formula from the project notes)
Vec2 rotateAbout(Vec2 p, Vec2 c, float deg) {
    float t = deg * PI / 180.0f, cs = cosf(t), sn = sinf(t);
    float dx = p.x - c.x, dy = p.y - c.y;
    return { c.x + dx * cs - dy * sn, c.y + dx * sn + dy * cs };
}

void getCorners(const Furniture& f, Vec2 out[4]) {
    float hw = f.w / 2, hh = f.h / 2;
    Vec2 c = {f.x, f.y};
    Vec2 raw[4] = {{f.x - hw, f.y - hh}, {f.x + hw, f.y - hh},
                   {f.x + hw, f.y + hh}, {f.x - hw, f.y + hh}};
    for (int i = 0; i < 4; i++) out[i] = rotateAbout(raw[i], c, f.angle);
}

// Picking: rotate the point by -theta about the centre, then AABB test.
bool hitTest(const Furniture& f, Vec2 p) {
    Vec2 q = rotateAbout(p, {f.x, f.y}, -f.angle);
    return fabsf(q.x - f.x) <= f.w / 2 && fabsf(q.y - f.y) <= f.h / 2;
}

// ---------------------- M6/M7: validation --------------------
bool insideRoom(const Furniture& f) {
    Vec2 c[4];
    getCorners(f, c);
    const float e = 0.001f;
    for (int i = 0; i < 4; i++)
        if (c[i].x < -e || c[i].x > roomW + e || c[i].y < -e || c[i].y > roomL + e)
            return false;
    return true;
}

// Separating Axis Theorem for two (possibly rotated) rectangles.
bool satOverlap(const Furniture& a, const Furniture& b) {
    Vec2 ca[4], cb[4];
    getCorners(a, ca);
    getCorners(b, cb);
    float angs[4] = {a.angle, a.angle + 90, b.angle, b.angle + 90};
    const float eps = 0.001f;
    for (int k = 0; k < 4; k++) {
        float t = angs[k] * PI / 180.0f, ax = cosf(t), ay = sinf(t);
        float minA = 1e9, maxA = -1e9, minB = 1e9, maxB = -1e9;
        for (int i = 0; i < 4; i++) {
            float pa = ca[i].x * ax + ca[i].y * ay;
            float pb = cb[i].x * ax + cb[i].y * ay;
            minA = min(minA, pa); maxA = max(maxA, pa);
            minB = min(minB, pb); maxB = max(maxB, pb);
        }
        if (maxA <= minB + eps || maxB <= minA + eps) return false; // gap found
    }
    return true; // no separating axis -> overlap
}

bool hasDoor() { return roomW >= 6; }

// Door-swing clearance: the quarter-disc (radius DOOR_W) swept by the door leaf.
// Sample the quarter disc and test whether any sample lies inside the furniture.
bool doorBlocked(const Furniture& f) {
    if (!doorClear || !hasDoor()) return false;
    for (float x = 0; x <= DOOR_W; x += 0.25f)
        for (float y = 0; y <= DOOR_W; y += 0.25f) {
            if (x * x + y * y > DOOR_W * DOOR_W) continue;
            if (hitTest(f, {DOOR_X + x, y})) return true;
        }
    return false;
}

bool isValid(int idx) {
    if (!insideRoom(items[idx])) return false;
    if (doorBlocked(items[idx])) return false;
    for (int j = 0; j < (int)items.size(); j++)
        if (j != idx && satOverlap(items[idx], items[j])) return false;
    return true;
}

// ---------------------- M7: undo / redo ----------------------
void commitUndo(const vector<Furniture>& before) {
    undoStack.push_back(before);
    if (undoStack.size() > 100) undoStack.erase(undoStack.begin());
    redoStack.clear();
}
void pushUndo() { commitUndo(items); }

void doUndo() {
    if (undoStack.empty()) { statusMsg = "Nothing to undo."; return; }
    redoStack.push_back(items);
    items = undoStack.back();
    undoStack.pop_back();
    sel = -1;
    statusMsg = "Undo.";
}
void doRedo() {
    if (redoStack.empty()) { statusMsg = "Nothing to redo."; return; }
    undoStack.push_back(items);
    items = redoStack.back();
    redoStack.pop_back();
    sel = -1;
    statusMsg = "Redo.";
}

// ---------------------- M7: save / load ----------------------
const char* SAVE_FILE = "layout.txt";

void saveLayout() {
    ofstream out(SAVE_FILE);
    if (!out) { statusMsg = "Could not write layout.txt!"; return; }
    out << "SMARTROOM 1\n" << roomW << " " << roomL << "\n" << items.size() << "\n";
    out << fixed << setprecision(3);
    for (auto& f : items)
        out << f.type << " " << f.x << " " << f.y << " " << f.w << " " << f.h << " " << f.angle << "\n";
    statusMsg = "Layout saved to layout.txt";
}

void loadLayout() {
    ifstream in(SAVE_FILE);
    if (!in) { statusMsg = "No layout.txt found (press S to save first)."; return; }
    string magic; int ver = 0;
    float rw, rl; int n;
    in >> magic >> ver >> rw >> rl >> n;
    if (!in || magic != "SMARTROOM" || rw < 4 || rw > 60 || rl < 4 || rl > 60 || n < 0 || n > 500) {
        statusMsg = "layout.txt is invalid or corrupted.";
        return;
    }
    vector<Furniture> loaded;
    for (int i = 0; i < n; i++) {
        int t; float x, y, w, h, a;
        in >> t >> x >> y >> w >> h >> a;
        if (!in || t < 0 || t >= FTYPE_COUNT) { statusMsg = "layout.txt is invalid or corrupted."; return; }
        Furniture f = makeItem(t);
        f.x = x; f.y = y; f.w = w; f.h = h; f.angle = a;
        loaded.push_back(f);
    }
    roomW = rw; roomL = rl;
    items = loaded;
    sel = -1;
    undoStack.clear(); redoStack.clear();
    zoom = 1; panX = panY = 0;
    computeView();
    statusMsg = "Layout loaded (" + to_string(n) + " items).";
}

// ---------------------- M3: drawing primitives ---------------
void fillRect(float x0, float y0, float x1, float y1, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}
void strokeRect(float x0, float y0, float x1, float y1, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}
void fillEllipse(float cx, float cy, float rx, float ry, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 48; i++) {
        float t = 2 * PI * i / 48;
        glVertex2f(cx + rx * cosf(t), cy + ry * sinf(t));
    }
    glEnd();
}
void strokeEllipse(float cx, float cy, float rx, float ry, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 48; i++) {
        float t = 2 * PI * i / 48;
        glVertex2f(cx + rx * cosf(t), cy + ry * sinf(t));
    }
    glEnd();
}

// Draws one furniture piece centred on (0,0) in local (feet) coordinates.
void drawShape(const Furniture& f, bool valid) {
    float hw = f.w / 2, hh = f.h / 2;
    float r = f.col[0], g = f.col[1], b = f.col[2];
    if (!valid) { r = 0.90f; g = 0.25f; b = 0.25f; }           // red = invalid placement
    float dr = r * 0.75f, dg = g * 0.75f, db = b * 0.75f;      // darker detail colour

    switch (f.type) {
    case BED:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, -hh, hw, hh - 1.7f, dr, dg, db);                    // blanket
        fillRect(-hw + 0.3f, hh - 1.4f, -0.1f, hh - 0.3f, 0.97f, 0.97f, 0.97f); // pillow 1
        fillRect(0.1f, hh - 1.4f, hw - 0.3f, hh - 0.3f, 0.97f, 0.97f, 0.97f);   // pillow 2
        break;
    case SOFA:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, hh - 0.7f, hw, hh, dr, dg, db);                      // backrest (top)
        fillRect(-hw, -hh, -hw + 0.6f, hh - 0.7f, dr, dg, db);             // left arm
        fillRect(hw - 0.6f, -hh, hw, hh - 0.7f, dr, dg, db);               // right arm
        break;
    case TABLE:
        fillEllipse(0, 0, hw, hh, r, g, b);
        fillEllipse(0, 0, hw * 0.7f, hh * 0.7f, dr, dg, db);
        break;
    case CHAIR:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, hh - 0.3f, hw, hh, dr, dg, db);                      // backrest
        break;
    case WARDROBE:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        glColor3f(dr, dg, db); glLineWidth(2);
        glBegin(GL_LINES); glVertex2f(0, -hh); glVertex2f(0, hh); glEnd();  // door split
        glColor3f(0.1f, 0.1f, 0.1f); glPointSize(5);
        glBegin(GL_POINTS); glVertex2f(-0.2f, 0); glVertex2f(0.2f, 0); glEnd(); // handles
        break;
    case DESK:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-0.6f, hh - 0.6f, 0.6f, hh - 0.2f, 0.15f, 0.15f, 0.2f);   // monitor
        fillRect(-0.5f, hh - 1.2f, 0.5f, hh - 0.8f, dr, dg, db);           // keyboard
        break;
    }
}

void drawFurniture(int idx) {
    const Furniture& f = items[idx];
    bool valid = isValid(idx);
    bool selected = (idx == sel);

    glPushMatrix();
    glTranslatef(f.x, f.y, 0);          // 3) move to position
    glRotatef(f.angle, 0, 0, 1);        // 2) rotate about own centre
    drawShape(f, valid);                // 1) shape defined around (0,0)

    // outline: blue when selected, dark red when invalid, black otherwise
    if (selected)      glColor3f(0.0f, 0.35f, 1.0f);
    else if (!valid)   glColor3f(0.6f, 0.0f, 0.0f);
    else               glColor3f(0.1f, 0.1f, 0.1f);
    float lw = selected ? 3.5f : 1.5f;
    if (f.type == TABLE) strokeEllipse(0, 0, f.w / 2, f.h / 2, lw);
    else                 strokeRect(-f.w / 2, -f.h / 2, f.w / 2, f.h / 2, lw);
    glPopMatrix();
}

// ---------------------- M2: room drawing ---------------------
void drawRoom() {
    // floor
    fillRect(0, 0, roomW, roomL, 0.93f, 0.90f, 0.82f);

    // grid (every foot; every 5 feet slightly darker)
    glLineWidth(1);
    for (int i = 0; i <= (int)roomW; i++) {
        if (i % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        glBegin(GL_LINES); glVertex2f((float)i, 0); glVertex2f((float)i, roomL); glEnd();
    }
    for (int j = 0; j <= (int)roomL; j++) {
        if (j % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        glBegin(GL_LINES); glVertex2f(0, (float)j); glVertex2f(roomW, (float)j); glEnd();
    }

    // walls
    glColor3f(0.15f, 0.15f, 0.15f);
    glLineWidth(8);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0); glVertex2f(roomW, 0); glVertex2f(roomW, roomL); glVertex2f(0, roomL);
    glEnd();

    if (hasDoor()) {
        float dx0 = DOOR_X, dx1 = DOOR_X + DOOR_W;

        // door-swing clearance zone (translucent quarter disc)
        if (doorClear) {
            glColor4f(0.2f, 0.45f, 1.0f, 0.18f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(dx0, 0);
            for (int i = 0; i <= 24; i++) {
                float t = (PI / 2) * i / 24;
                glVertex2f(dx0 + DOOR_W * cosf(t), DOOR_W * sinf(t));
            }
            glEnd();
        }

        glColor3f(0.93f, 0.90f, 0.82f); glLineWidth(10);           // opening in the wall
        glBegin(GL_LINES); glVertex2f(dx0, 0); glVertex2f(dx1, 0); glEnd();
        glColor3f(0.35f, 0.2f, 0.1f); glLineWidth(3);
        glBegin(GL_LINES); glVertex2f(dx0, 0); glVertex2f(dx0, DOOR_W); glEnd(); // door leaf
        glColor3f(0.35f, 0.2f, 0.1f); glLineWidth(1);
        glBegin(GL_LINE_STRIP);                                                  // swing arc
        for (int i = 0; i <= 24; i++) {
            float t = (PI / 2) * i / 24;
            glVertex2f(dx0 + DOOR_W * cosf(t), DOOR_W * sinf(t));
        }
        glEnd();

        // window on the top wall (4 ft wide)
        float wx0 = roomW / 2 - 2.0f, wx1 = roomW / 2 + 2.0f;
        glColor3f(0.65f, 0.85f, 0.95f); glLineWidth(8);
        glBegin(GL_LINES); glVertex2f(wx0, roomL); glVertex2f(wx1, roomL); glEnd();
        glColor3f(0.2f, 0.4f, 0.6f); glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2f(wx0, roomL + 0.06f); glVertex2f(wx1, roomL + 0.06f);
        glVertex2f(wx0, roomL - 0.06f); glVertex2f(wx1, roomL - 0.06f);
        glEnd();
    }
}

// ---------------------- HUD text -----------------------------
void drawText(float x, float y, const string& s) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

void drawHUD() {
    // free floor-area percentage
    float used = 0;
    for (auto& f : items)
        used += (f.type == TABLE) ? PI / 4 * f.w * f.h : f.w * f.h;
    float freePct = 100.0f * (1.0f - used / (roomW * roomL));

    glColor3f(0, 0, 0);
    char buf[240];
    snprintf(buf, sizeof buf,
             "SmartRoom | Room: %.1f x %.1f ft | Items: %d | Free floor: %.1f%% | Zoom: %d%% | Snap: %s | Door zone: %s",
             roomW, roomL, (int)items.size(), freePct, (int)(zoom * 100),
             snapOn ? "ON" : "OFF", doorClear ? "ON" : "OFF");
    drawText(15, winH - 22, buf);
    drawText(15, winH - 40,
             "ADD: [1]Bed [2]Sofa [3]Table [4]Chair [5]Wardrobe [6]Desk | EDIT: drag move, R rotate 90, E rotate 15, +/- scale, X/Del delete, C clear");
    drawText(15, winH - 58,
             "UNDO/REDO: U / Y (or Ctrl+Z/Y) | FILE: S save, L load | VIEW: wheel or Z/O zoom, right-drag or arrows pan, 0 reset | G snap, D door zone, Esc quit");
    glColor3f(0.0f, 0.3f, 0.7f);
    drawText(15, 15, statusMsg);
}

// ---------------------- Display / reshape --------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(offX, offY, 0);          // world (feet) -> screen (pixels), with zoom/pan
    glScalef(scaleF, scaleF, 1);
    drawRoom();
    for (int i = 0; i < (int)items.size(); i++) drawFurniture(i);
    glPopMatrix();

    drawHUD();
    glutSwapBuffers();                    // double buffering
}

void reshape(int w, int h) {
    winW = max(w, 1); winH = max(h, 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);         // 1 unit = 1 pixel
    computeView();
    glutPostRedisplay();
}

// ---------------------- M5: interaction ----------------------
Furniture makeItem(int type) {
    Furniture f = {type, 0, 0, 1, 1, 0, {0.6f, 0.6f, 0.6f}};
    switch (type) {
    case BED:      f.w = 5.0f; f.h = 6.5f; f.col[0] = 0.45f; f.col[1] = 0.60f; f.col[2] = 0.85f; break;
    case SOFA:     f.w = 6.0f; f.h = 2.8f; f.col[0] = 0.55f; f.col[1] = 0.75f; f.col[2] = 0.55f; break;
    case TABLE:    f.w = 3.0f; f.h = 3.0f; f.col[0] = 0.80f; f.col[1] = 0.60f; f.col[2] = 0.35f; break;
    case CHAIR:    f.w = 1.5f; f.h = 1.5f; f.col[0] = 0.75f; f.col[1] = 0.50f; f.col[2] = 0.50f; break;
    case WARDROBE: f.w = 4.0f; f.h = 2.0f; f.col[0] = 0.55f; f.col[1] = 0.40f; f.col[2] = 0.30f; break;
    case DESK:     f.w = 4.0f; f.h = 2.0f; f.col[0] = 0.70f; f.col[1] = 0.55f; f.col[2] = 0.75f; break;
    }
    return f;
}

// Try the room centre first, then scan the room for the first free valid spot.
void addItem(int type) {
    vector<Furniture> before = items;
    Furniture f = makeItem(type);
    f.x = roomW / 2; f.y = roomL / 2;
    items.push_back(f);
    if (isValid((int)items.size() - 1)) {
        sel = (int)items.size() - 1;
        commitUndo(before);
        statusMsg = string(FNAMES[type]) + " added.";
        return;
    }
    items.pop_back();

    for (float y = f.h / 2; y <= roomL - f.h / 2 + 0.001f; y += 0.5f)
        for (float x = f.w / 2; x <= roomW - f.w / 2 + 0.001f; x += 0.5f) {
            f.x = x; f.y = y;
            items.push_back(f);
            if (isValid((int)items.size() - 1)) {
                sel = (int)items.size() - 1;
                commitUndo(before);
                statusMsg = string(FNAMES[type]) + " added.";
                return;
            }
            items.pop_back();
        }
    statusMsg = "No free space for " + string(FNAMES[type]) + "!";
}

float snapVal(float v) { return roundf(v / gridSize) * gridSize; }

void mouse(int button, int state, int mx, int my) {
    // right / middle button = pan the view
    if (button == GLUT_RIGHT_BUTTON || button == GLUT_MIDDLE_BUTTON) {
        panning = (state == GLUT_DOWN);
        lastMX = mx; lastMY = my;
        return;
    }
    if (button != GLUT_LEFT_BUTTON) return;
    Vec2 p = toWorld(mx, my);

    if (state == GLUT_DOWN) {
        sel = -1;
        dragMoved = false;
        for (int i = (int)items.size() - 1; i >= 0; i--) {   // topmost first
            if (hitTest(items[i], p)) {
                sel = i;
                dragging = true;
                dragDX = items[i].x - p.x;
                dragDY = items[i].y - p.y;
                backX = items[i].x; backY = items[i].y;
                break;
            }
        }
        if (sel >= 0) {
            // bring to front so it is drawn on top
            Furniture f = items[sel];
            items.erase(items.begin() + sel);
            items.push_back(f);
            sel = (int)items.size() - 1;
        }
    } else if (dragging) {
        dragging = false;
        if (dragMoved && sel >= 0 && !isValid(sel)) {         // invalid drop -> revert
            items[sel].x = backX; items[sel].y = backY;
            if (!undoStack.empty()) undoStack.pop_back();     // drop the useless undo entry
            statusMsg = "Invalid position - item moved back.";
        }
        dragMoved = false;
    }
    glutPostRedisplay();
}

void motion(int mx, int my) {
    if (panning) {
        panX += (float)(mx - lastMX);
        panY -= (float)(my - lastMY);
        lastMX = mx; lastMY = my;
        applyView();
        glutPostRedisplay();
        return;
    }
    if (!dragging || sel < 0) return;
    if (!dragMoved) { pushUndo(); dragMoved = true; }          // snapshot before the first move
    Vec2 p = toWorld(mx, my);
    float nx = p.x + dragDX, ny = p.y + dragDY;
    if (snapOn) { nx = snapVal(nx); ny = snapVal(ny); }
    items[sel].x = nx;
    items[sel].y = ny;
    glutPostRedisplay();
}

void wheel(int, int dir, int mx, int my) {
    zoomAt(mx, my, dir > 0 ? 1.1f : 1.0f / 1.1f);
    glutPostRedisplay();
}

void special(int key, int, int) {
    const float step = 30.0f;
    if      (key == GLUT_KEY_LEFT)  panX -= step;
    else if (key == GLUT_KEY_RIGHT) panX += step;
    else if (key == GLUT_KEY_UP)    panY += step;
    else if (key == GLUT_KEY_DOWN)  panY -= step;
    applyView();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
    if (key >= '1' && key <= '6') addItem(key - '1');
    else if (key == 27) exit(0);
    else if (key == 'g' || key == 'G') { snapOn = !snapOn; statusMsg = snapOn ? "Snap ON" : "Snap OFF"; }
    else if (key == 'd' || key == 'D') { doorClear = !doorClear; statusMsg = doorClear ? "Door clearance zone ON" : "Door clearance zone OFF"; }
    else if (key == 'u' || key == 'U' || key == 26) doUndo();      // 26 = Ctrl+Z
    else if (key == 'y' || key == 'Y' || key == 25) doRedo();      // 25 = Ctrl+Y
    else if (key == 's' || key == 'S') saveLayout();
    else if (key == 'l' || key == 'L') loadLayout();
    else if (key == 'z' || key == 'Z') zoomAt(winW / 2, winH / 2, 1.15f);
    else if (key == 'o' || key == 'O') zoomAt(winW / 2, winH / 2, 1.0f / 1.15f);
    else if (key == '0') { zoom = 1; panX = panY = 0; applyView(); statusMsg = "View reset."; }
    else if (key == 'c' || key == 'C') {
        if (!items.empty()) { pushUndo(); items.clear(); sel = -1; statusMsg = "Room cleared (U to undo)."; }
    }
    else if (sel >= 0) {
        Furniture& f = items[sel];
        bool changed = true;
        if (key == 'r' || key == 'R')      { pushUndo(); f.angle = fmodf(f.angle + 90.0f, 360.0f); }
        else if (key == 'e' || key == 'E') { pushUndo(); f.angle = fmodf(f.angle + 15.0f, 360.0f); }
        else if (key == '+' || key == '=') { pushUndo(); f.w = min(f.w * 1.1f, 12.0f); f.h = min(f.h * 1.1f, 12.0f); }
        else if (key == '-')               { pushUndo(); f.w = max(f.w * 0.9f, 1.0f);  f.h = max(f.h * 0.9f, 1.0f); }
        else if (key == 'x' || key == 'X' || key == 127 || key == 8) {
            pushUndo();
            items.erase(items.begin() + sel);
            sel = -1;
            statusMsg = "Item deleted.";
            changed = false;
        } else changed = false;
        if (changed && sel >= 0 && !isValid(sel))
            statusMsg = "Warning: item overlaps, leaves the room or blocks the door (shown in red).";
    }
    glutPostRedisplay();
}

// ---------------------- main ---------------------------------
int main(int argc, char** argv) {
    cout << "SmartRoom - enter room size in feet.\nWidth (x): ";
    if (!(cin >> roomW) || roomW < 4 || roomW > 60) roomW = 12;
    cout << "Length (y): ";
    if (!(cin >> roomL) || roomL < 4 || roomL > 60) roomL = 10;
    cout << "Room: " << roomW << " x " << roomL << " ft\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("SmartRoom - 2D Room & Furniture Layout Planner");

    glClearColor(0.97f, 0.97f, 0.97f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMouseWheelFunc(wheel);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}