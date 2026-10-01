// ui.cpp - UI / HUD Module
#include "ui.h"
#include "common.h"
#include "gfx.h"
#include <GL/freeglut.h>
#include <cstdio>

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
