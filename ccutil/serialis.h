/**********************************************************************
 * File:        serialis.h  (Formerly serialmac.h)
 * Description: Inline routines and macros for serialisation functions
 * Author:      Phil Cheatle
 * Created:     Tue Oct 08 08:33:12 BST 1991
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef SERIALIS_H
#define SERIALIS_H

#include          <stdlib.h>
#include          <string.h>
#include          <stdio.h>
#include "memry.h"
#include "errcode.h"
#include "fileerr.h"

/* **************************************************************************

These are the only routines that write/read data to/from the serialisation.

"serialise_bytes" and "de_serialise_bytes" are used to serialise NON class
items.  The "make_serialise" macro generates "serialise" and "de_serialise"
member functions for the class name specified in the macro parameter.

************************************************************************** */

extern DLLSYM void *de_serialise_bytes(FILE *f, int size);
extern DLLSYM void serialise_bytes(FILE *f, void *ptr, int size);
extern DLLSYM void serialise_INT32(FILE *f, inT32 the_int);
extern DLLSYM inT32 de_serialise_INT32(FILE *f);
extern DLLSYM void serialise_FLOAT64(FILE *f, double the_float);
extern DLLSYM double de_serialise_FLOAT64(FILE *f);
// Switch endinan.
extern DLLSYM uinT64 reverse64(uinT64);
extern DLLSYM uinT32 reverse32(uinT32);
extern DLLSYM uinT16 reverse16(uinT16);

/***********************************************************************
  QUOTE_IT   MACRO DEFINITION
  ===========================
Replace <parm> with "<parm>".  <parm> may be an arbitrary number of tokens
***********************************************************************/

#define QUOTE_IT( parm ) #parm

#define make_serialise( CLASSNAME )                                            \
                                                                               \
  NEWDELETE2(CLASSNAME)                                                        \
                                                                               \
void            serialise(                                                     \
  FILE*          f)                                                            \
{                                                                              \
  CLASSNAME*      shallow_copy;                                                \
                                                                               \
  shallow_copy = (CLASSNAME*) alloc_struct( sizeof( *this ) );                 \
    memmove( shallow_copy, this, sizeof( *this ) );                            \
                                                                               \
  shallow_copy->prep_serialise();                                              \
    if (fwrite( (char*) shallow_copy, sizeof( *shallow_copy ), 1, f ) != 1)    \
    WRITEFAILED.error( QUOTE_IT( CLASSNAME::serialise ),                       \
                  ABORT, NULL );                                               \
                                                                               \
  free_struct( shallow_copy, sizeof( *this ) );                                \
    this->dump( f );                                                           \
}                                                                              \
                                                                               \
  static CLASSNAME*    de_serialise(                                           \
  FILE*          f)                                                            \
{                                                                              \
  CLASSNAME*      restored;                                                    \
                                                                               \
    restored = (CLASSNAME*) alloc_struct( sizeof( CLASSNAME ) );               \
    if (fread( (char*) restored, sizeof( CLASSNAME ), 1, f ) != 1)             \
    READFAILED.error( QUOTE_IT( CLASSNAME::de_serialise ),                     \
                  ABORT, NULL );                                               \
                                                                               \
  restored->de_dump( f );                                                      \
    return restored;                                                           \
}
#endif
