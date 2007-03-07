/**********************************************************************
 * File:        varabled.cpp  (Formerly varedit.c)
 * Description: Variables Editor
 * Author:		Phil Cheatle
 * Created:		Mon Nov 11 09:51:58 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "mfcpch.h"
#include          <stdio.h>
#include          "varable.h"
#include          "varblmen.h"
#include          "varblwin.h"
#include          "varabled.h"
#include          "cmndwin.h"
#include          "mainblk.h"

#define VARDIR        "configs/" /*variables files */

const ERRCODE NO_VARIABLES_TO_EDIT = "No Variables defined to edit";

/**********************************************************************
 * build_list_of_all_leaves()
 *
 * Generate a list of menu leaves for each of the variables. The list is
 * heterogeneous, different menu leaf types for each variable type.
 **********************************************************************/

MENU_L_LIST *build_list_of_all_leaves() {  //find all variables
  INT_VARIABLE_C_IT int_it (INT_VARIABLE::get_head ());
  BOOL_VARIABLE_C_IT bool_it (BOOL_VARIABLE::get_head ());
  STRING_VARIABLE_C_IT str_it (STRING_VARIABLE::get_head ());
  double_VARIABLE_C_IT dbl_it (double_VARIABLE::get_head ());

  MENU_L_LIST *all_leaves_list = new MENU_L_LIST;
  MENU_L_IT it;

  it.set_to_list (all_leaves_list);

  for (int_it.mark_cycle_pt (); !int_it.cycled_list (); int_it.forward ())
    it.add_to_end (new MENU_L (new INT_VAR_MENU_LEAF (int_it.data ())));

  for (bool_it.mark_cycle_pt (); !bool_it.cycled_list (); bool_it.forward ())
    it.add_to_end (new MENU_L (new BOOL_VAR_MENU_LEAF (bool_it.data ())));

  for (str_it.mark_cycle_pt (); !str_it.cycled_list (); str_it.forward ())
    it.add_to_end (new MENU_L (new STR_VAR_MENU_LEAF (str_it.data ())));

  for (dbl_it.mark_cycle_pt (); !dbl_it.cycled_list (); dbl_it.forward ())
    it.add_to_end (new MENU_L (new DBL_VAR_MENU_LEAF (dbl_it.data ())));

  return all_leaves_list;
}


/**********************************************************************
 * build_main_var_menu()
 *
 * Extract each element from the sorted, non empty list of menu leaves. Add
 * each to the appropriate menu.  Note that ALL menus for ALL variable editor
 * windows are built at this stage.  The VAR_SUB_MENU menu leaves used in the
 * leaves of the top level window hold the root menus for the sub windows for
 * use if and when that window is created.  A little wastful on space perhaps,
 * but easier than reconstructing/sorting/etc the all_leaves_list.
 *
 **********************************************************************/

MENU_ROOT *build_main_var_menu(  //build menus
                               MENU_L_LIST *all_leaves_list) {
  MENU_L_IT it;
  MENU_L_LIST sub_window_list;
  MENU_L_LIST sub_menu_list;

  MENU_ROOT *main_root;
  VAR_NON_RADIO_MENU *main_menu;
  NON_RADIO_MENU *std_menu;
  MENU_ROOT *main_misc_root = NULL;
  VAR_NON_RADIO_MENU *main_misc_menu = NULL;
  MENU_ROOT *sub_window_root;
  VAR_NON_RADIO_MENU *current_sub_win_menu;
  VAR_NON_RADIO_MENU *sub_window_misc_menu;
  MENU_L *leaf;
  const char *full_name;
  char first_word[80];
  char first_two_words[80];
  int header_len;
  STRING varfile;                //save config filename

  main_root = new MENU_ROOT;
  main_menu = new VAR_NON_RADIO_MENU ("SubMenus");
  main_root->add_child (main_menu);

  while (!all_leaves_list->empty ()) {
                                 // Whatever is left
    it.set_to_list (all_leaves_list);
    full_name = (it.data ()->ptr)->cmp_str ();
                                 // 1st word delim by _
    get_first_words (full_name, 1, first_word);
    extract_sublist(all_leaves_list, first_word, &sub_window_list); 

                                 //addto main misc menu
    if (sub_window_list.singleton ()) {
      if (main_misc_root == NULL) {
        main_misc_root = new MENU_ROOT;
        main_misc_menu = new VAR_NON_RADIO_MENU ("Miscellaneous");
        main_misc_root->add_child (main_misc_menu);
      }
      it.set_to_list (&sub_window_list);
      leaf = it.extract ();
      leaf->ptr->new_label (full_name);
      main_misc_menu->add_child (leaf);
    }
    else {                       //build subwindow menu
      sub_window_root = new MENU_ROOT;
      main_menu->add_child (new VAR_SUB_MENU (first_word,
        sub_window_root));
      sub_window_misc_menu = new VAR_NON_RADIO_MENU ("Miscellaneous");
      sub_window_misc_menu->
        add_child (new
        SIMPLE_MENU_LEAF ("Kill Sub Window", KILL_WINDOW_CMD));
      while (!sub_window_list.empty ()) {
        it.set_to_list (&sub_window_list);
        full_name = it.data ()->ptr->cmp_str ();
        get_first_words (full_name, 2, first_two_words);
        extract_sublist(&sub_window_list, first_two_words, &sub_menu_list); 

        it.set_to_list (&sub_menu_list);
                                 //addto subwin misc
        if (sub_menu_list.singleton ()) {
          leaf = it.extract ();
          leaf->ptr->new_label (full_name);
          sub_window_misc_menu->add_child (leaf);
        }
        else {
                                 //build sub menu
          current_sub_win_menu =
            new VAR_NON_RADIO_MENU(first_two_words); 
          sub_window_root->add_child (current_sub_win_menu);
          header_len = strlen (first_two_words);
          while (!sub_menu_list.empty ()) {
            it.set_to_list (&sub_menu_list);
            leaf = it.extract ();
            leaf->ptr->new_label (leaf->ptr->cmp_str () +
              header_len);
            current_sub_win_menu->add_child (leaf);
          }
        }
      }
      sub_window_root->add_child (sub_window_misc_menu);
    }
  }
  if (main_misc_root != NULL)
    main_root->add_child (main_misc_root);

  varfile = datadir;
  varfile += VARDIR;             /*variables dir */
  varfile += "edited";           /*actual name */

  std_menu = new NON_RADIO_MENU ("Build Config File");
  main_root->add_child (std_menu);
  std_menu->add_child (new VARIABLE_MENU_LEAF ("All Variables",
    WRITE_ALL_CMD,
    varfile.string ()));
  std_menu->add_child (new VARIABLE_MENU_LEAF ("Changed Variables Only",
    WRITE_CHANGED_CMD,
    varfile.string ()));
  return main_root;
}


