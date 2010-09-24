// File: gui.cpp
// Written by Joshua Green

#include "text3d.h"
#include "starburst.h"
#include "projectile.h"
#include "../shared/character.h"
#include "../shared/str/str.h"
#include "../shared/shared.h"
#include <GL/glu.h>
#include <GL/glut.h>
#include <map>
#include <cmath>
#include <iostream>
using namespace std;

extern int _argc;
extern char** _argv;
extern int CONNECTION_ID, TOON_ID;
extern bool QUIT;
extern map<int, character*>  toons;
extern map<int, starburst*>  hp_animations;
extern map<int, starburst*>  mp_animations;
extern map<int, projectile*> spell_animations;

void initRendering() {         // 3D Rendering
  glEnable(GL_DEPTH_TEST);     // Depth Testing
  glEnable(GL_COLOR_MATERIAL); // Colors
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  t3dInit();
}

void handleKeypress(unsigned char key, int x, int y) {
}

void handleResize(int w, int h) {
  if (w > 0 && h > 0) {
    glViewport(0, 0, w,h);       // Convert from coordinates to pixels values
    glMatrixMode(GL_PROJECTION); // Set Camera Mode
    glLoadIdentity();            // Reset the camera
    gluPerspective(90,           // The camera angle
             (float)w/(float)h,  // pixel ratio
             5.0f,               // Near-Z-clipping
             1000.0f);            // Far-Z-clipping
  }
}

void drawbar(const point3d& pos, const point3d& size, const point3d& color, const point3d& fill_color, float fill_percent, const string& bar_tag, const string& bar_text) {
  // wireframe:
  glBegin(GL_LINE_STRIP);
  glColor3f(color.x, color.y, color.z);
  glVertex3f(pos.x,            pos.y + size.y,   pos.z + size.z);
  glVertex3f(pos.x + size.x,  pos.y + size.y,   pos.z + size.z);
  glVertex3f(pos.x + size.x,  pos.y,             pos.z + size.z);
  glVertex3f(pos.x,            pos.y,             pos.z + size.z);
  glVertex3f(pos.x,            pos.y + size.y,   pos.z + size.z);
  glEnd();
  
  // fill:
  glBegin(GL_QUADS);
  glColor3f(fill_color.x, fill_color.y, fill_color.z);
  glVertex3f(pos.x,                          pos.y + size.y,   pos.z + size.z);
  glVertex3f(pos.x + size.x*fill_percent,   pos.y + size.y,   pos.z + size.z);
  glVertex3f(pos.x + size.x*fill_percent,   pos.y,             pos.z + size.z);
  glVertex3f(pos.x,                          pos.y,             pos.z + size.z);
  glEnd();
  
  // bar label
  glPushMatrix();
  glTranslatef(pos.x-5.0f, pos.y+size.y, pos.z);
  glScalef(3.0f,3.0f, 0.0f);
  t3dDraw2D(bar_tag, 1, -1);
  glPopMatrix();

 // bar text values
  glPushMatrix();
  glTranslatef(pos.x+size.x+1.0f, pos.y+size.y, pos.z);
  glScalef(3.0f,3.0f, 0.0f);
  t3dDraw2D(bar_text, -1, -1);
  glPopMatrix();
}

void drawScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear last draw buffer

  if (QUIT) return;

  glMatrixMode(GL_MODELVIEW); // Set Draw Mode
  glLoadIdentity();           // Reset Drawing Perspective

  glTranslatef(-133.0f, -100.0f, -100.0f);

  // (0,0) Marker:
  glBegin(GL_LINE_STRIP);
  glColor3f(1.0f, 1.0f, 1.0f);
  glVertex3f(-0.5f,   0.5f, 0.0f);
  glVertex3f( 0.5f,   0.5f, 0.0f);
  glVertex3f( 0.5f,  -0.5f, 0.0f);
  glVertex3f(-0.5f,  -0.5f, 0.0f);
  glVertex3f(-0.5f,   0.5f, 0.0f);
  glEnd();

  float w = 267.0f;

  // BACKGROUND INTERFACE GFX:
  float bg_h = 40.0f;
  glPushMatrix();
  glColor3f(0.2f, 0.2f, 0.25f);
  glBegin(GL_LINES);
  glVertex3f(0.0f,  bg_h, -0.1f);
  glVertex3f(w,     bg_h, -0.1f);
  glEnd();
  glColor3f(0.05f, 0.05f, 0.05f);
  glBegin(GL_QUADS);
  glVertex3f(0.0f,  bg_h,  -0.15f);
  glVertex3f(w,     bg_h,  -0.15f);
  glVertex3f(w,     0.0f,  -0.15f);
  glVertex3f(0.0f,  0.0f,  -0.15f);
  glEnd();
  glPopMatrix();

  // CHARACTER GFX:
  point3d offset          (75.0f, 0.0f, 0.0f);

  point3d base_pos        (0.0f, 150.0f, 0.0f);

  point3d name_pos        (30.0f, 33.0f, 0.0f);
  point3d name_size       (4.0f,   4.5f, 0.0f);
  point3d name_color      (0.8f,   0.8f, 0.8f);

  string time_tag = "TIME:";
  point3d time_pos        (20.0f,  20.0f, 0.0f);
  point3d time_size       (20.0f,  5.0f, 0.0f);
  point3d time_color      (0.6f,   0.6f, 0.6f);
  point3d time_fill       (0.4f,   0.4f, 0.4f);
  
  string hp_tag = "HP:";
  point3d hp_pos          (20.0f, 13.0f,  0.0f);
  point3d hp_size         (20.0f, 5.0f,  0.0f);
  point3d hp_color        (0.8f,  0.0f,  0.0f);
  point3d hp_fill         (0.5f,  0.0f,  0.0f);
  
  string mp_tag = "MP:";
  point3d mp_pos          (20.0f, 6.0f,  0.0f);
  point3d mp_size         (20.0f, 5.0f,  0.0f);
  point3d mp_color        (0.0f,  0.0f,  0.8f);
  point3d mp_fill         (0.0f,  0.0f,  0.4f);

  map<int, character*>::iterator it = toons.begin();
  while (it != toons.end()) {

    point3d restore_old_pos = base_pos;
    if (it->first == TOON_ID) base_pos = point3d(0.0f, 0.0f, 0.0f);

    int toon_id = it->first;
    character* toon = it->second;

    // update character gui pos:
    toon->gui_pos = base_pos;

    // name:
    glPushMatrix();
    glColor3f(name_color.x, name_color.y, name_color.z);
    glTranslatef(toon->gui_pos.x+name_pos.x, toon->gui_pos.y+name_pos.y, toon->gui_pos.z+name_pos.z);
    glScalef(name_size.x, name_size.y, name_size.z);
    t3dDraw2D(toon->name(), 0, -1);
    glPopMatrix();
    // time:
    string time_text = itos((int)(ceil(toon->timebar_percent()*100.0)));
    time_text += "%";
    drawbar(toon->gui_pos + time_pos, time_size, time_color, time_fill, (float)toon->timebar_percent(), time_tag, time_text);
    // hp:
    string hp_text = itos(toon->hp()); hp_text += "/"; hp_text += itos(toon->hp_max());
    hp_animations[toon_id]->pos(toons[toon_id]->gui_pos + hp_pos + point3d((hp_size.x/2.0f), (hp_size.y/2.0f), 0.5f));
    drawbar(toon->gui_pos + hp_pos, hp_size, hp_color, hp_fill, (float)toon->hp()/(float)toon->hp_max(), hp_tag, hp_text);
    // mp:
    string mp_text = itos(toon->mp()); mp_text += "/"; mp_text += itos(toon->mp_max());
    mp_animations[toon_id]->pos(toons[toon_id]->gui_pos + mp_pos + point3d((mp_size.x/2.0f), (mp_size.y/2.0f), 0.5f));
    drawbar(toon->gui_pos+mp_pos, mp_size, mp_color, mp_fill, (float)toon->mp()/(float)toon->mp_max(), mp_tag, mp_text);
    glPushMatrix();

    // dmg animations:
    hp_animations[toon_id]->animate();
    mp_animations[toon_id]->animate();
    spell_animations[toon_id]->animate();

    if (it->first != TOON_ID) base_pos = base_pos + offset;
    else base_pos  = restore_old_pos;
    it++;
  }

  glutSwapBuffers(); //Send the 3D scene to the screen
}

void tick(int value) {
  glutPostRedisplay();
  glutTimerFunc(125, tick, 0);
}

void glutbranch(void*) {
  cout << "Loading Graphic Interface..." << endl;
  //Initialize GLUT
  glutInit(&_argc, _argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(640, 480); //Set the window size

    glViewport(0, 0, 640,480);      // Convert from coordinates to pixels values
    glMatrixMode(GL_PROJECTION);    // Set Camera Mode
    glLoadIdentity();               // Reset the camera
    gluPerspective(90,              // The camera angle
             (float)640/(float)480, // pixel ratio
             5.0f,                  // Near-Z-clipping
             1000.0f);               // Far-Z-clipping

  //Create the window
  glutCreateWindow("OpenGL");
  initRendering(); //Initialize rendering

  //Set handler functions for drawing, keypresses, and window resizes
  glutDisplayFunc(drawScene);
  glutKeyboardFunc(handleKeypress);
  glutReshapeFunc(handleResize);

  glutTimerFunc(125, tick, 0);
  glutMainLoop();
  t3dCleanup();
}
