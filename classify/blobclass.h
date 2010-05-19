/******************************************************************************
 **	Filename:	blobclass.h
 **	Purpose:	Interface to high level classification and training.
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
#ifndef   BLOBCLASS_H
#define   BLOBCLASS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "oldlist.h"
#include "tessclas.h"

/*---------------------------------------------------------------------------
          Macros
----------------------------------------------------------------------------*/
/* macros for controlling the display of recognized characters */
#define EnableCharDisplay()   (DisplayCharacters = TRUE)
#define DisableCharDisplay()    (DisplayCharacters = FALSE)

/* macros for controlling the display of the entire match list */
#define EnableMatchDisplay()    (DisplayMatchList = TRUE)
#define DisableMatchDisplay()   (DisplayMatchList = FALSE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void LearnBlob(const STRING& filename,
               TBLOB * Blob, TEXTROW * Row, const char* BlobText);

void LearnBlob(FILE* File, TBLOB* Blob, TEXTROW* Row,
               const char* BlobText, const char* FontName);

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/*parameter used to turn on/off output of recognized chars to the screen */
#endif
