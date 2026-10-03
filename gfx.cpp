// gfx.cpp - Drawing pipeline
#include "gfx.h"
#include "clip.h"
#include "raster.h"
#include <GL/freeglut.h>
#include <cmath>
#include <algorithm>
using namespace std;

static Mat3  ctm = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
static float clipX0 = 0, clipY0 = 0, clipX1 = 100000, clipY1 = 100000;

void gfxSetCTM(const Mat3& m) { ctm = m; }
void gfxSetClip(float x0, float y0, float x1, float y1) {
    clipX0 = x0; clipY0 = y0; clipX1 = x1; clipY1 = y1;
}

// ---- lines ----
void gfxLine(float x0, float y0, float x1, float y1, float width) {
    // 1) transform both end points with our matrix
    Vec2 a = matApply(ctm, {x0, y0});
    Vec2 b = matApply(ctm, {x1, y1});
    float ax = a.x, ay = a.y, bx = b.x, by = b.y;

    // 2) Liang-Barsky clipping against the clip window
    if (!clipLineLB(ax, ay, bx, by, clipX0, clipY0, clipX1, clipY1)) return;

    // 3) rasterize
    if (lineAlgo == ALGO_GL) {                         // let OpenGL draw the line
        glLineWidth(width);
        glBegin(GL_LINES); glVertex2f(ax, ay); glVertex2f(bx, by); glEnd();
        return;
    }
    static vector<IVec2> pix;
    pix.clear();
    int ix0 = (int)lroundf(ax), iy0 = (int)lroundf(ay);
    int ix1 = (int)lroundf(bx), iy1 = (int)lroundf(by);
    if (lineAlgo == ALGO_DDA) lineDDA(ix0, iy0, ix1, iy1, pix);
    else                      lineBresenham(ix0, iy0, ix1, iy1, pix);

    glPointSize(max(1.0f, floorf(width)));             // OpenGL only plots the pixels
    glBegin(GL_POINTS);
    for (auto& p : pix) glVertex2f(p.x + 0.5f, p.y + 0.5f);
    glEnd();
}

// ---- polygons ----
void gfxFillPolygon(const vector<Vec2>& local) {
    vector<Vec2> poly;
    poly.reserve(local.size());
    for (auto& p : local) poly.push_back(matApply(ctm, p));       // transform
    poly = clipPolygonSH(poly, clipX0, clipY0, clipX1, clipY1);   // Sutherland-Hodgman
    if (poly.size() < 3) return;

    if (fillScanline) {                                           // own scanline fill
        static vector<Span> spans;
        spans.clear();
        scanlineSpans(poly, spans);
        glBegin(GL_QUADS);
        for (auto& s : spans) {
            glVertex2f((float)s.x0, (float)s.y);
            glVertex2f((float)s.x1 + 1, (float)s.y);
            glVertex2f((float)s.x1 + 1, (float)s.y + 1);
            glVertex2f((float)s.x0, (float)s.y + 1);
        }
        glEnd();
    } else {                                                      // OpenGL polygon fill
        glBegin(GL_POLYGON);
        for (auto& p : poly) glVertex2f(p.x, p.y);
        glEnd();
    }
}

// ---- shapes built from the two primitives ----
void fillRect(float x0, float y0, float x1, float y1, float r, float g, float b) {
    glColor3f(r, g, b);
    vector<Vec2> poly = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
    gfxFillPolygon(poly);
}

void strokeRect(float x0, float y0, float x1, float y1, float lw) {
    gfxLine(x0, y0, x1, y0, lw);
    gfxLine(x1, y0, x1, y1, lw);
    gfxLine(x1, y1, x0, y1, lw);
    gfxLine(x0, y1, x0, y0, lw);
}

static vector<Vec2> ellipsePoints(float cx, float cy, float rx, float ry) {
    vector<Vec2> pts;
    for (int i = 0; i < 48; i++) {
        float t = 2 * PI * i / 48;
        pts.push_back({cx + rx * cosf(t), cy + ry * sinf(t)});
    }
    return pts;
}

void fillEllipse(float cx, float cy, float rx, float ry, float r, float g, float b) {
    glColor3f(r, g, b);
    gfxFillPolygon(ellipsePoints(cx, cy, rx, ry));
}

void strokeEllipse(float cx, float cy, float rx, float ry, float lw) {
    vector<Vec2> pts = ellipsePoints(cx, cy, rx, ry);
    for (size_t i = 0; i < pts.size(); i++) {
        const Vec2& a = pts[i];
        const Vec2& b = pts[(i + 1) % pts.size()];
        gfxLine(a.x, a.y, b.x, b.y, lw);
    }
}

// ---- text ----
void drawText(float x, float y, const string& s) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

void drawTextWorld(float wx, float wy, const string& s, int align) {
    Vec2 p = matApply(ctm, {wx, wy});
    float widthPx = (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)s.c_str());
    float shift = (align == 1) ? widthPx / 2 : (align == 2 ? widthPx : 0);
    drawText(p.x - shift, p.y, s);
}
