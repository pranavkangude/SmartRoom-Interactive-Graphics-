// interaction.cpp - Interaction Module
#include "interaction.h"
#include "common.h"
#include "transform.h"
#include "collision.h"
#include "furniture.h"
#include "history.h"
#include "fileio.h"
#include "ui.h"
#include "view3d.h"
#include "autoarrange.h"
#include "roomshape.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
using namespace std;

// Try the room centre first, then scan the room for the first free valid spot.
static void addItem(int type) {
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

static float snapVal(float v) { return roundf(v / gridSize) * gridSize; }

static void clearRoom() {
    if (items.empty()) return;
    pushUndo();
    items.clear();
    sel = -1;
    statusMsg = "Room cleared (U to undo).";
}

// A click on one of the palette buttons
static void paletteAction(int b) {
    if (b < FTYPE_COUNT) addItem(b);
    else if (b == BTN_UNDO) doUndo();
    else if (b == BTN_REDO) doRedo();
    else if (b == BTN_SAVE) saveLayout();
    else if (b == BTN_LOAD) loadLayout();
    else if (b == BTN_CLEAR) clearRoom();
    else if (b == BTN_AUTO) autoArrange();
}

void passiveMotion(int mx, int my) {
    int h = paletteHit(mx, my);
    if (h != hoverBtn) { hoverBtn = h; glutPostRedisplay(); }
}

void mouse(int button, int state, int mx, int my) {
    // right / middle button = pan the view
    if (button == GLUT_RIGHT_BUTTON || button == GLUT_MIDDLE_BUTTON) {
        panning = (state == GLUT_DOWN);
        lastMX = mx; lastMY = my;
        return;
    }
    if (button != GLUT_LEFT_BUTTON) return;

    // clicks on the left palette never touch the room
    if (state == GLUT_DOWN && mx < PALETTE_W) {
        int b = paletteHit(mx, my);
        if (b >= 0) paletteAction(b);
        glutPostRedisplay();
        return;
    }

    // 3D preview: left drag orbits the camera instead of editing
    if (view3D) {
        orbiting = (state == GLUT_DOWN);
        lastMX = mx; lastMY = my;
        return;
    }
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
            dropLastUndo();                                   // drop the useless undo entry
            statusMsg = "Invalid position - item moved back.";
        }
        dragMoved = false;
    }
    glutPostRedisplay();
}

void motion(int mx, int my) {
    if (view3D) {
        if (orbiting) {
            orbitCamera(-0.4f * (mx - lastMX), 0.4f * (my - lastMY));
            lastMX = mx; lastMY = my;
            glutPostRedisplay();
        }
        return;
    }
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
    if (view3D) { zoomCamera(dir > 0 ? 1.0f / 1.1f : 1.1f); glutPostRedisplay(); return; }
    zoomAt(mx, my, dir > 0 ? 1.1f : 1.0f / 1.1f);
    glutPostRedisplay();
}

void special(int key, int, int) {
    if (view3D) {
        if      (key == GLUT_KEY_LEFT)  orbitCamera(-6, 0);
        else if (key == GLUT_KEY_RIGHT) orbitCamera(6, 0);
        else if (key == GLUT_KEY_UP)    orbitCamera(0, 4);
        else if (key == GLUT_KEY_DOWN)  orbitCamera(0, -4);
        glutPostRedisplay();
        return;
    }
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
    else if (key == 'v' || key == 'V') {
        view3D = !view3D;
        orbiting = false; dragging = false;
        statusMsg = view3D ? "3D preview: drag to orbit, wheel to zoom. Edit in the 2D plan (V to switch back)."
                           : "2D plan view.";
    }
    else if (key == '0') {
        if (view3D) { resetCamera3D(); statusMsg = "3D camera reset."; }
        else        { zoom = 1; panX = panY = 0; applyView(); statusMsg = "View reset."; }
    }
    else if (key == 'c' || key == 'C') clearRoom();
    else if (key == 'a' || key == 'A') autoArrange();
    else if (key == 'n' || key == 'N') {
        if (cycleRoomShape()) {
            computeView();
            statusMsg = "Room shape: " + roomShapeText() + ". Items that no longer fit turn red - press A to auto-arrange.";
        } else {
            statusMsg = "Room is too small for an L shape (needs at least 7 x 7 ft).";
        }
    }
    else if (key == 'b' || key == 'B') {
        lineAlgo = (lineAlgo + 1) % ALGO_COUNT;
        static const char* names[ALGO_COUNT] = {"OpenGL", "DDA", "Bresenham"};
        statusMsg = string("Line algorithm: ") + names[lineAlgo];
    }
    else if (key == 'f' || key == 'F') { fillScanline = !fillScanline; statusMsg = fillScanline ? "Polygon fill: own scanline algorithm" : "Polygon fill: OpenGL"; }
    else if (key == 'k' || key == 'K') { clipOn = !clipOn; statusMsg = clipOn ? "Clipping to viewport ON (Liang-Barsky / Sutherland-Hodgman)" : "Clipping to viewport OFF (window edge only)"; }
    else if (key == 'm' || key == 'M') { showDims = !showDims; statusMsg = showDims ? "Dimensions ON" : "Dimensions OFF"; }
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
