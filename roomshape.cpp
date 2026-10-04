// roomshape.cpp - Room Shape Module
#include "roomshape.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
using namespace std;

static float bottomX0 = 0, bottomX1 = 12, topX0 = 0, topX1 = 12;   // extent of the y=0 and y=roomL walls

bool isLShaped() { return notchW > 0 && notchL > 0; }
bool hasDoor()   { return bottomX1 - bottomX0 >= 6.0f; }
bool hasWindow() { return topX1 - topX0 >= 6.0f; }

void buildRoom() {
    float W = roomW, L = roomL;
    roomPoly.clear();
    floorRects.clear();

    if (!isLShaped()) {
        roomPoly = {{0, 0}, {W, 0}, {W, L}, {0, L}};
        floorRects = {{0, 0, W, L}};
        bottomX0 = 0; bottomX1 = W; topX0 = 0; topX1 = W;
    } else {
        float cw = notchW, cl = notchL;
        switch (notchCorner) {
        case 0:   // notch at top-right
            roomPoly = {{0, 0}, {W, 0}, {W, L - cl}, {W - cw, L - cl}, {W - cw, L}, {0, L}};
            floorRects = {{0, 0, W, L - cl}, {0, L - cl, W - cw, L}};
            bottomX0 = 0; bottomX1 = W; topX0 = 0; topX1 = W - cw;
            break;
        case 1:   // top-left
            roomPoly = {{0, 0}, {W, 0}, {W, L}, {cw, L}, {cw, L - cl}, {0, L - cl}};
            floorRects = {{0, 0, W, L - cl}, {cw, L - cl, W, L}};
            bottomX0 = 0; bottomX1 = W; topX0 = cw; topX1 = W;
            break;
        case 2:   // bottom-right
            roomPoly = {{0, 0}, {W - cw, 0}, {W - cw, cl}, {W, cl}, {W, L}, {0, L}};
            floorRects = {{0, cl, W, L}, {0, 0, W - cw, cl}};
            bottomX0 = 0; bottomX1 = W - cw; topX0 = 0; topX1 = W;
            break;
        default:  // bottom-left
            roomPoly = {{cw, 0}, {W, 0}, {W, L}, {0, L}, {0, cl}, {cw, cl}};
            floorRects = {{0, cl, W, L}, {cw, 0, W, cl}};
            bottomX0 = cw; bottomX1 = W; topX0 = 0; topX1 = W;
            break;
        }
    }

    // area and centroid (shoelace formulas)
    float a = 0, cx = 0, cy = 0;
    int n = (int)roomPoly.size();
    for (int i = 0; i < n; i++) {
        Vec2 p = roomPoly[i], q = roomPoly[(i + 1) % n];
        float cr = p.x * q.y - q.x * p.y;
        a += cr; cx += (p.x + q.x) * cr; cy += (p.y + q.y) * cr;
    }
    a *= 0.5f;
    roomArea = fabsf(a);
    if (fabsf(a) > 1e-6f) { roomCx = cx / (6 * a); roomCy = cy / (6 * a); }
    else { roomCx = W / 2; roomCy = L / 2; }

    doorX = bottomX0 + 1.5f;                         // door 1.5 ft from the start of the bottom wall
    float wc = (topX0 + topX1) / 2;                  // window centred on the top wall
    winX0 = wc - 2.0f; winX1 = wc + 2.0f;
}

bool setRoomShape(int corner, float nw, float nl) {
    if (corner < 0) { notchW = notchL = 0; buildRoom(); return true; }
    if (roomW < 7 || roomL < 7) return false;        // arms would be narrower than 3 ft
    notchW = min(max(nw, 3.0f), roomW - 3.0f);
    notchL = min(max(nl, 3.0f), roomL - 3.0f);
    notchCorner = corner;
    buildRoom();
    return true;
}

bool cycleRoomShape() {
    int cur = isLShaped() ? 1 + notchCorner : 0;
    int next = (cur + 1) % 5;
    if (next == 0) return setRoomShape(-1, 0, 0);
    float nw = isLShaped() ? notchW : floorf(roomW / 2);
    float nl = isLShaped() ? notchL : floorf(roomL / 2);
    return setRoomShape(next - 1, nw, nl);
}

string roomShapeText() {
    char buf[120];
    if (isLShaped()) {
        static const char* corner[4] = {"top-right", "top-left", "bottom-right", "bottom-left"};
        snprintf(buf, sizeof buf, "L-shaped %.1f x %.1f ft (notch %.1f x %.1f, %s)", roomW, roomL, notchW, notchL, corner[notchCorner]);
    } else {
        snprintf(buf, sizeof buf, "%.1f x %.1f ft", roomW, roomL);
    }
    return buf;
}

bool pointInPolygon(Vec2 p, const vector<Vec2>& poly) {
    bool in = false;
    int n = (int)poly.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
            in = !in;
    }
    return in;
}
