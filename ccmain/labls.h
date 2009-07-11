/**********************************************************************
 * File:        labls.h  (Formerly labels.h)
 * Description: Attribute definition tables
 * Author:					Sheelagh Lloyd?
 * Created:
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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
#ifndef           LABLS_H
#define           LABLS_H

#include          "host.h"
#include          "hpdsizes.h"

#include          "hpddef.h"     //must be last (handpd.dll)

extern DLLSYM inT32 tn[NUM_TEXT_ATTR];

extern DLLSYM char tlabel[NUM_TEXT_ATTR][4][MAXLENGTH];

extern DLLSYM inT32 bn[NUM_BLOCK_ATTR];

extern DLLSYM inT32 tvar[NUM_TEXT_ATTR];
extern DLLSYM inT32 bvar[NUM_BLOCK_ATTR];
extern DLLSYM char blabel[NUM_BLOCK_ATTR][4][MAXLENGTH];

extern DLLSYM char backlabel[NUM_BACKGROUNDS][MAXLENGTH];
#endif
