// raster.cpp - Rasterization Module
#include "raster.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
using namespace std;

// ---- DDA: step along the longer axis, increment the other by the slope ----
void lineDDA(int x0, int y0, int x1, int y1, vector<IVec2>& out) {
    int dx = x1 - x0, dy = y1 - y0;
    int steps = max(abs(dx), abs(dy));
    if (steps == 0) { out.push_back({x0, y0}); return; }
    float xInc = (float)dx / steps, yInc = (float)dy / steps;
    float x = (float)x0, y = (float)y0;
    for (int i = 0; i <= steps; i++) {
        out.push_back({(int)lroundf(x), (int)lroundf(y)});
        x += xInc; y += yInc;
    }
}

// ---- Bresenham: integer-only error term, works in all octants ----
void lineBresenham(int x0, int y0, int x1, int y1, vector<IVec2>& out) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    while (true) {
        out.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

// ---- Scanline polygon fill (even-odd rule) ----
void scanlineSpans(const vector<Vec2>& poly, vector<Span>& out) {
    int n = (int)poly.size();
    if (n < 3) return;
    float ymin = poly[0].y, ymax = poly[0].y;
    for (auto& p : poly) { ymin = min(ymin, p.y); ymax = max(ymax, p.y); }

    int yStart = (int)ceilf(ymin - 0.5f), yEnd = (int)floorf(ymax - 0.5f);
    vector<float> xs;
    for (int y = yStart; y <= yEnd; y++) {
        float yc = y + 0.5f;                       // scanline through the pixel centres
        xs.clear();
        for (int i = 0; i < n; i++) {
            const Vec2& a = poly[i];
            const Vec2& b = poly[(i + 1) % n];
            if ((a.y <= yc && b.y > yc) || (b.y <= yc && a.y > yc)) {  // edge crosses scanline
                float t = (yc - a.y) / (b.y - a.y);
                xs.push_back(a.x + t * (b.x - a.x));
            }
        }
        sort(xs.begin(), xs.end());
        for (size_t k = 0; k + 1 < xs.size(); k += 2) {                // fill between pairs
            int xa = (int)ceilf(xs[k] - 0.5f), xb = (int)floorf(xs[k + 1] - 0.5f);
            if (xb >= xa) out.push_back({y, xa, xb});
        }
    }
}
