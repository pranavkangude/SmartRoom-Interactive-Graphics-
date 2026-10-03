// matrix.h - 3x3 homogeneous transformation matrices (2D)
//   A point (x, y) is treated as the column vector [x y 1]^T.
//   Composite transformations are built by multiplying matrices:  M = T * R * S
#pragma once
#include "common.h"

struct Mat3 { float m[3][3]; };

Mat3 matIdentity();
Mat3 matTranslate(float tx, float ty);
Mat3 matRotate(float deg);                 // counter-clockwise, degrees
Mat3 matScale(float sx, float sy);
Mat3 matMul(const Mat3& a, const Mat3& b); // a * b  (b is applied first)
Mat3 matInverse(const Mat3& a);            // via determinant / adjugate
Vec2 matApply(const Mat3& a, Vec2 p);      // homogeneous point transform
