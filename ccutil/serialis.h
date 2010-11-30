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

#endif
