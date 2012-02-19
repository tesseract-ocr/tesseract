/* -*-C-*-
################################################################################
#
# File:						array.c
# Description:				Dynamic Array of Strings
# Author:					Mark Seaman, Software Productivity
# Created:					Thu Jul 23 13:24:09 1987
# Modified:     Wed Mar  6 15:18:33 1991 (Mark Seaman) marks@hpgrlt
# Language:					C
# Package:					N/A
# Status:					Reusable Software Component
#
# (c) Copyright 1987, Hewlett-Packard Company.
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
#
################################################################################

This file contains the implentations of a set of dynamic array of string
manipulation routines.		For the interface definitions and documentation
of these routines see the file "das.h".

***************************************************************************/

#include "tessarray.h"
#include "callcpp.h"
#include "freelist.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#endif
#include <ctype.h>
#if MAC_OR_DOS
#include <stdlib.h>
#endif

/**********************************************************************
 * array_insert
 *
 * Insert a data element into a particular spot in the array.  Move all
 * the elements in the array (past that spot) down one to make room for
 * the new element.
 **********************************************************************/
ARRAY array_insert(ARRAY array, int index, void *value) { 
  int x;

  array = array_push (array, NULL);
  for (x = array_count (array) - 1; x > index; x--)
    array_value (array, x) = array_value (array, x - 1);
  array_value (array, index) = value;
  return (array);
}


/**********************************************************************
 * array_new
 *
 * Create a new array with a certain number of elements.	If the number
 * of elements requested is 0 then the default number will be used.
 **********************************************************************/
ARRAY array_new(int num) { 
  ARRAY temp;
  int x;

  if (num == 0)
    num = DEFAULT_SIZE;
  temp = (ARRAY) memalloc ((num - 2) * sizeof (char *) +
    sizeof (struct array_record));
  if (!temp) {
    cprintf ("error: Out of memory in array_new\n");
    exit (1);                    //?err_exit ();
  }
  array_count (temp) = 0;
  array_limit (temp) = num;
  for (x = 0; x < num; x++)
    array_value (temp, x) = (char *) 0;
  return (temp);
}


/**********************************************************************
 * array_push
 *
 * Add a new element onto the top of the array.	If there is not room
 * more room is made by "realloc"ing the array.	This means that the
 * new array location may change.  All previous references to its old
 * location may no longer be valid.
 **********************************************************************/
ARRAY array_push(ARRAY array, void *value) { 
  if (array_count (array) == array_limit (array)) {
    array = (ARRAY) memrealloc (array, (array_limit (array) * 2 - 2) *
      sizeof (char *) +
      sizeof (struct array_record),
      (array_limit (array) -
      2) * sizeof (char *) +
      sizeof (struct array_record));
    if (!array) {
      cprintf ("error: Out of memory in array_push\n");
      exit (1);                  //?err_exit ();
    }
    array_limit (array) *= 2;
  }
  array_count (array)++;
  array_top (array) = value;
  return (array);
}
