// collision.cpp - Collision Module
#include "collision.h"
#include "transform.h"
#include <cmath>
#include <algorithm>
using namespace std;

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

// Door-swing clearance: quarter disc (radius DOOR_W) swept by the door leaf.
// Sample the quarter disc and test whether any sample lies inside the furniture.
bool doorBlocked(const Furniture& f) {
    if (!doorClear || !hasDoor()) return false;
    Mat3 inv = matInverse(modelMatrix(f));          // world -> furniture local space
    float hw = f.w / 2, hh = f.h / 2;
    for (float x = 0; x <= DOOR_W; x += 0.25f)
        for (float y = 0; y <= DOOR_W; y += 0.25f) {
            if (x * x + y * y > DOOR_W * DOOR_W) continue;
            Vec2 q = matApply(inv, {DOOR_X + x, y});
            if (fabsf(q.x) <= hw && fabsf(q.y) <= hh) return true;
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
