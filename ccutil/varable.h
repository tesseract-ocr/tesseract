/**********************************************************************
 * File:        varable.h  (Formerly variable.h)
 * Description: Class definitions of the *_VAR classes for tunable constants.
 * Author:          Ray Smith
 * Created:         Fri Feb 22 11:26:25 GMT 1991
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

#ifndef           VARABLE_H
#define           VARABLE_H

#include          <stdio.h>

#include          "clst.h"
#include          "strngs.h"

class DLLSYM INT_VARIABLE;

// Read config file.
extern DLLSYM BOOL8 read_variables_file(
    const char *file,   // filename to read
    bool global_only);  // only set variables starting with "global_"

// Read variables from the given file pointer (stop at end_offset).
bool read_variables_from_fp(FILE *fp, inT64 end_offset, bool global_only);

// Set a variable to have the given value.
bool set_variable(const char *variable, const char* value);

// Print variables to a file.
extern DLLSYM void print_variables(FILE *fp);

const char kGlobalVariablePrefix[] = "global_";

CLISTIZEH (INT_VARIABLE)
class DLLSYM INT_VAR_FROM
{
  friend class INT_VAR_TO;
  public:
    INT_VAR_FROM();  //constructor
  private:
    INT_VARIABLE_CLIST list;     //copy of list
};

class DLLSYM INT_VAR_TO
{
  public:
    INT_VAR_TO();  //constructor
  private:
    INT_VARIABLE_CLIST dummy;
};

class DLLSYM INT_VARIABLE
{
  friend class INT_VAR_TO;
  friend class INT_VAR_FROM;
                                 //for setting values
  friend bool set_variable(const char *variable, const char* value);

  public:
    INT_VARIABLE(inT32 v,               // initial value
                 const char *vname,     // name of variable
                 const char *comment);  // info on variable

    INT_VARIABLE() {  // for elist only
      value = 0;
      name = "NONAME";
      info = "Uninitialized";
    }
    ~INT_VARIABLE();            // for elist only

    operator inT32() {  // conversion
      return value;              // access as int
    }

    void set_value(inT32 v) {  // value to set
      value = v;
    }

    const char *name_str() {  // access name
      return name;
    }

    const char *info_str() {  // access name
      return info;
    }

                                 // access list head
    static INT_VARIABLE_CLIST *get_head();

    static void print(FILE *fp);  // file to print on

  private:
    inT32 value;                 // the variable
    const char *name;            // name of variable
    const char *info;            // for menus
    static INT_VAR_FROM copy;    // pre constructor
                                 // start  of list
    static INT_VARIABLE_CLIST head;
    static INT_VAR_TO replace;   // post constructor
};

class DLLSYM BOOL_VARIABLE;

CLISTIZEH(BOOL_VARIABLE)
class DLLSYM BOOL_VAR_FROM {
  friend class BOOL_VAR_TO;
  public:
    BOOL_VAR_FROM();  // constructor
  private:
    BOOL_VARIABLE_CLIST list;    // copy of list
};

class DLLSYM BOOL_VAR_TO {
  public:
    BOOL_VAR_TO();  // constructor
  private:
    BOOL_VARIABLE_CLIST dummy;
};

class DLLSYM BOOL_VARIABLE {
  friend class BOOL_VAR_FROM;
  friend class BOOL_VAR_TO;
                                 //for setting values
  friend bool set_variable(const char *variable, const char* value);

  public:
    BOOL_VARIABLE(                       //constructor
                  BOOL8 v,               //initial value
                  const char *vname,     //name of variable
                  const char *comment);  //info on variable

    BOOL_VARIABLE() {  //for elist only
      value = FALSE;
      name = "NONAME";
      info = "Uninitialized";
    }
    ~BOOL_VARIABLE ();           //for elist only

    operator BOOL8() {  //conversion
      return value;              //access as int
    }

    void set_value(            //assign to value
                   BOOL8 v) {  //value to set
      value = v;
    }

    const char *name_str() {  //access name
      return name;
    }

    const char *info_str() {  //access name
      return info;
    }

                                 //access list head
    static BOOL_VARIABLE_CLIST *get_head();

    static void print(            //print whole list
                      FILE *fp);  //file to print on

  private:
    BOOL8 value;                 //the variable
    const char *name;            //name of variable
    const char *info;            //for menus
    static BOOL_VAR_FROM copy;   //pre constructor
                                 //start  of list
    static BOOL_VARIABLE_CLIST head;
    static BOOL_VAR_TO replace;  //post constructor
};

class DLLSYM STRING_VARIABLE;

CLISTIZEH (STRING_VARIABLE)
class DLLSYM STRING_VAR_FROM
{
  friend class STRING_VAR_TO;
  public:
    STRING_VAR_FROM();  //constructor
  private:
    STRING_VARIABLE_CLIST list;  //copy of list
};

class DLLSYM STRING_VAR_TO
{
  public:
    STRING_VAR_TO();  //constructor
  private:
    STRING_VARIABLE_CLIST dummy;
};

class DLLSYM STRING_VARIABLE
{
  friend class STRING_VAR_TO;
  friend class STRING_VAR_FROM;
                                 //for setting values
  friend bool set_variable(const char *variable, const char* value);

  public:
    STRING_VARIABLE(                       //constructor
                    const char *v,         //initial value
                    const char *vname,     //name of variable
                    const char *comment);  //info on variable

    STRING_VARIABLE() {  //for elist only
      name = "NONAME";
      info = "Uninitialized";
    }
    ~STRING_VARIABLE ();         //for elist only

                                 //conversion
    operator const STRING &() {
      return value;              //access as int
    }

    void set_value(             //assign to value
                   STRING v) {  //value to set
      value = v;
    }

    const char *string() const {  //get string
      return value.string ();
    }

    const char *name_str() {  //access name
      return name;
    }

    const char *info_str() {  //access name
      return info;
    }

                                 //access list head
    static STRING_VARIABLE_CLIST *get_head();

    static void print(            //print whole list
                      FILE *fp);  //file to print on

  private:
    STRING value;                //the variable
    const char *name;            //name of variable
    const char *info;            //for menus
    static STRING_VAR_FROM copy; //pre constructor
                                 //start  of list
    static STRING_VARIABLE_CLIST head;
    static STRING_VAR_TO replace;//post constructor
};

class DLLSYM double_VARIABLE;

CLISTIZEH (double_VARIABLE)
class DLLSYM double_VAR_FROM
{
  friend class double_VAR_TO;
  public:
    double_VAR_FROM();  //constructor
  private:
    double_VARIABLE_CLIST list;  //copy of list
};

class DLLSYM double_VAR_TO
{
  public:
    double_VAR_TO();  //constructor
  private:
    double_VARIABLE_CLIST dummy;
};

class DLLSYM double_VARIABLE
{
  friend class double_VAR_TO;
  friend class double_VAR_FROM;
                                 //for setting values
  friend bool set_variable(const char *variable, const char* value);

  public:
    double_VARIABLE(                       //constructor
                    double v,              //initial value
                    const char *vname,     //name of variable
                    const char *comment);  //info on variable

    double_VARIABLE() {  //for elist only
      value = 0.0;
      name = "NONAME";
      info = "Uninitialized";
    }
    ~double_VARIABLE ();         //for elist only

    operator double() {  //conversion
      return value;              //access as int
    }

    void set_value(             //assign to value
                   double v) {  //value to set
      value = v;
    }

    const char *name_str() {  //access name
      return name;
    }

    const char *info_str() {  //access name
      return info;
    }

                                 //access list head
    static double_VARIABLE_CLIST *get_head();

    static void print(            //print whole list
                      FILE *fp);  //file to print on

  private:
    double value;                //the variable
    const char *name;            //name of variable
    const char *info;            //for menus
    static double_VAR_FROM copy; //pre constructor
                                 //start  of list
    static double_VARIABLE_CLIST head;
    static double_VAR_TO replace;//post constructor
};

/*************************************************************************
 * NOTE ON DEFINING VARIABLES
 *
 * For our normal code, the ***_VAR and ***_EVAR macros for variable
 * definitions are identical.  HOWEVER, for the code version to ship to NEVADA
 * (or anywhere else where we want to hide the majority of variables) the
 * **_VAR macros are changed so that the "#name" and "comment" parameters
 * to the variable constructor are changed to empty strings.  This prevents the
 * variable name or comment string appearing in the object code file (after it
 * has gone through strip).
 *
 * Certain variables can remain EXPOSED and hence be used in config files given
 * to UNLV. These are variable which have been declared with the ***_EVAR
 * macros.
 *
 *************************************************************************/

