/* -*-C-*-
 ********************************************************************************
 *
 * File:        debug.h  (Formerly debug.h)
 * Description:  Combinatorial Splitter
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Jul 27 08:59:01 1989
 * Modified:     Wed Feb 27 14:38:16 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 *********************************************************************************/
/* 	To hide variable names for relase to UNLV DONT put the variable names in
  the executable: Toggle be either defining or not defining SECURE_NAMES
  This stage prevents the menu construction*/
/* #define SECURE_NAMES done in secnames.h when necessary */

#ifndef DEBUG_H
#define DEBUG_H

#include "variables.h"
#include "callcpp.h"
#include <stdio.h>

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct
{
  const char *menu_string;
  void_proc menu_function;
} MENU_ITEM;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
#define NUM_MENUS       30
#define NUM_MENU_ITEMS  30
extern MENU_ITEM menu_table[NUM_MENUS][NUM_MENU_ITEMS];

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * float_value
 *
 * Template procedures to set a floating point value of a variable.
 **********************************************************************/

#define float_value(proc,string,variable,default)  \
																	\
float variable = default;                          \
																	\
int proc ()                                        \
{                                                  \
	return (set_float_value (string, &variable));   \
}                                                  \


/**********************************************************************
 * handle_menu_x
 *
 * Create a procedure to handle menu items.
 **********************************************************************/

#define handle_menu(menu,handle_menu_x)                              \
																							\
int handle_menu_x ()                                                 \
{                                                                    \
	int x;                                                            \
	cprintf ("\t 0. Continue\n");                                      \
	for (x = 0; x < NUM_MENU_ITEMS; x++) {                            \
		if (menu_table[menu][x].menu_string)                           \
		cprintf ("\t%2d. %s\n", x, menu_table[menu][x].menu_string);  \
	}                                                                 \
																							\
	scanf ("%d", &x);                                                 \
																							\
	if (x == 0) return (0);                                           \
	if ((0 < x && x < NUM_MENU_ITEMS) &&                              \
		(menu_table[menu][x].menu_function)) {                        \
		(*menu_table[menu][x].menu_function) ();                       \
		return (1);                                                    \
	}                                                                 \
	else {                                                            \
		cprintf ("Bad menu selection");                                 \
		return (0);                                                    \
	}                                                                 \
}                                                                    \


/**********************************************************************
 * int_value
 *
 * Template procedures to set a floating point value of a variable.
 **********************************************************************/

#define int_value(proc,string,variable,default)    \
																	\
int variable = default;                            \
																	\
int proc ()                                        \
{                                                  \
	return (set_int_value (string, &variable));     \
}                                                  \


/**********************************************************************
 * toggle_value
 *
 * Template procedures to toggle the value of a variable.
 **********************************************************************/

#ifdef SECURE_NAMES
#define toggle_value(proc,string,variable,default) \
																	\
int variable = default;                            \
																	\
int proc ()                                        \
{                                                  \
	if (variable) {                                \
		variable = 0;                              \
	}                                              \
	else {                                         \
		variable = 1;                              \
	}                                              \
	return (1);                                    \
}                                                  \

#else

#define toggle_value(proc,string,variable,default) \
																	\
int variable = default;                            \
																	\
int proc ()                                        \
{                                                  \
	if (variable) {                                \
		cprintf( "%s is OFF\n", string);   \
		variable = 0;                              \
	}                                              \
	else {                                         \
		cprintf( "%s is ON\n", string);    \
		variable = 1;                              \
	}                                              \
	return (1);                                    \
}                                                  \

#endif

/**********************************************************************
 * make_float_const
 *
 * Create a constant with a config file reader
 **********************************************************************/

#define make_float_const(name,default,installer)				\
																				\
float name = default;											\
																				\
void installer ()                                                  \
{                                                             \
float_variable (name, #name, default);                     \
}                                                             \


/**********************************************************************
 * make_int_const
 *
 * Create a constant with a config file reader
 **********************************************************************/

#define make_int_const(name,default,installer)					\
																				\
int name = default;												\
																				\
void installer ()                                                  \
{                                                             \
int_variable (name, #name, default);                       \
}                                                             \


/**********************************************************************
 * make_toggle_const
 *
 * Create a constant with a config file reader
 **********************************************************************/

#define make_toggle_const(name,default,installer)					\
																					\
int name = default;													\
																					\
void installer ()                                                     \
{                                                                \
int_variable (name, #name, default);                          \
}                                                                \


#ifdef SECURE_NAMES
/**********************************************************************
 * make_float_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_float_var(name,default,installer,menu,menuitem,menufunct,menustring) \
																				\
float_value (menufunct, "", name, default);           \
																				\
void installer ()                                                  \
{                                                             \
float_variable (name, #name, default);                     \
}                                                             \


/**********************************************************************
 * make_int_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_int_var(name,default,installer,menu,menuitem,menufunct,menustring)   \
																				\
int_value (menufunct, "", name, default);             \
																				\
void installer ()                                                  \
{                                                             \
int_variable (name, #name, default);                       \
}                                                             \


/**********************************************************************
 * make_toggle_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_toggle_var(name,default,installer,menu,menuitem,menufunct,menustring)   \
																					\
toggle_value (menufunct, "", name, default);             \
																					\
void installer ()                                                     \
{                                                                \
int_variable (name, #name, default);                          \
}                                                                \

#else

/**********************************************************************
 * make_float_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_float_var(name,default,installer,menu,menuitem,menufunct,menustring) \
																				\
float_value (menufunct, menustring, name, default);           \
																				\
void installer ()                                                  \
{                                                             \
float_variable (name, #name, default);                     \
make_menu_item (menu, menuitem, menustring, menufunct);     \
}                                                             \


/**********************************************************************
 * make_int_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_int_var(name,default,installer,menu,menuitem,menufunct,menustring)   \
																				\
int_value (menufunct, menustring, name, default);             \
																				\
void installer ()                                                  \
{                                                             \
int_variable (name, #name, default);                       \
make_menu_item (menu, menuitem, menustring, menufunct);     \
}                                                             \


/**********************************************************************
 * make_toggle_var
 *
 * Create a variable with a config file reader and a menu handler.
 **********************************************************************/

#define make_toggle_var(name,default,installer,menu,menuitem,menufunct,menustring)   \
																					\
toggle_value (menufunct, menustring, name, default);             \
																					\
void installer ()                                                     \
{                                                                \
int_variable (name, #name, default);                          \
make_menu_item (menu, menuitem, menustring, menufunct);        \
}                                                                \

#endif

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int set_float_value(const char *message, float *variable); 

int set_int_value(const char *message, int *variable); 

void make_menu_item(int menu,
                    int menu_item,
                    const char *menu_string,
                    int_void menu_funct);

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif

int set_float_value
_ARGS((char *message,
    float *variable));

int set_int_value
_ARGS((char *message,
    int *variable));

void make_menu_item
_ARGS((int menu,
    int menu_item,
    char *menu_string,
    int_proc menu_funct));

#undef _ARGS
*/
#endif
