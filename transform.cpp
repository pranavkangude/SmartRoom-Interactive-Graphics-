// transform.cpp - Transformation Module (all maths done with our own 3x3 matrices)
#include "transform.h"
#include <cmath>
#include <algorithm>
using namespace std;

// Furniture local space (centre at origin) -> world space
Mat3 modelMatrix(const Furniture& f) {
    return matMul(matTranslate(f.x, f.y), matRotate(f.angle));
}

// World (feet) -> screen (pixels): scale first, then move by the view offset
Mat3 viewMatrix() {
    return matMul(matTranslate(offX, offY), matScale(scaleF, scaleF));
}

// Rotation about an arbitrary centre = translate to origin, rotate, translate back
Vec2 rotateAbout(Vec2 p, Vec2 c, float deg) {
    Mat3 m = matMul(matTranslate(c.x, c.y), matMul(matRotate(deg), matTranslate(-c.x, -c.y)));
    return matApply(m, p);
}

void getCorners(const Furniture& f, Vec2 out[4]) {
    float hw = f.w / 2, hh = f.h / 2;
    Vec2 local[4] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};
    Mat3 m = modelMatrix(f);
    for (int i = 0; i < 4; i++) out[i] = matApply(m, local[i]);
}

// Picking: bring the mouse point into the furniture's local space with the inverse
// model matrix, then it is a simple axis-aligned rectangle test.
bool hitTest(const Furniture& f, Vec2 p) {
    Vec2 q = matApply(matInverse(modelMatrix(f)), p);
    return fabsf(q.x) <= f.w / 2 && fabsf(q.y) <= f.h / 2;
}

void applyView() {
    scaleF = baseScale * zoom;
    offX = baseOffX + panX;
    offY = baseOffY + panY;
}

// Window-to-viewport mapping: fit the room inside the area right of the palette.
void computeView() {
    const float margin = 70.0f;
    float availW = winW - PALETTE_W;
    float sx = (availW - 2 * margin) / roomW;
    float sy = (winH - 2 * margin) / roomL;
    baseScale = max(min(sx, sy), 1.0f);
    baseOffX = PALETTE_W + (availW - roomW * baseScale) / 2.0f;
    baseOffY = (winH - roomL * baseScale) / 2.0f - 25.0f;
    applyView();
}

// Screen (mouse, origin top-left) -> world: inverse of the view matrix
Vec2 toWorld(int mx, int my) {
    return matApply(matInverse(viewMatrix()), {(float)mx, (float)(winH - my)});
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
