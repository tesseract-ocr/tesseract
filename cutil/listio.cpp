/* -*-C-*-
################################################################################
#
# File:						listio.c
# Description:				List I/O processing procedures.
# Author:					Mark Seaman, Software Productivity
# Created:					Thu Jul 23 13:24:09 1987
# Modified:     Fri May 17 17:33:30 1991 (Mark Seaman) marks@hpgrlt
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

This file contains the implementations of a set of general purpose
list I/O routines.  For the interface definitions look in the file
"listio.h".
---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "listio.h"

/*---------------------------------------------------------------------------
        Public Function Code
---------------------------------------------------------------------------*/
/*************************************************************************
 *  R E A D   L I S T
 *
 *							Read a list of strings from a file.  Return the string list to the
 *							caller.
 *************************************************************************/
LIST read_list(const char *filename) {
  FILE *infile;
  char s[CHARS_PER_LINE];
  LIST list;

  if ((infile = open_file (filename, "r")) == NULL)
    return (NIL_LIST);

  list = NIL_LIST;
  while (fgets (s, CHARS_PER_LINE, infile) != NULL) {
    s[CHARS_PER_LINE - 1] = '\0';
    if (strlen (s) > 0) {
      if (s[strlen (s) - 1] == '\n')
        s[strlen (s) - 1] = '\0';
      if (strlen (s) > 0) {
        list = push (list, (LIST) strsave (s));
      }
    }
  }

  fclose(infile);
  return (reverse_d (list));
}
