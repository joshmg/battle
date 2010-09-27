// File: darkmatter.h
// Written by Joshua Green

#ifndef DARKMATTER_H
#define DARKMATTER_H

#include "spell.h"
#include "character.h"
#include "db/row.h"
#include <string>
#include <map>

class darkmatter {
  private:
    int _id;                                                // this materia's unique id
    int _owner_id;
    int _linked_to;
    int _exp, _level;
    int _wslot, _aslot;
    bool _equipped;
    std::string _name;                                      // also known as the materia's class
    std::map<std::string, spell*> _spells;

    int _hp_add, _mp_add;
    float _hp_mul, _mp_mul;

    int _str_add, _dex_add, _int_add, _spi_add, _fort_add;
    float _str_mul, _dex_mul, _int_mul, _spi_mul, _fort_mul;

  public:
    darkmatter();
    void clear();
    void define(const row&);
    row to_row() const;

    int id() const;
    std::string name() const;
    int level() const;

    int exp() const;

    void equipped(bool);
    bool equipped() const;

    bool has_spell(const std::string&) const;

    int hp_add();
    float hp_mul();
    int mp_add();
    float mp_mul();

    int str_add();
    float str_mul();
    int dex_add();
    float dex_mul();
    int int_add();
    float int_mul();
    int spi_add();
    float spi_mul();
    int fort_add();
    float fort_mul();

    void print() const;
};

#endif
