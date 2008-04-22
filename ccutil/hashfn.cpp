/**********************************************************************
 * File:        hashfn.c  (Formerly hash.c)
 * Description: Simple hash function.
 * Author:					Ray Smith
 * Created:					Thu Jan 16 11:47:59 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
#include          "hashfn.h"

/**********************************************************************
 * hash
 *
 * Simple hash function working on power of 2 table sizes.
 * Uses xor function. Needs linear rehash.
 **********************************************************************/

inT32 hash(               //hash function
           inT32 bits,    //bits in hash function
           void *key,     //key to hash
           inT32 keysize  //size of key
          ) {
  inT32 bitindex;                //current bit count
  uinT32 keybits;                //bit buffer
  uinT32 hcode;                  //current hash code
  uinT32 mask;                   //bit mask

  mask = (1 << bits) - 1;
  keysize *= 8;                  //in bits
  bitindex = 0;
  keybits = 0;
  hcode = 0;
  do {
    while (keysize > 0 && bitindex <= 24) {
      keybits |= *((uinT8 *) key) << bitindex;
      key = (uinT8 *) key + 1;
      bitindex += 8;
      keysize -= 8;
    }
    hcode ^= keybits & mask;     //key new key
    keybits >>= bits;
  }
  while (keysize > 0);
  return hcode;                  //initial hash
}
