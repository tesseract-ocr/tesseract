/**********************************************************************
 * File:        strngs.c  (Formerly strings.c)
 * Description: STRING class functions.
 * Author:					Ray Smith
 * Created:					Fri Feb 15 09:13:30 GMT 1991
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
#include          "tprintf.h"
#include          "strngs.h"

/**********************************************************************
 * STRING::operator=
 *
 * Assign a char* to a STRING.
 **********************************************************************/

STRING & STRING::operator= (     //assign char*
const char *string               //string to copy
) {
  if (string != NULL) {
    INT32
      length = strlen (string) + 1;//length of source

    if (ptr == NULL)
                                 //get space
        ptr = alloc_string (length);
                                 //got wrong size
    else if (strlen (ptr) != (UINT32) length - 1) {
      free_string(ptr);  //free old space
                                 //get new space
      ptr = alloc_string (length);
    }
    if (ptr == NULL) {
      tprintf ("No memory to allocate string");
      return *this;
    }
    strcpy(ptr, string);  //copy string
  }
  else {
    if (ptr == NULL)
      ptr = alloc_string (1);    //get space
                                 //got wrong size
    else if (strlen (ptr) != 0) {
      free_string(ptr);  //free old space
      ptr = alloc_string (1);    //get new space
    }
    if (ptr == NULL) {
      tprintf ("No memory to allocate string");
      return *this;
    }
    *ptr = '\0';                 //copy string
  }

  return *this;
}


/**********************************************************************
 * STRING::operator+
 *
 * Concatenate 2 STRINGs.
 **********************************************************************/

STRING
STRING::operator+ (              //concatenation
const STRING & string            //second string
) const
{
  INT32 length;                  //length of 1st op
  STRING result;                 //concatenated string

  if (ptr == NULL)
    length = 0;
  else
    length = strlen (ptr);
  result.ptr = alloc_string (length + strlen (string.ptr) + 1);
  //total length
  if (result.ptr == NULL) {
    tprintf ("No memory to allocate string");
    return result;
  }
  result.ptr[0] = '\0';
  if (ptr != NULL)
    strcpy (result.ptr, ptr);
  if (string.ptr != NULL)
                                 //put together
    strcpy (result.ptr + length, string.ptr);
  return result;
}


/**********************************************************************
 * STRING::operator+
 *
 * Concatenate char to STRING.
 **********************************************************************/

STRING
STRING::operator+ (              //concatenation
const char ch                    //char
) const
{
  INT32 length;                  //length of 1st op
  STRING result;                 //concatenated string

  if (ptr == NULL)
    length = 0;
  else
    length = strlen (ptr);
                                 //total length
  result.ptr = alloc_string (length + 2);
  if (result.ptr == NULL) {
    tprintf ("No memory to allocate string");
    return result;
  }
  if (ptr != NULL)
    strcpy (result.ptr, ptr);
  result.ptr[length] = ch;       //put together
  result.ptr[length + 1] = '\0';
  return result;
}


/**********************************************************************
 * STRING::operator+=
 *
 * Concatenate 2 strings putting the result straing back in the first
 **********************************************************************/

STRING & STRING::operator+= (    //inplace cat
const char *string               //string to add
) {
  INT32
    length;                      //length of 1st op
  char *
    src;                         //source string

  if (string == NULL || string[0] == '\0')
    return *this;                //unchanged
  if (ptr == NULL)
    length = 0;
  else
    length = strlen (ptr);       //length of 1st op
  src = ptr;                     //temp copy
                                 //new length
  ptr = alloc_string (length + strlen (string) + 1);
  if (ptr == NULL) {
    tprintf ("No memory to allocate string");
    ptr = src;
    return *this;
  }
  if (src != NULL) {
    strcpy(ptr, src);  //copy old
    free_string(src);  //free old one
  }
  strcpy (ptr + length, string); //add new
  return *this;
}


/**********************************************************************
 * STRING::operatot+=
 *
 * Concatenate a char t a string putting the result straing back in the string
 **********************************************************************/

STRING & STRING::operator+= (    //inplace cat
const char ch                    //char to add
) {
  INT32
    length;                      //length of 1st op
  char *
    src;                         //source string

  if (ch == '\0')
    return *this;                //unchanged
  if (ptr == NULL)
    length = 0;
  else
    length = strlen (ptr);       //length of 1st op
  src = ptr;                     //temp copy
                                 //new length
  ptr = alloc_string (length + 2);
  if (ptr == NULL) {
    tprintf ("No memory to allocate string");
    ptr = src;
    return *this;
  }
  if (src != NULL) {
    strcpy(ptr, src);  //copy old
    free_string(src);  //free old one
  }
  ptr[length] = ch;              //add new char
  ptr[length + 1] = '\0';
  return *this;
}
