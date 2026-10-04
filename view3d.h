// view3d.h - 3D preview: perspective camera, lighting, lit furniture models, planar shadows
#pragma once

extern bool  view3D;        // false = 2D plan editor, true = 3D preview
extern bool  orbiting;      // left mouse button is orbiting the camera

void resetCamera3D();                      // default camera for the current room size
void orbitCamera(float dYaw, float dPitch);// rotate around the room centre (degrees)
void zoomCamera(float factor);             // factor < 1 moves closer
void draw3D();                             // renders into the canvas area
