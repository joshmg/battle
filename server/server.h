// File: server.h
// Written by Joshua Green

#ifndef SERVER_H
#define SERVER_H

#include "../shared/character.h"
#include "../shared/connection/connection.h"
#include "../shared/str/str.h"
#include <string>
#include <vector>
#include <map>
#include <ctime>

class server;
void recv_branch(void*);
void disc_branch(void*);

struct ping_data;

class server {
  private:
    static int SERVER_COUNT;
    int _id;
    std::map<std::string, void (*)(int, int, std::string&)> _actions;
    std::map<int, p2p*> _connections;
    std::map<int, character*> _characters;
    std::map<int, ping_data> _pings;
    bool _shutting_down;

    bool _battle_server;

  public:
    server();
    ~server();
    int size() const;
    void shutdown(int);
    bool is_shutting_down();
    int id();
    void add_action(const std::string&, void (*)(int, int, std::string&));
    void add_connection(p2p*);                                                  // Adds the connection to the server's connection list.
                                                                                // Transmits the new server code to the connection.
                                                                                // Transmits the new id to the conneciton.
                                                                                // Creates a new ping within the server's ping list.

    void remove_connection(int, bool close_connection=true);                    // Removes the connection from the server's connection list,
                                                                                //   if close_connection is true (default) the connection is
                                                                                //   closed.
                                                                                // If the server is a battle server, a message is transmitted
                                                                                //   to the other connections, reporting the connection removal.
                                                                                // Removes the connection's ping from the server's ping list.
    bool has_connection_id(int);
    p2p* get_connection(int);
    void recv(int, std::string&, std::string&);
    void add_character(int, character*);                                        // Adds the character to the server's character list.
                                                                                //   If the character's ID existed previously, the old pointer is deleted.
                                                                                // Loads the character's darkmatter from the database (invokes character
                                                                                //   class's member function: load_dmatter())

    void remove_character(int, bool deallocate_mem=true);                       // Removes the character from the server's character list, 
                                                                                //   if deallocate_mem is true (default) the character (and
                                                                                //   its dark matter) is erased from memory.

    character* get_character(int);
    std::map<int, character*> get_char_map();

    std::multimap<int, darkmatter*> available_dmatter; // <database class id, darkmatter*>

    void operator++(int);

    void ping(int);
    time_t pong(int);

    void enable_battle(bool);
};

struct ping_data {
  time_t send_t;
  time_t recv_t;

  ping_data() {
    send_t = 0;
    recv_t = 0;
  }
};

#endif
