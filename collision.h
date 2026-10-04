// collision.h - boundary, overlap and door-clearance checks
#pragma once
#include "common.h"
#include "roomshape.h"

bool insideRoom(const Furniture& f);                  // whole rectangle inside the room polygon
bool satOverlap(const Furniture& a, const Furniture& b); // Separating Axis Theorem
bool doorBlocked(const Furniture& f);                 // furniture inside door-swing zone
bool isValid(int idx);                                // inside room, clear of door, no overlap
