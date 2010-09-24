// File: projectile.cpp
// Written by Joshua Green

#include "projectile.h"
#include "text3d.h"
#include <GL/glu.h>
#include <GL/glut.h>
#include <string>
using namespace std;

projectile::projectile() {
  clear();
}

void projectile::clear() {
  _step = 0;
  _animation_length = 0;
  _position = point3d();
  _pos_mod = point3d();
  _color = point3d();
  _color2 = point3d();

  _slope = point3d();

  _pause_animation = false;

  _slope_is_calculated = false;
}

void projectile::pos(const point3d& p) {
  _position = p;
  _slope_is_calculated = false;
}

void projectile::pos2(const point3d& p2) {
  _destination = p2;
  _slope_is_calculated = false;
}

void projectile::length(int l) {
  _animation_length = l;
}

void projectile::color(const point3d& c) {
  _color = c;
}

void projectile::color2(const point3d& c2) {
  _color2 = c2;
}

void projectile::animate() {
  if (!_slope_is_calculated) {
    _slope.x = (_destination.x - _position.x) / _animation_length;
    _slope.y = (_destination.y - _position.y) / _animation_length;
    _slope.z = (_destination.z - _position.z) / _animation_length;
    _slope_is_calculated = true;
  }

  if (_step < _animation_length && !_pause_animation) {
    point3d center(_position.x+_pos_mod.x, _position.y+_pos_mod.y, _position.z+_pos_mod.z);
    float r = 1.0f;
    float pi = 3.14159;

    glPushMatrix();
    glColor3f(_color.x+(_color2.x-_color.x)*((float)_step/(float)_animation_length),
              _color.y+(_color2.y-_color.y)*((float)_step/(float)_animation_length),
              _color.z+(_color2.z-_color.z)*((float)_step/(float)_animation_length)
             );
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(center.x, center.y, center.z);
    for(float theta=0.0;theta<2*pi;theta+=(2*pi)/10.0f) {
      glVertex3f(center.x+r*sin(theta), center.y+r*cos(theta), center.z);
    }
    glEnd();
    glPopMatrix();

    _step++;
    _pos_mod = _pos_mod + _slope;
  }
}

void projectile::toggle_animation(bool value) {
  _pause_animation = value;
}

void projectile::reset_animation() {
  _step = 0;
  _pos_mod = point3d();
}
