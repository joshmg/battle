// File: battle.cpp
// Written by Joshua Green

#include "starburst.h"
#include "projectile.h"
#include "../shared/connection/connection.h"
#include "../shared/connection/transmission.h"
#include "../shared/fileio/fileio.h"
#include "../shared/character.h"
#include "../shared/str/str.h"
#include "../shared/shared.h"
#include "../shared/spell.h"
#include "../shared/darkmatter.h"
#include <string>
#include <map>
#include <process.h>
#include <windows.h>
#include <iostream>

#include "../shared/db/row.h"
#include "../shared/db/column.h"
#pragma comment(lib, "../shared/db/db_lite.lib")
#define defn_once
#include "../shared/db_decl.h"

using namespace std;

bool QUIT;

map<string, spell*> all_spells;

void connection_loop(void*);

int _argc;
char** _argv;
p2p* connection;
string SERVER_CODE;
int CONNECTION_ID, TOON_ID;
map<int, character*>  toons;
map<int, starburst*>  hp_animations;
map<int, starburst*>  mp_animations;
map<int, projectile*> spell_animations;
bool MASTER_ABORT = false; // used to close main() upon critical server errors
bool KEEP_SYNC = true;
fileio save_file;

void glutbranch(void*);

void keep_sync(void*) {
  while (connection->is_linked()) {
    if (KEEP_SYNC) connection->transmit(transmission(SERVER_CODE).add(DELIM).add(REQUEST_TOON_DATA));
    Sleep(100);
    if (KEEP_SYNC) connection->transmit(transmission(SERVER_CODE).add(DELIM).add(REQUEST_TOON_DMATTER));
    Sleep(400);
  }
}

int find_toon_id(const string& name) {
  string toon_name = strtolower(name);
  map<int, character*>::iterator it = toons.begin();
  while (it != toons.end()) {
    if (toon_name == strtolower(it->second->name())) {
      return it->second->id();
    }
    it++;
  }
  return -1;
}

