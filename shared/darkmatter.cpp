// File: darkmatter.cpp
// Written by Joshua Green

#include "darkmatter.h"
#include "db_decl.h"
#include "spell.h"
#include "str/str.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

extern map<string, spell*> all_spells;

darkmatter::darkmatter() {
  clear();
}

void darkmatter::clear() {
  _id = -1;
  _owner_id = -1;
  _linked_to = -1;
  _exp = 0;
  _level = 0;
  _equipped = false;
  _wslot = 0;
  _aslot = 0;

  _name = "";
  _spells.clear();

  _hp_add = 0;
  _mp_add = 0;
  _hp_mul = 0.0f;
  _mp_mul = 0.0f;

  _str_add = 0;
  _dex_add = 0;
  _int_add = 0;
  _spi_add = 0;
  _fort_add = 0;
  
  _str_mul = 0.0f;
  _dex_mul = 0.0f;
  _int_mul = 0.0f;
  _spi_mul = 0.0f;
  _fort_mul = 0.0f;
}

void darkmatter::define(const row& data) {
  _id         = atoi(data[dmatter_id].c_str());
  _owner_id   = atoi(data[dmatter_owner_id].c_str());
  _linked_to  = atoi(data[dmatter_linked_to].c_str());
  _equipped   = (data[dmatter_equipped] == "true");
  _wslot      = atoi(data[dmatter_wslot].c_str());
  _aslot      = atoi(data[dmatter_aslot].c_str());
  _exp        = atoi(data[dmatter_exp].c_str());
  _level      = atoi(data[dmatter_level].c_str());

  _name       = data[dmatter_name];
  if (!data.is_defined(dmatter_name)) _name = data[dmclass_name];

  vector<string> spell_list = explode(strtolower(data[dmatter_spells]), ",", -1);
  for (int i=0;i<spell_list.size();i++) {
    if (all_spells.find(spell_list[i]) != all_spells.end()) {
      _spells[spell_list[i]] = all_spells[spell_list[i]];
    }
  }
  spell_list.clear();

  if (!data.is_defined(dmatter_id)) _id = atoi(data[dmclass_id].c_str());

  _hp_add     = atoi(data[dmatter_hp_add].c_str());
  _mp_add     = atoi(data[dmatter_mp_add].c_str());
  _hp_mul     = atoi(data[dmatter_hp_mul].c_str());
  _mp_mul     = atoi(data[dmatter_mp_mul].c_str());

  _str_add    = atoi(data[dmatter_str_add].c_str());
  _dex_add    = atoi(data[dmatter_dex_add].c_str());
  _int_add    = atoi(data[dmatter_int_add].c_str());
  _spi_add    = atoi(data[dmatter_spi_add].c_str());
  _fort_add   = atoi(data[dmatter_fort_add].c_str());
  
  _str_mul    = atoi(data[dmatter_str_mul].c_str());
  _dex_mul    = atoi(data[dmatter_dex_mul].c_str());
  _int_mul    = atoi(data[dmatter_int_mul].c_str());
  _spi_mul    = atoi(data[dmatter_spi_mul].c_str());
  _fort_mul   = atoi(data[dmatter_fort_mul].c_str());
}

