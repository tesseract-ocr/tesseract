/******************************************************************************
 **	Filename:	General.h
 **	Purpose:	this is the system independent typedefs and defines
 **	Author:		Mike Niquette / Dan Johnson
 **	History:	Creation Date:			09/13/1988, MLN
 **			Added UNIX:			11/10/88, DSJ
 **			Changed name to General.h	11/24/88, DSJ
 **			Added BOOL, CHAR, TRUE, FALSE,	11/24/88, DSJ
 **				STATUS
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
#ifndef   GENERAL_H
#define   GENERAL_H

#include "host.h"

typedef char CHAR;
typedef int STATUS;

#ifndef NULL
#define NULL        0
#endif
#endif
