// ============================================================
//  SmartRoom - 2D Room & Furniture Layout Planner (modular version)
//  C++ / OpenGL (FreeGLUT)
//
//  Modules:  common/state (shared data)   transform (matrices, view mapping)
//            collision (boundary, SAT)    room (floor, grid, door, window)
//            furniture (shapes)           interaction (mouse/keyboard)
//            history (undo/redo)          fileio (save/load)
//            ui (HUD)                     gfx (drawing pipeline)
//            matrix (3x3 homogeneous)     clip (Liang-Barsky, Sutherland-Hodgman)
//            raster (DDA, Bresenham, scanline fill)
//
//  Build:  see build.bat or README.md
// ============================================================
#include <GL/freeglut.h>
#include <algorithm>
#include <iostream>
#include "common.h"
#include "transform.h"
#include "gfx.h"
#include "room.h"
#include "furniture.h"
#include "ui.h"
#include "interaction.h"
using namespace std;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // scene: our own matrices + clipping to the canvas (right of palette, below the HUD text)
    if (clipOn) gfxSetClip(PALETTE_W, 0, (float)winW, winH - HUD_H);
    else        gfxSetClip(0, 0, (float)winW, (float)winH);
    gfxSetCTM(viewMatrix());              // world (feet) -> screen (pixels), with zoom/pan
    drawRoom();
    if (showDims) drawDimensions();
    for (int i = 0; i < (int)items.size(); i++) drawFurniture(i);

    // user interface in plain screen coordinates
    gfxSetCTM(matIdentity());
    gfxSetClip(0, 0, (float)winW, (float)winH);
    if (clipOn) {                         // show the top edge of the clip window
        glColor3f(0.65f, 0.65f, 0.72f);
        gfxLine(PALETTE_W, winH - HUD_H, (float)winW, winH - HUD_H, 1);
    }
    drawPalette();
    drawHUD();
    glutSwapBuffers();                    // double buffering
}

void reshape(int w, int h) {
    winW = max(w, 1); winH = max(h, 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);         // 1 unit = 1 pixel
    computeView();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    cout << "SmartRoom - enter room size in feet.\nWidth (x): ";
    if (!(cin >> roomW) || roomW < 4 || roomW > 60) roomW = 12;
    cout << "Length (y): ";
    if (!(cin >> roomL) || roomL < 4 || roomL > 60) roomL = 10;
    cout << "Room: " << roomW << " x " << roomL << " ft\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("SmartRoom - 2D Room & Furniture Layout Planner");

    glClearColor(0.97f, 0.97f, 0.97f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passiveMotion);
    glutMouseWheelFunc(wheel);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}
