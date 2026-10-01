// furniture.cpp - Furniture Module
#include "furniture.h"
#include "collision.h"
#include "gfx.h"
#include <GL/freeglut.h>

const char* FNAMES[FTYPE_COUNT] = {"Bed", "Sofa", "Table", "Chair", "Wardrobe", "Desk"};

Furniture makeItem(int type) {
    Furniture f = {type, 0, 0, 1, 1, 0, {0.6f, 0.6f, 0.6f}};
    switch (type) {
    case BED:      f.w = 5.0f; f.h = 6.5f; f.col[0] = 0.45f; f.col[1] = 0.60f; f.col[2] = 0.85f; break;
    case SOFA:     f.w = 6.0f; f.h = 2.8f; f.col[0] = 0.55f; f.col[1] = 0.75f; f.col[2] = 0.55f; break;
    case TABLE:    f.w = 3.0f; f.h = 3.0f; f.col[0] = 0.80f; f.col[1] = 0.60f; f.col[2] = 0.35f; break;
    case CHAIR:    f.w = 1.5f; f.h = 1.5f; f.col[0] = 0.75f; f.col[1] = 0.50f; f.col[2] = 0.50f; break;
    case WARDROBE: f.w = 4.0f; f.h = 2.0f; f.col[0] = 0.55f; f.col[1] = 0.40f; f.col[2] = 0.30f; break;
    case DESK:     f.w = 4.0f; f.h = 2.0f; f.col[0] = 0.70f; f.col[1] = 0.55f; f.col[2] = 0.75f; break;
    }
    return f;
}

// Draws one furniture piece centred on (0,0) in local (feet) coordinates.
static void drawShape(const Furniture& f, bool valid) {
    float hw = f.w / 2, hh = f.h / 2;
    float r = f.col[0], g = f.col[1], b = f.col[2];
    if (!valid) { r = 0.90f; g = 0.25f; b = 0.25f; }           // red = invalid placement
    float dr = r * 0.75f, dg = g * 0.75f, db = b * 0.75f;      // darker detail colour

    switch (f.type) {
    case BED:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, -hh, hw, hh - 1.7f, dr, dg, db);                    // blanket
        fillRect(-hw + 0.3f, hh - 1.4f, -0.1f, hh - 0.3f, 0.97f, 0.97f, 0.97f); // pillow 1
        fillRect(0.1f, hh - 1.4f, hw - 0.3f, hh - 0.3f, 0.97f, 0.97f, 0.97f);   // pillow 2
        break;
    case SOFA:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, hh - 0.7f, hw, hh, dr, dg, db);                      // backrest (top)
        fillRect(-hw, -hh, -hw + 0.6f, hh - 0.7f, dr, dg, db);             // left arm
        fillRect(hw - 0.6f, -hh, hw, hh - 0.7f, dr, dg, db);               // right arm
        break;
    case TABLE:
        fillEllipse(0, 0, hw, hh, r, g, b);
        fillEllipse(0, 0, hw * 0.7f, hh * 0.7f, dr, dg, db);
        break;
    case CHAIR:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-hw, hh - 0.3f, hw, hh, dr, dg, db);                      // backrest
        break;
    case WARDROBE:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        glColor3f(dr, dg, db); glLineWidth(2);
        glBegin(GL_LINES); glVertex2f(0, -hh); glVertex2f(0, hh); glEnd();  // door split
        glColor3f(0.1f, 0.1f, 0.1f); glPointSize(5);
        glBegin(GL_POINTS); glVertex2f(-0.2f, 0); glVertex2f(0.2f, 0); glEnd(); // handles
        break;
    case DESK:
        fillRect(-hw, -hh, hw, hh, r, g, b);
        fillRect(-0.6f, hh - 0.6f, 0.6f, hh - 0.2f, 0.15f, 0.15f, 0.2f);   // monitor
        fillRect(-0.5f, hh - 1.2f, 0.5f, hh - 0.8f, dr, dg, db);           // keyboard
        break;
    }
}

void drawFurniture(int idx) {
    const Furniture& f = items[idx];
    bool valid = isValid(idx);
    bool selected = (idx == sel);

    glPushMatrix();
    glTranslatef(f.x, f.y, 0);          // 3) move to position
    glRotatef(f.angle, 0, 0, 1);        // 2) rotate about own centre
    drawShape(f, valid);                // 1) shape defined around (0,0)

    // outline: blue when selected, dark red when invalid, black otherwise
    if (selected)      glColor3f(0.0f, 0.35f, 1.0f);
    else if (!valid)   glColor3f(0.6f, 0.0f, 0.0f);
    else               glColor3f(0.1f, 0.1f, 0.1f);
    float lw = selected ? 3.5f : 1.5f;
    if (f.type == TABLE) strokeEllipse(0, 0, f.w / 2, f.h / 2, lw);
    else                 strokeRect(-f.w / 2, -f.h / 2, f.w / 2, f.h / 2, lw);
    glPopMatrix();
}
