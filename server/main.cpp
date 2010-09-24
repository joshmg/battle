// File: main.cpp
// Written by Joshua Green

#include "server.h"
#include "../shared/shared.h"
#include "../shared/character.h"
#include "../shared/spell.h"
#include "../shared/darkmatter.h"
#include "../shared/connection/transmission.h"
#include "../shared/db/db.h"
#include "../shared/str/str.h"
#include <string>
#include <vector>
#include <process.h>
#include <windows.h>
#include <iostream>

#define defn_once
#include "../shared/db_decl.h"

using namespace std;

vector<server*> hosts;
map<string, spell*> all_spells;
const int CONNECTION_CAP = 2;
//map<string, darkmatter*> all_dmatter_class;

bool shutdown_flag;
p2p* new_connection;

extern void time_loop(void*);

void new_toon(int server_id, int connection_id, string& data) {
  p2p* connection = hosts[server_id]->get_connection(connection_id);

  vector<string> temp = explode(data, DELIM, 2);
  if (temp.size() < 2) {
    cout << "WARNING: connection " << connection_id << " requested new toon data with invalid params." << endl;
    return;
  }
  string toon_name = temp[0];
  string toon_class = temp[1];
  temp.clear();

  data.clear();

  // purge letters unsafe for filenames:
  for (int i=0;i<toon_name.length();i++) {
    if (!((toon_name[i] >= '0' && toon_name[i] <= '9') || (toon_name[i] >= 'A' && toon_name[i] <= 'Z') || (toon_name[i] >= 'a' && toon_name[i] <= 'z') )) {
      toon_name.erase(i);
      i--;
    }
  }

  row char_row, class_row;

  vector<row> results;
  results = char_table->select(query().where(predicate().And(equalto(char_name, toon_name))).limit(1));
  if (results.size() > 0) char_row = results[0];
  results.clear();

  if (!char_row.is_empty()) {
    connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(CHARACTER_TAKEN));
    return;
  }

  string toon_level = "1";

  cout << "Creating new character (" << toon_name << "::" << toon_class << ":" << toon_level << ")..." << endl;
  class_row = (class_table->select(query().where(predicate().And(equalto(class_name, toon_class)).And(equalto(class_level, toon_level))).limit(1)))[0];
  character* toon = new character(toon_name, class_row);

  char_table->add_row(row() .add(char_name,   toon_name)
                            .add(char_class,  toon->c_class())
                            .add(char_level,  itos(toon->level()))
                            .add(char_exp,    itos(toon->exp()))
                            .add(char_hp,     itos(toon->hp()))
                            .add(char_mp,     itos(toon->mp()))
                            .add(char_dead,   itos(toon->dead()))
                            .add(char_online, "true")
                     );

  // get toon ID and validate save:
  results = char_table->select(query().where(predicate().And(equalto(char_name, toon_name))).limit(1));
  if (results.size() > 0) char_row = results[0];
  else {
    delete toon;
    connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(CHARACTER_CORRUPTED));
    connection->transmit(transmission().add(EXECUTE).add(DELIM).add(DELETE_SAVE_FILE).add(DELIM).add("NULL"));
    hosts[server_id]->remove_connection(connection_id);
    cout << "WARNING: Error creating toon." << endl;
    return;
  }
  results.clear();
  toon->define(char_row); // or: toon->id(char_row[char_id]);
  toon->full_heal();
  hosts[server_id]->add_character(connection_id, toon);
  connection->transmit(transmission().add(TOON_DATA).add(DELIM).add(itos(toon->id())).add(DELIM).add(toon->encode()));
}

