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

#ifndef           TFACEP_H
#define           TFACEP_H

#include          "hosthplb.h"
#include          "tessclas.h"
#include          "tessarray.h"
#include          "tstruct.h"
#include          "notdll.h"
#include "choices.h"
#include "oldlist.h"
#include "tface.h"
#include "permute.h"
#include "adaptmatch.h"
#include "blobclass.h"
#include "stopper.h"
#include "associate.h"
#include "chop.h"
#include "expandblob.h"
#include "tordvars.h"
#include "metrics.h"
#include "tface.h"
#include "badwords.h"
#include "structures.h"

typedef void (*TESS_TESTER) (TBLOB *, BOOL8, char *, inT32, LIST);
typedef LIST (*TESS_MATCHER) (TBLOB *, TBLOB *, TBLOB *, void *, TEXTROW *);

extern TEXTROW normalized_row;
extern int display_ratings;

#if 0
#define strsave(s)    \
	((s) ?  \
	((char*) strcpy ((char*)alloc_string (strlen(s)+1), s))  :  \
	(NULL))
#endif

#define BOLD_ON				"&dB(s3B"
#define BOLD_OFF			"&d@(s0B"
#define UNDERLINE_ON		"&dD"
#define UNDERLINE_OFF		"&d@"
#endif
