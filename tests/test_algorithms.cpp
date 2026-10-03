// Unit tests for the graphics algorithms (no OpenGL needed).
// Build and run (from the project folder):
//   g++ tests/test_algorithms.cpp matrix.cpp clip.cpp raster.cpp -I. -o test_algorithms_v1.exe
//   .\test_algorithms_v1.exe
#include "common.h"
#include "matrix.h"
#include "clip.h"
#include "raster.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)

int main() {
    // ---- matrices ----
    Mat3 m = matMul(matTranslate(3, 4), matMul(matRotate(30), matScale(2, 2)));
    Mat3 id = matMul(m, matInverse(m));
    bool isId = true;
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) if (fabsf(id.m[i][j] - (i == j)) > 1e-4f) isId = false;
    CHECK(isId, "M * inverse(M) = identity");
    Vec2 p = matApply(matRotate(90), {1, 0});
    CHECK(fabsf(p.x) < 1e-5f && fabsf(p.y - 1) < 1e-5f, "rotate (1,0) by 90 deg -> (0,1)");
    Mat3 about = matMul(matTranslate(1, 1), matMul(matRotate(90), matTranslate(-1, -1)));
    Vec2 q = matApply(about, {2, 1});
    CHECK(fabsf(q.x - 1) < 1e-4f && fabsf(q.y - 2) < 1e-4f, "rotation about (1,1): T * R * T^-1");

    // ---- Liang-Barsky ----
    float a = -5, b = 5, c = 15, d = 5;
    bool vis = clipLineLB(a, b, c, d, 0, 0, 10, 10);
    CHECK(vis && fabsf(a) < 1e-4f && fabsf(c - 10) < 1e-4f && fabsf(b - 5) < 1e-4f, "Liang-Barsky clips a horizontal line");
    float e = -5, f = -5, g = -1, h = -1;
    CHECK(!clipLineLB(e, f, g, h, 0, 0, 10, 10), "Liang-Barsky rejects a line fully outside");
    float e2 = 2, f2 = 2, g2 = 8, h2 = 8;
    CHECK(clipLineLB(e2, f2, g2, h2, 0, 0, 10, 10) && e2 == 2 && g2 == 8, "Liang-Barsky keeps an inside line unchanged");
    float e3 = -2, f3 = -2, g3 = 12, h3 = 12;
    CHECK(clipLineLB(e3, f3, g3, h3, 0, 0, 10, 10) && fabsf(e3) < 1e-4f && fabsf(g3 - 10) < 1e-4f, "Liang-Barsky clips a diagonal at both ends");

    // ---- Sutherland-Hodgman ----
    std::vector<Vec2> sq = {{-5, -5}, {5, -5}, {5, 5}, {-5, 5}};
    std::vector<Vec2> cl = clipPolygonSH(sq, 0, 0, 10, 10);
    float minx = 1e9, maxx = -1e9, miny = 1e9, maxy = -1e9;
    for (auto& v : cl) { minx = fminf(minx, v.x); maxx = fmaxf(maxx, v.x); miny = fminf(miny, v.y); maxy = fmaxf(maxy, v.y); }
    CHECK(cl.size() == 4 && minx == 0 && maxx == 5 && miny == 0 && maxy == 5, "Sutherland-Hodgman clips a square to a 5x5 corner");
    std::vector<Vec2> outside = {{20, 20}, {30, 20}, {30, 30}};
    CHECK(clipPolygonSH(outside, 0, 0, 10, 10).empty(), "Sutherland-Hodgman removes a polygon fully outside");

    // ---- DDA / Bresenham ----
    std::vector<IVec2> d1, b1;
    lineDDA(0, 0, 10, 4, d1);
    lineBresenham(0, 0, 10, 4, b1);
    CHECK(d1.size() == 11 && b1.size() == 11 && b1.back().x == 10 && b1.back().y == 4, "DDA and Bresenham: 11 pixels and correct end point");
    int diff = 0;
    for (size_t i = 0; i < b1.size(); i++) if (abs(d1[i].y - b1[i].y) > 1) diff++;
    CHECK(diff == 0, "DDA and Bresenham agree within 1 pixel");
    std::vector<IVec2> b2, b3;
    lineBresenham(10, 10, 0, 0, b2);
    lineBresenham(0, 0, 3, 9, b3);
    CHECK(b2.size() == 11, "Bresenham works in the reverse direction");
    CHECK(b3.size() == 10 && b3.back().x == 3 && b3.back().y == 9, "Bresenham works on a steep line");

    // ---- scanline fill ----
    std::vector<Span> sp;
    scanlineSpans({{0, 0}, {10, 0}, {10, 6}, {0, 6}}, sp);
    int area = 0;
    for (auto& s : sp) area += s.x1 - s.x0 + 1;
    CHECK(area == 60, "scanline fill of a 10x6 rectangle covers 60 pixels");
    std::vector<Span> sp2;
    scanlineSpans({{0, 0}, {10, 0}, {0, 10}}, sp2);
    int a2 = 0;
    for (auto& s : sp2) a2 += s.x1 - s.x0 + 1;
    CHECK(a2 > 40 && a2 < 60, "scanline fill of a right triangle covers about half of the square");

    printf("\n%s (%d failures)\n", fails ? "SOME TESTS FAILED" : "ALL TESTS PASSED", fails);
    return fails;
}
