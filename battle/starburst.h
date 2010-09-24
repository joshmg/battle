// File: starburst.h
// Written by Joshua Green

#ifndef STARBURST_H
#define STARBURST_H

#include "../shared/shared.h"
#include <string>

class starburst {
  private:
    int _step;
    int _animation_length;
    int _delay;
    point3d _line;
    point3d _line_mod;
    point3d _line_color;
    point3d _line_color2;
    std::string _text;
    bool _pause_animation;

  public:
    starburst();
    void clear();
    void pos(const point3d&);
    void length(int);
    void color(const point3d&);
    void color2(const point3d&);
    void text(const std::string&);
    void animate();
    void toggle_animation(bool=true);
    void reset_animation();
    void delay(int);
};

#endif