int main(int argc, char** argv) {
  _argc = argc;
  _argv = argv;

  CONNECTION_ID = -1;
  TOON_ID = -1;
  QUIT = false;

  connection = new p2p;
  connection->set_recv_func(&connection_loop);
  load_columns();

  cout << "Your IP: " << connection->get_local_ip() << endl;
  cout << "Connecting to " << SERVER_IP << "..." << endl;
  connection->link(SERVER_IP);
  if (connection->is_linked()) cout << "Connected! (" << connection->get_remote_ip() << ")" << endl;
  else {
    cout << "Connection failed." << endl;
    Sleep(2000);
    return 1;
  }

  connection->enable_receive();

  cout << endl << endl;

  // SERVER CODE:
  cout << "Handshaking with server..";
  while (SERVER_CODE.length() == 0 && CONNECTION_ID < 0 && !MASTER_ABORT) {
    cout << ".";
    Sleep(250);
  }
  cout << " done." << endl;

  //toons[LOCAL_ID] = new character;
  //hp_animations[LOCAL_ID] = new starburst;
  //mp_animations[LOCAL_ID] = new starburst;
  //spell_animations[LOCAL_ID] = new projectile;

  // CHARACTER INFORMATION:
  bool new_toon = false;
  string toon_name;
  save_file.open("battle.sav", "r");
  if (save_file.is_open()) {
    toon_name = save_file.read(-1);
    save_file.close();

    if (toon_name.length() > 0) {
      cout << "Load " << toon_name << "? (y/n) ";
      string t;
      getline(cin, t);
      if (strtolower(t)[0] != 'y') toon_name.clear();
    }
  }

  if (toon_name.length() == 0) {
    new_toon = true;
    cout << "Enter new character's name: ";
    getline(cin, toon_name);
  }
  if (new_toon) connection->transmit(transmission(SERVER_CODE).add(DELIM).add(NEW_TOON) .add(DELIM).add(toon_name).add(DELIM).add("Swordsman"));
  else          connection->transmit(transmission(SERVER_CODE).add(DELIM).add(LOAD_TOON).add(DELIM).add(toon_name));

  cout << "Downloading character profile...";
  while (TOON_ID < 0 && !MASTER_ABORT) { // !toons[TOON_ID]->is_defined()
    cout << ".";
    Sleep(1000);
  }
  if (!MASTER_ABORT) cout << " done." << endl;

  if (!MASTER_ABORT) {
    cout << "Saving character file...";
    save_file.open("battle.sav", "w");
    save_file.write(toons[TOON_ID]->name());
    save_file.close();
    cout << " done." << endl;
  }

  if (!MASTER_ABORT) _beginthread(&keep_sync, 0, (void*)0);

  /*if (!MASTER_ABORT) {
    cout << "Waiting for remote character profile...";
    while (toons.size() < 2 && !MASTER_ABORT) {
      cout << ".";
      Sleep(1000);
    }
    cout << " done." << endl;
  }*/

  // ---------------------- OPENGL ---------------------- //
  if (!MASTER_ABORT) _beginthread(&glutbranch, 0, (void*)0);
  // -------------------- END OPENGL -------------------- //

  string input;
  if (!MASTER_ABORT) cout << "Ready..." << endl << endl;
  
  // ---------------- ACTION LOOP ---------------- //
  while ((!QUIT) && connection->is_linked() && !MASTER_ABORT) {
    getline(cin, input);
    vector<string> words = explode(strtolower(input), " ", -1);
    //words[0] = strtolower(words[0]);
    // ------------------ MELEE ATTACK ------------------- //
    if (words[0] == "attack") {
      if (words.size() < 2) cout << "Invalid target1." << endl;
      else {
        int target_id = find_toon_id(words[1]);
        if (target_id < 0) cout << "Invalid target2." << endl;

        else if (toons[TOON_ID]->dead()) cout << "You are dead." << endl;
        else if (toons[target_id]->dead()) cout << toons[target_id]->name() << " is dead." << endl;
        else if (!toons[TOON_ID]->can_act(0.1f)) cout << "Your timebar is not full. (" << toons[TOON_ID]->timebar_percent()*100.0f << "%)" << endl;
        else {
          connection->transmit(transmission(SERVER_CODE).add(DELIM).add(MELEE).add(DELIM).add(itos(target_id)));
        }
      }
    }
    // ------------------ CAST SPELL ------------------- //
    else if (words[0] == "cast") {
      if (words.size() < 3) cout << "Invalid target." << endl;
      else if (toons[TOON_ID]->dead()) cout << "You are dead." << endl;
      else if (!toons[TOON_ID]->can_act(0.1f)) cout << "Your timebar is not full. (" << toons[TOON_ID]->timebar_percent()*100.0f << "%)" << endl;
      else {
        int target_id = find_toon_id(words[2]);
        if (target_id < 0) cout << "Invalid target." << endl;
        else connection->transmit(transmission(SERVER_CODE).add(DELIM).add(SPELL).add(DELIM).add(itos(target_id)).add(DELIM).add(words[1]));
      }
    }
    // -------------------- SHOP -------------------- //
    else if (words[0] == "shop") {
      if (words.size() < 3) {
        cout << "Browse:" << endl;
        cout << "  Darkmatter" << endl;
      }
      else {
        if (words[1] == "browse") {
          if (words[2] == "darkmatter") {
            connection->transmit(transmission(SERVER_CODE).add(DELIM).add(SHOP).add(DELIM).add(BROWSE_DMATTER));
            cout << "Requesting..." << endl;
          }
        }
      }
      if (words.size() > 3) {
        if (words[1] == "buy") {
          if (words[2] == "darkmatter") {
            if (words.size() < 4) cout << "Invalid darkmatter type." << endl;
            else if (words.size() < 5) cout << "Invalid darkmatter level." << endl;
            else connection->transmit(transmission(SERVER_CODE).add(DELIM).add(SHOP).add(DELIM).add(BUY_DMATTER).add(DELIM).add(words[3]).add(DELIM).add(words[4]));
          }
        }
      }
    }
    // -------------------- LIST -------------------- //
    else if (words[0] == "list") {
      if (words.size() > 1) {
        if (words[1] == "darkmatter") {
          map<int, const darkmatter&> toon_dmatter = toons[TOON_ID]->get_dmatter();
          map<int, const darkmatter&>::iterator toon_dmatter_it = toon_dmatter.begin();
          while (toon_dmatter_it != toon_dmatter.end()) {
            toon_dmatter_it->second.print();
            toon_dmatter_it++;
          }
        }
      }
    }
    // -------------------- EQUIP -------------------- //
    else if (words[0] == "equip") {
      if (words.size() > 3) {
        if (words[1] == "darkmatter") {
          map<int, const darkmatter&> toon_dmatter = toons[TOON_ID]->get_dmatter();
          map<int, const darkmatter&>::iterator toon_dmatter_it = toon_dmatter.begin();
          while (toon_dmatter_it != toon_dmatter.end()) {
            if (strtolower(toon_dmatter_it->second.name()) == words[2]) {
              connection->transmit(transmission(SERVER_CODE).add(DELIM).add(EQUIP_DMATTER).add(DELIM).add(itos(toon_dmatter_it->second.id())).add(DELIM).add(strtolower(words[3])));
              /*int slot = 1;
              if (atoi(words[3].c_str()) < 0) slot = -1;
              toons[TOON_ID]->equip(toon_dmatter_it->first, 1, slot);*/
            }
            toon_dmatter_it++;
          }
        }
      }
    }
    // -------------------- GOTO BATTLE SERVER -------------------- //
    else if (words[0] == "battle") {
      cout << endl << "Transferring to battle server..." << endl;
      //cout << "Pausing keepsync..." << endl;
      //KEEP_SYNC = false;
      connection->transmit(transmission(SERVER_CODE).add(DELIM).add(GOTO_SERV).add(DELIM).add(BATTLE_SERV));
      //Sleep(1000);
      //cout << "Resuming keepsync..." << endl;
      //KEEP_SYNC = true;
    }
    // -------------------- GOTO STORE SERVER -------------------- //
    else if (words[0] == "store") {
      cout << endl << "Transferring to store server..." << endl;
      connection->transmit(transmission(SERVER_CODE).add(DELIM).add(GOTO_SERV).add(DELIM).add(STORE_SERV));
    }
    // -------------------- SPEAK -------------------- //
    else if (words[0] == "say") {
      transmission data(SERVER_CODE);
      data.add(DELIM).add(GLOBAL_MESSAGE).add(DELIM);
      // reconstruct message...
      for (int i=1;i<words.size();i++) {
        data.add(words[i]).add(" ");
      }
      connection->transmit(data);
    }
    // ------------------ QUIT --------------------- //
    else if (words[0] == "quit" || words[0] == "q" || words[0] == "exit") QUIT = true;
  }

  // --------------- DICONNECT/CLOSE --------------- //
  if (connection->is_linked()) {
    connection->transmit(transmission(SERVER_CODE).add(DELIM).add(DISCONNECT).add(DELIM).add("NULL"));
    connection->close();
  }

  delete connection;

  map<int, character*>::iterator it = toons.begin();
  while (it != toons.end()) {
    delete hp_animations[it->first];
    delete mp_animations[it->first];
    delete spell_animations[it->first];

    delete it->second;
    it++;
  }

  Sleep(2000);

  return 0;
}

