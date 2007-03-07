/******************************************************************************
**	Filename:    name2char.h
**	Purpose:     Routines to convert between classes and class names.
**	Author:      Dan Johnson
**	History:     Fri Feb 23 08:10:40 1990, DSJ, Created.
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
#ifndef   __NAME2CHAR__
#define   __NAME2CHAR__

/**----------------------------------------------------------------------------
					Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "matchdefs.h"


/**----------------------------------------------------------------------------
					Public Function Prototypes
----------------------------------------------------------------------------**/
CLASS_ID NameToChar (
     char	CharName[]);

void CharToName (
     CLASS_ID	Char,
     char	CharName[]);


#endif
