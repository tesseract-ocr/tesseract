/**********************************************************************
 * File:        tessinit.h  (Formerly tessinit.h)
 * Description: Stuff from recog needed by tessedit.
 * Author:		Ray Smith
 * Created:		Thu Jan 23 09:36:25 GMT 1992
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
#ifndef TESSINIT_H
#define TESSINIT_H

#include "varable.h"

#undef _ARGS
extern int row_number;
extern FILE *boxfp;

/*---------------------------------------------------------------------------
        Variables
----------------------------------------------------------------------------*/
extern INT_VAR_H(wordrec_corner_0, "corner_0", 0);
extern INT_VAR_H(wordrec_corner_1, "corner_1", 0);
extern INT_VAR_H(wordrec_corner_2, "corner_2", 2550);
extern INT_VAR_H(wordrec_corner_3, "corner_3", 3508);

#endif
