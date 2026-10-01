// interaction.h - GLUT input callbacks (mouse, wheel, keyboard, special keys)
#pragma once
void mouse(int button, int state, int mx, int my);
void motion(int mx, int my);
void wheel(int wheelNo, int dir, int mx, int my);
void special(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
