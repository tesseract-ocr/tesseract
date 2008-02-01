/* -*-C-*-
 ********************************************************************************
 *
 * File:        variables.h  (Formerly variables.h)
 * Description:  Variable handler for control flags
 * Author:       Mark Seaman, OCR Technology
 * Created:      Tue Dec 12 09:03:49 1989
 * Modified:     Fri Jan 12 16:57:36 1990 (Mark Seaman) marks@hpgrlt
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
#ifndef VARIABLES_H
#define VARIABLES_H

#include "cutil.h"
#include "oldlist.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef union
{
  float float_part;
  int int_part;
  void *ptr_part;
  const char *char_part;
} VALUE;

class VARIABLE;

typedef void (*variables_io) (VARIABLE * variable, char *string);

class VARIABLE
{
  public:
    void *address;
    const char *string;
    VALUE default_value;
    variables_io type_reader;
    variables_io type_writer;
};

extern VALUE dummy;              /* Needed for macros */

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/* 	To hide variable names for relase to UNLV DONT put the variable names in
  the executable: Toggle be either defining or not defining SECURE_NAMES*/
/* #define SECURE_NAMES defined in secnames.h when necessary */
#ifdef SECURE_NAMES
/**********************************************************************
 * float_variable
 *
 * Create a floating point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define float_variable(name,string,default)                \
dummy.float_part = default;                              \
add_32bit_variable (&name, "", dummy, float_read, float_write)

/**********************************************************************
 * string_variable
 *
 * Create a string point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define string_variable(name,string,default)              \
dummy.char_part = default;                    \
add_ptr_variable (&name, "", dummy, string_read, string_write)

/**********************************************************************
 * int_variable
 *
 * Create a int point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define int_variable(name,string,default)                 \
dummy.int_part = default;                              \
add_32bit_variable (&name, "", dummy, int_read, int_write)

#else
/**********************************************************************
 * float_variable
 *
 * Create a floating point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define float_variable(name,string,default)                \
dummy.float_part = default;                              \
add_32bit_variable (&name, string, dummy, float_read, float_write)

/**********************************************************************
 * string_variable
 *
 * Create a string point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define string_variable(name,string,default)              \
dummy.char_part = default;                    \
add_ptr_variable (&name, string, dummy, string_read, string_write)

/**********************************************************************
 * int_variable
 *
 * Create a int point variable that can be read and written from
 * a configuration file.
 **********************************************************************/

#define int_variable(name,string,default)                 \
dummy.int_part = default;                              \
add_32bit_variable (&name, string, dummy, int_read, int_write)
#endif

/*--------------------------------------------------------------------------
        Public Function Prototoypes
----------------------------------------------------------------------------*/
void free_variables();

void add_ptr_variable(void *address,
                      const char *string,
                      VALUE default_value,
                      variables_io reader,
                      variables_io writer);

void add_32bit_variable(void *address,
                        const char *string,
                        VALUE default_value,
                        variables_io reader,
                        variables_io writer);

void float_read(VARIABLE *variable, char *string);

void float_write(VARIABLE *variable, char *string);

void int_read(VARIABLE *variable, char *string);

void int_write(VARIABLE *variable, char *string);

void read_variables(const char *filename);

bool set_old_style_variable(const char* variable, const char* value);

int same_var_name(void *item1,   //VARIABLE *variable,
                  void *item2);  //char     *string)

void string_read(VARIABLE *variable, char *string);

void string_write(VARIABLE *variable, char *string);

char *strip_line(char *string);
#endif
