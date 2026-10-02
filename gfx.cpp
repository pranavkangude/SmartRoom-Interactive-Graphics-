// gfx.cpp - drawing primitives (rectangles, ellipses, text)
#include "gfx.h"
#include "common.h"
#include <GL/freeglut.h>
#include <cmath>

void fillRect(float x0, float y0, float x1, float y1, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}
void strokeRect(float x0, float y0, float x1, float y1, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}
void fillEllipse(float cx, float cy, float rx, float ry, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 48; i++) {
        float t = 2 * PI * i / 48;
        glVertex2f(cx + rx * cosf(t), cy + ry * sinf(t));
    }
    glEnd();
}
void strokeEllipse(float cx, float cy, float rx, float ry, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 48; i++) {
        float t = 2 * PI * i / 48;
        glVertex2f(cx + rx * cosf(t), cy + ry * sinf(t));
    }
    glEnd();
}
void drawText(float x, float y, const std::string& s) {
    glRasterPos2f(x, y);
    for (char c : s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

void drawTextWorld(float wx, float wy, const std::string& s, int align) {
    float widthPx = (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)s.c_str());
    float shift = (align == 1) ? widthPx / 2 : (align == 2 ? widthPx : 0);
    drawText(wx - shift / scaleF, wy, s);
}
