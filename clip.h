// clip.h - Clipping Module: Liang-Barsky (lines) and Sutherland-Hodgman (polygons)
#pragma once
#include "common.h"

// Clip the segment (x0,y0)-(x1,y1) to the window [xmin,xmax] x [ymin,ymax].
// Returns false if the segment is completely outside. Endpoints are updated in place.
bool clipLineLB(float& x0, float& y0, float& x1, float& y1,
                float xmin, float ymin, float xmax, float ymax);

// Clip a polygon to the window (clips against the 4 edges one after another).
std::vector<Vec2> clipPolygonSH(const std::vector<Vec2>& poly,
                                float xmin, float ymin, float xmax, float ymax);
