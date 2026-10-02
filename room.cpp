// room.cpp - Room Module
#include "room.h"
#include "common.h"
#include "collision.h"
#include "gfx.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>

void drawRoom() {
    // floor
    fillRect(0, 0, roomW, roomL, 0.93f, 0.90f, 0.82f);

    // grid (every foot; every 5 feet slightly darker)
    glLineWidth(1);
    for (int i = 0; i <= (int)roomW; i++) {
        if (i % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        glBegin(GL_LINES); glVertex2f((float)i, 0); glVertex2f((float)i, roomL); glEnd();
    }
    for (int j = 0; j <= (int)roomL; j++) {
        if (j % 5 == 0) glColor3f(0.65f, 0.65f, 0.65f); else glColor3f(0.82f, 0.82f, 0.82f);
        glBegin(GL_LINES); glVertex2f(0, (float)j); glVertex2f(roomW, (float)j); glEnd();
    }

    // walls
    glColor3f(0.15f, 0.15f, 0.15f);
    glLineWidth(8);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0, 0); glVertex2f(roomW, 0); glVertex2f(roomW, roomL); glVertex2f(0, roomL);
    glEnd();

    if (hasDoor()) {
        float dx0 = DOOR_X, dx1 = DOOR_X + DOOR_W;

        // door-swing clearance zone (translucent quarter disc)
        if (doorClear) {
            glColor4f(0.2f, 0.45f, 1.0f, 0.18f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(dx0, 0);
            for (int i = 0; i <= 24; i++) {
                float t = (PI / 2) * i / 24;
                glVertex2f(dx0 + DOOR_W * cosf(t), DOOR_W * sinf(t));
            }
            glEnd();
        }

        glColor3f(0.93f, 0.90f, 0.82f); glLineWidth(10);           // opening in the wall
        glBegin(GL_LINES); glVertex2f(dx0, 0); glVertex2f(dx1, 0); glEnd();
        glColor3f(0.35f, 0.2f, 0.1f); glLineWidth(3);
        glBegin(GL_LINES); glVertex2f(dx0, 0); glVertex2f(dx0, DOOR_W); glEnd(); // door leaf
        glColor3f(0.35f, 0.2f, 0.1f); glLineWidth(1);
        glBegin(GL_LINE_STRIP);                                                  // swing arc
        for (int i = 0; i <= 24; i++) {
            float t = (PI / 2) * i / 24;
            glVertex2f(dx0 + DOOR_W * cosf(t), DOOR_W * sinf(t));
        }
        glEnd();

        // window on the top wall (4 ft wide)
        float wx0 = roomW / 2 - 2.0f, wx1 = roomW / 2 + 2.0f;
        glColor3f(0.65f, 0.85f, 0.95f); glLineWidth(8);
        glBegin(GL_LINES); glVertex2f(wx0, roomL); glVertex2f(wx1, roomL); glEnd();
        glColor3f(0.2f, 0.4f, 0.6f); glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2f(wx0, roomL + 0.06f); glVertex2f(wx1, roomL + 0.06f);
        glVertex2f(wx0, roomL - 0.06f); glVertex2f(wx1, roomL - 0.06f);
        glEnd();
    }
}

// Dimension lines drawn a fixed number of pixels outside the walls.
void drawDimensions() {
    float d = 22.0f / scaleF;          // distance of the dimension line from the wall
    float t = 5.0f / scaleF;           // half length of end ticks
    glColor3f(0.25f, 0.25f, 0.30f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    // width (below the room)
    glVertex2f(0, -d);      glVertex2f(roomW, -d);
    glVertex2f(0, -d - t);  glVertex2f(0, -d + t);
    glVertex2f(roomW, -d - t); glVertex2f(roomW, -d + t);
    // length (left of the room)
    glVertex2f(-d, 0);      glVertex2f(-d, roomL);
    glVertex2f(-d - t, 0);  glVertex2f(-d + t, 0);
    glVertex2f(-d - t, roomL); glVertex2f(-d + t, roomL);
    glEnd();

    char buf[40];
    snprintf(buf, sizeof buf, "%.1f ft", roomW);
    drawTextWorld(roomW / 2, -d - 16.0f / scaleF, buf, 1);
    snprintf(buf, sizeof buf, "%.1f ft", roomL);
    drawTextWorld(-d - 8.0f / scaleF, roomL / 2 - 4.0f / scaleF, buf, 2);

}
