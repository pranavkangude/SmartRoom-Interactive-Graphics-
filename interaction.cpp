// interaction.cpp - Interaction Module
#include "interaction.h"
#include "common.h"
#include "transform.h"
#include "collision.h"
#include "furniture.h"
#include "history.h"
#include "fileio.h"
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
            dropLastUndo();                                   // drop the useless undo entry
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
