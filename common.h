// common.h - shared types and global state used by all SmartRoom modules
#pragma once
#include <vector>
#include <string>

const float PI = 3.14159265f;

enum FType { BED, SOFA, TABLE, CHAIR, WARDROBE, DESK, FTYPE_COUNT };

struct Furniture {
    int   type;
    float x, y;       // centre position (world units = feet)
    float w, h;       // width (x) and height (y) before rotation
    float angle;      // rotation in degrees
    float col[3];     // base RGB colour
};

struct Vec2 { float x, y; };
struct RectF { float x0, y0, x1, y1; };      // axis-aligned rectangle (world feet)

// ---- window / room ----
extern int   winW, winH;
extern float roomW, roomL;               // bounding box of the room in feet

// ---- room shape: rectangle, or L-shaped (a rectangle with a notch cut from one corner) ----
extern float notchW, notchL;             // size of the notch (0 = plain rectangle)
extern int   notchCorner;                // 0 top-right, 1 top-left, 2 bottom-right, 3 bottom-left
extern std::vector<Vec2>  roomPoly;      // room outline, counter-clockwise
extern std::vector<RectF> floorRects;    // the outline split into 1 or 2 rectangles
extern float roomArea, roomCx, roomCy;   // area and centroid of the room
extern float doorX;                      // hinge x of the door (on the bottom wall, y = 0)
extern float winX0, winX1;               // window extent on the top wall (y = roomL)

// ---- view: window-to-viewport mapping with zoom and pan ----
extern float baseScale, baseOffX, baseOffY;   // "fit room in window" view
extern float zoom, panX, panY;                // user zoom and pan (pixels)
extern float scaleF, offX, offY;              // final world -> screen mapping
extern bool  panning;
extern int   lastMX, lastMY;

// ---- settings ----
extern float gridSize;                   // snap step in feet
extern bool  snapOn;
extern bool  doorClear;                  // door-swing clearance zone on/off
extern bool  showDims;                   // wall dimension labels on/off

// rendering options
enum LineAlgo { ALGO_GL, ALGO_DDA, ALGO_BRESENHAM, ALGO_COUNT };
extern int   lineAlgo;                   // which line algorithm draws grid, walls, outlines
extern bool  fillScanline;               // own scanline polygon fill (true) or OpenGL fill (false)
extern bool  clipOn;                     // clip the scene to the canvas viewport
const float HUD_H = 84.0f;               // height (pixels) of the text area at the top
extern int   hoverBtn;                   // palette button under the mouse (-1 = none)

const float PALETTE_W = 130.0f;          // width (pixels) of the left furniture palette

// door (bottom wall): hinge at doorX, opening width DOOR_W, swings into the room
const float DOOR_W = 3.0f;

// ---- layout and interaction state ----
extern std::vector<Furniture> items;
extern int   sel;
extern bool  dragging, dragMoved;
extern float dragDX, dragDY;             // offset between mouse and item centre
extern float backX, backY;               // position before drag (to revert if invalid)
extern std::string statusMsg;
