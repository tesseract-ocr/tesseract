///////////////////////////////////////////////////////////////////////
// File:        dawg_cache.h
// Description: A class that knows about loading and caching dawgs.
// Author:      David Eger
// Created:     Fri Jan 27 12:08:00 PST 2012
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
#include "strngs.h"
#include "tessdatamanager.h"

namespace tesseract {

struct DawgLoader {
  DawgLoader(const STRING &lang,
             const char *data_file_name,
             TessdataType tessdata_dawg_type,
             int dawg_debug_level)
      : lang_(lang),
        data_file_name_(data_file_name),
        tessdata_dawg_type_(tessdata_dawg_type),
        dawg_debug_level_(dawg_debug_level) {}

  Dawg *Load();

  STRING lang_;
  const char *data_file_name_;
  TessdataType tessdata_dawg_type_;
  int dawg_debug_level_;
};

Dawg *DawgCache::GetSquishedDawg(
    const STRING &lang,
    const char *data_file_name,
    TessdataType tessdata_dawg_type,
    int debug_level) {
  STRING data_id = data_file_name;
  data_id += kTessdataFileSuffixes[tessdata_dawg_type];
  DawgLoader loader(lang, data_file_name, tessdata_dawg_type, debug_level);
  return dawgs_.Get(data_id, NewTessCallback(&loader, &DawgLoader::Load));
}

Dawg *DawgLoader::Load() {
  TessdataManager data_loader;
  if (!data_loader.Init(data_file_name_, dawg_debug_level_)) {
    return NULL;
  }
  if (!data_loader.SeekToStart(tessdata_dawg_type_)) {
    data_loader.End();
    return NULL;
  }
  FILE *fp = data_loader.GetDataFilePtr();
  DawgType dawg_type;
  PermuterType perm_type;
  switch (tessdata_dawg_type_) {
    case TESSDATA_PUNC_DAWG:
      dawg_type = DAWG_TYPE_PUNCTUATION;
      perm_type = PUNC_PERM;
      break;
    case TESSDATA_SYSTEM_DAWG:
      dawg_type = DAWG_TYPE_WORD;
      perm_type = SYSTEM_DAWG_PERM;
      break;
    case TESSDATA_NUMBER_DAWG:
      dawg_type = DAWG_TYPE_NUMBER;
      perm_type = NUMBER_PERM;
      break;
    case TESSDATA_BIGRAM_DAWG:
      dawg_type = DAWG_TYPE_WORD;  // doesn't actually matter
      perm_type = COMPOUND_PERM;   // doesn't actually matter
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
      data_loader.End();
      return NULL;
  }
  SquishedDawg *retval =
      new SquishedDawg(fp, dawg_type, lang_, perm_type, dawg_debug_level_);
  data_loader.End();
  return retval;
}

}  // namespace tesseract
