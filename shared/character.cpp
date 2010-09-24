// File: character.cpp
// Written by Joshua Green

#include "character.h"
#include "connection/transmission.h"
#include "str/str.h"
#include <time.h>
#include <cmath>
#include "fileio/fileio.h"
#include "shared.h"
#include "darkmatter.h"
#include "db_decl.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
using namespace std;

#ifdef SERVER
  #include "db/db.h"
#endif

character::character() { clear(); }

character::character(const string& name, const row& class_data) {
  clear();
  _name = name;

  _class      = class_data[class_name];
  _level      = atoi(class_data[class_level].c_str());
  _hp_base    = atoi(class_data[class_hp].c_str());
  _mp_base    = atoi(class_data[class_mp].c_str());
  _dmg_base   = atoi(class_data[class_dmg].c_str());
  _speed_base = atoi(class_data[class_speed].c_str());
  _tmax       = atoi(class_data[class_tmax].c_str());

  _dead       = false;
  _hp         = _hp_base;
  _mp         = _mp_base;
}

void character::clear() {
  _id = -1;
  _hp = 0;
  _hp_max = 0;
  _mp_max = 0;
  _hp_base = 0;
  _mp = 0;
  _mp_base = 0;
  _level = 0;
  _exp = 0;
  _dead = true;
  _name = "";
  _class = "";
  _dmg_base = 10;
  _speed = 0;
  _speed_base = 10;
  _time = 0;
  _tmax = 100;
  
  _str_base = 0;
  _dex_base = 0;
  _int_base = 0;
  _spi_base = 0;
  _fort_base = 0;

  map<int, darkmatter*>::iterator dmatter_it = _dmatter.begin();
  while (dmatter_it != _dmatter.end()) {
    delete dmatter_it->second;
    dmatter_it++;
  }
  _dmatter.clear();

  _void_cache();
}

character::~character() {
  clear();
}

row character::to_row() const {
  row data;
  data.add(char_id, itos(_id));
  data.add(char_name, _name);
  data.add(char_class, _class);
  data.add(char_level, itos(_level));
  data.add(char_exp, itos(_exp));
  data.add(char_hp, itos(_hp));
  data.add(char_mp, itos(_mp));
  if (_dead) data.add(char_dead, "true");
  else       data.add(char_dead, "false");
  data.add(char_time, itos(_time));

  data.add(class_hp, itos(_hp_base));
  data.add(class_mp, itos(_mp_base));
  data.add(class_dmg, itos(_dmg_base));
  data.add(class_speed, itos(_speed_base));
  data.add(class_tmax, itos(_tmax));
  
  data.add(class_str, itos(_str_base));
  data.add(class_dex, itos(_dex_base));
  data.add(class_int, itos(_int_base));
  data.add(class_spi, itos(_spi_base));
  data.add(class_fort, itos(_fort_base));
  
  return data;
}

void character::define(const row& char_row) {
  _id = atoi(char_row[char_id].c_str());
  _name = char_row[char_name];
  _class = char_row[char_class];
  _level = atoi(char_row[char_level].c_str());
  _exp = atoi(char_row[char_exp].c_str());
  _hp = atoi(char_row[char_hp].c_str());
  _mp = atoi(char_row[char_mp].c_str());
  _dead = (char_row[char_dead] == "true");
  _time = atoi(char_row[char_time].c_str());

  #ifdef SERVER
  row class_row = (class_table->select(query().where(predicate().And(equalto(class_name, _class)).And(equalto(class_level, itos(_level)))).limit(1)))[0];
  #else
  row class_row = char_row;
  #endif

  _hp_base    = atoi(class_row[class_hp].c_str());
  _mp_base    = atoi(class_row[class_mp].c_str());
  _dmg_base   = atoi(class_row[class_dmg].c_str());
  _speed_base = atoi(class_row[class_speed].c_str());
  _tmax       = atoi(class_row[class_tmax].c_str());
  
  _str_base   = atoi(class_row[class_str].c_str());
  _dex_base   = atoi(class_row[class_dex].c_str());
  _int_base   = atoi(class_row[class_int].c_str());
  _spi_base   = atoi(class_row[class_spi].c_str());
  _fort_base   = atoi(class_row[class_fort].c_str());

  _void_cache();
}

