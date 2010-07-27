/* -*-C-*-
 ********************************************************************************
 *
 * File:        fxid.h  (Formerly fxid.h)
 * Description:  Feature extractor related includes
 * Author:       Mark Seaman, OCR Technology
 * Created:      Thu Oct 19 14:59:51 1989
 * Modified:     Thu Jan 31 16:57:07 1991 (Dan Johnson) danj@hpgrlj
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1989, Hewlett-Packard Company.
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
 ********************************************************************************

 ***************************************************************************
 **********                    A T T E N T I O N                    ********
 ***************************************************************************

This module is divided into two sections the declarations for this module
(i.e. the function signatures) are listed in 'fxid.h'.  The definitions
(i.e. the actual code and variables) are listed in 'fxid1.c' and 'fxid2.c'.
The appropriate piece of code you need for your application should be
included in your top level program file.

*/

#ifndef FXID_H
#define FXID_H

#include "extern.h"
#include "const.h"
#include "tessclas.h"
#include "oldlist.h"

#define FEATURE_MATCHER    1     /**< Use micro-features */

#define WO_UNSCALED        0     /**< first square scaled fx */
#define STATISTICAL_WO     1     /**< new wo */
#define MICRO_FEATURES     2     /**< microfeature extractor */
#define WO_SCALED          3     /**< wiseowl scaled to baseline */
#define MAX_FX             3     /**< no of working fx-ers */
#define NO_FUNCTION        0     /**< special value for nothing */

/* This file contains declarations of the top-level feature
extractor functions as used by the Classify process*/

typedef LIST (*LISTFUNC) ();

//extern FUNCPTR             word_matchers[MAX_FX];

//extern LISTFUNC            blob_matchers[MAX_FX];

//extern FUNCPTR             feature_learners[MAX_FX];

extern char fx_ids[MAX_FX];      /*one-char ids */

extern char *fx_names[MAX_FX];
#endif
