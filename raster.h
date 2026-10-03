// raster.h - Rasterization Module: DDA, Bresenham and scanline polygon fill
//   These functions only compute which pixels to light; gfx.cpp plots them.
#pragma once
#include "common.h"

struct IVec2 { int x, y; };
struct Span  { int y, x0, x1; };      // horizontal run of pixels x0..x1 on row y

void lineDDA(int x0, int y0, int x1, int y1, std::vector<IVec2>& out);
void lineBresenham(int x0, int y0, int x1, int y1, std::vector<IVec2>& out);

// Even-odd scanline fill: one span per pixel row (pixel centres at y + 0.5)
void scanlineSpans(const std::vector<Vec2>& poly, std::vector<Span>& out);
