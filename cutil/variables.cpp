/* -*-C-*-
 ********************************************************************************
 *
 * File:        variables.c  (Formerly variables.c)
 * Description:  Variable handler for control flags
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Dec 12 09:03:49 1989
 * Modified:     Thu Apr  4 11:01:37 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
          I n c l u d e s
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "variables.h"
#include "tprintf.h"
#include "listio.h"
#include "globals.h"
#include "scanutils.h"

/*----------------------------------------------------------------------
          V a r i a b l e s
----------------------------------------------------------------------*/
static LIST variable_list = NIL;
//extern char                                   *demodir;

VALUE dummy;

/*----------------------------------------------------------------------
            Macros
----------------------------------------------------------------------*/
/**********************************************************************
 * scan_int
 *
 * Scan in an integer value from a string.  It might be in decimal or
 * hex.  Read it into "integer".
 **********************************************************************/

#define scan_int(stripped,integer)             \
if (stripped [2] == 'x')                     \
	sscanf (&stripped[3], "%x", &integer);    \
else                                         \
	sscanf (&stripped[1], "%d", &integer)

/*----------------------------------------------------------------------
          F u n c t i o n s
----------------------------------------------------------------------*/

void free_variables() {
  destroy_nodes(variable_list, free);
  variable_list = NIL;
}
/**********************************************************************
 * add_ptr_variable
 *
 * Add a new ptr variable to the global variable list.  Initalize its
 * value.
 **********************************************************************/
void add_ptr_variable(void *address,
                      const char *string,
                      VALUE default_value,
                      variables_io reader,
                      variables_io writer) {
  VARIABLE *this_var;
  this_var = (VARIABLE *) malloc (sizeof (VARIABLE));

  this_var->address = address;
  this_var->string = string;
  this_var->default_value = default_value;
  this_var->type_reader = reader;
  this_var->type_writer = writer;

  *((void **) this_var->address) = default_value.ptr_part;
  variable_list = push (variable_list, this_var);
}


/**********************************************************************
 * add_32bit_variable
 *
 * Add a new 32bit variable to the global variable list.  Initalize
 * its value.
 **********************************************************************/
void add_32bit_variable(void *address,
                        const char *string,
                        VALUE default_value,
                        variables_io reader,
                        variables_io writer) {
  VARIABLE *this_var;
  this_var = (VARIABLE *) malloc (sizeof (VARIABLE));

  this_var->address = address;
  this_var->string = string;
  this_var->default_value = default_value;
  this_var->type_reader = reader;
  this_var->type_writer = writer;

  *((int *) this_var->address) = default_value.int_part;
  variable_list = push (variable_list, this_var);
}


/**********************************************************************
 * float_read
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void float_read(VARIABLE *variable, char *string) {
  float f;

  #ifdef EMBEDDED
    // We have no sscanf with float functionality here
    *((float *) variable->address) = strtofloat(strip_line (string));
  #else
    sscanf (strip_line (string), "%f", &f);
    *((float *) variable->address) = f;
  #endif
}


/**********************************************************************
 * float_write
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void float_write(VARIABLE *variable, char *string) {
  float *f;
  f = (float *) variable->address;
  sprintf (string, "%s\t%4.2f", variable->string, *f);
}


/**********************************************************************
 * int_read
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void int_read(VARIABLE *variable, char *string) {
  char *stripped;
  int integer;

  stripped = strip_line (string);
  /* Add the value */
  if (stripped[0] == '+') {
    scan_int(stripped, integer);
    *((int *) variable->address) += integer;
  }
  else if (stripped[0] == '|') {
    scan_int(stripped, integer);
    *((int *) variable->address) = integer | *((int *) variable->address);
  }                              /* Subtract the value */
  else if (stripped[0] == '_') {
    scan_int(stripped, integer);
    *((int *) variable->address) = (~integer) &
      *((int *) variable->address);
  }
  else {
                                 /* Set the value */
    if (stripped[1] == 'x') {
      sscanf (&stripped[2], "%x", &integer);
    }
    else {
      sscanf (stripped, "%d", &integer);
    }
    *((int *) variable->address) = integer;
  }
}


/**********************************************************************
 * int_write
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void int_write(VARIABLE *variable, char *string) {
  sprintf (string, "%s\t%d", variable->string, *((int *) variable->address));
}


/**********************************************************************
 * read_variables
 *
 * Read a file that contains assignments for all the desired variables.
 * This type of file can be written using the function write_variables.
 **********************************************************************/
void read_variables(const char *filename) {
  int x = 0;
  char *this_string;
  LIST var_strings;
  char name[1024];
  FILE *fp;
  /* Read the strings */
  if (filename == NULL || filename[0] == '\0')
    return;

  strcpy(name, demodir);
  strcat (name, "tessdata/tessconfigs/");
  strcat(name, filename);
  if ((fp = fopen (name, "r")) == NULL)
    strcpy(name, filename);
  else
    fclose(fp);
  var_strings = read_list (name);
  iterate(var_strings) {
    /* Get the name string */
    this_string = (char *) first_node (var_strings);
    if (this_string[0] != '#') {
      for (x = 0;
        x < strlen (this_string) && this_string[x] != ' '
        && this_string[x] != '\t'; x++);
      this_string[x] = '\0';
      /* Find variable record */
      if (!set_old_style_variable(this_string, this_string + x + 1)) {
        tprintf("error: Could not find variable '%s'\n", this_string);
        exit(1);                // ?err_exit ();
      }
    }
  }
}

bool set_old_style_variable(const char* variable, const char* value) {
  char* var_variable = strdup(variable);
  char* var_value = strdup(value);

  VARIABLE *this_var;
  this_var = (VARIABLE *)first_node(search(variable_list, var_variable,
                                           same_var_name));
  if (this_var != NULL) {
    (*(this_var->type_reader)) (this_var, var_value);
  }
  free(var_variable);
  free(var_value);
  return this_var != NULL;
}


/**********************************************************************
 * same_var_name
 *
 * Return TRUE if the VARIABLE has the name string in it.
 **********************************************************************/
int same_var_name(void *item1,    //VARIABLE *variable,
                  void *item2) {  //char     *string)
  VARIABLE *variable;
  char *string;

  variable = (VARIABLE *) item1;
  string = (char *) item2;

  if (strcmp (variable->string, string))
    return (FALSE);
  else
    return (TRUE);
}


/**********************************************************************
 * string_read
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void string_read(VARIABLE *variable, char *string) {
  char *value;

  value = strsave (strip_line (string));
  *((char **) variable->address) = value;
}


/**********************************************************************
 * string_write
 *
 * Read an integer value and save it in a variable structure.
 **********************************************************************/
void string_write(VARIABLE *variable, char *string) {
  sprintf (string, "%s\t%s", variable->string,
    *((char **) variable->address));
}


/**********************************************************************
 * strip_line
 *
 * Remove the name off the front of the line and strip the white space
 * before and after the value.
 **********************************************************************/
char *strip_line(char *string) {
  int x;
  int y;

  x = 0;
  /* Skip over whitespace */
  while (x < strlen (string) && (string[x] == '\t' || string[x] == ' '))
    x++;
  /* Strip trailing whitespace */
  for (y = strlen (string);
    y >= 0 && (string[y - 1] == '\t' || string[y - 1] == ' '); y--)
  string[y] = '\0';
  /* Return result */
  return (&string[x]);
}