void load_toon(int server_id, int connection_id, string& toon_name) {
  p2p* connection = hosts[server_id]->get_connection(connection_id);

  vector<row> results;
  row char_row, class_row;

  results = char_table->select(query().where(predicate().And(equalto(char_name, toon_name))).limit(1));
  if (results.size() > 0) char_row = results[0];
  results.clear();

  if (char_row.is_empty()) {
    cout << "WARNING: Connection " << connection_id << " requesting character that does not exist." << endl;
    connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(CHARACTER_NOT_FOUND));
    connection->transmit(transmission().add(EXECUTE).add(DELIM).add(DELETE_SAVE_FILE).add(DELIM).add("NULL"));
    hosts[server_id]->remove_connection(connection_id);
    return;
  }

  character* toon = new character();
  toon->define(char_row);
  
  if (!toon->is_defined()) {
    cout << "WARNING: Row " << char_row[char_id] << " is corrupted. (Requested by Connection " << connection_id << ")" << endl;
    char_table->delete_row(char_row);
    delete toon;
    connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(CHARACTER_CORRUPTED));
    connection->transmit(transmission().add(EXECUTE).add(DELIM).add(DELETE_SAVE_FILE).add(DELIM).add("NULL"));
    hosts[server_id]->remove_connection(connection_id);
    return;
  }

  hosts[server_id]->add_character(connection_id, toon);
  toon->full_heal();
  cout << "Loaded character..." << endl;
  connection->transmit(transmission().add(TOON_DATA).add(DELIM).add(itos(toon->id())).add(DELIM).add(toon->encode()));

  // update status to online:
  char_table->edit_row(char_row, row().add(char_online, "true"));
}

void transmit_toons(int server_id, int connection_id, string&) {
  p2p* connection = hosts[server_id]->get_connection(connection_id);

  map<int, character*> all_toons = hosts[server_id]->get_char_map();
  map<int, character*>::iterator it = all_toons.begin();
  while (it != all_toons.end()) {
    connection->transmit(transmission().add(TOON_DATA).add(DELIM).add(itos(it->second->id())).add(DELIM).add(it->second->encode()));
    it++;
  }
}

void transmit_dmatter(int server_id, int connection_id, string&) {
  p2p* connection = hosts[server_id]->get_connection(connection_id);

  map<int, character*> all_toons = hosts[server_id]->get_char_map();
  map<int, character*>::iterator it = all_toons.begin();
  while (it != all_toons.end()) {
    map<int, const darkmatter&> toon_dmatter = it->second->get_dmatter();
    map<int, const darkmatter&>::const_iterator dmatter_it = toon_dmatter.begin();
    //connection->transmit(transmission().add(CLEAR_TOON_DMATTER));
    while (dmatter_it != toon_dmatter.end()) {
      connection->transmit(transmission().add(TOON_DMATTER).add(DELIM).add(itos(it->second->id())).add(DELIM).add(dmatter_it->second.to_row().to_str()));
      dmatter_it++;
    }
    it++;
  }
}

void add_exp(int server_id, int connection_id, int gained_exp) {
  p2p* connection = hosts[server_id]->get_connection(connection_id);
  character* toon = hosts[server_id]->get_character(connection_id);

  int old_level = toon->level();

  toon->exp(toon->exp()+gained_exp);  // add exp
  connection->transmit(transmission().add(TOON_DATA).add(DELIM).add(itos(connection_id)).add(DELIM).add(toon->encode()));
  Sleep(750); // wait for new toon data to arrive...
  connection->transmit(transmission().add(GAINED_EXP).add(DELIM).add(itos(gained_exp)));

  if (toon->level() > old_level) {
    transmission message;
    // transmit level-up data to all players connected to the host:
    // (Player A) has leveled to (level).
    message.add(LEVEL_UP).add(DELIM).add(itos(toon->id())).add(DELIM).add(itos(toon->level()));
    map<int, character*> all_characters = hosts[server_id]->get_char_map();
    map<int, character*>::iterator it = all_characters.begin();
    while (it != all_characters.end()) {
      hosts[server_id]->get_connection(it->first)->transmit(message);
      it++;
    }
  }
}

