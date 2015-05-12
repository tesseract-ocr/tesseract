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
#include "strngs.h"

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
namespace tesseract {
// Finds the name of the training font and returns it in fontname, by cutting
// it out based on the expectation that the filename is of the form:
// /path/to/dir/[lang].[fontname].exp[num]
// The [lang], [fontname] and [num] fields should not have '.' characters.
// If the global parameter classify_font_name is set, its value is used instead.
void ExtractFontName(const STRING& filename, STRING* fontname);

}  // namespace tesseract.

#endif
