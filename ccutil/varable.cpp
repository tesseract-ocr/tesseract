/**********************************************************************
 * File:        varable.c  (Formerly variable.c)
 * Description: Initialization and setting of VARIABLEs.
 * Author:					Ray Smith
 * Created:					Fri Feb 22 16:22:34 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers

#include          <stdio.h>
#include          <string.h>
#include          <stdlib.h>

#include          "scanutils.h"
#include          "tprintf.h"
#include          "varable.h"

#define PLUS          '+'        //flag states
#define MINUS         '-'
#define EQUAL         '='

CLISTIZE (INT_VARIABLE)
CLISTIZE (BOOL_VARIABLE) CLISTIZE (STRING_VARIABLE) CLISTIZE (double_VARIABLE)
INT_VAR_FROM
INT_VARIABLE::copy;
INT_VARIABLE_CLIST
INT_VARIABLE::head;              //global definition
INT_VAR_TO
INT_VARIABLE::replace;
BOOL_VAR_FROM
BOOL_VARIABLE::copy;
BOOL_VARIABLE_CLIST
BOOL_VARIABLE::head;             //global definition
BOOL_VAR_TO
BOOL_VARIABLE::replace;
STRING_VAR_FROM
STRING_VARIABLE::copy;
STRING_VARIABLE_CLIST
STRING_VARIABLE::head;           //global definition
STRING_VAR_TO
STRING_VARIABLE::replace;
double_VAR_FROM
double_VARIABLE::copy;
double_VARIABLE_CLIST
double_VARIABLE::head;           //global definition
double_VAR_TO
double_VARIABLE::replace;

/**********************************************************************
 * INT_VAR_FROM::INT_VAR_FROM
 *
 * Constructor to copy the list to a temporary location while the
 * list head gets constructed.
 **********************************************************************/

