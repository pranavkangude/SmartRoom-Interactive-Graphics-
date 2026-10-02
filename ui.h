// ui.h - heads-up display and the left furniture palette
#pragma once

// Palette buttons: 0..5 add a furniture type, then the action buttons below.
enum { BTN_UNDO = 6, BTN_REDO, BTN_SAVE, BTN_LOAD, BTN_CLEAR, BTN_COUNT };

void drawHUD();
void drawPalette();
int  paletteHit(int mx, int my);   // button index under the mouse (mouse coords), or -1
