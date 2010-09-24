// File: character.h
// Written by Joshua Green

#ifndef CHARACTER_H
#define CHARACTER_H

#include "../shared/shared.h"
#include "../shared/db/row.h"
#include "../shared/db/column.h"
#include <string>
#include <map>

class darkmatter;

class character {
  private:
    int _id;
    int _hp_base, _mp_base;
    mutable int _hp, _hp_max, _mp, _mp_max;
    mutable bool _hp_max_valid, _mp_max_valid;
    int _level, _exp;
    mutable bool _dead;
    std::string _name;
    std::string _class;                 // _class is the name of the character class
    bool _is_defined;
    int _dmg_base;
    int _speed_base;
    int _speed;                         // _speed is the number of ticks added to _time upon time incriment (operator(++))
    int _time;
    int _tmax;

    int _str_base, _dex_base, _int_base,
        _spi_base, _fort_base;
    mutable int  _str, _dex, _int, _spi, _fort;
    mutable bool _str_valid, _dex_valid, _int_valid,
                 _spi_valid, _fort_valid;

    std::map<int, darkmatter*> _dmatter; // dmatter_id, dmatter_data

    void _level_up();
    void _void_cache() const;

  public:
    character();
    character(const std::string&, const row&);
    ~character();
    void clear();

    void define(const row&);

    row to_row() const;
    std::string encode() const;
    void decode(const std::string&);

    int id() const;
    void id(int);

    std::string name() const;

    std::string c_class() const;
    int hp_max() const;
    int mp_max() const;

    int hp() const;
    void hp(int) const;

    int mp() const;
    void mp(int) const;

    bool dead() const;
    void dead(bool);

    int exp() const;
    void exp(int);

    int exp_max() const;

    int level() const;

    int speed() const;
    int tmax() const;

    int Str() const;
    int Dex() const;
    int Int() const;
    int Spi() const;
    int Fort() const;

    bool is_defined() const;
    void operator++(int);

    int dmg() const;

    void full_heal();

    bool can_act(float threshold=0.0f) const;
    float timebar_percent() const;

    void set_timebar(float);

    void load_dmatter();                        // designed for server use

    void add_dmatter(const row&);
    void clear_dmatter();

    void equip_dmatter(int, int, int);          // dmatter id, weap/armor id, slot id

    std::map<int, const darkmatter&> get_dmatter() const;

    bool has_dmatter(int) const;
    const darkmatter& get_dmatter(int) const;

    point3d gui_pos;
};

#endif