row darkmatter::to_row() const {
  row data;

  data.add(dmatter_id,        itos(_id));
  data.add(dmatter_owner_id,  itos(_owner_id));
  data.add(dmatter_linked_to, itos(_linked_to));
  if (_equipped)  data.add(dmatter_equipped, "true");
  else            data.add(dmatter_equipped, "false");
  data.add(dmatter_wslot, itos(_wslot));
  data.add(dmatter_aslot, itos(_aslot));
  data.add(dmatter_exp,       itos(_exp));
  data.add(dmatter_level,     itos(_level));
  data.add(dmatter_name,      _name);

  string spell_list;
  map<string, spell*>::const_iterator spell_it = _spells.begin();
  while (spell_it != _spells.end()) {
    spell_list += spell_it->second->name();
    spell_list += ",";
    spell_it++;
  }
  if (spell_list.length() > 0) spell_list.erase(spell_list.length()-1);
  data.add(dmatter_spells,    spell_list);

  data.add(dmatter_hp_add,    itos(_hp_add));
  data.add(dmatter_mp_add,    itos(_mp_add));
  data.add(dmatter_hp_mul,    ftos(_hp_mul));
  data.add(dmatter_mp_mul,    ftos(_mp_mul));
  data.add(dmatter_str_add,   itos(_str_add));
  data.add(dmatter_dex_add,   itos(_dex_add));
  data.add(dmatter_int_add,   itos(_int_add));
  data.add(dmatter_spi_add,   itos(_spi_add));
  data.add(dmatter_fort_add,  itos(_fort_add));
  data.add(dmatter_str_mul,   ftos(_str_mul));
  data.add(dmatter_dex_mul,   ftos(_dex_mul));
  data.add(dmatter_int_mul,   ftos(_int_mul));
  data.add(dmatter_spi_mul,   ftos(_spi_mul));
  data.add(dmatter_fort_mul,  ftos(_fort_mul));

  return data;
}

int darkmatter::id() const { return _id; }
string darkmatter::name() const { return _name; }
int darkmatter::level() const { return _level; }
int darkmatter::exp() const{ return _exp; }

void darkmatter::equipped(bool equip) { _equipped = equip; }
bool darkmatter::equipped() { return _equipped; }

bool darkmatter::has_spell(const string& s) const {
  string spell_name = strtolower(s);

  map<string, spell*>::const_iterator it = _spells.begin();
  while (it != _spells.end()) {
    if (it->first == spell_name) return true;
    it++;
  }
  return false;
}

int darkmatter::hp_add() { return _hp_add; }
float darkmatter::hp_mul() { return _hp_mul; }

int darkmatter::mp_add() { return _mp_add; }
float darkmatter::mp_mul() { return _mp_mul; }

int darkmatter::str_add() { return _str_add; }
float darkmatter::str_mul() { return _str_mul; }

int darkmatter::dex_add() { return _dex_add; }
float darkmatter::dex_mul() { return _dex_mul; }

int darkmatter::int_add() { return _int_add; }
float darkmatter::int_mul() { return _int_mul; }

int darkmatter::spi_add() { return _spi_add; }
float darkmatter::spi_mul() { return _spi_mul; }

int darkmatter::fort_add() { return _fort_add; }
float darkmatter::fort_mul() { return _fort_mul; }

void darkmatter::print() const {
    cout << "LVL" << _level << " " << _name << " (" << _exp << "exp)";
    if (!_equipped) cout << " NOT";
    cout << " EQUIPPED" << endl << "  ";
    
    map<string, spell*>::const_iterator spell_it  = _spells.begin();
    while (spell_it != _spells.end()) {
      cout << spell_it->second->name() << "( " << spell_it->second->mp() << "mp)" << endl;
      spell_it++;
    }

    if (_hp_add != 0)   cout << " " << _hp_add << " hp";
    if (_hp_mul != 0.0) cout << " " << _hp_mul << "% hp";
    if (_mp_add != 0)   cout << " " << _mp_add << " mp";
    if (_mp_mul != 0.0) cout << " " << _mp_mul << "% mp";

    if (_str_add != 0)   cout << " " << _str_add << " str";
    if (_str_mul != 0.0) cout << " " << _str_mul << "% str";
    if (_dex_add != 0)   cout << " " << _dex_add << " dex";
    if (_dex_mul != 0.0) cout << " " << _dex_mul << "% dex";
    if (_int_add != 0)   cout << " " << _int_add << " int";
    if (_int_mul != 0.0) cout << " " << _int_mul << "% int";
    if (_spi_add != 0)   cout << " " << _spi_add << " spi";
    if (_spi_mul != 0.0) cout << " " << _spi_mul << "% spi";
    if (_fort_add != 0)   cout << " " << _fort_add << " fort";
    if (_fort_mul != 0.0) cout << " " << _fort_mul << "% fort";

    cout << endl;
}
