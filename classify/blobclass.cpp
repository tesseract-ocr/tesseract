/******************************************************************************
 **      Filename:       blobclass.c
 **      Purpose:        High level blob classification and training routines.
 **      Author:         Dan Johnson
 **      History:        7/21/89, DSJ, Created.
 **
 **      (c) Copyright Hewlett-Packard Company, 1988.
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

/**----------------------------------------------------------------------------
      Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "blobclass.h"
#include "fxdefs.h"
#include "extract.h"
#include "efio.h"
#include "callcpp.h"
#include "chartoname.h"

#include <math.h>
#include <stdio.h>
#include <signal.h>

#define MAXFILENAME             80
#define MAXMATCHES              10

STRING_VAR(classify_font_name, "UnknownFont",
           "Default font name to be used in training");

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* name of current image file being processed */
extern char imagefile[];

/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/

/*---------------------------------------------------------------------------*/
void
LearnBlob (const STRING& filename,
           TBLOB * Blob, TEXTROW * Row, char BlobText[])
/*
 **      Parameters:
 **              Blob            blob whose micro-features are to be learned
 **              Row             row of text that blob came from
 **              BlobText        text that corresponds to blob
 **              TextLength      number of characters in blob
 **      Globals:
 **              imagefile       base filename of the page being learned
 **              classify_font_name
 **                              name of font currently being trained on
 **      Operation:
 **              Extract micro-features from the specified blob and append
 **              them to the appropriate file.
 **      Return: none
 **      Exceptions: none
 **      History: 7/28/89, DSJ, Created.
 */
#define TRAIN_SUFFIX    ".tr"
{
  static FILE *FeatureFile = NULL;
  STRING Filename(filename);
  CHAR_DESC CharDesc;
  LINE_STATS LineStats;

  EnterLearnMode;

  GetLineStatsFromRow(Row, &LineStats);

  CharDesc = ExtractBlobFeatures (Blob, &LineStats);
  if (CharDesc == NULL) {
    cprintf("LearnBLob: CharDesc was NULL. Aborting.\n");
    return;
  }

  // If no fontname was set, try to extract it from the filename
  char CurrFontName[32] = "";
  strncpy(CurrFontName, static_cast<STRING>(classify_font_name).string(), 32);
  if (!strcmp(CurrFontName, "UnknownFont")) {
    // filename is expected to be of the form [lang].[fontname].exp[num]
    // The [lang], [fontname] and [num] fields should not have '.' characters.
    const char *basename = strrchr(filename.string(), '/');
    const char *firstdot  = strchr(basename, '.');
    const char *lastdot  = strrchr(filename.string(), '.');
    if (firstdot != lastdot && firstdot != NULL && lastdot != NULL) {
      strncpy(CurrFontName, firstdot + 1, lastdot - firstdot - 1);
    }
  }

  // if a feature file is not yet open, open it
  // the name of the file is the name of the image plus TRAIN_SUFFIX
  if (FeatureFile == NULL) {
    Filename += TRAIN_SUFFIX;
    FeatureFile = Efopen (Filename.string(), "w");
    cprintf ("TRAINING ... Font name = %s\n", CurrFontName);
  }

  // label the features with a class name and font name
  fprintf (FeatureFile, "\n%s %s ", CurrFontName, BlobText);

  // write micro-features to file and clean up
  WriteCharDescription(FeatureFile, CharDesc);
  FreeCharDescription(CharDesc);

}                                // LearnBlob
