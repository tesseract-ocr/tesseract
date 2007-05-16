/******************************************************************************
 **	Filename:    sigmenu.c
 **	Purpose:     General purpose, menu-oriented signal handling routines
 **	Author:      Dan Johnson
 **	History:     Mon Oct  2 07:25:50 1989, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "sigmenu.h"
#include "oldlist.h"
#include "emalloc.h"
#include "secname.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

#define MAX_COMMAND_LENGTH  128

typedef struct
{
  int ItemNum;
  char *ItemLabel;
  int_void ItemFunc;
} SIG_MENU_ITEM;

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
static LIST SignalMenus[NSIG];   /* must be initialized to NIL */

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
void MainSignalHandler(int Signal);

SIG_MENU_ITEM *NewSignalMenuItem (int ItemNum,
const char ItemLabel[], int_void ItemFunc);

int ItemCompare(void *arg1,   //SIG_MENU_ITEM         *Item1,
                void *arg2);  //SIG_MENU_ITEM         *Item2);

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void
AddSignalMenuItem (int Signal,
int ItemNum, const char ItemLabel[], int_void ItemFunc) {
/*
 **	Parameters:
 **		Signal		signal to be trapped for this menu
 **		ItemNum		menu number for this item
 **		ItemLabel	menu label for this item
 **		ItemFunc	function to be called when item is selected
 **	Globals:
 **		SignalMenus	list of menu items for each possible signal
 **	Operation:
 **		Add a new menu item to the list of menu items for Signal.
 **		Whenever Signal is encountered, the user will be given
 **		a list of options to choose from.  This list is the list
 **		of all of the menu items that have been specified for that
 **		Signal.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Oct  2 07:42:19 1989, DSJ, Created.
 */
  #if 0
  #ifndef SECURE_NAMES
  SIG_MENU_ITEM *NewItem;

  /* check for a valid Signal */
  if (Signal >= NSIG || Signal <= 0) {
    cprintf ("Illegal signal (%d) specified for menu item!\n", Signal);
    return;
  }

  /* if this is the first item for this signal, indicate that the
     appropriate signal handler has been enabled */
  if (SignalMenus[Signal] == NIL)
    cprintf ("Signal handler enabled for signal %d.\n", Signal);

  /* add the new menu item to the appropriate list of menu items */
  NewItem = NewSignalMenuItem (ItemNum, ItemLabel, ItemFunc);
  SignalMenus[Signal] = s_adjoin (SignalMenus[Signal], NewItem, ItemCompare);

  /* set up the trap for the appropriate signal */
  signal(Signal, MainSignalHandler);
  #endif
  #endif
}                                /* AddSignalMenuItem */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void MainSignalHandler(int Signal) {
/*
 **	Parameters:
 **		Signal		signal that caused this function to be called
 **	Globals:
 **		SignalMenus	list of menu items for each possible signal
 **	Operation: Provide the user with a menu of actions for the trapped
 **		signal.  Execute the appropriate function.  If the function
 **		returns SIG_RESUME, then terminate the signal handler and
 **		resume normal processing.  If the function does not return
 **		SIG_RESUME, remain in the main signal handler menu.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Oct  2 08:18:52 1989, DSJ, Created.
 */
  #ifndef SECURE_NAMES
  int Command;
  char CommandLine[MAX_COMMAND_LENGTH];
  char *Params;
  LIST Items;
  SIG_MENU_ITEM *MenuItem;

  while (TRUE) {
    Command = -1;
    cprintf ("\nMAIN SIGNAL HANDLER FOR SIGNAL %d\n", Signal);
    cprintf ("0. Resume normal operation\n");

    Items = SignalMenus[Signal];
    iterate(Items) {
      MenuItem = (SIG_MENU_ITEM *) first_node (Items);
      cprintf ("%d. %s\n", MenuItem->ItemNum, MenuItem->ItemLabel);
    }
    cprintf ("\nEnter Selection: ");

    while (fgets (CommandLine, MAX_COMMAND_LENGTH, stdin) == NULL
      || strlen (CommandLine) <= 0);

    Command = strtol (CommandLine, &Params, 10);
    if (CommandLine == Params) {
      cprintf ("\nIllegal command! - Try again.\n");
      continue;
    }

    if (Command == 0)
      signal(Signal, MainSignalHandler);

    Items = SignalMenus[Signal];
    iterate(Items) {
      MenuItem = (SIG_MENU_ITEM *) first_node (Items);
      if (Command == MenuItem->ItemNum) {
        if ((*MenuItem->ItemFunc) ( /*Params */ ) == SIG_RESUME)
          signal(Signal, MainSignalHandler);
        break;
      }
    }
    if (Items == NIL)
      cprintf ("\nIllegal command! - Try again.\n");
  }
  #endif
}                                /* MainSignalHandler */


/*---------------------------------------------------------------------------*/
SIG_MENU_ITEM *
NewSignalMenuItem (int ItemNum, const char ItemLabel[], int_void ItemFunc) {
/*
 **	Parameters:
 **		ItemNum		menu number for this item
 **		ItemLabel	menu label for this item
 **		ItemFunc	function to be called when item is selected
 **	Globals: none
 **	Operation: Allocate, initialize, and return a new signal menu item.
 **	Return: Ptr to new signal menu item data structure.
 **	Exceptions: none
 **	History: Mon Oct  2 08:04:20 1989, DSJ, Created.
 */
  SIG_MENU_ITEM *NewItem;

  NewItem = (SIG_MENU_ITEM *) Emalloc (sizeof (SIG_MENU_ITEM));
  NewItem->ItemNum = ItemNum;
  NewItem->ItemFunc = ItemFunc;
  NewItem->ItemLabel = (char *) Emalloc (strlen (ItemLabel) + 1);
  strcpy (NewItem->ItemLabel, ItemLabel);
  return (NewItem);

}                                /* NewSignalMenuItem */


/*---------------------------------------------------------------------------*/
int ItemCompare(void *arg1,    //SIG_MENU_ITEM         *Item1,
                void *arg2) {  //SIG_MENU_ITEM         *Item2)
/*
 **	Parameters:
 **		Item1, Item2	two menu items to be compared
 **	Globals: none
 **	Operation: Return -1 if the ItemNum of Item1 is less than the
 **		ItemNum of Item2.  Return 0 if they are equal.  Return +1
 **		if the ItemNum of Item1 is greater than the ItemNum of
 **		Item2.  This routine is used by the list sorter to sort
 **		lists of menu items according to their item number.
 **	Return: -1, 0, or 1
 **	Exceptions: none
 **	History: Mon Oct  2 08:11:59 1989, DSJ, Created.
 */
  SIG_MENU_ITEM *Item1 = (SIG_MENU_ITEM *) arg1;
  SIG_MENU_ITEM *Item2 = (SIG_MENU_ITEM *) arg2;

  if (Item1->ItemNum < Item2->ItemNum)
    return (-1);
  else if (Item1->ItemNum == Item2->ItemNum)
    return (0);
  else if (Item1->ItemNum > Item2->ItemNum)
    return (1);
  else
    return 0;
}                                /* ItemCompare */
