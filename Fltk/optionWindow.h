// Gmsh - Copyright (C) 1997-2008 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _OPTION_WINDOW_H_
#define _OPTION_WINDOW_H_

#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Value_Input.H>
#include "spherePositionWidget.h"
#include "colorbarWindow.h"

#define NUM_FONTS 14
extern Fl_Menu_Item menu_font_names[];

class optionWindow{
 private:
  int _fontsize;
 public:
  Fl_Window *win;
  Fl_Hold_Browser *browser;
  Fl_Return_Button *redraw;
  struct{
    Fl_Group *group;
    Fl_Check_Button *butt[20];
    Fl_Button *push[20];
    Fl_Value_Input *value[50];
    Fl_Button *color[50];
    Fl_Input *input[20];
    Fl_Choice *choice[20];
    spherePositionWidget *sphere;
  } general;
  struct{
    Fl_Group *group;
    Fl_Check_Button *butt[20];
    Fl_Value_Input *value[20];
    Fl_Button *color[50];
    Fl_Choice *choice[20];
  } geo;
  struct{
    Fl_Group *group;
    Fl_Check_Button *butt[50];
    Fl_Return_Button *retbutt[50];
    Fl_Input *input[20];
    Fl_Value_Input *value[50];
    Fl_Button *color[50];
    Fl_Choice *choice[20];
    Fl_Menu_Button *menu;
  } mesh;
  struct{
    Fl_Group *group;
    Fl_Check_Button *butt[20];
    Fl_Value_Input *value[20];
    Fl_Input *input[20];
  } solver;
  struct{
    Fl_Group *group;
    Fl_Check_Button *butt[20];
    Fl_Value_Input *value[20];
    Fl_Choice *choice[20];
  } post;
  struct{
    int index;
    Fl_Group *group, *range, *vector;
    Fl_Check_Button *butt[100];
    Fl_Value_Input *value[100];
    Fl_Input *input[100];
    Fl_Repeat_Button *butt_rep[100];
    Fl_Button *push[100];
    Fl_Choice *choice[100];
    Fl_Button *color[50];
    colorbarWindow *colorbar;
    Fl_Return_Button *ok;
    Fl_Menu_Button *menu[2];
  } view;

 public:
  optionWindow(int fontsize);
  void showGroup(int num, bool showWindow=true);
  void resetBrowser();
  void resetExternalViewList();
  void updateViewGroup(int index);
};

#endif