/* SECURE_NAMES is defined in senames.h when necessary */
#ifdef SECURE_NAMES

#define INT_VAR(name,val,comment)           /*make INT_VARIABLE*/\
  INT_VARIABLE      name(val,"","")

#define BOOL_VAR(name,val,comment)            /*make BOOL_VARIABLE*/\
  BOOL_VARIABLE     name(val,"","")

#define STRING_VAR(name,val,comment)          /*make STRING_VARIABLE*/\
  STRING_VARIABLE     name(val,"","")

#define double_VAR(name,val,comment)          /*make double_VARIABLE*/\
  double_VARIABLE     name(val,"","")

#else

#define INT_VAR(name,val,comment)           /*make INT_VARIABLE*/\
  INT_VARIABLE      name(val,#name,comment)

#define BOOL_VAR(name,val,comment)            /*make BOOL_VARIABLE*/\
  BOOL_VARIABLE     name(val,#name,comment)

#define STRING_VAR(name,val,comment)          /*make STRING_VARIABLE*/\
  STRING_VARIABLE     name(val,#name,comment)

#define double_VAR(name,val,comment)          /*make double_VARIABLE*/\
  double_VARIABLE     name(val,#name,comment)
#endif

#define INT_VAR_H(name,val,comment)           /*declare one*/\
  INT_VARIABLE      name

#define BOOL_VAR_H(name,val,comment)          /*declare one*/\
  BOOL_VARIABLE     name

#define STRING_VAR_H(name,val,comment)          /*declare one*/\
  STRING_VARIABLE     name

#define double_VAR_H(name,val,comment)          /*declare one*/\
  double_VARIABLE     name

#define INT_MEMBER(name, val, comment)          /*make INT_VARIABLE*/\
  name(val, #name, comment)

#define BOOL_MEMBER(name, val, comment)         /*make BOOL_VARIABLE*/\
  name(val, #name, comment)

#define STRING_MEMBER(name, val, comment)       /*make STRING_VARIABLE*/\
  name(val, #name, comment)

#define double_MEMBER(name, val, comment)       /*make double_VARIABLE*/\
  name(val, #name, comment)

#define INT_EVAR(name,val,comment)            /*make INT_VARIABLE*/\
  INT_VARIABLE      name(val,#name,comment)

#define INT_EVAR_H(name,val,comment)          /*declare one*/\
  INT_VARIABLE      name

#define BOOL_EVAR(name,val,comment)           /*make BOOL_VARIABLE*/\
  BOOL_VARIABLE     name(val,#name,comment)

#define BOOL_EVAR_H(name,val,comment)         /*declare one*/\
  BOOL_VARIABLE     name

#define STRING_EVAR(name,val,comment)         /*make STRING_VARIABLE*/\
  STRING_VARIABLE     name(val,#name,comment)

#define STRING_EVAR_H(name,val,comment)         /*declare one*/\
  STRING_VARIABLE     name

#define double_EVAR(name,val,comment)         /*make double_VARIABLE*/\
  double_VARIABLE     name(val,#name,comment)

#define double_EVAR_H(name,val,comment)         /*declare one*/\
  double_VARIABLE     name
#endif
