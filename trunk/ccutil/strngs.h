/**********************************************************************
 * File:        strngs.h  (Formerly strings.h)
 * Description: STRING class definition.
 * Author:					Ray Smith
 * Created:					Fri Feb 15 09:15:01 GMT 1991
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

#ifndef           STRNGS_H
#define           STRNGS_H

#include          <string.h>
#include          "memry.h"
#include          "serialis.h"

class DLLSYM STRING
{
  char *ptr;                     //ptr to the chars

  public:
    STRING() {  //constructor
      ptr = NULL;                //empty string
    }

    STRING(  //classwise copy
           const STRING &string) {
      if (string.ptr != NULL) {
                                 //length of source
        INT32 length = strlen (string.ptr) + 1;

                                 //get space
        ptr = alloc_string (length);
        strcpy (ptr, string.ptr);//and copy it
      }
      else {
        ptr = alloc_string (1);
        if (ptr != NULL)
          *ptr = '\0';
      }
    }

    STRING(  //contruct from char*
           const char *string) {
      if (string != NULL) {
                                 //length of source
        INT32 length = strlen (string) + 1;

                                 //get space
        ptr = alloc_string (length);
        if (ptr != NULL)
          strcpy(ptr, string);  //and copy it
      }
      else {
        ptr = alloc_string (1);
        if (ptr != NULL)
          *ptr = '\0';
      }
    }

    ~STRING () {                 //destructor
      if (ptr != NULL)
        free_string(ptr);  //give it back
    }

    char &operator[] (           //access function
      INT32 index) const         //string index
    {
      return ptr[index];         //no bounds checks
    }

    BOOL8 contains(  //char in string?
                   const char c) const {
      if ((ptr == NULL) || ((c != '\0') && strchr (ptr, c) == NULL))
        return FALSE;
      else
        return TRUE;
    }

    INT32 length() const {  //string length
      if (ptr != NULL)
        return strlen (ptr);
      else
        return 0;
    }

    const char *string() const {  //ptr to string
      return ptr;
    }

    BOOL8 operator== (           //string equality
      const STRING & string) const
    {
      if (ptr != NULL && string.ptr != NULL)
        return strcmp (ptr, string.ptr) == 0;
      else
        return (ptr == NULL || *ptr == '\0')
          && (string.ptr == NULL || *(string.ptr) == '\0');
    }

    BOOL8 operator!= (           //string equality
      const STRING & string) const
    {
      if (ptr != NULL && string.ptr != NULL)
        return strcmp (ptr, string.ptr) != 0;
      else
        return !((ptr == NULL || *ptr == '\0')
          && (string.ptr == NULL || *(string.ptr) == '\0'));
    }

    BOOL8 operator!= (           //string equality
      const char *string) const
    {
      if (ptr != NULL && string != NULL)
        return strcmp (ptr, string) != 0;
      else
        return !((ptr == NULL || *ptr == '\0')
          && (string == NULL || *string == '\0'));
    }

    STRING & operator= (         //assignment
      const char *string);       //of char*

    STRING & operator= (         //assignment
    const STRING & string) {     //of string
      *this = string.ptr;        //as for char*
      return *this;
    }

    STRING operator+ (           //concatenation
      const STRING & string) const;

    STRING operator+ (           //char concatenation
      const char ch) const;

    STRING & operator+= (        //inplace cat
      const char *string);
    STRING & operator+= (        //inplace cat
    const STRING & string) {
      *this += string.ptr;
      return *this;
    }

    STRING & operator+= (        //inplace char cat
      const char ch);

    void prep_serialise() {  //set ptrs to counts
      ptr = (char *) (length () + 1);
    }

    void dump(  //write external bits
              FILE *f) {
      serialise_bytes (f, (void *) ptr, (int) (length () + 1));
    }

    void de_dump(  //read external bits
                 FILE *f) {
      char *instring;            //input from read

      instring = (char *) de_serialise_bytes (f, (ptrdiff_t) ptr);
      ptr = NULL;
      *this = instring;
      free_mem(instring);
    }

    make_serialise (STRING)
};
#endif
