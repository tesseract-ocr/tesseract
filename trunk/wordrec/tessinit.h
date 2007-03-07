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

/*---------------------------------------------------------------------------
        Public Function Prototypes
----------------------------------------------------------------------------*/
void program_variables(); 

void program_init(); 

void program_init _ARGS ((void));

void program_variables _ARGS ((void));

#undef _ARGS

//extern int imagescale;
//extern int edgescale;

extern int corner_0;
extern int corner_1;
extern int corner_2;
extern int corner_3;

extern int row_number;

extern FILE *boxfp;
#endif
