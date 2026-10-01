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

// ---- window / room ----
extern int   winW, winH;
extern float roomW, roomL;               // room size in feet

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

// door (bottom wall): hinge at DOOR_X, opening width DOOR_W, swings into the room
const float DOOR_X = 1.5f, DOOR_W = 3.0f;

// ---- layout and interaction state ----
extern std::vector<Furniture> items;
extern int   sel;
extern bool  dragging, dragMoved;
extern float dragDX, dragDY;             // offset between mouse and item centre
extern float backX, backY;               // position before drag (to revert if invalid)
extern std::string statusMsg;