string character::encode() const {
  return to_row().to_str();
}

void character::decode(const string& data) {
  define(row().from_str(data));
}


void character::_level_up() {
#ifdef SERVER
  while (_exp <= exp_max()) {
    _exp -= exp_max();
    _level++;

    row class_row = (class_table->select(query().where(predicate().And(equalto(class_name, _class)).And(equalto(class_level, itos(_level)))).limit(1)))[0];
    _hp_base    = atoi(class_row[class_hp].c_str());
    _mp_base    = atoi(class_row[class_mp].c_str());
    _dmg_base   = atoi(class_row[class_dmg].c_str());
    _speed_base = atoi(class_row[class_speed].c_str());
    _tmax       = atoi(class_row[class_tmax].c_str());
  }
  _void_cache();
#endif
}


int character::id() const { return _id; }
void character::id(int c) { _id = c; }

int character::hp_max() const {
  if (!_hp_max_valid) {
    _hp_max = _hp_base + Fort()*15;

    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _hp_max += dmatter_it->second->hp_add();
        total_mul += dmatter_it->second->hp_mul();
      }
      dmatter_it++;
    }

    _hp_max *= total_mul;
    _hp_max_valid = true;
  }
  return _hp_max;
}

int character::mp_max() const {
  if (!_mp_max_valid) {
    _mp_max = _mp_base + Int()*8;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _mp_max += dmatter_it->second->mp_add();
        total_mul += dmatter_it->second->mp_mul();
      }
      dmatter_it++;
    }
  
    _mp_max *= total_mul;
    _mp_max_valid = true;
  }
  return _mp_max;
}

int character::hp() const { return _hp; }
void character::hp(int a) const {
  _hp = a;
  if (_hp > hp_max()) _hp = hp_max();
  else if (_hp < 1) {
    _hp = 0;
    _dead = true;
  }
}

int character::mp() const { return _mp; } 
void character::mp(int a) const {
  _mp = a;
  if (_mp > mp_max()) _mp = mp_max();
  else if (_mp < 0) _mp = 0;
}

string character::name() const { return _name; }

string character::c_class() const { return _class; }

bool character::dead() const { return _dead; } 
void character::dead(bool a) { _dead = a; }

int character::exp() const { return _exp; } 
void character::exp(int a) {
  _exp = a;
  if (_exp >= exp_max()) _level_up();
}

int character::exp_max() const { return _level*100; }

int character::level() const { return _level; } 

int character::speed() const { return _speed_base; } 

int character::tmax() const { return _tmax; } 

bool character::is_defined() const {
  return ( _name.length() > 0 && 
           _hp_base       > 0 &&
           _level         > 0 &&
           _speed_base    > 0.0
         );
}

void character::operator++(int) {
  _time += speed();
  if (_time > _tmax) _time = _tmax;
}

int character::dmg() const {
  srand(time(NULL));
  if (rand()%100+1 < 25) return 0;
  int damage = _dmg_base + Str()*10;
  if (rand()%100+1 < 5+Dex()/10) damage *= 2;

  damage += (int)ceil((float)damage*((float)((rand()%20+1)-10.0)/100.0)) + (rand()%10+1)-5;
  return damage;
}

void character::full_heal() {
  hp(hp_max());
  mp(mp_max());
  dead(false);
}

int character::Str() const  {
  if (!_str_valid) {
    _str = _str_base;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _str += dmatter_it->second->str_add();
        total_mul += dmatter_it->second->str_mul();
      }
      dmatter_it++;
    }
  
    _str *= total_mul;
    _str_valid = true;
  }
  return _str;
}

int character::Dex() const  {
  if (!_dex_valid) {
    _dex = _dex_base;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _dex += dmatter_it->second->dex_add();
        total_mul += dmatter_it->second->dex_mul();
      }
      dmatter_it++;
    }
  
    _dex *= total_mul;
    _dex_valid = true;
  }
  return _dex;
}

