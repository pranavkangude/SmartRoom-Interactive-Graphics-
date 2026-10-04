// view3d.cpp - 3D View Module (OpenGL fixed-function pipeline with lighting)
#include "view3d.h"
#include "common.h"
#include "collision.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
using namespace std;

bool  view3D = false;
bool  orbiting = false;
static float camYaw = -60.0f, camPitch = 42.0f, camDist = 0.0f;   // camDist 0 = auto

static const float WALL_H = 3.5f;       // cutaway walls so the interior stays visible
static const float WALL_T = 0.4f;       // wall thickness (outside the room)

void resetCamera3D() { camYaw = -60.0f; camPitch = 42.0f; camDist = 0.0f; }
void orbitCamera(float dYaw, float dPitch) {
    camYaw += dYaw;
    camPitch = min(max(camPitch + dPitch, 8.0f), 88.0f);
}
void zoomCamera(float factor) {
    if (camDist <= 0) camDist = max(roomW, roomL) * 1.6f + 4.0f;
    camDist = min(max(camDist * factor, 5.0f), 300.0f);
}

// ---------- geometry helpers (local coordinates, z is up) ----------
static void box(float x0, float y0, float z0, float x1, float y1, float z1) {
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);  glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glNormal3f(0, 0, -1); glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0);
    glNormal3f(0, -1, 0); glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glNormal3f(0, 1, 0);  glVertex3f(x1, y1, z0); glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1);
    glNormal3f(-1, 0, 0); glVertex3f(x0, y1, z0); glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1);
    glNormal3f(1, 0, 0);  glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1); glVertex3f(x1, y0, z1);
    glEnd();
}

static void cylinder(float cx, float cy, float z0, float rx, float ry, float h) {
    const int N = 32;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= N; i++) {
        float a = 2 * PI * i / N, c = cosf(a), s = sinf(a);
        glNormal3f(c / rx, s / ry, 0);
        glVertex3f(cx + rx * c, cy + ry * s, z0);
        glVertex3f(cx + rx * c, cy + ry * s, z0 + h);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex3f(cx, cy, z0 + h);
    for (int i = 0; i <= N; i++) {
        float a = 2 * PI * i / N;
        glVertex3f(cx + rx * cosf(a), cy + ry * sinf(a), z0 + h);
    }
    glEnd();
}

