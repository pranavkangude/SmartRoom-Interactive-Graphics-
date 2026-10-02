// ui.cpp - UI / HUD Module (status text + clickable palette)
#include "ui.h"
#include "common.h"
#include "gfx.h"
#include "furniture.h"
#include <GL/freeglut.h>
#include <cstdio>
using namespace std;

// ---- palette geometry (screen pixels, origin bottom-left) ----
static const float BTN_X = 10.0f, BTN_W = PALETTE_W - 20.0f, BTN_H = 36.0f, BTN_GAP = 8.0f;
static const char* ACTION_LABELS[BTN_COUNT - BTN_UNDO] = {"Undo", "Redo", "Save", "Load", "Clear"};

static float btnTop(int i) {
    float y = winH - 90.0f - i * (BTN_H + BTN_GAP);
    if (i >= BTN_UNDO) y -= 14.0f;               // extra gap between furniture and actions
    return y;
}

int paletteHit(int mx, int my) {
    float sy = (float)(winH - my);
    if (mx < BTN_X || mx > BTN_X + BTN_W) return -1;
    for (int i = 0; i < BTN_COUNT; i++) {
        float y1 = btnTop(i), y0 = y1 - BTN_H;
        if (sy >= y0 && sy <= y1) return i;
    }
    return -1;
}

void drawPalette() {
    // panel background and divider
    fillRect(0, 0, PALETTE_W, (float)winH, 0.90f, 0.90f, 0.93f);
    glColor3f(0.6f, 0.6f, 0.68f);
    glLineWidth(1.5f);
    glBegin(GL_LINES); glVertex2f(PALETTE_W, 0); glVertex2f(PALETTE_W, (float)winH); glEnd();

    glColor3f(0.25f, 0.25f, 0.35f);
    drawText(BTN_X, winH - 80.0f, "FURNITURE");

    for (int i = 0; i < BTN_COUNT; i++) {
        float y1 = btnTop(i), y0 = y1 - BTN_H;
        bool hot = (i == hoverBtn);
        fillRect(BTN_X, y0, BTN_X + BTN_W, y1, hot ? 0.78f : 0.98f, hot ? 0.88f : 0.98f, hot ? 1.0f : 0.98f);
        glColor3f(0.4f, 0.4f, 0.48f);
        strokeRect(BTN_X, y0, BTN_X + BTN_W, y1, 1.5f);

        glColor3f(0.1f, 0.1f, 0.15f);
        if (i < FTYPE_COUNT) {
            Furniture f = makeItem(i);                      // colour swatch of the item
            fillRect(BTN_X + 6, y0 + 8, BTN_X + 26, y1 - 8, f.col[0], f.col[1], f.col[2]);
            glColor3f(0.1f, 0.1f, 0.15f);
            strokeRect(BTN_X + 6, y0 + 8, BTN_X + 26, y1 - 8, 1.0f);
            char buf[40];
            snprintf(buf, sizeof buf, "%d  %s", i + 1, FNAMES[i]);
            drawText(BTN_X + 34, y0 + 14, buf);
        } else {
            drawText(BTN_X + 12, y0 + 14, ACTION_LABELS[i - BTN_UNDO]);
        }
    }
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
             "ADD: click the palette or keys 1-6 | EDIT: drag move, R rotate 90, E rotate 15, +/- scale, X/Del delete, C clear");
    drawText(15, winH - 58,
             "U/Y undo/redo | S/L save/load | wheel or Z/O zoom, right-drag or arrows pan, 0 reset | G snap, D door zone, M dimensions, Esc quit");

    // selected item info
    if (sel >= 0 && sel < (int)items.size()) {
        const Furniture& f = items[sel];
        snprintf(buf, sizeof buf, "Selected: %s  %.1f x %.1f ft  angle %.0f deg  at (%.1f, %.1f)",
                 FNAMES[f.type], f.w, f.h, f.angle, f.x, f.y);
        glColor3f(0.0f, 0.35f, 1.0f);
        drawText(PALETTE_W + 15, 33, buf);
    }
    glColor3f(0.0f, 0.3f, 0.7f);
    drawText(PALETTE_W + 15, 15, statusMsg);
}
