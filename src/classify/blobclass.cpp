/******************************************************************************
 **      Filename:       blobclass.c
 **      Purpose:        High level blob classification and training routines.
 **      Author:         Dan Johnson
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

#include <cstdio>

#include "classify.h"
#include "featdefs.h"
#include "mf.h"
#include "normfeat.h"

static const char kUnknownFontName[] = "UnknownFont";

static STRING_VAR(classify_font_name, kUnknownFontName,
                  "Default font name to be used in training");

namespace tesseract {
/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/

// Finds the name of the training font and returns it in fontname, by cutting
// it out based on the expectation that the filename is of the form:
// /path/to/dir/[lang].[fontname].exp[num]
// The [lang], [fontname] and [num] fields should not have '.' characters.
// If the global parameter classify_font_name is set, its value is used instead.
void ExtractFontName(const char* filename, STRING* fontname) {
  *fontname = classify_font_name;
  if (*fontname == kUnknownFontName) {
    // filename is expected to be of the form [lang].[fontname].exp[num]
    // The [lang], [fontname] and [num] fields should not have '.' characters.
    const char *basename = strrchr(filename, '/');
    const char *firstdot = strchr(basename ? basename : filename, '.');
    const char *lastdot  = strrchr(filename, '.');
    if (firstdot != lastdot && firstdot != nullptr && lastdot != nullptr) {
      ++firstdot;
      *fontname = firstdot;
      fontname->truncate_at(lastdot - firstdot);
    }
  }
}


/*---------------------------------------------------------------------------*/

// Extracts features from the given blob and saves them in the tr_file_data_
// member variable.
// fontname:  Name of font that this blob was printed in.
// cn_denorm: Character normalization transformation to apply to the blob.
// fx_info:   Character normalization parameters computed with cn_denorm.
// blob_text: Ground truth text for the blob.
void Classify::LearnBlob(const STRING& fontname, TBLOB* blob,
                         const DENORM& cn_denorm,
                         const INT_FX_RESULT_STRUCT& fx_info,
                         const char* blob_text) {
  CHAR_DESC CharDesc = NewCharDescription(feature_defs_);
  CharDesc->FeatureSets[0] = ExtractMicros(blob, cn_denorm);
  CharDesc->FeatureSets[1] = ExtractCharNormFeatures(fx_info);
  CharDesc->FeatureSets[2] = ExtractIntCNFeatures(*blob, fx_info);
  CharDesc->FeatureSets[3] = ExtractIntGeoFeatures(*blob, fx_info);

  if (ValidCharDescription(feature_defs_, CharDesc)) {
    // Label the features with a class name and font name.
    tr_file_data_ += "\n";
    tr_file_data_ += fontname;
    tr_file_data_ += " ";
    tr_file_data_ += blob_text;
    tr_file_data_ += "\n";

    // write micro-features to file and clean up
    WriteCharDescription(feature_defs_, CharDesc, &tr_file_data_);
  } else {
    tprintf("Blob learned was invalid!\n");
  }
  FreeCharDescription(CharDesc);
}                                // LearnBlob

// Writes stored training data to a .tr file based on the given filename.
// Returns false on error.
bool Classify::WriteTRFile(const char* filename) {
  bool result = false;
  std::string tr_filename = filename;
  tr_filename += ".tr";
  FILE* fp = fopen(tr_filename.c_str(), "wb");
  if (fp) {
    result =
      tesseract::Serialize(fp, &tr_file_data_[0], tr_file_data_.length());
    fclose(fp);
  }
  tr_file_data_.truncate_at(0);
  return result;
}

}  // namespace tesseract.
