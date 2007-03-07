/******************************************************************************
 **	Filename:	mfx.h
 **	Purpose:	Definition of micro-feature extraction routines
 **	Author:		Dan Johnson
 **	History:	5/29/89, DSJ, Created.
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
#ifndef   MFX_H
#define   MFX_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "fxdefs.h"

extern FLOAT32 MinSlope;
extern FLOAT32 MaxSlope;
/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void InitMicroFxVars(); 

CHAR_FEATURES BlobMicroFeatures(TBLOB *Blob, LINE_STATS *LineStats); 

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* mfx.c
void InitMicroFxVars
  _ARGS((void));

CHAR_FEATURES BlobMicroFeatures
  _ARGS((BLOB *Blob,
  LINE_STATS *LineStats));

#undef _ARGS
*/
#endif
