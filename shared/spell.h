// File: spell.h
// Written by Joshua Green

#ifndef SPELL_H
#define SPELL_H

#include "character.h"
#include "connection/transmission.h"
#include "db/row.h"
#include <string>

class spell {
  private:
    int _id, _level;
    int _mp;
    float _hp_add, _hp_mul, _hp_max_mul;
    float _mp_add, _mp_mul, _mp_max_mul;

    bool _all;
    bool _castable_on_dead;

    std::string _name, _element;

  public:
    spell();
    void define(const row&);
    void clear();
    transmission* cast(character*, character*) const;

    std::string name() const;
    std::string element() const;
    int mp() const;
    
    bool castable_on_dead() const;
};

#endif
