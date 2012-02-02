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
#include "extract.h"
#include "efio.h"
#include "featdefs.h"
#include "callcpp.h"
#include "chartoname.h"

#include <math.h>
#include <stdio.h>
#include <signal.h>

#define MAXFILENAME             80
#define MAXMATCHES              10

static const char kUnknownFontName[] = "UnknownFont";

STRING_VAR(classify_font_name, kUnknownFontName,
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
void LearnBlob(const FEATURE_DEFS_STRUCT &FeatureDefs, const STRING& filename,
               TBLOB * Blob, const DENORM& denorm, const char* BlobText) {
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
  static FILE *FeatureFile = NULL;
  STRING Filename(filename);

  // If no fontname was set, try to extract it from the filename
  STRING CurrFontName = classify_font_name;
  if (CurrFontName == kUnknownFontName) {
    // filename is expected to be of the form [lang].[fontname].exp[num]
    // The [lang], [fontname] and [num] fields should not have '.' characters.
    const char *basename = strrchr(filename.string(), '/');
    const char *firstdot = strchr(basename ? basename : filename.string(), '.');
    const char *lastdot  = strrchr(filename.string(), '.');
    if (firstdot != lastdot && firstdot != NULL && lastdot != NULL) {
      ++firstdot;
      CurrFontName = firstdot;
      CurrFontName[lastdot - firstdot] = '\0';
    }
  }

  // if a feature file is not yet open, open it
  // the name of the file is the name of the image plus TRAIN_SUFFIX
  if (FeatureFile == NULL) {
    Filename += TRAIN_SUFFIX;
    FeatureFile = Efopen(Filename.string(), "wb");
    cprintf("TRAINING ... Font name = %s\n", CurrFontName.string());
  }

  LearnBlob(FeatureDefs, FeatureFile, Blob, denorm, BlobText,
            CurrFontName.string());
}                                // LearnBlob

void LearnBlob(const FEATURE_DEFS_STRUCT &FeatureDefs, FILE* FeatureFile,
               TBLOB* Blob, const DENORM& denorm,
               const char* BlobText, const char* FontName) {
  CHAR_DESC CharDesc;

  ASSERT_HOST(FeatureFile != NULL);

  CharDesc = ExtractBlobFeatures(FeatureDefs, denorm, Blob);
  if (CharDesc == NULL) {
    cprintf("LearnBLob: CharDesc was NULL. Aborting.\n");
    return;
  }

  if (ValidCharDescription(FeatureDefs, CharDesc)) {
    // label the features with a class name and font name
    fprintf(FeatureFile, "\n%s %s\n", FontName, BlobText);

    // write micro-features to file and clean up
    WriteCharDescription(FeatureDefs, FeatureFile, CharDesc);
  } else {
    tprintf("Blob learned was invalid!\n");
  }
  FreeCharDescription(CharDesc);

}                                // LearnBlob
