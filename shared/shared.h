// File: shared.h
// Written by Joshua Green

#ifndef SHARED_H
#define SHARED_H

#include <string>

#undef ERROR    // Where is this even defined?

// -------------- CONSTANTS -------------- //
const int SERVER_PORT = 4444;
const std::string SERVER_IP = "192.168.1.201";

// Transmission Variables
// Errors
const std::string SERVER_ERROR = "!ERROR";
const std::string CHARACTER_TAKEN = "!CHARACTER_TAKEN";
const std::string CHARACTER_NOT_FOUND = "!CHARACTER_NOT_FOUND";
const std::string CHARACTER_CORRUPTED = "!CHARACTER_CORRUPTED";
const std::string INVALID_REQUEST = "!INVALID_REQUEST";
const std::string TIMEBAR_NOT_FULL = "!TIMEBAR_NOT_FULL";
const std::string LOCAL_IS_DEAD = "!LOCAL_TOON_IS_DEAD";
const std::string REMOTE_IS_DEAD = "!REMOTE_TOON_IS_DEAD";
const std::string SPELL_DOES_NOT_EXIST = "!NOSPELL";
const std::string TOON_DOESNT_HAVE_SPELL = "!TOON_MISSING_DMATTER";
const std::string DMATTER_DOES_NOT_EXIST = "!DMATTER_NOT_EXIST";
const std::string DMATTER_NOT_FOR_SALE = "!DMATTER_NOT_FOR_SALE";
const std::string NOT_ENOUGH_EXP = "!LACKING_EXP";
// Data
const std::string REQUEST_TOON_DATA = "(CHAR DATA?)";
const std::string TOON_DATA = "(CHAR DATA)";
const std::string NEW_TOON = "(NEW CHARACTER)";
const std::string LOAD_TOON = "(LOAD CHARACTER)";
const std::string CLEAR_TOON_DMATTER = "(CLEAR TOON DMATTER)";
const std::string REQUEST_TOON_DMATTER = "(TOON DMATTER?)";
const std::string TOON_DMATTER = "(TOON DMATTER)";
// Misc
const std::string DELIM = std::string(1, (char)128);
const std::string DEFN_SERVER_CODE = "(SERVERCODE)";
const std::string DEFN_ID = "(ID::)";
const std::string SERVER_SHUTDOWN = "(SERVER SHUTDOWN)";
const std::string REMOVE_REMOTE_CHAR = "(REMOVE REMOTE)";
const std::string EXECUTE = "!DO";
const std::string DELETE_SAVE_FILE = "!DELETE_SAVE";
const std::string PING = "(PING)";
const std::string PONG = "(PONG)";
const std::string DISCONNECT = "(DISCONNECT)";
const std::string GLOBAL_MESSAGE = "(GLOBAL_MESSAGE)";
// Battle
const std::string LOCAL_TOON = "(LOCAL_TOON)";
const std::string REMOTE_TOON = "(REMOTE_TOON)";
const std::string MELEE = "(MELEE)";
const std::string DAMAGE = "(DAMAGE)";
const std::string HEAL = "(HEAL)";
const std::string TARGET_HP = "(->HP)";
const std::string TARGET_MP = "(->MP)";
const std::string DEATH = "(DEATH)";
const std::string GAINED_EXP = "(+EXP)";
const std::string LEVEL_UP = "(+LVL)";
const std::string NOT_ENOUGH_MANA = "(~MANA)";
const std::string SPELL = "(SPELL)";
const std::string NO_AFFECT = "(NO_AFFECT)";
// Shop
const std::string SHOP = "(SHOP)";
const std::string BROWSE_DMATTER = "(BROWSE?DMATTER)";
const std::string DMATTER_LIST = "(LIST:DMATTER)";
const std::string END_DMATTER_LIST = "(_LIST:DMATTER)";
const std::string AVAILABLE_DMATTER = "(DMATTER4SALE:)";
const std::string BUY_DMATTER = "(BUY:DMATTER)";
// CTRL
const std::string EQUIP_DMATTER = "(EQUIP:DMATTER)";
const std::string GOTO_SERV = "(GOTO:)";
const std::string BATTLE_SERV = "(BATTLE_SERVER)";
const std::string STORE_SERV = "(STORE_SERVER)";
// --------------------------------------- //

struct point3d {
  float x, y, z;

  point3d() {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
  }
  point3d(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
  }
  point3d operator+(const point3d& p) {
    return point3d(p.x+x, p.y+y, p.z+z);
  }
  point3d operator-(const point3d& p) {
    return point3d(p.x-x, p.y-y, p.z-z);
  }
  point3d operator/(float c) {
    return point3d(x/c, y/c, z/c);
  }
};

#endif
