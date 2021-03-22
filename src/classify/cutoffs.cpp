/******************************************************************************
 ** Filename:    cutoffs.c
 ** Purpose:     Routines to manipulate an array of class cutoffs.
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/

#include <cstdio>
#include <sstream> // for std::istringstream
#include <string>  // for std::string

#include <tesseract/unichar.h>
#include "classify.h"
#include "helpers.h"
#include "serialis.h"

#define MAX_CUTOFF 1000

namespace tesseract {
/**
 * Open file, read in all of the class-id/cutoff pairs
 * and insert them into the Cutoffs array.  Cutoffs are
 * indexed in the array by class id.  Unused entries in the
 * array are set to an arbitrarily high cutoff value.
 * @param fp file containing cutoff definitions
 * @param Cutoffs array to put cutoffs into
 */
void Classify::ReadNewCutoffs(TFile *fp, uint16_t *Cutoffs) {
  int Cutoff;

  if (shape_table_ != nullptr) {
    if (!fp->DeSerialize(shapetable_cutoffs_)) {
      tprintf("Error during read of shapetable pffmtable!\n");
    }
  }
  for (int i = 0; i < MAX_NUM_CLASSES; i++) {
    Cutoffs[i] = MAX_CUTOFF;
  }

  const int kMaxLineSize = 100;
  char line[kMaxLineSize];
  while (fp->FGets(line, kMaxLineSize) != nullptr) {
    std::string Class;
    CLASS_ID ClassId;
    std::istringstream stream(line);
    stream.imbue(std::locale::classic());
    stream >> Class >> Cutoff;
    if (stream.fail()) {
      break;
    }
    if (Class.compare("NULL") == 0) {
      ClassId = unicharset.unichar_to_id(" ");
    } else {
      ClassId = unicharset.unichar_to_id(Class.c_str());
    }
    ASSERT_HOST(ClassId >= 0 && ClassId < MAX_NUM_CLASSES);
    Cutoffs[ClassId] = Cutoff;
  }
}

} // namespace tesseract
