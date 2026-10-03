// transform.h - geometric transformations (using Mat3) and window-to-viewport mapping
#pragma once
#include "common.h"
#include "matrix.h"

Mat3 modelMatrix(const Furniture& f);                 // local -> world:  T(x,y) * R(angle)
Mat3 viewMatrix();                                    // world -> screen: T(offset) * S(scale)

Vec2 rotateAbout(Vec2 p, Vec2 c, float deg);          // composite: T(c) * R * T(-c)
void getCorners(const Furniture& f, Vec2 out[4]);     // 4 rotated corners (world)
bool hitTest(const Furniture& f, Vec2 p);             // point inside rotated rectangle

void applyView();                                     // base view + zoom/pan -> final mapping
void computeView();                                   // fit room inside window
Vec2 toWorld(int mx, int my);                         // screen (mouse) -> world (feet)
void zoomAt(int mx, int my, float factor);            // zoom about a screen point
