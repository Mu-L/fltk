//
// Code dialogs for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// generated by Fast Light User Interface Designer (fluid) version 1.0500

#ifndef codeview_panel_h
#define codeview_panel_h
#include <FL/Fl.H>
extern int cv_code_choice;
void update_codeview_position();
void update_codeview_position_cb(class Fl_Tabs*, void*);
void update_codeview_cb(class Fl_Button*, void*);
void update_codeview_timer(void*);
void codeview_defer_update();
void codeview_toggle_visibility();
#include <FL/Fl_Double_Window.H>
extern void toggle_codeview_cb(Fl_Double_Window*, void*);
extern Fl_Double_Window *codeview_panel;
#include <FL/Fl_Tabs.H>
extern Fl_Tabs *cv_tab;
#include <FL/Fl_Group.H>
extern Fl_Group *cv_source_tab;
#include "widgets/Code_Viewer.h"
extern fld::widget::Code_Viewer *cv_source;
extern fld::widget::Code_Viewer *cv_header;
#include "widgets/Text_Viewer.h"
extern fld::widget::Text_Viewer *cv_strings;
extern fld::widget::Text_Viewer *cv_project;
extern Fl_Group *cv_find_row;
#include <FL/Fl_Button.H>
extern Fl_Button *cv_find_text_case;
#include <FL/Fl_Input.H>
extern Fl_Input *cv_find_text;
#include <FL/Fl_Box.H>
extern Fl_Group *cv_settings_row;
#include <FL/Fl_Light_Button.H>
extern Fl_Light_Button *cv_autorefresh;
extern Fl_Light_Button *cv_autoposition;
#include <FL/Fl_Choice.H>
extern Fl_Choice *cv_code_choice_w;
extern void toggle_codeview_b_cb(Fl_Button*, void*);
Fl_Double_Window* make_codeview();
extern Fl_Menu_Item menu_cv_code_choice_w[];
#endif

//
