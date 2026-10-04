// state.cpp - definitions of the shared global state declared in common.h
#include "common.h"

int   winW = 1000, winH = 700;
float roomW = 12.0f, roomL = 10.0f;

float notchW = 0, notchL = 0;
int   notchCorner = 0;
std::vector<Vec2>  roomPoly;
std::vector<RectF> floorRects;
float roomArea = 120.0f, roomCx = 6.0f, roomCy = 5.0f;
float doorX = 1.5f;
float winX0 = 4.0f, winX1 = 8.0f;

float baseScale = 1, baseOffX = 0, baseOffY = 0;
float zoom = 1, panX = 0, panY = 0;
float scaleF = 1, offX = 0, offY = 0;
bool  panning = false;
int   lastMX = 0, lastMY = 0;

float gridSize = 0.5f;
bool  snapOn = true;
bool  doorClear = true;
bool  showDims = true;
int   lineAlgo = ALGO_BRESENHAM;
bool  fillScanline = true;
bool  clipOn = true;
int   hoverBtn = -1;

std::vector<Furniture> items;
int   sel = -1;
bool  dragging = false, dragMoved = false;
float dragDX = 0, dragDY = 0;
float backX = 0, backY = 0;
std::string statusMsg = "Press 1-6 to add furniture. Click an item to select it.";
