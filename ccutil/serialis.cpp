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
