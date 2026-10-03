// room.cpp - Room Module (drawn with our own pipeline: matrices, clipping, rasterization)
#include "room.h"
#include "common.h"
#include "collision.h"
#include "gfx.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>

// quarter circle of radius r around (cx,cy), from angle 0 to 90 degrees
static std::vector<Vec2> quarterArc(float cx, float cy, float r) {
    std::vector<Vec2> pts;
    for (int i = 0; i <= 24; i++) {
        float t = (PI / 2) * i / 24;
        pts.push_back({cx + r * cosf(t), cy + r * sinf(t)});
    }
    return pts;
}

void drawRoom() {
    // floor
    fillRect(0, 0, roomW, roomL, 0.93f, 0.90f, 0.82f);

    // grid (every foot; every 5 feet slightly darker)
    for (int i = 0; i <= (int)roomW; i++) {
        if (i % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        gfxLine((float)i, 0, (float)i, roomL, 1);
    }
    for (int j = 0; j <= (int)roomL; j++) {
        if (j % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        gfxLine(0, (float)j, roomW, (float)j, 1);
    }

    // walls
    glColor3f(0.15f, 0.15f, 0.15f);
    gfxLine(0, 0, roomW, 0, 8);
    gfxLine(roomW, 0, roomW, roomL, 8);
    gfxLine(roomW, roomL, 0, roomL, 8);
    gfxLine(0, roomL, 0, 0, 8);

    if (hasDoor()) {
        float dx0 = DOOR_X, dx1 = DOOR_X + DOOR_W;
        std::vector<Vec2> arc = quarterArc(dx0, 0, DOOR_W);

        // door-swing clearance zone (translucent quarter disc)
        if (doorClear) {
            std::vector<Vec2> zone;
            zone.push_back({dx0, 0});
            zone.insert(zone.end(), arc.begin(), arc.end());
            glColor4f(0.2f, 0.45f, 1.0f, 0.18f);
            gfxFillPolygon(zone);
        }

        glColor3f(0.93f, 0.90f, 0.82f);                   // opening in the wall
        gfxLine(dx0, 0, dx1, 0, 10);
        glColor3f(0.35f, 0.2f, 0.1f);
        gfxLine(dx0, 0, dx0, DOOR_W, 3);                  // door leaf
        for (size_t i = 0; i + 1 < arc.size(); i++)       // swing arc
            gfxLine(arc[i].x, arc[i].y, arc[i + 1].x, arc[i + 1].y, 1);

        // window on the top wall (4 ft wide)
        float wx0 = roomW / 2 - 2.0f, wx1 = roomW / 2 + 2.0f;
        glColor3f(0.65f, 0.85f, 0.95f);
        gfxLine(wx0, roomL, wx1, roomL, 8);
        glColor3f(0.2f, 0.4f, 0.6f);
        gfxLine(wx0, roomL + 0.06f, wx1, roomL + 0.06f, 1);
        gfxLine(wx0, roomL - 0.06f, wx1, roomL - 0.06f, 1);
    }
}

// Dimension lines drawn a fixed number of pixels outside the walls.
void drawDimensions() {
    float d = 22.0f / scaleF;          // distance of the dimension line from the wall
    float t = 5.0f / scaleF;           // half length of end ticks
    glColor3f(0.25f, 0.25f, 0.30f);
    gfxLine(0, -d, roomW, -d, 1);                        // width (below the room)
    gfxLine(0, -d - t, 0, -d + t, 1);
    gfxLine(roomW, -d - t, roomW, -d + t, 1);
    gfxLine(-d, 0, -d, roomL, 1);                        // length (left of the room)
    gfxLine(-d - t, 0, -d + t, 0, 1);
    gfxLine(-d - t, roomL, -d + t, roomL, 1);

    char buf[40];
    snprintf(buf, sizeof buf, "%.1f ft", roomW);
    drawTextWorld(roomW / 2, -d - 16.0f / scaleF, buf, 1);
    snprintf(buf, sizeof buf, "%.1f ft", roomL);
    drawTextWorld(-d - 8.0f / scaleF, roomL / 2 - 4.0f / scaleF, buf, 2);
}
