// File: server.cpp
// Written by Joshua Green

#include "server.h"
#include "../shared/shared.h"
#include "../shared/darkmatter.h"
#include "../shared/db/db.h"
#include "../shared/db_decl.h"
#include "../shared/connection/transmission.h"
#include "../shared/str/str.h"
#include <string>
#include <vector>
#include <ctime>
#include <windows.h>
#include <process.h>
#include <iostream>
using namespace std;

extern vector<server*> hosts;
std::map<int, server*> all_servers;
int server::SERVER_COUNT;
bool STOP_TIME_LOOP = false;
//extern map<string, darkmatter*> all_dmatter_class;

extern void do_nothing(void*);
void time_loop(void*);

server::server() {
  _id = SERVER_COUNT;
  all_servers[_id] = this;
  SERVER_COUNT++;
  _shutting_down = false;

  _battle_server = true;
}

server::~server() {
  shutdown(1);
}

int server::size() const {
  return _connections.size();
}

void server::shutdown(int WAIT_TIME) {
  cout << "Server (ID " << _id << ") shutting down in " << WAIT_TIME << " seconds..." << endl;
  _shutting_down = true;
  map<int, p2p*>::iterator connection_it = _connections.begin();
  while (connection_it != _connections.end()) {
    connection_it->second->transmit(transmission().add(SERVER_SHUTDOWN).add(DELIM).add(itos(WAIT_TIME)));
    connection_it++;
  }

  Sleep(WAIT_TIME*1000);

  while (connection_it != _connections.end()) {
    connection_it->second->set_disc_func(&do_nothing);
    remove_connection(connection_it->first);
    connection_it = _connections.begin();
  }
  _connections.clear();

  map<int, character*>::iterator toon_it = _characters.begin();
  while (toon_it != _characters.end()) {
    remove_character(toon_it->first);
    toon_it = _characters.begin();
  }
}

bool server::is_shutting_down() {return _shutting_down;}

int server::id() {
  return _id;
}

void server::add_action(const string& name, void (*function)(int, int, string&)) {
  _actions[name] = function;
}

void server::add_connection(p2p* connection) {
  connection->transmit(transmission().add(DEFN_SERVER_CODE).add(DELIM).add(itos(_id)).add(DELIM).add(itos(connection->id())));
  connection->transmit(transmission().add(DEFN_ID).add(DELIM).add(itos(connection->id())));
  _connections[connection->id()] = connection;
  _pings[connection->id()];
}

void server::remove_connection(int connection_id, bool close_connection) {
  if (_connections.find(connection_id) != _connections.end()) {
    if (close_connection) {
      _connections[connection_id]->close();
      delete _connections[connection_id];
    }
    _connections.erase(connection_id);
  
    if (_pings.find(connection_id) != _pings.end()) {
      _pings.erase(connection_id);
    }
    
    if (_battle_server) {
      map<int, p2p*>::iterator it = _connections.begin();
      while (it != _connections.end()) {
        it->second->transmit(transmission().add(EXECUTE).add(DELIM).add(REMOVE_REMOTE_CHAR).add(DELIM).add(itos(connection_id)));
        it++;
      }
    }
  }
}

bool server::has_connection_id(int connection_id) {
  return (_connections.find(connection_id) != _connections.end());
}

void server::recv(int connection_id, string& code, string& data) {
  if (code == PONG) {
    if (_connections.find(connection_id) != _connections.end()) {
      _pings[connection_id].recv_t = time(NULL);
    }
    return;
  }

  if (_actions.find(code) == _actions.end()) {
    cout << "Invalid action code: " << code << ". Connection ID: " << connection_id << endl;
    return;
  }
  else (_actions[code])(_id, connection_id, data);
}

p2p* server::get_connection(int connection_id) {
  if (_connections.find(connection_id) == _connections.end()) return 0;
  return _connections[connection_id];
}

void server::add_character(int connection_id, character* toon) {
  if (_characters.find(connection_id) != _characters.end()) delete _characters[connection_id];
  toon->id(connection_id);
  _characters[connection_id] = toon;

  // add all of user's dark matter to memory:
  _characters[connection_id]->load_dmatter();
}

character* server::get_character(int connection_id) {
  if (_characters.find(connection_id) == _characters.end()) return 0;
  return _characters[connection_id];
}