void attack(int server_id, int local_id, string& data) {
  p2p* local_connection = hosts[server_id]->get_connection(local_id);
  character* local_toon = hosts[server_id]->get_character(local_id);

  cout << "attack1" << endl;

  int remote_id = atoi(data.c_str());
  p2p* remote_connection = hosts[server_id]->get_connection(remote_id);
  character* remote_toon = hosts[server_id]->get_character(remote_id);

  if (remote_toon == 0) {
    cout << "WARNING: connection " << local_id << " attacking invalid remote toon id (" << data << ")." << endl;
    return;
  }

  cout << "remote_id = " << remote_id << endl;

  cout << "attack2" << endl;

  if (!local_toon->can_act()) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(TIMEBAR_NOT_FULL));
    return;
  }
  cout << "a";
  if (local_toon->dead()) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(LOCAL_IS_DEAD));
    return;
  }
  cout << "b";
  if (remote_toon->dead()) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(REMOTE_IS_DEAD));
    return;
  }

  cout << endl << "attack3" << endl;

  int damage = local_toon->dmg();
  remote_toon->hp(remote_toon->hp()-damage);
  local_toon->set_timebar(0.0f);

  cout << "attack4" << endl;

  transmission message;
  // transmit damage data to all players connected to the host:
  // (Player A) attacks (Player B) for (1234)hp.
  message.add(DAMAGE).add(DELIM).add(itos(local_id)).add(DELIM).add(itos(remote_id)).add(DELIM).add(itos(damage));
  map<int, character*> all_characters = hosts[server_id]->get_char_map();
  map<int, character*>::iterator it = all_characters.begin();
  while (it != all_characters.end()) {
    hosts[server_id]->get_connection(it->first)->transmit(message);
    it++;
  }
  message.clear();

  cout << "attack5" << endl;

  if (remote_toon->dead()) {
    // transmit death data to all players connected to the host:
    // (Player A) has died.
    message = transmission().add(DEATH).add(DELIM).add(itos(remote_id));
    all_characters = hosts[server_id]->get_char_map();
    it = all_characters.begin();
    while (it != all_characters.end()) {
      hosts[server_id]->get_connection(it->first)->transmit(message);
      it++;
    }
    message.clear();

    cout << "attack6" << endl;

    int local_exp = ((float)remote_toon->level()/(float)local_toon->level())*10.0f;
    int remote_exp = 5;

    add_exp(server_id, local_id, local_exp);
    add_exp(server_id, remote_id, remote_exp);
  }

  cout << "attack end" << endl;
}

//void disconnect(int server_id, int connection_id, string& data) {
//  
//}

void cast_spell(int server_id, int local_id, string& data) {
  p2p* local_connection = hosts[server_id]->get_connection(local_id);
  character* local_toon = hosts[server_id]->get_character(local_id);

  vector<string> temp = explode(data, DELIM, 1);
  if (temp.size() < 2) {
    cout << "WARNING: Invalid data for spell cast." << endl;
    return;
  }
  int remote_id = atoi(temp[0].c_str());
  string spell_name = strtolower(temp[1]);
  temp.clear();

  bool toon_can_cast = false;
  map<int, const darkmatter&> toon_dmatter = local_toon->get_dmatter();
  map<int, const darkmatter&>::const_iterator dmatter_it = toon_dmatter.begin();
  while (dmatter_it != toon_dmatter.end()) {
    if (dmatter_it->second.has_spell(spell_name)) {
      toon_can_cast = true;
      break;
    }
    dmatter_it++;
  }
  
  if (!toon_can_cast) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(TOON_DOESNT_HAVE_SPELL));
    return;
  }

  p2p* remote_connection = hosts[server_id]->get_connection(remote_id);
  character* remote_toon = hosts[server_id]->get_character(remote_id);

  if (all_spells.find(spell_name) == all_spells.end()) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(SPELL_DOES_NOT_EXIST));
    return;
  }
  const spell* spell_data = (all_spells.find(spell_name)->second);
  if (remote_toon->dead() && !spell_data->castable_on_dead()) {
    local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(REMOTE_IS_DEAD));
    return;
  }

  transmission* transmissions = spell_data->cast(local_toon, remote_toon);
  local_connection->transmit(transmissions[0]);
  map<int, character*> all_characters = hosts[server_id]->get_char_map();
  map<int, character*>::iterator it = all_characters.begin();
  while (it != all_characters.end()) {
    if (it->first != local_id) hosts[server_id]->get_connection(it->first)->transmit(transmissions[1]);
    it++;
  }
  delete[] transmissions;

  if (remote_toon->dead()) {
    // transmit death data to all players connected to the host:
    // (Player A) has died.
    transmission message = transmission().add(DEATH).add(DELIM).add(itos(remote_id));
    all_characters = hosts[server_id]->get_char_map();
    it = all_characters.begin();
    while (it != all_characters.end()) {
      hosts[server_id]->get_connection(it->first)->transmit(message);
      it++;
    }
    message.clear();

    int local_exp = ((float)remote_toon->level()/(float)local_toon->level())*10.0f;
    int remote_exp = 5;

    add_exp(server_id, local_id, local_exp);
    add_exp(server_id, remote_id, remote_exp);
  }
}

