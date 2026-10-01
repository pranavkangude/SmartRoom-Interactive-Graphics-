// furniture.h - furniture data and drawing
#pragma once
#include "common.h"

extern const char* FNAMES[FTYPE_COUNT];
Furniture makeItem(int type);          // default size and colour for a furniture type
void drawFurniture(int idx);           // draw items[idx] (with selection/invalid feedback)
