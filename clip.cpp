// clip.cpp - Clipping Module
#include "clip.h"
using namespace std;

// ---- Liang-Barsky: parametric line clipping ----
//   P(u) = P0 + u*(P1-P0),  0 <= u <= 1.   For each window edge: p*u <= q.
bool clipLineLB(float& x0, float& y0, float& x1, float& y1,
                float xmin, float ymin, float xmax, float ymax) {
    float dx = x1 - x0, dy = y1 - y0;
    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {x0 - xmin, xmax - x0, y0 - ymin, ymax - y0};
    float u1 = 0.0f, u2 = 1.0f;
    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {                 // parallel to this edge
            if (q[i] < 0) return false;  // and outside it
        } else {
            float t = q[i] / p[i];
            if (p[i] < 0) { if (t > u2) return false; if (t > u1) u1 = t; }  // entering
            else          { if (t < u1) return false; if (t < u2) u2 = t; }  // leaving
        }
    }
    float nx0 = x0 + u1 * dx, ny0 = y0 + u1 * dy;
    float nx1 = x0 + u2 * dx, ny1 = y0 + u2 * dy;
    x0 = nx0; y0 = ny0; x1 = nx1; y1 = ny1;
    return true;
}

// ---- Sutherland-Hodgman: clip a polygon against one edge at a time ----
static vector<Vec2> clipAgainstEdge(const vector<Vec2>& in, int edge,
                                    float xmin, float ymin, float xmax, float ymax) {
    vector<Vec2> out;
    int n = (int)in.size();
    if (n == 0) return out;

    // edge: 0 = left, 1 = right, 2 = bottom, 3 = top
    auto inside = [&](const Vec2& p) {
        switch (edge) {
        case 0: return p.x >= xmin;
        case 1: return p.x <= xmax;
        case 2: return p.y >= ymin;
        default: return p.y <= ymax;
        }
    };
    auto intersect = [&](const Vec2& a, const Vec2& b) {
        Vec2 r;
        if (edge < 2) {
            float x = (edge == 0) ? xmin : xmax;
            float t = (x - a.x) / (b.x - a.x);
            r.x = x; r.y = a.y + t * (b.y - a.y);
        } else {
            float y = (edge == 2) ? ymin : ymax;
            float t = (y - a.y) / (b.y - a.y);
            r.x = a.x + t * (b.x - a.x); r.y = y;
        }
        return r;
    };

    Vec2 s = in[n - 1];
    for (int i = 0; i < n; i++) {
        Vec2 e = in[i];
        bool eIn = inside(e), sIn = inside(s);
        if (eIn) {
            if (!sIn) out.push_back(intersect(s, e));   // entering
            out.push_back(e);
        } else if (sIn) {
            out.push_back(intersect(s, e));             // leaving
        }
        s = e;
    }
    return out;
}

vector<Vec2> clipPolygonSH(const vector<Vec2>& poly,
                           float xmin, float ymin, float xmax, float ymax) {
    vector<Vec2> r = poly;
    for (int edge = 0; edge < 4 && !r.empty(); edge++)
        r = clipAgainstEdge(r, edge, xmin, ymin, xmax, ymax);
    return r;
}
