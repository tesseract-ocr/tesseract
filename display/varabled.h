/**********************************************************************
 * File:        varabled.h  (Formerly varedit.h)
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

#ifndef           VARABLED_H
#define           VARABLED_H
                                 //find all variables
MENU_L_LIST *build_list_of_all_leaves(); 
MENU_ROOT *build_main_var_menu(  //build menus
                               MENU_L_LIST *all_leaves_list);

void extract_sublist(                             //remove initial items
                     MENU_L_LIST *source_list,    //source list
                     char *leading_str,           //string to match
                     MENU_L_LIST *extracted_list  //extracted list
                    );
void get_first_words(                //copy first N words
                     const char *s,  //source string
                     int n,          //number of words
                     char *t         //target string
                    );
int menu_item_sorter(  //sorter
                     const void *item1,
                     const void *item2);
void start_variables_editor();  //create top level win
#endif
