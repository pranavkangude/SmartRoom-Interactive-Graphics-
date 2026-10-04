// room.cpp - Room Module (2D): floor, grid, walls, door, window, dimension labels
//   Drawn with our own pipeline (matrices, clipping, rasterization). Works for a
//   rectangular or an L-shaped room because it uses the outline polygon and floor rectangles.
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
    // floor: one or two rectangles
    for (const RectF& r : floorRects)
        fillRect(r.x0, r.y0, r.x1, r.y1, 0.93f, 0.90f, 0.82f);

    // grid inside each floor rectangle (every foot; every 5 feet slightly darker)
    for (const RectF& r : floorRects) {
        for (int i = (int)ceilf(r.x0); i <= (int)floorf(r.x1); i++) {
            if (i % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
            gfxLine((float)i, r.y0, (float)i, r.y1, 1);
        }
        for (int j = (int)ceilf(r.y0); j <= (int)floorf(r.y1); j++) {
            if (j % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
            gfxLine(r.x0, (float)j, r.x1, (float)j, 1);
        }
    }

    // walls = every edge of the outline polygon
    glColor3f(0.15f, 0.15f, 0.15f);
    int n = (int)roomPoly.size();
    for (int i = 0; i < n; i++) {
        const Vec2& a = roomPoly[i];
        const Vec2& b = roomPoly[(i + 1) % n];
        gfxLine(a.x, a.y, b.x, b.y, 8);
    }

    if (hasDoor()) {
        float dx0 = doorX, dx1 = doorX + DOOR_W;
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
    }

    if (hasWindow()) {                                    // window on the top wall (4 ft wide)
        glColor3f(0.65f, 0.85f, 0.95f);
        gfxLine(winX0, roomL, winX1, roomL, 8);
        glColor3f(0.2f, 0.4f, 0.6f);
        gfxLine(winX0, roomL + 0.06f, winX1, roomL + 0.06f, 1);
        gfxLine(winX0, roomL - 0.06f, winX1, roomL - 0.06f, 1);
    }
}

// Rectangular room: width below and length on the left.
// L-shaped room: every wall gets a dimension line and a label.
// Offsets are a fixed number of pixels, so the labels stay readable when zooming.
void drawDimensions() {
    char buf[40];
    glColor3f(0.25f, 0.25f, 0.30f);

    if (!isLShaped()) {
        float d = 22.0f / scaleF, t = 5.0f / scaleF;
        gfxLine(0, -d, roomW, -d, 1);
        gfxLine(0, -d - t, 0, -d + t, 1);
        gfxLine(roomW, -d - t, roomW, -d + t, 1);
        gfxLine(-d, 0, -d, roomL, 1);
        gfxLine(-d - t, 0, -d + t, 0, 1);
        gfxLine(-d - t, roomL, -d + t, roomL, 1);
        snprintf(buf, sizeof buf, "%.1f ft", roomW);
        drawTextWorld(roomW / 2, -d - 16.0f / scaleF, buf, 1);
        snprintf(buf, sizeof buf, "%.1f ft", roomL);
        drawTextWorld(-d - 8.0f / scaleF, roomL / 2 - 4.0f / scaleF, buf, 2);
        return;
    }

    float d = 16.0f / scaleF, t = 4.0f / scaleF, px = 1.0f / scaleF;
    int n = (int)roomPoly.size();
    for (int i = 0; i < n; i++) {
        Vec2 a = roomPoly[i], b = roomPoly[(i + 1) % n];
        float dx = b.x - a.x, dy = b.y - a.y, len = hypotf(dx, dy);
        if (len < 1.0f) continue;
        float nx = dy / len, ny = -dx / len;                 // outward normal (polygon is counter-clockwise)
        float ux = dx / len, uy = dy / len;                  // along the wall
        Vec2 p0 = {a.x + nx * d, a.y + ny * d}, p1 = {b.x + nx * d, b.y + ny * d};
        gfxLine(p0.x, p0.y, p1.x, p1.y, 1);                  // dimension line
        gfxLine(p0.x - nx * t, p0.y - ny * t, p0.x + nx * t, p0.y + ny * t, 1);   // end ticks
        gfxLine(p1.x - nx * t, p1.y - ny * t, p1.x + nx * t, p1.y + ny * t, 1);
        (void)ux; (void)uy;
        snprintf(buf, sizeof buf, "%.1f ft", len);
        float mx = (p0.x + p1.x) / 2, my = (p0.y + p1.y) / 2;
        if (ny > 0.5f)       drawTextWorld(mx, my + 6 * px, buf, 1);              // above the top walls
        else if (ny < -0.5f) drawTextWorld(mx, my - 14 * px, buf, 1);             // below the bottom walls
        else if (nx > 0.5f)  drawTextWorld(mx + 6 * px, my - 4 * px, buf, 0);     // right of right walls
        else                 drawTextWorld(mx - 6 * px, my - 4 * px, buf, 2);     // left of left walls
    }
}