void server::remove_character(int connection_id, bool deallocate_mem) {
  if (_characters.find(connection_id) != _characters.end()) {
    character* toon = _characters[connection_id];

    // update character's dmatter and delete from memory:
    map<int, const darkmatter&> toon_dmatter = toon->get_dmatter();
    map<int, const darkmatter&>::const_iterator dmatter_it = toon_dmatter.begin();
    while (dmatter_it != toon_dmatter.end()) {
    row new_dmatter_data = dmatter_it->second.to_row();
    new_dmatter_data.remove(dmatter_id); // to avoid changing the unique ID...
      if (dmatter_table->edit_row(dmatter_table->select(query().where(predicate().And(equalto(dmatter_id, itos(dmatter_it->second.id()))))), new_dmatter_data) == 0) {
        cout << "WARNING: Error updating Darkmatter data upon disconnect." << endl;
      }
      dmatter_it++;
    }
    if (deallocate_mem) toon->clear_dmatter(); // delete dmatter from memory

    // update character's db entry:
    row char_data = _characters[connection_id]->to_row();
    char_data.remove(char_id);
    if (char_table->edit_row(char_table->select(query().where(predicate().And(equalto(char_name, _characters[connection_id]->name())))), char_data) == 0) {
      cout << "WARNING: Error updating character data upon disconnect." << endl;
    }
    if (deallocate_mem) delete toon;
    _characters.erase(connection_id);
  }
}

map<int, character*> server::get_char_map() {
  return _characters;
}

void server::ping(int connection_id) {
  _pings[connection_id].send_t = time(NULL);
  _pings[connection_id].recv_t = 0;
  if (_connections.find(connection_id) != _connections.end()) {
    _connections[connection_id]->transmit(transmission().add(PING));
  }
}

time_t server::pong(int connection_id) {
  if (_connections.find(connection_id) != _connections.end()) {
    return _pings[connection_id].recv_t;
  }
  else return 0;
}

void recv_branch(void* ptr) {
  std::string transmission_stream = *(std::string*)ptr;
  delete (std::string*)ptr;

  int i = 0;
  while (i<transmission_stream.length()) {
    string single_transmission = transmission_stream.substr(i, TRANSMISSION_SIZE);                  // get the entire transmission
    single_transmission = single_transmission.substr(0, single_transmission.find_first_of(0x04));   // truncate to the first EOT marker
    i += TRANSMISSION_SIZE;                                                                         // advance i used to find following transmissions

    vector<std::string> temp = explode(single_transmission, DELIM, 3);
    if (temp.size() < 4) temp.push_back(string());
    int server_id = atoi(temp[0].c_str());
    int connection_id = atoi(temp[1].c_str());
    string code = temp[2];
    string data = temp[3];
    temp.clear();
    
    if (all_servers.find(server_id) == all_servers.end()) {
      std::cout << "WARNING: Invalid Server ID: " << server_id << endl;
      return;
    }

    all_servers[server_id]->recv(connection_id, code, data);
  }
}

void disc_branch(void* ptr) {
  int connection_id = *(int*)ptr;
  delete (int*)ptr;

  cout << "Connection " << connection_id << " disconnected." << endl;
  for (int i=0;i<hosts.size();i++) {
    if (hosts[i]->has_connection_id(connection_id)) {
      hosts[i]->remove_character(connection_id);
      hosts[i]->remove_connection(connection_id);
      break;
    }
  }
}

void time_loop(void*) {
  int desired_wait = 1000; // 1 second
  int wait_time = 1000;
  bool wait = true;
  int lag = 0;
  int total_lag = 0;
  clock_t last_t = 0;
  while (!STOP_TIME_LOOP) {
    if (last_t != 0) {
      lag = (clock() - last_t) - desired_wait;
      total_lag += lag;
      wait_time -= (lag + total_lag*0.10f);
      if (wait_time < 15) {
        wait_time = 15;
        wait = false;
      }
      else wait = true;
    }
    last_t = clock();

    for (int host_id=0;host_id<hosts.size();host_id++) {
      (*hosts[host_id])++;
    }

    if (wait) Sleep(wait_time);
    else cout << "WARNING: Tick timer is lacking available resources! (Lagging: " << lag << ", Total: " << total_lag <<")" << endl;
    //cout << "(Lagging: " << lag << ", Total: " << total_lag <<")" << endl;
  }
}

void server::enable_battle(bool value) {
  _battle_server = value;
}

void server::operator++(int) {
  map<int, character*>::iterator char_it = _characters.begin();

  if (_battle_server) {
    while (char_it != _characters.end()) {
      (*char_it->second)++;
      char_it++;
    }
  }
  else {
    static int tick_count = 0;
    if (tick_count == 0) {
      cout << "Adding darkmatter (Host " << _id << ")" << endl;
      srand(time(NULL));
      vector<row> results = dmatter_class_table->select(query().where(predicate().And(equalto(dmclass_level, "1"))));
      if (results.size() != 0) {
        row dmatter_row = results[rand()%results.size()];
        darkmatter* temp_dmatter = new darkmatter;
        temp_dmatter->define(dmatter_row);
        available_dmatter.insert(pair<int, darkmatter*>(temp_dmatter->id(), temp_dmatter));
      }
      else {
        cout << "WARNING: Server++ found zero darkmatter objects." << endl;
      }
    }

    while (char_it != _characters.end()) {
      char_it->second->full_heal();
      char_it++;
    }

    tick_count = (++tick_count)%60;
  }
}