// ---------- furniture models (same local axes as the 2D drawing) ----------
static void drawItem3D(const Furniture& f, bool valid, bool selected, bool shadow) {
    float hw = f.w / 2, hh = f.h / 2;
    auto col = [&](float k) {                       // base colour times a shade factor
        if (shadow) return;
        float r = f.col[0], g = f.col[1], b = f.col[2];
        if (!valid) { r = 0.90f; g = 0.25f; b = 0.25f; }
        else if (selected) { r = min(1.0f, r * 1.2f + 0.08f); g = min(1.0f, g * 1.2f + 0.08f); b = min(1.0f, b * 1.2f + 0.12f); }
        glColor3f(r * k, g * k, b * k);
    };
    auto fixedCol = [&](float r, float g, float b) { if (!shadow) glColor3f(r, g, b); };

    switch (f.type) {
    case BED:
        col(1.0f);  box(-hw, -hh, 0, hw, hh, 1.1f);                                // frame + mattress
        col(0.78f); box(-hw + 0.1f, -hh + 0.1f, 1.1f, hw - 0.1f, hh - 1.8f, 1.45f);// blanket
        fixedCol(0.97f, 0.97f, 0.97f);
        box(-hw + 0.3f, hh - 1.4f, 1.1f, -0.1f, hh - 0.3f, 1.5f);                  // pillows
        box(0.1f, hh - 1.4f, 1.1f, hw - 0.3f, hh - 0.3f, 1.5f);
        col(0.6f);  box(-hw, hh - 0.2f, 0, hw, hh, 2.6f);                          // headboard
        break;
    case SOFA:
        col(1.0f);  box(-hw, -hh, 0, hw, hh - 0.7f, 1.4f);                         // seat
        col(0.8f);  box(-hw, hh - 0.7f, 0, hw, hh, 2.8f);                          // backrest
        box(-hw, -hh, 0, -hw + 0.6f, hh - 0.7f, 2.1f);                             // arms
        box(hw - 0.6f, -hh, 0, hw, hh - 0.7f, 2.1f);
        break;
    case TABLE:
        col(1.0f);  cylinder(0, 0, 2.3f, hw, hh, 0.2f);                            // top
        col(0.75f); cylinder(0, 0, 0, 0.25f, 0.25f, 2.3f);                         // pedestal
        break;
    case CHAIR:
        col(1.0f);  box(-hw, -hh, 1.2f, hw, hh, 1.45f);                            // seat
        col(0.75f);
        box(-hw, -hh, 0, -hw + 0.15f, -hh + 0.15f, 1.2f);                          // legs
        box(hw - 0.15f, -hh, 0, hw, -hh + 0.15f, 1.2f);
        box(-hw, hh - 0.15f, 0, -hw + 0.15f, hh, 1.2f);
        box(hw - 0.15f, hh - 0.15f, 0, hw, hh, 1.2f);
        col(0.9f);  box(-hw, hh - 0.2f, 1.45f, hw, hh, 3.0f);                      // backrest
        break;
    case WARDROBE:
        col(1.0f);  box(-hw, -hh, 0, hw, hh, 6.5f);
        col(0.7f);  box(-0.03f, -hh - 0.01f, 0.2f, 0.03f, -hh + 0.02f, 6.3f);      // door split
        fixedCol(0.1f, 0.1f, 0.1f);
        box(-0.30f, -hh - 0.08f, 3.0f, -0.18f, -hh, 3.6f);                         // handles
        box(0.18f, -hh - 0.08f, 3.0f, 0.30f, -hh, 3.6f);
        break;
    case DESK:
        col(1.0f);  box(-hw, -hh, 2.3f, hw, hh, 2.5f);                             // top
        col(0.75f);
        box(-hw, -hh, 0, -hw + 0.2f, hh, 2.3f);                                    // side panels
        box(hw - 0.2f, -hh, 0, hw, hh, 2.3f);
        fixedCol(0.12f, 0.12f, 0.16f);
        box(-0.6f, hh - 0.6f, 2.5f, 0.6f, hh - 0.4f, 3.4f);                        // monitor
        col(0.8f);  box(-0.5f, hh - 1.2f, 2.5f, 0.5f, hh - 0.8f, 2.58f);           // keyboard
        break;
    }
}

// ---------- room: floor, cutaway walls, door, window (rectangular or L-shaped) ----------
// One wall slab along an axis-aligned polygon edge piece, thickness WALL_T on the outside.
//   horizontal: x from xa to xb on line y;  ny = +1 (outside is above) or -1 (below)
static void wallH(float xa, float xb, float y, float ny, float z0, float z1) {
    float y0 = (ny > 0) ? y : y - WALL_T, y1 = (ny > 0) ? y + WALL_T : y;
    box(xa, y0, z0, xb, y1, z1);
}
static void wallV(float x, float ya, float yb, float nx, float z0, float z1) {
    float x0 = (nx > 0) ? x : x - WALL_T, x1 = (nx > 0) ? x + WALL_T : x;
    box(x0, ya, z0, x1, yb, z1);
}

