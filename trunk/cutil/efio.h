/******************************************************************************
 **	Filename:	efio.h
 **	Purpose:	Definition of file I/O routines
 **	Author:		Dan Johnson
 **	History:	5/21/89, DSJ, Created.
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
#ifndef   EFIO_H
#define   EFIO_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include <stdio.h>

#define FOPENERROR      3000

/**----------------------------------------------------------------------------
          Public Function Prototype
----------------------------------------------------------------------------**/
FILE *Efopen(const char *Name, const char *Mode); 
#endif
