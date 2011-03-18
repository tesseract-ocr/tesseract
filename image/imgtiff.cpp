/**********************************************************************
 * File:        imgtiff.c  (Formerly tiff.c)
 * Description: Max format image reader/writer.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 14:00:21 BST 1990
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

#include <stdio.h>
/*
** Include automatically generated configuration file if running autoconf
*/
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#if defined(MOTOROLA_BYTE_ORDER) || defined(WORDS_BIGENDIAN)
#define __MOTO__  // Big-endian.
#endif
#endif

#include          "imgtiff.h"
#include          "helpers.h"

#define INTEL       0x4949
#define MOTO        0x4d4d

/*************************************************************************
 * NOTE ON BIG-ENDIAN vs LITTLE-ENDIAN
 *
 * Intel machines store numbers with LSByte in the left position.
 * Motorola	(and PA_RISC) machines use the opposite byte ordering.
 *
 * This code is written so that:
 *   a) it will compile and run on EITHER machine type   AND
 *   b) the program (on either machine) will process tiff file written in either
 *      Motorola or Intel format.
 *
 * The code is compiled with a __NATIVE__ define which is either MOTO or INTEL.
 * MOTO and INTEL are defined (above) to be the value of the first two bytes of
 * a tiff file in either format. (This identifies the filetype).
 *
 * Subsequent reads and writes normally just reverse the byte order if the
 * machine type (__NATIVE__) is not equal to the filetype determined from the
 * first two bytes of the tiff file.
 *
 * A special case is the "value" field of the tag structure. This can contain
 * EITHER a 16bit or a 32bit value. According to the "type" field. The 4 cases
 * of machine type / file type combinations need to be treated differently in
 * the case of 16 bit values
 *************************************************************************/

#define ENTRIES       19         /*no of entries */
#define START       8            /*start of tag table */

typedef struct
{
  uinT16 tag;                    //entry tag
  uinT16 type;
  uinT32 length;
  inT32 value;
} TIFFENTRY;                     //tiff tag entry


// CountTiffPages
// Returns the number of pages in the file if it is a tiff file, otherwise 0.
// WARNING: requires __MOTO__ to be #defined on a big-endian system.
// On linux this is handled by configure - see above.
int CountTiffPages(FILE* fp) {
  if (fp == NULL) return 0;
  // Read header
  inT16 filetype = 0;
  if (fread(&filetype, sizeof(filetype), 1, fp) != 1 ||
      (filetype != INTEL && filetype != MOTO)) {
    return 0;
  }
  fseek(fp, 4L, SEEK_SET);
  int npages = 0;
  do {
    inT32 start;                   // Start of tiff directory.
    if (fread(&start, sizeof(start), 1, fp) != 1) {
      return npages;
    }
    if (filetype != __NATIVE__)
      ReverseN(&start, sizeof(start));
    if (start <= 0) {
      return npages;
    }
    fseek(fp, start, SEEK_SET);
    inT16 entries;                 // No of tiff entries.
    if (fread(&entries, sizeof(entries), 1, fp) != 1) {
      return npages;
    }
    if (filetype != __NATIVE__)
      ReverseN(&entries, sizeof(entries));
    // Skip the tags and get to the next start.
    fseek(fp, entries * sizeof(TIFFENTRY), SEEK_CUR);
    ++npages;
  } while (1);
  return 0;
}

