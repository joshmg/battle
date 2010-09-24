// File: starburst.cpp
// Written by Joshua Green

#include "starburst.h"
#include "text3d.h"
#include <GL/glu.h>
#include <GL/glut.h>
#include <string>
using namespace std;

starburst::starburst() {
  clear();
}

void starburst::clear() {
  _step = 0;
  _animation_length = 0;
  _delay = 0;
  _line = point3d();
  _line_mod = point3d();
  _line_color = point3d();
  _line_color2 = point3d();
  
  _text.clear();
  _pause_animation = false;
}

void starburst::pos(const point3d& p) {
  _line = p;
}

void starburst::length(int l) {
  _animation_length = l;
}

void starburst::color(const point3d& c) {
  _line_color = c;
}

void starburst::color2(const point3d& c2) {
  _line_color2 = c2;
}

void starburst::text(const string& data) {
  _text = data;
}

void starburst::animate() {
  if (_delay > 0) _delay--;

  if (_step < _animation_length && !_pause_animation && _delay < 1) {
    glColor3f(_line_color.x+(_line_color2.x-_line_color.x)*((float)_step/(float)_animation_length),
              _line_color.y+(_line_color2.y-_line_color.y)*((float)_step/(float)_animation_length),
              _line_color.z+(_line_color2.z-_line_color.z)*((float)_step/(float)_animation_length)
             );
    
    glPushMatrix();
    glBegin(GL_LINES);
    // 45 degrees
    glVertex3f(_line.x+_line_mod.x-2.0f, _line.y+_line_mod.y-2.0f, _line.z);
    glVertex3f(_line.x+_line_mod.x, _line.y+_line_mod.y, _line.z);
    // 135 degrees
    glVertex3f(_line.x-_line_mod.x+2.0f, _line.y+_line_mod.y-2.0f, _line.z);
    glVertex3f(_line.x-_line_mod.x, _line.y+_line_mod.y, _line.z);
    // 225 degrees
    glVertex3f(_line.x-_line_mod.x+2.0f, _line.y-_line_mod.y+2.0f, _line.z);
    glVertex3f(_line.x-_line_mod.x, _line.y-_line_mod.y, _line.z);
    // 0 degrees
    glVertex3f(_line.x+_line_mod.x, _line.y, _line.z);
    glVertex3f(_line.x+_line_mod.x+4.0f, _line.y, _line.z);
    // 180 degrees
    glVertex3f(_line.x-_line_mod.x, _line.y, _line.z);
    glVertex3f(_line.x-_line_mod.x-4.0f, _line.y, _line.z);
    // 90 degrees
    glVertex3f(_line.x, _line.y+_line_mod.x, _line.z);
    glVertex3f(_line.x, _line.y+_line_mod.x+4.0f, _line.z);
    // 270 degrees
    glVertex3f(_line.x, _line.y-_line_mod.x, _line.z);
    glVertex3f(_line.x, _line.y-_line_mod.x-4.0f, _line.z);
    // 45 degrees
    glVertex3f(_line.x+_line_mod.x-2.0f, _line.y-_line_mod.y+2.0f, _line.z);
    glVertex3f(_line.x+_line_mod.x, _line.y-_line_mod.y, _line.z);
    glEnd();
    glPopMatrix();

    // draw text
    glPushMatrix();
    glTranslatef(_line.x, _line.y, _line.z+0.0f);
    glScalef(4.0f,4.0f, 0.0f);
    t3dDraw2D(_text, 0, 0);
    glPopMatrix();

    _step++;
    _line_mod = _line_mod + point3d(1.0f, 1.0f, 0.0f);
  }
}

void starburst::toggle_animation(bool value) {
  _pause_animation = value;
}

void starburst::reset_animation() {
  _step = 0;
  _line_mod = point3d();
}

void starburst::delay(int value) {
  _delay = value;
}
