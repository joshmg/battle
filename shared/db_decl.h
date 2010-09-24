// File: db_decl.h
// Written by Joshua Green

#ifndef DB_DECL_H
#define DB_DECL_H

#include "db/column.h"

#ifndef defn_once
  #define prefix extern
#else
  #define prefix
#endif

#ifdef SERVER
#include "db/db.h"
prefix database *db;     // database pointer
prefix table* class_table;
prefix table* char_table;
prefix table* spell_table;
prefix table* dmatter_class_table;
prefix table* dmatter_table;
#endif

// character class table
prefix column class_id, class_name, class_level, class_hp, class_mp, class_dmg, class_speed, class_tmax, class_str, class_dex, class_int, class_spi, class_fort;
// character table
prefix column char_id, char_name, char_class, char_level, char_exp, char_hp, char_mp, char_dead, char_online, char_time;
// spell table
prefix column spell_id, spell_name, spell_level, spell_element, spell_mp, spell_hp_mul, spell_hp_max_mul, spell_hp_add, spell_mp_mul, spell_mp_max_mul, spell_mp_add, spell_all, spell_on_dead;
// dark matter class table
prefix column dmclass_id, dmclass_name, dmclass_level, dmclass_exp, dmclass_spells, dmclass_hp_add, dmclass_mp_add, dmclass_hp_mul, dmclass_mp_mul, dmclass_str_add, dmclass_dex_add, dmclass_int_add, dmclass_spi_add, dmclass_fort_add, dmclass_str_mul, dmclass_dex_mul, dmclass_int_mul, dmclass_spi_mul, dmclass_fort_mul;
// dark matter table
prefix column dmatter_id, dmatter_owner_id, dmatter_linked_to, dmatter_equipped, dmatter_wslot, dmatter_aslot, dmatter_exp, dmatter_level, dmatter_name, dmatter_spells, dmatter_hp_add, dmatter_mp_add, dmatter_hp_mul, dmatter_mp_mul, dmatter_str_add, dmatter_dex_add, dmatter_int_add, dmatter_spi_add, dmatter_fort_add, dmatter_str_mul, dmatter_dex_mul, dmatter_int_mul, dmatter_spi_mul, dmatter_fort_mul;

  #ifdef defn_once
    void load_columns() {
      char_id      = column("char_id",      12).auto_inc();
      char_name    = column("char_name",    25);
      char_class   = column("char_class",   25).set_default("Swordsman");
      char_level   = column("char_level",   25).set_default("1");
      char_exp     = column("char_exp",     12).set_default("0");
      char_hp      = column("char_hp",      12).set_default("100");
      char_mp      = column("char_mp",      12).set_default("10");
      char_dead    = column("char_dead",     5).set_default("false");
      char_online  = column("char_online",   5).set_default("false");
      char_time    = column("char_time",    12).set_default("0");
    
      class_id     = column("class_id",       12).auto_inc();
      class_name   = column("class_name",     25);
      class_level  = column("class_level",    25).set_default("1");
      class_hp     = column("class_hp",       12).set_default("100");
      class_mp     = column("class_mp",       12).set_default("10");
      class_dmg    = column("class_dmg",      12).set_default("10");
      class_speed  = column("class_speed",    12).set_default("10");
      class_tmax   = column("class_time_max", 12).set_default("100");
      class_str    = column("class_str",      12).set_default("1");
      class_dex    = column("class_dex",      12).set_default("1");
      class_int    = column("class_int",      12).set_default("1");
      class_spi    = column("class_spi",      12).set_default("1");
      class_fort   = column("class_fort",     12).set_default("1");
      
      spell_id          = column("spell_id",         12).auto_inc();
      spell_name        = column("spell_name",       25);
      spell_level       = column("spell_level",      25).set_default("1");
      spell_element     = column("spell_element",    25);
      spell_mp          = column("spell_mp",         12).set_default("0");
      spell_hp_add      = column("spell_hp_add",     15).set_default("0.0");
      spell_hp_mul      = column("spell_hp_mul",     15).set_default("0.0");
      spell_hp_max_mul  = column("spell_hp_max_mul", 15).set_default("0.0");
      spell_mp_add      = column("spell_mp_add",     15).set_default("0.0");
      spell_mp_mul      = column("spell_mp_mul",     15).set_default("0.0");
      spell_mp_max_mul  = column("spell_mp_max_mul", 15).set_default("0.0");
      spell_all         = column("spell_all",         5).set_default("false");
      spell_on_dead     = column("spell_on_dead",     5).set_default("false");
    
      dmclass_id        = column("dmatter_id",       12).auto_inc();
      dmclass_name      = column("dmatter_name",     25).set_default("");
      dmclass_level     = column("dmatter_level",    12).set_default("1");
      dmclass_exp       = column("dmatter_exp",      12).set_default("0");
      dmclass_spells    = column("dmatter_spells",   75);
      dmclass_hp_add    = column("dmatter_hp_add",   12).set_default("0");
      dmclass_mp_add    = column("dmatter_mp_add",   12).set_default("0");
      dmclass_hp_mul    = column("dmatter_hp_mul",   15).set_default("0.0");
      dmclass_mp_mul    = column("dmatter_mp_mul",   15).set_default("0.0");
      dmclass_str_add   = column("dmatter_str_add",  12).set_default("0");
      dmclass_dex_add   = column("dmatter_dex_add",  12).set_default("0");
      dmclass_int_add   = column("dmatter_int_add",  12).set_default("0");
      dmclass_spi_add   = column("dmatter_spi_add",  12).set_default("0");
      dmclass_fort_add  = column("dmatter_fort_add", 12).set_default("0");
      dmclass_str_mul   = column("dmatter_str_mul",  15).set_default("0.0");
      dmclass_dex_mul   = column("dmatter_dex_mul",  15).set_default("0.0");
      dmclass_int_mul   = column("dmatter_int_mul",  15).set_default("0.0");
      dmclass_spi_mul   = column("dmatter_spi_mul",  15).set_default("0.0");
      dmclass_fort_mul  = column("dmatter_fort_mul", 15).set_default("0.0");
      
      dmatter_id        = column("dmatter_id",        12).auto_inc();
      dmatter_owner_id  = column("dmatter_owner_id",  12).set_default("-1");
      dmatter_linked_to = column("dmatter_linked_to", 12).set_default("-1");
      dmatter_equipped  = column("dmatter_equipped",  12).set_default("false");
      dmatter_wslot     = column("weapon_slot",       12).set_default("-1");
      dmatter_aslot     = column("armor_slot",        12).set_default("-1");
      dmatter_exp       = column("dmatter_exp",       12).set_default("0");
      dmatter_level     = column("dmatter_level",     12).set_default("1");
      dmatter_name      = column("dmatter_name",      25);
      dmatter_spells    = column("dmatter_spells",    75);
      dmatter_hp_add    = column("dmatter_hp_add",    12).set_default("0");
      dmatter_mp_add    = column("dmatter_mp_add",    12).set_default("0");
      dmatter_hp_mul    = column("dmatter_hp_mul",    15).set_default("0.0");
      dmatter_mp_mul    = column("dmatter_mp_mul",    15).set_default("0.0");
      dmatter_str_add   = column("dmatter_str_add",   12).set_default("0");
      dmatter_dex_add   = column("dmatter_dex_add",   12).set_default("0");
      dmatter_int_add   = column("dmatter_int_add",   12).set_default("0");
      dmatter_spi_add   = column("dmatter_spi_add",   12).set_default("0");
      dmatter_fort_add  = column("dmatter_fort_add",  12).set_default("0");
      dmatter_str_mul   = column("dmatter_str_mul",   15).set_default("0.0");
      dmatter_dex_mul   = column("dmatter_dex_mul",   15).set_default("0.0");
      dmatter_int_mul   = column("dmatter_int_mul",   15).set_default("0.0");
      dmatter_spi_mul   = column("dmatter_spi_mul",   15).set_default("0.0");
      dmatter_fort_mul  = column("dmatter_fort_mul",  15).set_default("0.0");
    }
  #endif

#endif