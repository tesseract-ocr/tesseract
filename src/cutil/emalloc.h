/******************************************************************************
 **	Filename:	emalloc.h
 **	Purpose:	Definition of memory allocation routines.
 **	Author:		Dan Johnson
 **	History:	4/3/89, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
#ifndef   EMALLOC_H
#define   EMALLOC_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "host.h"
#include "callcpp.h"

#define NOTENOUGHMEMORY   2000
#define ILLEGALMALLOCREQUEST  2001

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void *Emalloc(int Size);

void *Erealloc(void *ptr, int size);

void Efree(void *ptr);


/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/

#endif
