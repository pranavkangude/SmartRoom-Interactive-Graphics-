// collision.h - boundary, overlap and door-clearance checks
#pragma once
#include "common.h"

bool insideRoom(const Furniture& f);                  // all 4 corners inside the room
bool satOverlap(const Furniture& a, const Furniture& b); // Separating Axis Theorem
bool hasDoor();
bool doorBlocked(const Furniture& f);                 // furniture inside door-swing zone
bool isValid(int idx);                                // inside room, clear of door, no overlap
