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

#include          "mfcpch.h"     //precompiled headers
#include "serialis.h"
#include "scanutils.h"

/* **************************************************************************

These are the only routines that write/read data to/from the serialisation.

"serialise_bytes" and "de_serialise_bytes" are used to serialise NON class
items.  The "make_serialise" macro generates "serialise" and "de_serialise"
member functions for the class name specified in the macro parameter.

************************************************************************** */

DLLSYM void *de_serialise_bytes(FILE *f, int size) {
  void *ptr;

  ptr = alloc_mem (size);
  /*
  printf( "De_serialising bytes\n" );
  printf( "  Addr: %d    Size: %d\n", int(ptr), size );
  */
  if (fread (ptr, size, 1, f) != 1)
    READFAILED.error ("de_serialise_bytes", ABORT, NULL);
  return ptr;
}


DLLSYM void serialise_bytes(FILE *f, void *ptr, int size) {
  /*
  printf( "Serialising bytes\n" );
  printf( "  Addr: %d    Size: %d\n", int(ptr), size );
  */
  if (fwrite (ptr, size, 1, f) != 1)
    WRITEFAILED.error ("serialise_bytes", ABORT, NULL);
}


DLLSYM void serialise_INT32(FILE *f, inT32 the_int) {
  if (fprintf (f, INT32FORMAT "\n", the_int) < 0)
    WRITEFAILED.error ("serialise_INT32", ABORT, NULL);
}


DLLSYM inT32 de_serialise_INT32(FILE *f) {
  inT32 the_int;

  if (fscanf (f, INT32FORMAT, &the_int) != 1)
    READFAILED.error ("de_serialise_INT32", ABORT, NULL);
  return the_int;
}


DLLSYM void serialise_FLOAT64(FILE *f, double the_float) {
  if (fprintf (f, "%g\n", the_float) < 0)
    WRITEFAILED.error ("serialise_FLOAT64", ABORT, NULL);
}


DLLSYM double de_serialise_FLOAT64(FILE *f) {
  double the_float;

#ifndef _MSC_VER
  if (tess_fscanf (f, "%lg", &the_float) != 1)
#else
  if (fscanf (f, "%lg", &the_float) != 1)
#endif
    READFAILED.error ("de_serialise_FLOAT64", ABORT, NULL);
  return the_float;
}

// Byte swap an inT64 or uinT64.
DLLSYM uinT64 reverse64(uinT64 num) {
  return ((uinT64)reverse32((uinT32)(num & 0xffffffff)) << 32)
    | reverse32((uinT32)((num >> 32) & 0xffffffff));
}

/**********************************************************************
 * reverse32
 *
 * Byte swap an inT32 or uinT32.
 **********************************************************************/

DLLSYM uinT32 reverse32(            //switch endian
                        uinT32 num  //number to fix
                       ) {
  return (reverse16 ((uinT16) (num & 0xffff)) << 16)
    | reverse16 ((uinT16) ((num >> 16) & 0xffff));
}


/**********************************************************************
 * reverse16
 *
 * Byte swap an inT16 or uinT16.
 **********************************************************************/

DLLSYM uinT16 reverse16(            //switch endian
                        uinT16 num  //number to fix
                       ) {
  return ((num & 0xff) << 8) | ((num >> 8) & 0xff);
}