void chat_message(int server_id, int connection_id, string& data) {
  map<int, character*> all_characters = hosts[server_id]->get_char_map();
  map<int, character*>::iterator it = all_characters.begin();
  while (it != all_characters.end()) {
    if (it->first != connection_id) hosts[server_id]->get_connection(it->first)->transmit(transmission().add(GLOBAL_MESSAGE).add(DELIM).add(itos(connection_id)).add(DELIM).add(data));
    it++;
  }
}

void shop(int server_id, int connection_id, string& data) {
  p2p* local_connection = hosts[server_id]->get_connection(connection_id);
  character* local_toon = hosts[server_id]->get_character(connection_id);

  if (data == BROWSE_DMATTER) {
    multimap<int, darkmatter*>::iterator available_dmatter_it = hosts[server_id]->available_dmatter.begin();
    local_connection->transmit(transmission().add(DMATTER_LIST).add(DELIM).add("NULL"));
    while (available_dmatter_it != hosts[server_id]->available_dmatter.end()) {
      local_connection->transmit(transmission().add(AVAILABLE_DMATTER).add(DELIM).add(available_dmatter_it->second->to_row().to_str()));
      available_dmatter_it++;
    }
    local_connection->transmit(transmission().add(END_DMATTER_LIST).add(DELIM).add("NULL"));
  }
  else {
    vector<string> params = explode(data, DELIM, -1);
    if (params.size() < 3) {
      cout << "WARNING: Shop parameters invalid. (Connection " << connection_id << ")" << endl;
      return;
    }
    if (params[0] == BUY_DMATTER) {
      string buy_dmatter = strtolower(params[1]); // the name of the dmatter class
      string buy_dmatter_level = params[2];
      if (buy_dmatter.size() == 0) {
        cout << "WARNING: NULL darkmatter params." << endl;
        return;
      }

      buy_dmatter[0] = buy_dmatter[0]-32; // capitalize first letter...
      vector<row> results = dmatter_class_table->select(query().where(predicate().And(equalto(dmclass_name, buy_dmatter)).And(equalto(dmclass_level, buy_dmatter_level))).limit(1));

      if (results.size() == 0) {
        cout << "WARNING: Connection " << connection_id << " is requesting a darkmatter that does not exist. (" << buy_dmatter << ")" << endl;
        local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(DMATTER_DOES_NOT_EXIST));
        return;
      }
      row buy_dmatter_class = results[0];
      int dmatter_c_id = atoi(buy_dmatter_class[dmclass_id].c_str());
      if (hosts[server_id]->available_dmatter.count(dmatter_c_id) != 0) {
        multimap<int, darkmatter*>::iterator available_it = hosts[server_id]->available_dmatter.find(dmatter_c_id);
        if (available_it == hosts[server_id]->available_dmatter.end()) {
          cout << "WARNING: Connection " << connection_id << " is requesting darkmatter that is not available." << endl;
          local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(DMATTER_DOES_NOT_EXIST));
          return;
        }
        if (local_toon->exp() >= available_it->second->exp()) {
          row dmatter_data = available_it->second->to_row();
          delete available_it->second;
          hosts[server_id]->available_dmatter.erase(dmatter_c_id);
          dmatter_data.remove(dmatter_id);
          dmatter_data.remove(dmclass_id);
          dmatter_data.remove(dmatter_owner_id);
          dmatter_data.add(dmatter_owner_id, itos(local_toon->id()));
          dmatter_data = dmatter_table->add_row(dmatter_data); // add dmatter to database

          local_toon->add_dmatter(dmatter_data); // add dmatter to character

          local_toon->exp(local_toon->exp() - available_it->second->exp()); // subtract exp

          //local_toon->load_dmatter();
        }
        else {
          cout << local_toon->exp() << " < " << available_it->second->exp() << endl;
          local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(NOT_ENOUGH_EXP));
          return;
        }
      }
      else {
        cout << "WARNING: Character requesting darkmatter that is not available." << endl;
        local_connection->transmit(transmission().add(SERVER_ERROR).add(DELIM).add(DMATTER_NOT_FOR_SALE));
        return;
      }
    }
  }
}

