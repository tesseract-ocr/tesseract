/**********************************************************************
 * File:        blobcmp.c  (Formerly blobcmp.c)
 * Description: Code to compare blobs using the adaptive matcher.
 * Author:		Ray Smith
 * Created:		Wed Apr 21 09:28:51 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include "mfcpch.h"
#include "fxdefs.h"
#include "ocrfeatures.h"
#include "intmatcher.h"
#include "intproto.h"
#include "adaptive.h"
#include "adaptmatch.h"
#include "const.h"
#include "tessvars.h"
#include "tesseractclass.h"

#define CMP_CLASS       0

/**********************************************************************
 * compare_tess_blobs
 *
 * Match 2 blobs using the adaptive classifier.
 **********************************************************************/
namespace tesseract {
float Tesseract::compare_tess_blobs(TBLOB *blob1,
                                    TEXTROW *row1,
                                    TBLOB *blob2,
                                    TEXTROW *row2) {
  int fcount;                    /*number of features */
  ADAPT_CLASS adapted_class;
  ADAPT_TEMPLATES ad_templates;
  LINE_STATS line_stats1, line_stats2;
  INT_FEATURE_ARRAY int_features;
  FEATURE_SET float_features;
  INT_RESULT_STRUCT int_result;  /*output */

  BIT_VECTOR AllProtosOn = NewBitVector (MAX_NUM_PROTOS);
  BIT_VECTOR AllConfigsOn = NewBitVector (MAX_NUM_CONFIGS);
  set_all_bits (AllProtosOn, WordsInVectorOfSize (MAX_NUM_PROTOS));
  set_all_bits (AllConfigsOn, WordsInVectorOfSize (MAX_NUM_CONFIGS));

  EnterClassifyMode;
  ad_templates = NewAdaptedTemplates (false);
  GetLineStatsFromRow(row1, &line_stats1);
                                 /*copy baseline stuff */
  GetLineStatsFromRow(row2, &line_stats2);
  adapted_class = NewAdaptedClass ();
  AddAdaptedClass (ad_templates, adapted_class, CMP_CLASS);
  InitAdaptedClass(blob1, &line_stats1, CMP_CLASS, adapted_class, ad_templates);
  fcount = GetAdaptiveFeatures (blob2, &line_stats2,
    int_features, &float_features);
  if (fcount > 0) {
    SetBaseLineMatch();
    IntegerMatcher (ClassForClassId (ad_templates->Templates, CMP_CLASS),
      AllProtosOn, AllConfigsOn, fcount, fcount,
      int_features, 0, &int_result, testedit_match_debug);
    FreeFeatureSet(float_features);
    if (int_result.Rating < 0)
      int_result.Rating = MAX_FLOAT32;
  }

  free_adapted_templates(ad_templates);
  FreeBitVector(AllConfigsOn);
  FreeBitVector(AllProtosOn);

  return fcount > 0 ? int_result.Rating * fcount : MAX_FLOAT32;
}
}  // namespace tesseract
