// transform.cpp - Transformation Module
#include "transform.h"
#include <cmath>
#include <algorithm>
using namespace std;

// Rotation about the object's centre (formula from the project notes)
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

void applyView() {
    scaleF = baseScale * zoom;
    offX = baseOffX + panX;
    offY = baseOffY + panY;
}

// Window-to-viewport mapping: fit the room inside the window with a margin.
void computeView() {
    const float margin = 70.0f;
    float availW = winW - PALETTE_W;         // area right of the palette
    float sx = (availW - 2 * margin) / roomW;
    float sy = (winH - 2 * margin) / roomL;
    baseScale = max(min(sx, sy), 1.0f);
    baseOffX = PALETTE_W + (availW - roomW * baseScale) / 2.0f;
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
