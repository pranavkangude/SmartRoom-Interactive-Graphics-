// roomshape.h - Room Shape Module: rectangle or L-shaped room as a polygon
#pragma once
#include "common.h"

// Rebuild everything derived from roomW, roomL and the notch: outline polygon,
// floor rectangles, area, centroid, door and window positions.
void buildRoom();

// corner < 0 -> plain rectangle; otherwise cut a notch (nw x nl feet) from that corner
// (0 top-right, 1 top-left, 2 bottom-right, 3 bottom-left). Returns false if the room is too small.
bool setRoomShape(int corner, float nw, float nl);
bool cycleRoomShape();                    // rectangle -> L (4 orientations) -> rectangle ...
bool isLShaped();
std::string roomShapeText();              // e.g. "12 x 10 ft" or "L-shaped 12 x 10 ft, notch 6 x 5"

bool pointInPolygon(Vec2 p, const std::vector<Vec2>& poly);   // ray casting (even-odd)
bool hasDoor();                           // bottom wall long enough for the door
bool hasWindow();                         // top wall long enough for the window