INT_VAR_FROM::INT_VAR_FROM() {  //constructor
  INT_VARIABLE_C_IT start_it = &INT_VARIABLE::head;
  INT_VARIABLE_C_IT end_it = &INT_VARIABLE::head;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
                                 //move to copy
    list.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * INT_VAR_TO::INT_VAR_TO
 *
 * Constructor to copy the list back to its rightful place.
 **********************************************************************/

INT_VAR_TO::INT_VAR_TO() {  //constructor
  INT_VARIABLE_C_IT start_it = &INT_VARIABLE::copy.list;
  INT_VARIABLE_C_IT end_it = &INT_VARIABLE::copy.list;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
    INT_VARIABLE::head.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * INT_VARIABLE::INT_VARIABLE
 *
 * Constructor for INT_VARIABLE. Add the variable to the static list.
 **********************************************************************/

INT_VARIABLE::INT_VARIABLE(                     //constructor
                           inT32 v,             //the variable
                           const char *vname,   //of variable
                           const char *comment  //info on variable
                          ) {
  INT_VARIABLE_C_IT it = &head;  //list iterator

  //tprintf("Constructing %s\n",vname);
  set_value(v);  //set the value
  name = vname;                  //strings must be static
  info = comment;
  it.add_before_stay_put (this); //add it to stack
}


INT_VARIABLE::~INT_VARIABLE (    //constructor
) {
  INT_VARIABLE_C_IT it = &head;  //list iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    if (it.data () == this)
      it.extract ();
}


/**********************************************************************
 * INT_VARIABLE::get_head
 *
 * Get the head of the list of the variables.
 **********************************************************************/

INT_VARIABLE_CLIST *INT_VARIABLE::get_head() {  //access to static
  return &head;
}


/**********************************************************************
 * INT_VARIABLE::print
 *
 * Print the entire list of INT_VARIABLEs.
 **********************************************************************/

void INT_VARIABLE::print(          //print full list
                         FILE *fp  //file to print on
                        ) {
  INT_VARIABLE_C_IT it = &head;  //list iterator
  INT_VARIABLE *elt;             //current element

  if (fp == stdout) {
    tprintf ("#Variables of type INT_VARIABLE:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      elt = it.data ();
      tprintf ("%s %d #%s\n", elt->name, elt->value, elt->info);
    }
  }
  else {
    fprintf (fp, "#Variables of type INT_VARIABLE:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      elt = it.data ();
      fprintf (fp, "%s " INT32FORMAT " #%s\n", elt->name, elt->value,
        elt->info);
    }
  }
}


/**********************************************************************
 * BOOL_VAR_FROM::BOOL_VAR_FROM
 *
 * Constructor to copy the list to a temporary location while the
 * list head gets constructed.
 **********************************************************************/

BOOL_VAR_FROM::BOOL_VAR_FROM() {  //constructor
  BOOL_VARIABLE_C_IT start_it = &BOOL_VARIABLE::head;
  BOOL_VARIABLE_C_IT end_it = &BOOL_VARIABLE::head;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
                                 //move to copy
    list.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * BOOL_VAR_TO::BOOL_VAR_TO
 *
 * Constructor to copy the list back to its rightful place.
 **********************************************************************/

BOOL_VAR_TO::BOOL_VAR_TO() {  //constructor
  BOOL_VARIABLE_C_IT start_it = &BOOL_VARIABLE::copy.list;
  BOOL_VARIABLE_C_IT end_it = &BOOL_VARIABLE::copy.list;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
    BOOL_VARIABLE::head.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * BOOL_VARIABLE::BOOL_VARIABLE
 *
 * Constructor for BOOL_VARIABLE. Add the variable to the static list.
 **********************************************************************/

BOOL_VARIABLE::BOOL_VARIABLE(                     //constructor
                             BOOL8 v,             //the variable
                             const char *vname,   //of variable
                             const char *comment  //info on variable
                            ) {
  BOOL_VARIABLE_C_IT it = &head; //list iterator

  //tprintf("Constructing %s\n",vname);
  set_value(v);  //set the value
  name = vname;                  //strings must be static
  info = comment;
  it.add_before_stay_put (this); //add it to stack

}


/**********************************************************************
 * BOOL_VARIABLE::BOOL_VARIABLE
 *
 * Constructor for BOOL_VARIABLE. Add the variable to the static list.
 **********************************************************************/

BOOL_VARIABLE::~BOOL_VARIABLE () {
  BOOL_VARIABLE_C_IT it = &head; //list iterator

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    if (it.data () == this)
      it.extract ();
}


/**********************************************************************
 * BOOL_VARIABLE::get_head
 *
 * Get the head of the list of the variables.
 **********************************************************************/

BOOL_VARIABLE_CLIST *BOOL_VARIABLE::get_head() {  //access to static
  return &head;
}


/**********************************************************************
 * BOOL_VARIABLE::print
 *
 * Print the entire list of BOOL_VARIABLEs.
 **********************************************************************/

void BOOL_VARIABLE::print(          //print full list
                          FILE *fp  //file to print on
                         ) {
  BOOL_VARIABLE_C_IT it = &head; //list iterator
  BOOL_VARIABLE *elt;            //current element

  if (fp == stdout) {
    tprintf ("#Variables of type BOOL_VARIABLE:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      elt = it.data ();
      tprintf ("%s %c #%s\n",
        elt->name, elt->value ? 'T' : 'F', elt->info);
    }
  }
  else {
    fprintf (fp, "#Variables of type BOOL_VARIABLE:\n");
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      elt = it.data ();
      fprintf (fp, "%s %c #%s\n",
        elt->name, elt->value ? 'T' : 'F', elt->info);
    }
  }
}


/**********************************************************************
 * STRING_VAR_FROM::STRING_VAR_FROM
 *
 * Constructor to copy the list to a temporary location while the
 * list head gets constructed.
 **********************************************************************/

STRING_VAR_FROM::STRING_VAR_FROM() {  //constructor
  STRING_VARIABLE_C_IT start_it = &STRING_VARIABLE::head;
  STRING_VARIABLE_C_IT end_it = &STRING_VARIABLE::head;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
                                 //move to copy
    list.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * STRING_VAR_TO::STRING_VAR_TO
 *
 * Constructor to copy the list back to its rightful place.
 **********************************************************************/

STRING_VAR_TO::STRING_VAR_TO() {  //constructor
  STRING_VARIABLE_C_IT start_it = &STRING_VARIABLE::copy.list;
  STRING_VARIABLE_C_IT end_it = &STRING_VARIABLE::copy.list;

  if (!start_it.empty ()) {
    while (!end_it.at_last ())
      end_it.forward ();
    STRING_VARIABLE::head.assign_to_sublist (&start_it, &end_it);
  }
}


/**********************************************************************
 * STRING_VARIABLE::STRING_VARIABLE
 *
 * Constructor for STRING_VARIABLE. Add the variable to the static list.
 **********************************************************************/

STRING_VARIABLE::STRING_VARIABLE (
                                 //constructor
const char *v,                   //the variable
const char *vname,               //of variable
const char *comment              //info on variable
):
value(v) {
                                 // list iterator
  STRING_VARIABLE_C_IT it = &head;

  name = vname;                  // strings must be static
  info = comment;
  it.add_before_stay_put(this);  // add it to stack
}


/**********************************************************************
 * STRING_VARIABLE::~STRING_VARIABLE
 *
 * Destructor for STRING_VARIABLE. Add the variable to the static list.
 **********************************************************************/

                                 // constructor
STRING_VARIABLE::~STRING_VARIABLE(
) {
                                 // list iterator
  STRING_VARIABLE_C_IT it = &head;

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())
    if (it.data() == this)
      it.extract();
}


/**********************************************************************
 * STRING_VARIABLE::get_head
 *
 * Get the head of the list of the variables.
 **********************************************************************/

STRING_VARIABLE_CLIST *STRING_VARIABLE::get_head() {  // access to static
  return &head;
}


/**********************************************************************
 * STRING_VARIABLE::print
 *
 * Print the entire list of STRING_VARIABLEs.
 **********************************************************************/

void STRING_VARIABLE::print(FILE *fp) {
  STRING_VARIABLE_C_IT it = &head;  // list iterator
  STRING_VARIABLE *elt;          // current element

  // Comments aren't allowed with string variables, so the # character can
  // be part of a string.
  if (fp == stdout) {
    tprintf("#Variables of type STRING_VARIABLE:\n");
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      elt = it.data();
      tprintf("%s %s\n", elt->name, elt->value.string());
    }
  } else {
    fprintf(fp, "#Variables of type STRING_VARIABLE:\n");
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      elt = it.data();
      fprintf(fp, "%s %s\n", elt->name, elt->value.string());
    }
  }
}


/**********************************************************************
 * double_VAR_FROM::double_VAR_FROM
 *
 * Constructor to copy the list to a temporary location while the
 * list head gets constructed.
 **********************************************************************/

double_VAR_FROM::double_VAR_FROM() {  // constructor
  double_VARIABLE_C_IT start_it = &double_VARIABLE::head;
  double_VARIABLE_C_IT end_it = &double_VARIABLE::head;

  if (!start_it.empty()) {
    while (!end_it.at_last())
      end_it.forward();
                                 // move to copy
    list.assign_to_sublist(&start_it, &end_it);
  }
}


/**********************************************************************
 * double_VAR_TO::double_VAR_TO
 *
 * Constructor to copy the list back to its rightful place.
 **********************************************************************/

double_VAR_TO::double_VAR_TO() {  // constructor
  double_VARIABLE_C_IT start_it = &double_VARIABLE::copy.list;
  double_VARIABLE_C_IT end_it = &double_VARIABLE::copy.list;

  if (!start_it.empty()) {
    while (!end_it.at_last())
      end_it.forward();
    double_VARIABLE::head.assign_to_sublist(&start_it, &end_it);
  }
}


/**********************************************************************
 * double_VARIABLE::double_VARIABLE
 *
 * Constructor for double_VARIABLE. Add the variable to the static list.
 **********************************************************************/

double_VARIABLE::double_VARIABLE(double v,            // the variable
                                 const char *vname,   // of variable
                                 const char *comment  // info on variable
                                ) {
                                 // list iterator
  double_VARIABLE_C_IT it = &head;

  set_value(v);  // set the value
  name = vname;                  // strings must be static
  info = comment;
  it.add_before_stay_put(this); // add it to stack
}


double_VARIABLE::~double_VARIABLE() {
                                 // list iterator
  double_VARIABLE_C_IT it = &head;

  for (it.mark_cycle_pt(); !it.cycled_list(); it.forward())
    if (it.data() == this)
      it.extract();
}


/**********************************************************************
 * double_VARIABLE::get_head
 *
 * Get the head of the list of the variables.
 **********************************************************************/

double_VARIABLE_CLIST *double_VARIABLE::get_head() {  // access to static
  return &head;
}


/**********************************************************************
 * double_VARIABLE::print
 *
 * Print the entire list of double_VARIABLEs.
 **********************************************************************/

void double_VARIABLE::print(FILE *fp  // file to print on
                           ) {
                                 // list iterator
  double_VARIABLE_C_IT it = &head;
  double_VARIABLE *elt;          // current element

  if (fp == stdout) {
    tprintf("#Variables of type double_VARIABLE:\n");
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      elt = it.data();
      tprintf ("%s %lg #%s\n", elt->name, elt->value, elt->info);
    }
  } else {
    fprintf(fp, "#Variables of type double_VARIABLE:\n");
    for (it.mark_cycle_pt(); !it.cycled_list(); it.forward()) {
      elt = it.data();
      fprintf(fp, "%s %g #%s\n", elt->name, elt->value, elt->info);
    }
  }
}


