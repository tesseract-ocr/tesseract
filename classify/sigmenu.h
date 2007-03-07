/******************************************************************************
 **	Filename:	sigmenu.h
 **	Purpose:	Definition of signal handler routines
 **	Author:		Dan Johnson
 **	History:	10/2/89, DSJ, Created.
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
#ifndef   SIGMENU_H
#define   SIGMENU_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "cutil.h"
#include <signal.h>

/* functions to be placed in the signal menu look like: */
//typedef int   (*SIG_MENU_FUNC)(...);
/* the value returned from a SIG_MENU_FUNC must be one of the following */
#define SIG_RESUME      1
#define SIG_MENU      0

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void AddSignalMenuItem (int Signal,
int ItemNum,
const char ItemLabel[], int_void ItemFunc);
#endif