/**********************************************************************
 * extract_sublist()
 *
 * Extract a sublist containing elements whose name initially matches the
 * specified string
 *
 **********************************************************************/

void extract_sublist(                             //remove initial items
                     MENU_L_LIST *source_list,    //source list
                     char *leading_str,           //string to match
                     MENU_L_LIST *extracted_list  //extracted list
                    ) {
  MENU_L_IT start_it(source_list); 
  MENU_L_IT end_it(source_list); 
  int match_len = strlen (leading_str);

  while (!end_it.at_last () &&
    (strncmp (leading_str, end_it.data_relative (1)->ptr->cmp_str (),
    match_len) == 0))
    end_it.forward ();
  extracted_list->assign_to_sublist (&start_it, &end_it);
}


/**********************************************************************
 * get_first_words()
 *
 * Copy the first N words from the source string to the target string.
 * Words are delimited by "_"
 *
 **********************************************************************/

void get_first_words(                //copy first N words
                     const char *s,  //source string
                     int n,          //number of words
                     char *t         //target string
                    ) {
  int full_length = strlen (s);
  int reqd_len = 0;              //No. of chars requird
  const char *next_word = s;

  while ((n > 0) && reqd_len < full_length) {
    reqd_len += strcspn (next_word, "_") + 1;
    next_word += reqd_len;
    n--;
  }
  strncpy(t, s, reqd_len); 
  t[reqd_len] = '\0';            //ensure null terminal
}


/**********************************************************************
 * menu_item_sorter()   LIST SORT COMPARATOR
 *
 * Given two MENU_L items, sort them on the basis of their cmp_str  s
 *
 **********************************************************************/

int menu_item_sorter(  //sorter
                     const void *item1,
                     const void *item2) {
  MENU_L *menu_l_1 = (*(MENU_L **) item1);
  MENU_L *menu_l_2 = (*(MENU_L **) item2);

  MENU_NODE *node1;
  MENU_NODE *node2;

  const char *str1;
  const char *str2;

  node1 = menu_l_1->ptr;
  node2 = menu_l_2->ptr;

  str1 = node1->cmp_str ();
  str2 = node2->cmp_str ();

  return strcmp (str1, str2);
}


/**********************************************************************
 * start_variables_editor()
 *
 * Create the top level variables editor window.  Build all the menus for all
 * the subwindows in the process.
 **********************************************************************/

void start_variables_editor() {  //create top level win
  MENU_L_LIST *all_leaves_list = new MENU_L_LIST;
  MENU_L_IT it;

  all_leaves_list = build_list_of_all_leaves ();
  it.set_to_list (all_leaves_list);
  if (it.empty ()) {
    NO_VARIABLES_TO_EDIT.error ("start_variables_editor", LOG, NULL);
  }
  it.sort (&menu_item_sorter);

  new VARIABLES_WINDOW ("VarEditorMAIN",
    build_main_var_menu (all_leaves_list), NULL);
}