/**********************************************************************
 * read_variables_file
 *
 * Read a file of variables definitions and set/modify the values therein.
 * If the filename begins with a + or -, the BOOL_VARIABLEs will be
 * ORed or ANDed with any current values.
 * Blank lines and lines beginning # are ignored.
 * Values may have any whitespace after the name and are the rest of line.
 **********************************************************************/

DLLSYM BOOL8 read_variables_file(const char *file,  // name to read
                                 bool global_only   // only set variables
                                 ) {                // starting with "global_"
  char flag;                     // file flag
  inT16 nameoffset;              // offset for real name
  FILE *fp;                      // file pointer
                                 // iterators
  bool ret;

  if (*file == PLUS) {
    flag = PLUS;                 // file has flag
    nameoffset = 1;
  } else if (*file == MINUS) {
    flag = MINUS;
    nameoffset = 1;
  } else {
    flag = EQUAL;
    nameoffset = 0;
  }

  fp = fopen(file + nameoffset, "r");
  if (fp == NULL) {
    tprintf("read_variables_file: Can't open %s\n", file + nameoffset);
    return TRUE;                 // can't open it
  }
  ret = read_variables_from_fp(fp, -1, global_only);
  fclose(fp);
  return ret;
}

bool read_variables_from_fp(FILE *fp, inT64 end_offset, bool global_only) {
  char line[MAX_PATH];           // input line
  bool anyerr = false;          // true if any error
  bool foundit;                 // found variable
  inT16 length;                  // length of line
  char *valptr;                  // value field

  while ((end_offset < 0 || ftell(fp) < end_offset) &&
         fgets(line, MAX_PATH, fp)) {
    if (line[0] != '\n' && line[0] != '#') {
      length = strlen (line);
      if (line[length - 1] == '\n')
        line[length - 1] = '\0';  // cut newline
      for (valptr = line; *valptr && *valptr != ' ' && *valptr != '\t';
        valptr++);
      if (*valptr) {             // found blank
        *valptr = '\0';          // make name a string
        do
          valptr++;              // find end of blanks
        while (*valptr == ' ' || *valptr == '\t');
      }
      if (global_only && strstr(line, kGlobalVariablePrefix) == NULL) continue;
      foundit = set_variable(line, valptr);

      if (!foundit) {
        anyerr = TRUE;         // had an error
        tprintf("read_variables_file: variable not found: %s\n", line);
        exit(1);
      }
    }
  }
  return anyerr;
}

