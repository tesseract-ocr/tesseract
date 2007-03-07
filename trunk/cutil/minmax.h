/******************************************************************************
 **	Filename:    minmax.h
 **	Purpose:     Utility macros for min and max functions.
 **	Author:      Dan Johnson
 **	History:     Wed Oct 17 09:54:32 1990, DSJ, Created.
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
#ifndef MINMAX_H
#define MINMAX_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/

#ifndef MAX
#define MAX(x,y)    (((x)>=(y))?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y)    (((x)<=(y))?(x):(y))
#endif

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#endif
