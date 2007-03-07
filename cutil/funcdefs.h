/******************************************************************************
 **	Filename:    funcdefs.h
 **	Purpose:     Definition of function types for passing as params.
 **	Author:      Dan Johnson
 **	History:     Fri Sep 14 10:04:47 1990, DSJ, Created.
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
#ifndef FUNCDEFS_H
#define FUNCDEFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "general.h"

typedef INT8 (*INT8_FUNC) ();
typedef UINT8 (*UINT8_FUNC) ();
typedef INT16 (*INT16_FUNC) ();
typedef UINT16 (*UINT16_FUNC) ();
typedef INT32 (*INT32_FUNC) ();
typedef UINT32 (*UINT32_FUNC) ();
typedef FLOAT32 (*FLOAT32_FUNC) ();
typedef FLOAT64 (*FLOAT64_FUNC) ();
typedef PINT8 (*PINT8_FUNC) ();
typedef PUINT8 (*PUINT8_FUNC) ();
typedef PINT16 (*PINT16_FUNC) ();
typedef PUINT16 (*PUINT16_FUNC) ();
typedef PINT32 (*PINT32_FUNC) ();
typedef PUINT32 (*PUINT32_FUNC) ();
typedef PFLOAT32 (*PFLOAT32_FUNC) ();
typedef PFLOAT64 (*PFLOAT64_FUNC) ();

typedef CHAR (*CHAR_FUNC) ();
typedef BOOL8 (*BOOL8_FUNC) ();
typedef int (*INT_FUNC) ();
typedef void (*VOID_FUNC) ();

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#endif
