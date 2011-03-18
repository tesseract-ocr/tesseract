/**********************************************************************
 * File:        tfacep.h  (Formerly tfacep.h)
 * Description: Declarations of C functions and C owned data.
 * Author:					Ray Smith
 * Created:					Mon Apr 27 12:51:28 BST 1992
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

#ifndef TFACEP_H
#define TFACEP_H

#include "hosthplb.h"
#include "blobs.h"
#include "tessarray.h"
#include "notdll.h"
#include "oldlist.h"
#include "permute.h"
#include "blobclass.h"
#include "stopper.h"
#include "associate.h"
#include "chop.h"
#include "structures.h"

typedef void (*TESS_TESTER) (TBLOB *, BOOL8, char *, inT32, LIST);
typedef LIST (*TESS_MATCHER) (TBLOB *, TBLOB *, TBLOB *);

#endif