int character::Int() const  {
  if (!_int_valid) {
    _int = _int_base;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _int += dmatter_it->second->int_add();
        total_mul += dmatter_it->second->int_mul();
      }
      dmatter_it++;
    }
  
    _int *= total_mul;
    _int_valid = true;
  }
  return _int;
}

int character::Spi() const  {
  if (!_spi_valid) {
    _spi = _spi_base;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _spi += dmatter_it->second->spi_add();
        total_mul += dmatter_it->second->spi_mul();
      }
      dmatter_it++;
    }
  
    _spi *= total_mul;
    _spi_valid = true;
  }
  return _spi;
}
int character::Fort() const {
  if (!_fort_valid) {
    _fort = _fort_base;
  
    float total_mul = 1.0f;
    map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
    while (dmatter_it != _dmatter.end()) {
      if (dmatter_it->second->equipped()) {
        _fort += dmatter_it->second->fort_add();
        total_mul += dmatter_it->second->fort_mul();
      }
      dmatter_it++;
    }
  
    _fort *= total_mul;
    _fort_valid = true;
  }
  return _fort;
}

bool character::can_act(float threshold) const {
  return (((_time >= _tmax) || (_tmax-_time <= _tmax*threshold)) && (!_dead));
}

float character::timebar_percent() const {
  if (_dead) return 0.0;
  float percent = (float)_time/(float)_tmax;
  if (percent > 1.0) percent = 1.0;
  return percent;
}

void character::set_timebar(float percent) {
  _time = _tmax*percent;
}

void character::load_dmatter() {
  #ifdef SERVER
  map<int, darkmatter*>::iterator toon_dmatter_it = _dmatter.begin();
  while (toon_dmatter_it != _dmatter.end()) {
    delete toon_dmatter_it->second;
    toon_dmatter_it++;
  }
  _dmatter.clear();

  vector<row> results = dmatter_table->select(query().where(predicate().And(equalto(dmatter_owner_id, itos(_id)))));
  for (int i=0;i<results.size();i++) {
    darkmatter* new_dmatter = new darkmatter;
    new_dmatter->define(results[i]);
    _dmatter[new_dmatter->id()] = new_dmatter;
  }

  _void_cache();
  #endif
}

void character::add_dmatter(const row& dmatter_data) {
  int d_id = atoi(dmatter_data[dmatter_id].c_str());
  if (_dmatter.count(d_id) == 0) {
    _dmatter[d_id] = new darkmatter;
  }
  _dmatter[d_id]->define(dmatter_data);
  _void_cache();
}

void character::clear_dmatter() {
  map<int, darkmatter*>::iterator dmatter_it = _dmatter.begin();
  while (dmatter_it != _dmatter.end()) {
    delete dmatter_it->second;
    dmatter_it++;
  }
  _dmatter.clear();

  _void_cache();
}

void character::equip_dmatter(int dmatter_id, int armor_id, int slot_id) {
  if (_dmatter.find(dmatter_id) != _dmatter.end()) {
    _dmatter[dmatter_id]->equipped((slot_id >= 0));
    _void_cache();
  }
}

std::map<int, const darkmatter&> character::get_dmatter() const {
  map<int, const darkmatter&> return_val;
  map<int, darkmatter*>::const_iterator dmatter_it = _dmatter.begin();
  while (dmatter_it != _dmatter.end()) {
    return_val.insert(pair<int, const darkmatter&>(dmatter_it->first, *dmatter_it->second));
    dmatter_it++;
  }
  return return_val;
}

void character::_void_cache() const {
  _str_valid = false;
  _dex_valid = false;
  _int_valid = false;
  _spi_valid = false;
  _fort_valid = false;
  _hp_max_valid = false;
  _mp_max_valid = false;

  hp(_hp);
  mp(_mp);
}

bool character::has_dmatter(int dmatter_id) const {
  return (_dmatter.count(dmatter_id) > 0);
}

const darkmatter& character::get_dmatter(int dmatter_id) const {
  if (_dmatter.count(dmatter_id) == 0) return *(_dmatter.end()->second);
  return (*_dmatter.find(dmatter_id)->second);
}