static void drawRoom3D() {
    const float H = WALL_H;

    // floor (slightly below z=0 so furniture bases do not z-fight with it)
    glColor3f(0.93f, 0.90f, 0.82f);
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);
    for (const RectF& r : floorRects) {
        glVertex3f(r.x0, r.y0, -0.01f); glVertex3f(r.x1, r.y0, -0.01f);
        glVertex3f(r.x1, r.y1, -0.01f); glVertex3f(r.x0, r.y1, -0.01f);
    }
    glEnd();

    // grid
    glDisable(GL_LIGHTING);
    glColor3f(0.80f, 0.80f, 0.78f);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (const RectF& r : floorRects) {
        for (int i = (int)ceilf(r.x0); i <= (int)floorf(r.x1); i++) { glVertex3f((float)i, r.y0, -0.005f); glVertex3f((float)i, r.y1, -0.005f); }
        for (int j = (int)ceilf(r.y0); j <= (int)floorf(r.y1); j++) { glVertex3f(r.x0, (float)j, -0.005f); glVertex3f(r.x1, (float)j, -0.005f); }
    }
    glEnd();
    // door-swing clearance zone
    if (doorClear && hasDoor()) {
        glColor4f(0.2f, 0.45f, 1.0f, 0.25f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(doorX, 0, 0.01f);
        for (int i = 0; i <= 24; i++) {
            float t = (PI / 2) * i / 24;
            glVertex3f(doorX + DOOR_W * cosf(t), DOOR_W * sinf(t), 0.01f);
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);

    // walls: one slab per polygon edge; the bottom wall gets the door gap, the top wall the window
    glColor3f(0.86f, 0.86f, 0.90f);
    int n = (int)roomPoly.size();
    const float eps = 1e-3f;
    for (int i = 0; i < n; i++) {
        Vec2 a = roomPoly[i], b = roomPoly[(i + 1) % n];
        float dx = b.x - a.x, dy = b.y - a.y, len = hypotf(dx, dy);
        float nx = dy / len, ny = -dx / len;                      // outward normal (CCW polygon)
        if (fabsf(dy) < eps) {                                    // horizontal wall
            float xa = min(a.x, b.x), xb = max(a.x, b.x), y = a.y;
            if (fabsf(y) < eps && hasDoor() && xa <= doorX + eps && doorX + DOOR_W <= xb + eps) {
                wallH(xa, doorX, y, ny, 0, H);
                wallH(doorX + DOOR_W, xb, y, ny, 0, H);
            } else if (fabsf(y - roomL) < eps && hasWindow() && xa <= winX0 + eps && winX1 <= xb + eps) {
                wallH(xa, winX0, y, ny, 0, H);
                wallH(winX1, xb, y, ny, 0, H);
                wallH(winX0, winX1, y, ny, 0, 1.3f);              // sill
                wallH(winX0, winX1, y, ny, 3.0f, H);              // header
            } else {
                wallH(xa, xb, y, ny, 0, H);
            }
        } else {                                                  // vertical wall
            wallV(a.x, min(a.y, b.y), max(a.y, b.y), nx, 0, H);
        }
    }
    // corner posts on the outside of convex corners
    for (int i = 0; i < n; i++) {
        Vec2 p = roomPoly[(i + n - 1) % n], v = roomPoly[i], q = roomPoly[(i + 1) % n];
        float e1x = v.x - p.x, e1y = v.y - p.y, e2x = q.x - v.x, e2y = q.y - v.y;
        float l1 = hypotf(e1x, e1y), l2 = hypotf(e2x, e2y);
        if (e1x * e2y - e1y * e2x <= 0) continue;                 // reflex corner: slabs already meet
        float sx = e1y / l1 + e2y / l2, sy = -e1x / l1 - e2x / l2;
        float xa = min(v.x, v.x + WALL_T * sx), xb = max(v.x, v.x + WALL_T * sx);
        float ya = min(v.y, v.y + WALL_T * sy), yb = max(v.y, v.y + WALL_T * sy);
        box(xa, ya, 0, xb, yb, H);
    }

    if (hasDoor()) {                                              // open door leaf
        glColor3f(0.45f, 0.28f, 0.15f);
        box(doorX - 0.06f, 0, 0, doorX + 0.06f, DOOR_W, H);
    }
}

static void drawWindowGlass() {
    if (!hasWindow()) return;
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glColor4f(0.55f, 0.78f, 1.0f, 0.35f);
    glBegin(GL_QUADS);
    glVertex3f(winX0, roomL + WALL_T / 2, 1.3f); glVertex3f(winX1, roomL + WALL_T / 2, 1.3f);
    glVertex3f(winX1, roomL + WALL_T / 2, 3.0f); glVertex3f(winX0, roomL + WALL_T / 2, 3.0f);
    glEnd();
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
}

// Planar shadow matrix: projects geometry onto 'plane' from the light position L
static void shadowMatrix(float m[4][4], const float plane[4], const float L[4]) {
    float dot = plane[0] * L[0] + plane[1] * L[1] + plane[2] * L[2] + plane[3] * L[3];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = (i == j ? dot : 0.0f) - L[j] * plane[i];
}

// ---------- main entry ----------
void draw3D() {
    int vx = (int)PALETTE_W, vw = winW - vx, vh = (int)(winH - HUD_H);
    if (vw < 20 || vh < 20) return;

    // backdrop gradient (still in the 2D pixel projection)
    glBegin(GL_QUADS);
    glColor3f(0.93f, 0.95f, 0.98f); glVertex2f((float)vx, 0);        glVertex2f((float)winW, 0);
    glColor3f(0.74f, 0.83f, 0.93f); glVertex2f((float)winW, (float)vh); glVertex2f((float)vx, (float)vh);
    glEnd();

    if (camDist <= 0) camDist = max(roomW, roomL) * 1.6f + 4.0f;

    glViewport(vx, 0, vw, vh);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(40.0, (double)vw / vh, 1.0, 400.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // orbit camera around the centre of the room (z is up)
    float cx = roomCx, cy = roomCy;
    float yaw = camYaw * PI / 180.0f, pit = camPitch * PI / 180.0f;
    float ex = cx + camDist * cosf(pit) * cosf(yaw);
    float ey = cy + camDist * cosf(pit) * sinf(yaw);
    float ez = camDist * sinf(pit);
    gluLookAt(ex, ey, ez, cx, cy, 0, 0, 0, 1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    // light (fixed in world space) and material colours from glColor
    float L[4] = {roomW * 0.25f, -roomL * 0.25f, 14.0f + max(roomW, roomL) * 0.4f, 1.0f};
    float amb[4] = {0.38f, 0.38f, 0.40f, 1}, dif[4] = {0.85f, 0.83f, 0.78f, 1};
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, L);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    drawRoom3D();

    // ---- shadows: flatten every furniture piece onto the floor ----
    float plane[4] = {0, 0, 1, -0.03f}, sm[4][4];
    shadowMatrix(sm, plane, L);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glColor4f(0, 0, 0, 0.30f);
    for (const RectF& r : floorRects) {                      // clip shadows to each floor rectangle
        double cp0[4] = {1, 0, 0, -r.x0}, cp1[4] = {-1, 0, 0, r.x1}, cp2[4] = {0, 1, 0, -r.y0}, cp3[4] = {0, -1, 0, r.y1};
        glClipPlane(GL_CLIP_PLANE0, cp0); glClipPlane(GL_CLIP_PLANE1, cp1);
        glClipPlane(GL_CLIP_PLANE2, cp2); glClipPlane(GL_CLIP_PLANE3, cp3);
        glEnable(GL_CLIP_PLANE0); glEnable(GL_CLIP_PLANE1); glEnable(GL_CLIP_PLANE2); glEnable(GL_CLIP_PLANE3);
        for (int i = 0; i < (int)items.size(); i++) {
            const Furniture& f = items[i];
            glPushMatrix();
            glMultMatrixf(&sm[0][0]);
            glTranslatef(f.x, f.y, 0);
            glRotatef(f.angle, 0, 0, 1);
            drawItem3D(f, true, false, true);
            glPopMatrix();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_CLIP_PLANE0); glDisable(GL_CLIP_PLANE1); glDisable(GL_CLIP_PLANE2); glDisable(GL_CLIP_PLANE3);
    glEnable(GL_LIGHTING);

    // ---- furniture ----
    for (int i = 0; i < (int)items.size(); i++) {
        const Furniture& f = items[i];
        bool valid = isValid(i), selected = (i == sel);
        glPushMatrix();
        glTranslatef(f.x, f.y, 0);
        glRotatef(f.angle, 0, 0, 1);
        drawItem3D(f, valid, selected, false);
        if (selected) {                                     // blue outline on the floor
            glDisable(GL_LIGHTING);
            glColor3f(0.0f, 0.35f, 1.0f);
            glLineWidth(3);
            glBegin(GL_LINE_LOOP);
            glVertex3f(-f.w / 2, -f.h / 2, 0.04f); glVertex3f(f.w / 2, -f.h / 2, 0.04f);
            glVertex3f(f.w / 2, f.h / 2, 0.04f);   glVertex3f(-f.w / 2, f.h / 2, 0.04f);
            glEnd();
            glEnable(GL_LIGHTING);
        }
        glPopMatrix();
    }

    drawWindowGlass();

    // ---- restore 2D state ----
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, winW, winH);
}