void connection_loop(void* ptr) {
  string transmission_stream = (*(string*)ptr);
  delete (string*)ptr;

  static vector<darkmatter> available_dmatter;

  int i = 0;
  while (i<transmission_stream.length()) {
    string data = transmission_stream.substr(i, TRANSMISSION_SIZE); // get the entire transmission
    data = data.substr(0, data.find_first_of(0x04));                // truncate to the first EOT marker
    i += TRANSMISSION_SIZE;                                         // advance i used to find following transmissions

    if (data.length() == 0) continue;

    // handle pings:
    if (data == PING) {
      connection->transmit(transmission(SERVER_CODE).add(DELIM).add(PONG).add(DELIM).add("NULL"));
      continue;
    }

    vector<string> temp = explode(data, DELIM, 1);
    if (temp.size() < 2) {
      cout << "WARNING: Transmission parsing error. (" << data << ")" << endl;
      return;
    }
    string code = temp[0];
    data = temp[1];
    temp.clear();

    if (code == EXECUTE) {
      temp = explode(data, DELIM, 1);
      if (temp.size() < 2) {
        cout << "WARNING: Execute is lacking parameters: " << data << endl;
        return;
      }
      string action = temp[0];
      string params = temp[1];
      temp.clear();

      if (action == DELETE_SAVE_FILE) {
        fileio load_file;
        load_file.open("battle.sav");
        if (load_file.is_open()) load_file.rm();
      }
      else if (action == REMOVE_REMOTE_CHAR) {
        int toon_id = atoi(params.c_str());
        cout << toons[toon_id]->name() << " has disconnected." << endl;

        delete hp_animations[toon_id];
        hp_animations.erase(toon_id);

        delete mp_animations[toon_id];
        mp_animations.erase(toon_id);
        
        delete spell_animations[toon_id];
        spell_animations.erase(toon_id);

        delete toons[toon_id];
        toons.erase(toon_id);
      }
      else {
        cout << "Unable to execute server command." << endl;
      }
    }
    else if (code == SERVER_ERROR) {
      if (data == CHARACTER_TAKEN) {
        cout << "Error: Name taken." << endl;
        MASTER_ABORT = true;
        cout << endl;
      }
      else if (data == CHARACTER_NOT_FOUND) {
        cout << "Error: Character not found." << endl;
        MASTER_ABORT = true;
        cout << endl;
      }
      else if (data == CHARACTER_CORRUPTED) {
        cout << "Error: Unable to load character file." << endl;
        MASTER_ABORT = true;
        cout << endl;
      }
      else if (data == TIMEBAR_NOT_FULL) {
        cout << "Your timebar is not full yet." << endl;
      }
      else if (data == LOCAL_IS_DEAD) {
        cout << "You are dead." << endl;
      }
      else if (data == REMOTE_IS_DEAD) {
        cout << "Target is dead." << endl;
      }
      else if (data == NOT_ENOUGH_MANA) {
        cout << "Not enough mana." << endl;
      }
      else if (data == TOON_DOESNT_HAVE_SPELL) {
        cout << "You cannot cast that spell." << endl;
      }
      else cout << "Undefined Error: " << data << endl;
    }
    else if (code == TOON_DATA) {
      temp = explode(data, DELIM, 1);
      if (temp.size() < 2) {
        cout << "WARNING: Toon Data is lacking parameters: " << data << endl;
        return;
      }
      int toon_id = atoi(temp[0].c_str());
      if (TOON_ID < 0) TOON_ID = toon_id;
      string toon_data = temp[1];
      temp.clear();

      if (toons.find(toon_id) == toons.end()) {
        toons[toon_id] = new character;
        hp_animations[toon_id] = new starburst;
        mp_animations[toon_id] = new starburst;
        spell_animations[toon_id] = new projectile;
      }
      toons[toon_id]->decode(toon_data);
      toons[toon_id]->id(toon_id);
    }
    else if (code == CLEAR_TOON_DMATTER) {
      int toon_id = atoi(data.c_str());
      if (toons.find(toon_id) != toons.end()) toons[toon_id]->clear_dmatter();
    }
    else if (code == TOON_DMATTER) {
      temp = explode(data, DELIM, 1);
      if (temp.size() < 2) {
        cout << "WARNING: Darkmatter Data is lacking parameters: " << data << endl;
        return;
      }
      int toon_id = atoi(temp[0].c_str());
      string dmatter_data = temp[1];
      temp.clear();

      if (toons.find(toon_id) != toons.end()) {
        toons[toon_id]->add_dmatter(row().from_str(dmatter_data));
      }
    }
    else if (code == SERVER_SHUTDOWN) {
      cout << endl << "(The server is shutting down in " << data << " second(s)...)" << endl;
      cout << "Shutting down..." << endl;
      //Sleep(atoi(data.c_str())*1000);
      MASTER_ABORT = true;
    }
    else if (code == DEFN_SERVER_CODE) {
      SERVER_CODE = data;
    }
    else if (code == DEFN_ID) {
      CONNECTION_ID = atoi(data.c_str());
      connection->id(CONNECTION_ID);
    }
    else if (code == DAMAGE) {
      temp = explode(data, DELIM, 2);
      if (temp.size() < 3) {
        cout << "WARNING: Damage is lacking parameters: " << data << endl;
        return;
      }
      int attacker_id = atoi(temp[0].c_str());
      int attacked_id = atoi(temp[1].c_str());
      int damage = atoi(temp[2].c_str());
      temp.clear();

      cout << toons[attacker_id]->name() << " hit " << toons[attacked_id]->name() << " for " << damage << " damage." << endl;

      // damage animation:
      hp_animations[attacked_id]->length(40);
      hp_animations[attacked_id]->text(itos(damage));
      if (damage > 0)       hp_animations[attacked_id]->color(point3d(1.0f, 0.0f, 0.0f));
      else if (damage < 0)  hp_animations[attacked_id]->color(point3d(1.0f, 1.0f, 1.0f));
      else {
                            hp_animations[attacked_id]->text("Miss!");
                            hp_animations[attacked_id]->color(point3d(0.8f, 0.8f, 0.8f));
      }
      hp_animations[attacked_id]->color2(point3d(0.0f, 0.0f, 0.0f));
      hp_animations[attacked_id]->reset_animation();
    }
    else if (code == SPELL) {
      temp = explode(data, DELIM, 4);
      if (temp.size() < 5) {
        cout << "WARNING: Spell is lacking parameters: " << data << endl;
        return;
      }
      string spell_name = temp[0];
      int caster_id = atoi(temp[1].c_str());
      int receiving_id = atoi(temp[2].c_str());
      int hp_dmg = atoi(temp[3].c_str());
      int mp_dmg = atoi(temp[4].c_str());
      temp.clear();

      cout << toons[caster_id]->name() << " cast " << spell_name << " on " << toons[receiving_id]->name() << "." << endl;

      if (hp_dmg > 0) cout << spell_name << " healed " << toons[receiving_id]->name() << " for " << hp_dmg << "hp." << endl;
      else if (hp_dmg < 0) cout << spell_name << " hit " << toons[receiving_id]->name() << " for " << -1*hp_dmg << "hp." << endl;

      if (mp_dmg > 0) cout << spell_name << " healed " << toons[receiving_id]->name() << " for " << mp_dmg << "mp." << endl;
      else if (mp_dmg < 0) cout << spell_name << " hit " << toons[receiving_id]->name() << " for " << -1*mp_dmg << "mp." << endl;

      if (hp_dmg == 0 && mp_dmg == 0) cout << spell_name << " had no affect on " << toons[receiving_id]->name() << "." << endl;

      int spell_animation_length = 0;
      // damage animations:
      if ((hp_dmg != 0 || mp_dmg != 0) && (caster_id != receiving_id)) {
        spell_animation_length = 40;
        point3d animation_color(1.0f, 1.0f, 1.0f);
        if (hp_dmg+mp_dmg < 0) animation_color = point3d(1.0f, 0.0f, 0.0f);
        spell_animations[caster_id]->length(spell_animation_length);
        spell_animations[caster_id]->color(animation_color);
        spell_animations[caster_id]->color2(animation_color);
        spell_animations[caster_id]->pos(toons[caster_id]->gui_pos + point3d(30.0f, 15.0f, 1.0f));
        spell_animations[caster_id]->pos2(toons[receiving_id]->gui_pos + point3d(30.0f, 15.0f, 1.0f));
        spell_animations[caster_id]->reset_animation();
      }

      if (hp_dmg != 0) {
        hp_animations[receiving_id]->length(40);
        hp_animations[receiving_id]->text(itos(hp_dmg));
        if (hp_dmg > 0) hp_animations[receiving_id]->color(point3d(1.0f, 1.0f, 1.0f));
        else hp_animations[receiving_id]->color(point3d(1.0f, 0.0f, 0.0f));
        hp_animations[receiving_id]->color2(point3d(0.0f, 0.0f, 0.0f));
        hp_animations[receiving_id]->reset_animation();
        hp_animations[receiving_id]->delay(spell_animation_length);
      }
      if (mp_dmg != 0) {
        mp_animations[receiving_id]->length(40);
        mp_animations[receiving_id]->text(itos(mp_dmg));
        if (mp_dmg > 0) mp_animations[receiving_id]->color(point3d(1.0f, 1.0f, 1.0f));
        else mp_animations[receiving_id]->color(point3d(1.0f, 0.0f, 0.0f));
        mp_animations[receiving_id]->color2(point3d(0.0f, 0.0f, 0.0f));
        mp_animations[receiving_id]->reset_animation();
        mp_animations[receiving_id]->delay(spell_animation_length);
      }
    }
    else if (code == DEATH) {
      int toon_id = atoi(data.c_str());
      cout << toons[toon_id]->name() << " has died." << endl;
    }
    else if (code == GAINED_EXP) {
      cout << toons[TOON_ID]->name() << " gained " << data << " exp! (" << toons[TOON_ID]->exp() << "/" << toons[TOON_ID]->exp_max() << ")" << endl;
    }
    else if (code == LEVEL_UP) {
      int toon_id = atoi(data.c_str());
      cout << toons[toon_id]->name() << " grew to level " << toons[toon_id]->level() << "!" << endl;
    }
    else if (code == GLOBAL_MESSAGE) {
      temp = explode(data, DELIM, 1);
      if (temp.size() < 2) {
        cout << "WARNING: Global Message is lacking parameters: " << data << endl;
        return;
      }
      int toon_id = atoi(temp[0].c_str());
      string message;
      if (temp.size() > 1) message = temp[1];
      temp.clear();

      cout << toons[toon_id]->name() << ": " << message << endl;
    }
    else if (code == DMATTER_LIST) {
      available_dmatter.clear();
    }
    else if (code == AVAILABLE_DMATTER) {
      available_dmatter.push_back(darkmatter());
      available_dmatter.back().define(row().from_str(data));
    }
    else if (code == END_DMATTER_LIST) {
      cout << "Purchasable Dark Matter: " << endl;
      for (int d=0; d<available_dmatter.size(); d++) {
        cout << "LVL" << available_dmatter[d].level() << " " << available_dmatter[d].name() << " :: " << available_dmatter[d].exp() << endl;
      }
    }
  }
}
