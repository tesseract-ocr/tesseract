/******************************************************************************
 ** Filename:    mf.c
 ** Purpose:     Micro-feature interface to flexible feature extractor.
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
#include "mf.h"

#include "featdefs.h"
#include "mfdefs.h"
#include "mfx.h"

#include <cmath>

namespace tesseract {

/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/
/**
 * Call the old micro-feature extractor and then copy
 * the features into the new format.  Then deallocate the
 * old micro-features.
 * @param Blob  blob to extract micro-features from
 * @param cn_denorm  control parameter to feature extractor.
 * @return Micro-features for Blob.
 */
FEATURE_SET ExtractMicros(TBLOB *Blob, const DENORM &cn_denorm) {
  auto features = BlobMicroFeatures(Blob, cn_denorm);
  if (features.empty()) {
    return nullptr;
  }
  int n = 0;
  for ([[maybe_unused]] auto &f: features) {
    ++n;
  }
  auto FeatureSet = new FEATURE_SET_STRUCT(n);

  for (auto &f : features) {
    auto Feature = new FEATURE_STRUCT(&MicroFeatureDesc);
    for (int i = 0; i < (int)MicroFeatureParameter::MFCount; ++i)
      Feature->Params[i] = f[i];
    // Bulge features are deprecated and should not be used. Set to 0.
    Feature->Params[(int)MicroFeatureParameter::MFBulge1] = 0.0f;
    Feature->Params[(int)MicroFeatureParameter::MFBulge2] = 0.0f;

#ifndef _WIN32
    // Assert that feature parameters are well defined.
    for (int i = 0; i < Feature->Type->NumParams; i++) {
      ASSERT_HOST(!std::isnan(Feature->Params[i]));
    }
#endif

    AddFeature(FeatureSet, Feature);
  }
  return FeatureSet;
} /* ExtractMicros */

} // namespace tesseract
