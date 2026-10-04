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
//            view3d (3D preview: camera, lighting, shadows)
//            roomshape (rectangular or L-shaped room polygon)
//            autoarrange (rule-based layout)
//
//  Build:  see build.bat or README.md
// ============================================================
#include <GL/freeglut.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "common.h"
#include "transform.h"
#include "gfx.h"
#include "room.h"
#include "furniture.h"
#include "ui.h"
#include "interaction.h"
#include "view3d.h"
#include "roomshape.h"
using namespace std;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (view3D) {
        draw3D();                         // 3D preview: perspective camera, lighting, shadows
    } else {
        // 2D plan: our own matrices + clipping to the canvas (right of palette, below the HUD text)
        if (clipOn) gfxSetClip(PALETTE_W, 0, (float)winW, winH - HUD_H);
        else        gfxSetClip(0, 0, (float)winW, (float)winH);
        gfxSetCTM(viewMatrix());          // world (feet) -> screen (pixels), with zoom/pan
        drawRoom();
        if (showDims) drawDimensions();
        for (int i = 0; i < (int)items.size(); i++) drawFurniture(i);
    }

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

// read a number from the console, falling back to a default for bad input
static float askFloat(const char* prompt, float def, float lo, float hi) {
    float v;
    cout << prompt;
    if (!(cin >> v) || v < lo || v > hi) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  (using " << def << ")\n";
        return def;
    }
    return v;
}

int main(int argc, char** argv) {
    cout << "SmartRoom - enter the room size in feet.\n";
    roomW = askFloat("Width (x), 4 to 60: ", 12, 4, 60);
    roomL = askFloat("Length (y), 4 to 60: ", 10, 4, 60);

    int shape = (int)askFloat("Room shape - 1: rectangle, 2: L-shaped: ", 1, 1, 2);
    if (shape == 2) {
        if (roomW < 7 || roomL < 7) {
            cout << "  Room too small for an L shape (needs 7 x 7 ft), using a rectangle.\n";
        } else {
            float nw = askFloat("Notch width (ft, the part cut out): ", floorf(roomW / 2), 3, roomW - 3);
            float nl = askFloat("Notch length (ft): ", floorf(roomL / 2), 3, roomL - 3);
            int c = (int)askFloat("Notch corner - 1: top-right, 2: top-left, 3: bottom-right, 4: bottom-left: ", 1, 1, 4);
            setRoomShape(c - 1, nw, nl);
        }
    }
    buildRoom();
    cout << "Room: " << roomShapeText() << "\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
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