void equip_dmatter(int server_id, int connection_id, string& data) {
  p2p* local_connection = hosts[server_id]->get_connection(connection_id);
  character* local_toon = hosts[server_id]->get_character(connection_id);

  vector<string> temp = explode(data, DELIM, -1);
  if (temp.size() < 2) return;
  int dmatter_id = atoi(temp[0].c_str());
  int slot_id = atoi(temp[1].c_str());
  temp.clear();

  if (local_toon->has_dmatter(dmatter_id)) {
    local_toon->equip_dmatter(dmatter_id, 1, slot_id);
  }
}
void server_change(int server_id, int connection_id, string& data) {
  p2p* local_connection = hosts[server_id]->get_connection(connection_id);
  character* local_toon = hosts[server_id]->get_character(connection_id);

  local_connection->disable_receive();

  if (data == BATTLE_SERV) {
    hosts[server_id]->remove_character(connection_id, false);
    hosts[server_id]->remove_connection(connection_id, false);

    server* add_to = 0;
    for (int i=1;i<hosts.size();i++) {
      if (hosts[i]->size() < CONNECTION_CAP) {
        add_to = hosts[i];
        break;
      }
    }
    if (add_to == 0) {
      add_to = new server;
      hosts.push_back(add_to);
    }

    server_id = add_to->id();

    add_to->add_connection(local_connection);
    add_to->add_character(local_connection->id(), local_toon);
  }
  else if (data == STORE_SERV) {
    hosts[server_id]->remove_character(connection_id, false);
    hosts[server_id]->remove_connection(connection_id, false);

    server_id = 0;

    hosts[0]->add_connection(local_connection);
    hosts[0]->add_character(local_connection->id(), local_toon);
  }

  local_connection->enable_receive();
}

void input_branch(void*) {
  string input;
  while (strtolower(input) != "shutdown") {
    getline(cin, input);
    if (input == "db") {
      query q;
      cout << "Class data:" << endl;
      vector<row> results = class_table->select(query());
      class_table->print_structure(); cout << endl;
      for (int i=0;i<results.size();i++) {
        results[i].print(class_table);
      }
      cout << "Char data:" << endl;
      results = char_table->select(query());
      for (int i=0;i<results.size();i++) {
        results[i].print(char_table);
      }
      cout << "Darkmatter Class data:" << endl;
      results = dmatter_class_table->select(query());
      for (int i=0;i<results.size();i++) {
        results[i].print(dmatter_class_table);
      }
      cout << "Darkmatter data:" << endl;
      results = dmatter_table->select(query());
      for (int i=0;i<results.size();i++) {
        results[i].print(dmatter_table);
      }
    }
    else if (input == "optimize") {
      cout << "Optimizing... ";
      class_table->optimize();
      char_table->optimize();
      spell_table->optimize();
      dmatter_class_table->optimize();
      dmatter_table->optimize();
      cout << "done." << endl;
    }
  }
  shutdown_flag = true;
  new_connection->abort_link();
}

