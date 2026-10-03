// gfx.h - drawing pipeline:  local coords --(CTM)--> screen --(clip)--> rasterize
//   The current transformation matrix (CTM) is our own Mat3, not OpenGL's matrix stack.
#pragma once
#include <string>
#include <vector>
#include "common.h"
#include "matrix.h"

void gfxSetCTM(const Mat3& m);                                   // local -> screen matrix
void gfxSetClip(float x0, float y0, float x1, float y1);         // screen-space clip window

void gfxLine(float x0, float y0, float x1, float y1, float width);  // clipped + rasterized line
void gfxFillPolygon(const std::vector<Vec2>& poly);                 // clipped + filled polygon

// convenience shapes (colour = current glColor, except the fill* helpers that take r,g,b)
void fillRect(float x0, float y0, float x1, float y1, float r, float g, float b);
void strokeRect(float x0, float y0, float x1, float y1, float lw);
void fillEllipse(float cx, float cy, float rx, float ry, float r, float g, float b);
void strokeEllipse(float cx, float cy, float rx, float ry, float lw);

void drawText(float x, float y, const std::string& s);            // screen pixels
// text anchored at a point given in local coordinates; align: 0 left, 1 centre, 2 right
void drawTextWorld(float wx, float wy, const std::string& s, int align);
