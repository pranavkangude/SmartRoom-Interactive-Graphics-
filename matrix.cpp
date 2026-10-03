// matrix.cpp - Matrix part of the Transformation Module
#include "matrix.h"
#include <cmath>

Mat3 matIdentity() {
    Mat3 r = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    return r;
}

Mat3 matTranslate(float tx, float ty) {
    Mat3 r = {{{1, 0, tx}, {0, 1, ty}, {0, 0, 1}}};
    return r;
}

Mat3 matRotate(float deg) {
    float t = deg * PI / 180.0f, c = cosf(t), s = sinf(t);
    Mat3 r = {{{c, -s, 0}, {s, c, 0}, {0, 0, 1}}};
    return r;
}

Mat3 matScale(float sx, float sy) {
    Mat3 r = {{{sx, 0, 0}, {0, sy, 0}, {0, 0, 1}}};
    return r;
}

Mat3 matMul(const Mat3& a, const Mat3& b) {
    Mat3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            r.m[i][j] = 0;
            for (int k = 0; k < 3; k++) r.m[i][j] += a.m[i][k] * b.m[k][j];
        }
    return r;
}

Mat3 matInverse(const Mat3& a) {
    const float (*m)[3] = a.m;
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
              - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
              + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    if (fabsf(det) < 1e-12f) return matIdentity();
    float d = 1.0f / det;
    Mat3 r;
    r.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * d;
    r.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * d;
    r.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * d;
    r.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * d;
    r.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * d;
    r.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * d;
    r.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * d;
    r.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * d;
    r.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * d;
    return r;
}

Vec2 matApply(const Mat3& a, Vec2 p) {
    float x = a.m[0][0] * p.x + a.m[0][1] * p.y + a.m[0][2];
    float y = a.m[1][0] * p.x + a.m[1][1] * p.y + a.m[1][2];
    float w = a.m[2][0] * p.x + a.m[2][1] * p.y + a.m[2][2];
    if (fabsf(w) > 1e-12f && fabsf(w - 1.0f) > 1e-9f) { x /= w; y /= w; }
    return {x, y};
}
