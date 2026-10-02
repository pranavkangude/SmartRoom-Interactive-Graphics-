// gfx.h - small OpenGL drawing helpers shared by the drawing modules
#pragma once
#include <string>

void fillRect(float x0, float y0, float x1, float y1, float r, float g, float b);
void strokeRect(float x0, float y0, float x1, float y1, float lw);
void fillEllipse(float cx, float cy, float rx, float ry, float r, float g, float b);
void strokeEllipse(float cx, float cy, float rx, float ry, float lw);
void drawText(float x, float y, const std::string& s);
// text anchored at a world point; align: 0 = left, 1 = centre, 2 = right
void drawTextWorld(float wx, float wy, const std::string& s, int align);
