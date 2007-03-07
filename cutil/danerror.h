/******************************************************************************
 **	Filename:	danerror.h
 **	Purpose:	Definition of error trapping routines.
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
#ifndef   DANERROR_H
#define   DANERROR_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include <setjmp.h>

#define SetErrorTrap(Proc)  setjmp(PushErrorTrap(Proc))
#define NOERROR       0
#define DO_NOTHING      0

typedef int TRAPERROR;
typedef void (*VOID_PROC) ();

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void ReleaseErrorTrap(); 

void DoError(int Error, const char *Message); 

jmp_buf &PushErrorTrap(VOID_PROC Procedure); 
#endif
