// File: projectile.h
// Written by Joshua Green

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "../shared/shared.h"
#include <string>

class projectile {
  private:
    int _step;
    int _animation_length;
    point3d _position;
    point3d _destination;
    point3d _pos_mod;
    point3d _color;
    point3d _color2;
    point3d _slope;
    bool _pause_animation;
    bool _slope_is_calculated;

  public:
    projectile();
    void clear();
    void length(int);
    void pos(const point3d&);
    void pos2(const point3d&);
    void color(const point3d&);
    void color2(const point3d&);
    void animate();
    void toggle_animation(bool=true);
    void reset_animation();
};

#endif