int main() {
  // initialize DB:
  db = new database;
  db->initialize("db");
  load_columns();

  cout << "Aquiring tables..." << endl;
  char_table            = db->get_table("_char_table");
  class_table           = db->get_table("_class_table");
  spell_table           = db->get_table("_spell_table");
  dmatter_class_table   = db->get_table("_dmatter_class_table");
  dmatter_table         = db->get_table("_dmatter_table");

  if (char_table == 0 || class_table == 0 || spell_table == 0 || dmatter_class_table == 0 || dmatter_table == 0) {
    cout << "Database tables not found... Run db_make.exe." << endl;
    char c;
    cin >> c;
    return 1;
  }

  // load spells
  cout << "Loading spell data..." << endl;
  vector<row> results = spell_table->select(query().where());
  for (int i=0;i<results.size();i++) {
    spell* temp_spell = new spell;
    temp_spell->define(results[i]);
    all_spells[strtolower(results[i][spell_name])] = temp_spell;
  }
  results.clear();

  /*// load darkmatter
  cout << "Loading darkmatter class data..." << endl;
  results = dmatter_class_table->select(query().where());
  for (int i=0; i<results.size();i++) {
    darkmatter* temp_dmatter = new darkmatter;
    temp_dmatter->define(results[i]);
    all_dmatter_class[strtolower(results[i][dmclass_name])] = temp_dmatter;
  }*/

  shutdown_flag = false;

  cout << "Defining world-host commands..." << endl;
  hosts.push_back(new server);
  hosts.back()->enable_battle(false);
  hosts.back()->add_action(NEW_TOON, &new_toon);
  hosts.back()->add_action(LOAD_TOON, &load_toon);
  hosts.back()->add_action(REQUEST_TOON_DATA, &transmit_toons);
  hosts.back()->add_action(REQUEST_TOON_DMATTER, &transmit_dmatter);
  //hosts.back()->add_action(DISCONNECT, &disconnect);
  hosts.back()->add_action(GLOBAL_MESSAGE, &chat_message);
  hosts.back()->add_action(SHOP, &shop);
  hosts.back()->add_action(EQUIP_DMATTER, &equip_dmatter);
  hosts.back()->add_action(GOTO_SERV, &server_change);

  cout << "Defining battle-host commands..." << endl;
  hosts.push_back(new server);
  hosts.back()->add_action(NEW_TOON, &new_toon);
  hosts.back()->add_action(LOAD_TOON, &load_toon);
  hosts.back()->add_action(REQUEST_TOON_DATA, &transmit_toons);
  hosts.back()->add_action(REQUEST_TOON_DMATTER, &transmit_dmatter);
  hosts.back()->add_action(MELEE, &attack);
  //hosts.back()->add_action(DISCONNECT, &disconnect);
  hosts.back()->add_action(SPELL, &cast_spell);
  hosts.back()->add_action(GLOBAL_MESSAGE, &chat_message);
  hosts.back()->add_action(GOTO_SERV, &server_change);

  new_connection = new p2p;
  int connection_id_count = 0;
  cout << "Server started..." << endl;

  _beginthread(&input_branch, 0, (void*)0);
  _beginthread(&time_loop, 0, (void*)0);

  while (!shutdown_flag) {
    new_connection->set_port(SERVER_PORT);
    new_connection->set_recv_func(&recv_branch);
    new_connection->set_disc_func(&disc_branch);
    new_connection->id(connection_id_count);
    new_connection->link(SERVER_IP);

    if (new_connection->is_linked()) {
      new_connection->enable_receive();
      hosts[0]->add_connection(new_connection);
      new_connection = new p2p;
      connection_id_count++;
    }
    else Sleep(250);
  }

  delete new_connection;

  cout << "Shutting down..." << endl;
  for (int i=0;i<hosts.size();i++) {
    hosts[i]->shutdown(3);
  }

  db->close();

  // deallocate all spell data from memory
  map<string, spell*>::iterator spell_it = all_spells.begin();
  while (spell_it != all_spells.end()) {
    delete spell_it->second;
    spell_it++;
  }

  /*// deallocate all dmatter class data from memory
  map<string, darkmatter*>::iterator dmatter_it = all_dmatter_class.begin();
  while (dmatter_it != all_dmatter_class.end()) {
    delete dmatter_it->second;
    dmatter_it++;
  }*/

  Sleep(1000);

  return 0;
}
