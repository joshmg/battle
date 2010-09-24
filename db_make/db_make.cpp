// File: db_make.cpp
// Written by Joshua Green

#include "../shared/db/db.h"
#include <string>
#include <iostream>
using namespace std;

#define SERVER

#define defn_once
#include "../shared/db_decl.h"

int main() {
  database* db = new database;
  db->initialize("db");
  load_columns();


  // ---------------- CHARACTER TABLE ---------------- //
  string char_table_name = "_char_table";

  cout << "Creating char table..." << endl;
  char_table = db->add_table(char_table_name);
  char_table->add_column( char_id );
  char_table->add_column( char_name );
  char_table->add_column( char_class );
  char_table->add_column( char_level );
  char_table->add_column( char_exp );
  char_table->add_column( char_hp );
  char_table->add_column( char_mp );
  char_table->add_column( char_dead );
  char_table->add_column( char_online );
  char_table->add_column( char_time );
  char_table->set_key(char_name);
  char_table->optimize();

  // ---------------- CHARACTER CLASS TABLE ---------------- //
  string class_table_name = "_class_table";

  cout << "Creating class table..." << endl;
  class_table = db->add_table(class_table_name);
  class_table->add_column( class_id );
  class_table->add_column( class_name );
  class_table->add_column( class_level );
  class_table->add_column( class_hp );
  class_table->add_column( class_mp );
  class_table->add_column( class_dmg );
  class_table->add_column( class_speed );
  class_table->add_column( class_tmax );
  class_table->add_column( class_str );
  class_table->add_column( class_dex );
  class_table->add_column( class_int );
  class_table->add_column( class_spi );
  class_table->add_column( class_fort );
  class_table->set_key(class_name);

  cout << "Inserting class data..." << endl;
  string name = "Swordsman";
  float hp = 100.0f, mp = 10.0f;
  float dmg = 10.0f;
  float speed = 10.0f, tmax = 100.0f;
  for (int level=1;level<100;level++) {
    if (level > 1) {
      hp *= 1.10f;
      mp = 1.10f*(mp + 1);
      if (speed < 50.0f) speed += 1.0f;
      dmg = dmg*1.05f + level;
    }
    class_table->add_row(row().add(class_name, name).add(class_level, itos(level)).add(class_hp, itos((int)hp)).add(class_mp, itos((int)mp)).add(class_dmg, itos((int)dmg)).add(class_speed, itos((int)speed)).add(class_tmax, itos((int)tmax)));
  }
  class_table->optimize();


  // ---------------- SPELL TABLE ---------------- //
  string spell_table_name = "_spell_table";

  cout << "Creating spell table..." << endl;
  spell_table       = db->add_table(spell_table_name);
  spell_table->add_column( spell_id );
  spell_table->add_column( spell_name );
  spell_table->add_column( spell_level );
  spell_table->add_column( spell_element );
  spell_table->add_column( spell_mp );
  spell_table->add_column( spell_hp_add );
  spell_table->add_column( spell_hp_mul );
  spell_table->add_column( spell_hp_max_mul );
  spell_table->add_column( spell_mp_add );
  spell_table->add_column( spell_mp_mul );
  spell_table->add_column( spell_mp_max_mul );
  spell_table->add_column( spell_all );
  spell_table->add_column( spell_on_dead );
  spell_table->set_key(spell_name);

  cout << "Inserting spell data..." << endl;
  // fire:
  //        mp: base 3
  //       dmg: base 6
  spell_table->add_row(row().add(spell_name, "Fire")  .add(spell_level, "1").add(spell_element, "fire") .add(spell_mp, "9")  .add(spell_hp_add, "-36"));
  spell_table->add_row(row().add(spell_name, "Fira")  .add(spell_level, "2").add(spell_element, "fire") .add(spell_mp, "27") .add(spell_hp_add, "-216"));
  spell_table->add_row(row().add(spell_name, "Firaga").add(spell_level, "3").add(spell_element, "fire") .add(spell_mp, "81") .add(spell_hp_add, "-1296"));
  spell_table->add_row(row().add(spell_name, "Firara").add(spell_level, "4").add(spell_element, "fire") .add(spell_mp, "243").add(spell_hp_add, "-7776"));
  spell_table->add_row(row().add(spell_name, "Cure")  .add(spell_level, "1").add(spell_element, "holy") .add(spell_mp, "4")  .add(spell_hp_add, "25"));
  spell_table->add_row(row().add(spell_name, "Cura")  .add(spell_level, "2").add(spell_element, "holy") .add(spell_mp, "8")  .add(spell_hp_add, "125"));
  spell_table->add_row(row().add(spell_name, "Curaga").add(spell_level, "3").add(spell_element, "holy") .add(spell_mp, "16") .add(spell_hp_add, "625"));
  spell_table->add_row(row().add(spell_name, "Curara").add(spell_level, "4").add(spell_element, "holy") .add(spell_mp, "32") .add(spell_hp_add, "3125"));
  spell_table->add_row(row().add(spell_name, "Autophagy") .add(spell_level, "1").add(spell_element, "none").add(spell_mp, "0").add(spell_hp_max_mul, "-0.1").add(spell_mp_max_mul, "0.2"));
  spell_table->add_row(row().add(spell_name, "Autophagia").add(spell_level, "2").add(spell_element, "none").add(spell_mp, "0").add(spell_hp_max_mul, "-0.3").add(spell_mp_max_mul, "0.5"));
  spell_table->add_row(row().add(spell_name, "Ether") .add(spell_level, "1").add(spell_element, "none").add(spell_mp, "0").add(spell_mp_add, "20"));
  spell_table->optimize();



  // ---------------- DMATTER CLASS TABLE ---------------- //
  string dmclass_table_name = "_dmatter_class_table";

  cout << "Creating dark matter class table..." << endl;
  dmatter_class_table  = db->add_table(dmclass_table_name);
  dmatter_class_table->add_column( dmclass_id );
  dmatter_class_table->add_column( dmclass_name );
  dmatter_class_table->add_column( dmclass_level );
  dmatter_class_table->add_column( dmclass_exp );
  dmatter_class_table->add_column( dmclass_spells );
  dmatter_class_table->add_column( dmclass_hp_add );
  dmatter_class_table->add_column( dmclass_mp_add );
  dmatter_class_table->add_column( dmclass_hp_mul );
  dmatter_class_table->add_column( dmclass_mp_mul );
  dmatter_class_table->add_column( dmclass_str_add );
  dmatter_class_table->add_column( dmclass_dex_add );
  dmatter_class_table->add_column( dmclass_int_add );
  dmatter_class_table->add_column( dmclass_spi_add );
  dmatter_class_table->add_column( dmclass_fort_add );
  dmatter_class_table->add_column( dmclass_str_mul );
  dmatter_class_table->add_column( dmclass_dex_mul );
  dmatter_class_table->add_column( dmclass_int_mul );
  dmatter_class_table->add_column( dmclass_spi_mul );
  dmatter_class_table->add_column( dmclass_fort_mul );
  dmatter_class_table->set_key(dmclass_name);

  cout << "Inserting dark matter class data..." << endl;
  // exp = 250*(5*x^2 - 11*x + 6)
  dmatter_class_table->add_row(row().add(dmclass_name, "Firestone").add(dmclass_level, "1").add(dmclass_exp, "0")    .add(dmclass_spells, "Fire")  .add(dmclass_mp_add, "18") .add(dmclass_str_add, "-2"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Firestone").add(dmclass_level, "2").add(dmclass_exp, "1000") .add(dmclass_spells, "Fira")  .add(dmclass_mp_add, "54") .add(dmclass_spi_add, "-2"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Firestone").add(dmclass_level, "3").add(dmclass_exp, "4500") .add(dmclass_spells, "Firaga").add(dmclass_mp_add, "162").add(dmclass_spi_add, "-4"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Firestone").add(dmclass_level, "4").add(dmclass_exp, "10500").add(dmclass_spells, "Firara").add(dmclass_mp_add, "486").add(dmclass_int_add,  "6"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Lightstone").add(dmclass_level, "1").add(dmclass_exp, "0")   .add(dmclass_spells, "Cure")  .add(dmclass_mp_add, "8") .add(dmclass_str_add, "-2"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Lightstone").add(dmclass_level, "2").add(dmclass_exp, "500") .add(dmclass_spells, "Cura")  .add(dmclass_mp_add, "16").add(dmclass_spi_add,  "4"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Lightstone").add(dmclass_level, "3").add(dmclass_exp, "2250").add(dmclass_spells, "Curaga").add(dmclass_mp_add, "32").add(dmclass_int_add,  "4"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Lightstone").add(dmclass_level, "4").add(dmclass_exp, "5250").add(dmclass_spells, "Curara").add(dmclass_mp_add, "64").add(dmclass_spi_add,  "6"));
  dmatter_class_table->add_row(row().add(dmclass_name, "Amberstone").add(dmclass_level, "1").add(dmclass_exp, "0")   .add(dmclass_spells, "Autophagy") .add(dmclass_hp_add, "20") );
  dmatter_class_table->add_row(row().add(dmclass_name, "Amberstone").add(dmclass_level, "2").add(dmclass_exp, "5500").add(dmclass_spells, "Autophagia").add(dmclass_hp_add, "800"));
  dmatter_class_table->optimize();



  // ---------------- DMATTER TABLE ---------------- //
  string dmatter_table_name = "_dmatter_table";

  cout << "Creating dark matter table..." << endl;
  dmatter_table  = db->add_table(dmatter_table_name);
  dmatter_table->add_column( dmatter_id );
  dmatter_table->add_column( dmatter_owner_id );
  dmatter_table->add_column( dmatter_linked_to );
  dmatter_table->add_column( dmatter_equipped );
  dmatter_table->add_column( dmatter_wslot );
  dmatter_table->add_column( dmatter_aslot );
  dmatter_table->add_column( dmatter_exp );
  dmatter_table->add_column( dmatter_level );
  dmatter_table->add_column( dmatter_name );
  dmatter_table->add_column( dmatter_spells );
  dmatter_table->add_column( dmatter_hp_add );
  dmatter_table->add_column( dmatter_mp_add );
  dmatter_table->add_column( dmatter_hp_mul );
  dmatter_table->add_column( dmatter_mp_mul );
  dmatter_table->add_column( dmatter_str_add );
  dmatter_table->add_column( dmatter_dex_add );
  dmatter_table->add_column( dmatter_int_add );
  dmatter_table->add_column( dmatter_spi_add );
  dmatter_table->add_column( dmatter_fort_add );
  dmatter_table->add_column( dmatter_str_mul );
  dmatter_table->add_column( dmatter_dex_mul );
  dmatter_table->add_column( dmatter_int_mul );
  dmatter_table->add_column( dmatter_spi_mul );
  dmatter_table->add_column( dmatter_fort_mul );
  dmatter_table->set_key(dmatter_owner_id);
  dmatter_table->optimize();


  db->close();
  delete db;
  return 0;
}
