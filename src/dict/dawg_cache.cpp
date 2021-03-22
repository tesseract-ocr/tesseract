///////////////////////////////////////////////////////////////////////
// File:        dawg_cache.cpp
// Description: A class that knows about loading and caching dawgs.
// Author:      David Eger
//
// (C) Copyright 2012, Google Inc.
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

#include "dawg_cache.h"

#include "dawg.h"
#include "object_cache.h"
#include "tessdatamanager.h"

namespace tesseract {

struct DawgLoader {
  DawgLoader(const std::string &lang, TessdataType tessdata_dawg_type, int dawg_debug_level,
             TessdataManager *data_file)
      : lang_(lang)
      , data_file_(data_file)
      , tessdata_dawg_type_(tessdata_dawg_type)
      , dawg_debug_level_(dawg_debug_level) {}

  Dawg *Load();

  std::string lang_;
  TessdataManager *data_file_;
  TessdataType tessdata_dawg_type_;
  int dawg_debug_level_;
};

Dawg *DawgCache::GetSquishedDawg(const std::string &lang, TessdataType tessdata_dawg_type,
                                 int debug_level, TessdataManager *data_file) {
  std::string data_id = data_file->GetDataFileName();
  data_id += kTessdataFileSuffixes[tessdata_dawg_type];
  DawgLoader loader(lang, tessdata_dawg_type, debug_level, data_file);
  return dawgs_.Get(data_id, std::bind(&DawgLoader::Load, &loader));
}

Dawg *DawgLoader::Load() {
  TFile fp;
  if (!data_file_->GetComponent(tessdata_dawg_type_, &fp)) {
    return nullptr;
  }
  DawgType dawg_type;
  PermuterType perm_type;
  switch (tessdata_dawg_type_) {
    case TESSDATA_PUNC_DAWG:
    case TESSDATA_LSTM_PUNC_DAWG:
      dawg_type = DAWG_TYPE_PUNCTUATION;
      perm_type = PUNC_PERM;
      break;
    case TESSDATA_SYSTEM_DAWG:
    case TESSDATA_LSTM_SYSTEM_DAWG:
      dawg_type = DAWG_TYPE_WORD;
      perm_type = SYSTEM_DAWG_PERM;
      break;
    case TESSDATA_NUMBER_DAWG:
    case TESSDATA_LSTM_NUMBER_DAWG:
      dawg_type = DAWG_TYPE_NUMBER;
      perm_type = NUMBER_PERM;
      break;
    case TESSDATA_BIGRAM_DAWG:
      dawg_type = DAWG_TYPE_WORD; // doesn't actually matter
      perm_type = COMPOUND_PERM;  // doesn't actually matter
      break;
    case TESSDATA_UNAMBIG_DAWG:
      dawg_type = DAWG_TYPE_WORD;
      perm_type = SYSTEM_DAWG_PERM;
      break;
    case TESSDATA_FREQ_DAWG:
      dawg_type = DAWG_TYPE_WORD;
      perm_type = FREQ_DAWG_PERM;
      break;
    default:
      return nullptr;
  }
  auto *retval = new SquishedDawg(dawg_type, lang_, perm_type, dawg_debug_level_);
  if (retval->Load(&fp)) {
    return retval;
  }
  delete retval;
  return nullptr;
}

} // namespace tesseract
