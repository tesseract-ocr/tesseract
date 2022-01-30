///////////////////////////////////////////////////////////////////////
// File:        par_control.cpp
// Description: Control code for parallel implementation.
// Author:      Ray Smith
//
// (C) Copyright 2013, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#include "tesseractclass.h"
#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

namespace tesseract {

struct BlobData {
  BlobData() = default;
  BlobData(int index, Tesseract *tess, const WERD_RES &word)
      : blob(word.chopped_word->blobs[index])
      , tesseract(tess)
      , choices(&(*word.ratings)(index, index)) {}

  TBLOB *blob = nullptr;
  Tesseract *tesseract = nullptr;
  BLOB_CHOICE_LIST **choices = nullptr;
};

void Tesseract::PrerecAllWordsPar(const std::vector<WordData> &words) {
  // Prepare all the blobs.
  std::vector<BlobData> blobs;
  for (const auto &w : words) {
    if (w.word->ratings != nullptr && w.word->ratings->get(0, 0) == nullptr) {
      for (size_t s = 0; s < w.lang_words.size(); ++s) {
        Tesseract *sub = s < sub_langs_.size() ? sub_langs_[s] : this;
        const WERD_RES &word = *w.lang_words[s];
        for (unsigned b = 0; b < word.chopped_word->NumBlobs(); ++b) {
          blobs.emplace_back(b, sub, word);
        }
      }
    }
  }
  // Pre-classify all the blobs.
  if (tessedit_parallelize > 1) {
#ifdef _OPENMP
#  pragma omp parallel for num_threads(10)
#endif // _OPENMP
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (size_t b = 0; b < blobs.size(); ++b) {
      *blobs[b].choices =
          blobs[b].tesseract->classify_blob(blobs[b].blob, "par", ScrollView::WHITE, nullptr);
    }
  } else {
    // TODO(AMD) parallelize this.
    for (auto &blob : blobs) {
      *blob.choices = blob.tesseract->classify_blob(blob.blob, "par", ScrollView::WHITE, nullptr);
    }
  }
}

} // namespace tesseract.
