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
#include "variables.h"
#include "extract.h"
#include "efio.h"
#include "callcpp.h"
#include "chartoname.h"

#include <math.h>
#include <stdio.h>
#include <signal.h>

#define MAXFILENAME             80
#define MAXMATCHES              10

// define default font name to be used in training
#define FONT_NAME       "UnknownFont"

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* name of current image file being processed */
extern char imagefile[];

/* parameters used to control the training process */
static const char *FontName = FONT_NAME;

/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void InitBlobClassifierVars() {
/*
 **      Parameters: none
 **      Globals:
 **              FontName        name of font being trained on
 **      Operation: Install blob classifier variables into the wiseowl
 **              variable system.
 **      Return: none
 **      Exceptions: none
 **      History: Fri Jan 19 16:13:33 1990, DSJ, Created.
 */
  VALUE dummy;

  string_variable (FontName, "FontName", FONT_NAME);

}                                /* InitBlobClassifierVars */


/*---------------------------------------------------------------------------*/
void
LearnBlob (TBLOB * Blob, TEXTROW * Row, char BlobText[])
/*
 **      Parameters:
 **              Blob            blob whose micro-features are to be learned
 **              Row             row of text that blob came from
 **              BlobText        text that corresponds to blob
 **              TextLength      number of characters in blob
 **      Globals:
 **              imagefile       base filename of the page being learned
 **              FontName        name of font currently being trained on
 **      Operation:
 **              Extract micro-features from the specified blob and append
 **              them to the appropriate file.
 **      Return: none
 **      Exceptions: none
 **      History: 7/28/89, DSJ, Created.
 */
#define MAXFILENAME     80
#define MAXCHARNAME     20
#define MAXFONTNAME     20
#define TRAIN_SUFFIX    ".tr"
{
  static FILE *FeatureFile = NULL;
  char Filename[MAXFILENAME];
  CHAR_DESC CharDesc;
  LINE_STATS LineStats;

  EnterLearnMode;

  GetLineStatsFromRow(Row, &LineStats);

  CharDesc = ExtractBlobFeatures (Blob, &LineStats);

  // if a feature file is not yet open, open it
  // the name of the file is the name of the image plus TRAIN_SUFFIX
  if (FeatureFile == NULL) {
    strcpy(Filename, imagefile);
    strcat(Filename, TRAIN_SUFFIX);
    FeatureFile = Efopen (Filename, "w");

    cprintf ("TRAINING ... Font name = %s.\n", FontName);
  }

  // label the features with a class name and font name
  fprintf (FeatureFile, "\n%s %s ", FontName, BlobText);

  // write micro-features to file and clean up
  WriteCharDescription(FeatureFile, CharDesc);
  FreeCharDescription(CharDesc);

}                                // LearnBlob