bool set_variable(const char *variable, const char* value) {
  INT_VARIABLE_C_IT int_it = &INT_VARIABLE::head;
  BOOL_VARIABLE_C_IT BOOL_it = &BOOL_VARIABLE::head;
  STRING_VARIABLE_C_IT STRING_it = &STRING_VARIABLE::head;
  double_VARIABLE_C_IT double_it = &double_VARIABLE::head;

  bool foundit = false;
  // find name
  for (STRING_it.mark_cycle_pt();
       !STRING_it.cycled_list() && strcmp(variable, STRING_it.data()->name);
       STRING_it.forward());
  if (!STRING_it.cycled_list()) {
    foundit = true;          // found the varaible
    STRING_it.data()->set_value(value);  // set its value
  }

  if (*value) {
    // find name
    for (int_it.mark_cycle_pt();
         !int_it.cycled_list() && strcmp(variable, int_it.data()->name);
         int_it.forward());
    int intval;
    if (!int_it.cycled_list()
    && sscanf(value, INT32FORMAT, &intval) == 1) {
      foundit = true;        // found the varaible
      int_it.data()->set_value(intval);  // set its value.
    }
    for (BOOL_it.mark_cycle_pt();
         !BOOL_it.cycled_list() && strcmp(variable, BOOL_it.data()->name);
         BOOL_it.forward());
    if (!BOOL_it.cycled_list()) {
      if (*value == 'T' || *value == 't' ||
          *value == 'Y' || *value == 'y' || *value == '1') {
        foundit = true;
        BOOL_it.data()->set_value(TRUE);
      }
      else if (*value == 'F' || *value == 'f' ||
               *value == 'N' || *value == 'n' || *value == '0') {
        foundit = true;
        BOOL_it.data()->set_value(FALSE);
      }
    }
    for (double_it.mark_cycle_pt();
         !double_it.cycled_list() && strcmp(variable, double_it.data ()->name);
         double_it.forward());
    double doubleval;
#ifdef EMBEDDED
    if (!double_it.cycled_list ()) {
      doubleval = strtofloat(value);
#else
    if (!double_it.cycled_list()
        && sscanf(value, "%lf", &doubleval) == 1) {
#endif
      foundit = true;        // found the variable
      double_it.data()->set_value(doubleval);
    }
  }
  return foundit;
}

/**********************************************************************
 * print_variables
 *
 * Print all variable types to the given file
 **********************************************************************/

DLLSYM void print_variables(          //print all vars
                            FILE *fp  //file to print on
                           ) {
  INT_VARIABLE::print(fp);  //print INTs
  BOOL_VARIABLE::print(fp);  //print BOOLs
  STRING_VARIABLE::print(fp);  //print STRINGs
  double_VARIABLE::print(fp);  //print doubles
}
