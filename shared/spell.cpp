// File: spell.cpp
// Written by Joshua Green

#include "spell.h"
#include "character.h"
#include "connection/transmission.h"
#include "db/row.h"
#include "str/str.h"
#include "shared.h"
#include "db_decl.h"
#include <string>
#include <iostream>
using namespace std;

spell::spell() {
  clear();
}

void spell::define(const row& data) {
  _id          = atoi(data[spell_id].c_str());
	_name        = data[spell_name];
	_level       = atoi(data[spell_level].c_str());
	_element     = data[spell_element];
	_mp          = atoi(data[spell_mp].c_str());
	_hp_mul      = atof(data[spell_hp_mul].c_str());
  _hp_max_mul  = atof(data[spell_hp_max_mul].c_str());
	_hp_add      = atof(data[spell_hp_add].c_str());
	_mp_mul      = atof(data[spell_mp_mul].c_str());
  _mp_max_mul  = atof(data[spell_mp_max_mul].c_str());
	_mp_add      = atof(data[spell_mp_add].c_str());
	_all         = (data[spell_all] == "true");
  _castable_on_dead = (data[spell_on_dead] == "true");
}

void spell::clear() {
    _id = -1;
    _level = 0;
    _mp = 0;
    
    _hp_add     = 0.0f;
    _hp_mul     = 0.0f;
    _hp_max_mul = 0.0f;
    _mp_add     = 0.0f;
    _mp_mul     = 0.0f;
    _mp_max_mul = 0.0f;

    _all = false;
    _castable_on_dead = false;

    _name = "";
    _element = "";
}

transmission* spell::cast(character* caster, character* receiver) const {
  transmission* messages = new transmission[2];

  if (caster->dead()) {
    messages[0].add(SERVER_ERROR).add(DELIM).add(LOCAL_IS_DEAD);
    return messages;
  }

  if (caster->mp() < _mp) {
    messages[0].add(SERVER_ERROR).add(DELIM).add(NOT_ENOUGH_MANA);
    return messages;
  }

  messages[0].add(SPELL).add(DELIM).add(_name).add(DELIM).add(itos(caster->id())).add(DELIM).add(itos(receiver->id()));

  caster->mp(caster->mp()-_mp);
  caster->set_timebar(0.0f);

  int old_hp = receiver->hp(), old_mp = receiver->mp();
  int hp_dmg = (int)(receiver->hp_max()*_hp_max_mul + receiver->hp()*_hp_mul + _hp_add);
  int mp_dmg = (int)(receiver->mp_max()*_mp_max_mul + receiver->mp()*_mp_mul + _mp_add);
  receiver->hp(receiver->hp()+hp_dmg);
  receiver->mp(receiver->mp()+mp_dmg);

  messages[0].add(DELIM).add(itos(hp_dmg));
  messages[0].add(DELIM).add(itos(mp_dmg));
  
  messages[1] = messages[0];

  // message: (SPELL) [name_of_spell] [caster_id] [recipient_id] [delta_hp] [delta_mp]
  return messages;
}

string spell::name() const {
  return _name;
}

string spell::element() const {
  return _element;
}

int spell::mp() const {
  return _mp;
}

bool spell::castable_on_dead() const {
  return _castable_on_dead;
}
