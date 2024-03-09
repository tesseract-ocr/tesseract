/**********************************************************************
 * File:        ligature_table.cpp
 * Description: Class for adding and removing optional latin ligatures,
 *              conditional on codepoint support by a specified font
 *              (if specified).
 * Author:      Ranjith Unnikrishnan
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

#include "ligature_table.h"

#include <tesseract/unichar.h>
#include "pango_font_info.h"
#include "tlog.h"
#include "unicharset.h"
#include "unicode/errorcode.h" // from libicu
#include "unicode/normlzr.h"   // from libicu
#include "unicode/unistr.h"    // from libicu
#include "unicode/utypes.h"    // from libicu

#include <utility>

namespace tesseract {

static std::string EncodeAsUTF8(const char32 ch32) {
  UNICHAR uni_ch(ch32);
  return std::string(uni_ch.utf8(), uni_ch.utf8_len());
}

// Range of optional latin ligature characters in Unicode to build ligatures
// from. Note that this range does not contain the custom ligatures that we
// encode in the private use area.
const int kMinLigature = 0xfb00;
const int kMaxLigature = 0xfb17; // Don't put the wide Hebrew letters in.

/* static */
std::unique_ptr<LigatureTable> LigatureTable::instance_;

/* static */
LigatureTable *LigatureTable::Get() {
  if (instance_ == nullptr) {
    instance_.reset(new LigatureTable());
    instance_->Init();
  }
  return instance_.get();
}

LigatureTable::LigatureTable()
    : min_lig_length_(0), max_lig_length_(0), min_norm_length_(0), max_norm_length_(0) {}

void LigatureTable::Init() {
  if (norm_to_lig_table_.empty()) {
    for (char32 lig = kMinLigature; lig <= kMaxLigature; ++lig) {
      // For each char in the range, convert to utf8, nfc normalize, and if
      // the strings are different put the both mappings in the hash_maps.
      std::string lig8 = EncodeAsUTF8(lig);
      icu::UnicodeString unicode_lig8(static_cast<UChar32>(lig));
      icu::UnicodeString normed8_result;
      icu::ErrorCode status;
      icu::Normalizer::normalize(unicode_lig8, UNORM_NFC, 0, normed8_result, status);
      std::string normed8;
      normed8_result.toUTF8String(normed8);
      int lig_length = lig8.length();
      int norm_length = normed8.size();
      if (normed8 != lig8 && lig_length > 1 && norm_length > 1) {
        norm_to_lig_table_[normed8] = lig8;
        lig_to_norm_table_[lig8] = std::move(normed8);
        if (min_lig_length_ == 0 || lig_length < min_lig_length_) {
          min_lig_length_ = lig_length;
        }
        if (lig_length > max_lig_length_) {
          max_lig_length_ = lig_length;
        }
        if (min_norm_length_ == 0 || norm_length < min_norm_length_) {
          min_norm_length_ = norm_length;
        }
        if (norm_length > max_norm_length_) {
          max_norm_length_ = norm_length;
        }
      }
    }
    // Add custom extra ligatures.
    for (int i = 0; UNICHARSET::kCustomLigatures[i][0] != nullptr; ++i) {
      norm_to_lig_table_[UNICHARSET::kCustomLigatures[i][0]] = UNICHARSET::kCustomLigatures[i][1];
      int norm_length = strlen(UNICHARSET::kCustomLigatures[i][0]);
      if (min_norm_length_ == 0 || norm_length < min_norm_length_) {
        min_norm_length_ = norm_length;
      }
      if (norm_length > max_norm_length_) {
        max_norm_length_ = norm_length;
      }

      lig_to_norm_table_[UNICHARSET::kCustomLigatures[i][1]] = UNICHARSET::kCustomLigatures[i][0];
    }
  }
}

std::string LigatureTable::RemoveLigatures(const std::string &str) const {
  std::string result;
  UNICHAR::const_iterator it_begin = UNICHAR::begin(str.c_str(), str.length());
  UNICHAR::const_iterator it_end = UNICHAR::end(str.c_str(), str.length());
  char tmp[5];
  int len;
  for (UNICHAR::const_iterator it = it_begin; it != it_end; ++it) {
    len = it.get_utf8(tmp);
    tmp[len] = '\0';
    auto lig_it = lig_to_norm_table_.find(tmp);
    if (lig_it != lig_to_norm_table_.end()) {
      result += lig_it->second;
    } else {
      result += tmp;
    }
  }
  return result;
}

std::string LigatureTable::RemoveCustomLigatures(const std::string &str) const {
  std::string result;
  UNICHAR::const_iterator it_begin = UNICHAR::begin(str.c_str(), str.length());
  UNICHAR::const_iterator it_end = UNICHAR::end(str.c_str(), str.length());
  char tmp[5];
  int len;
  int norm_ind;
  for (UNICHAR::const_iterator it = it_begin; it != it_end; ++it) {
    len = it.get_utf8(tmp);
    tmp[len] = '\0';
    norm_ind = -1;
    for (int i = 0; UNICHARSET::kCustomLigatures[i][0] != nullptr && norm_ind < 0; ++i) {
      if (!strcmp(tmp, UNICHARSET::kCustomLigatures[i][1])) {
        norm_ind = i;
      }
    }
    if (norm_ind >= 0) {
      result += UNICHARSET::kCustomLigatures[norm_ind][0];
    } else {
      result += tmp;
    }
  }
  return result;
}

std::string LigatureTable::AddLigatures(const std::string &str, const PangoFontInfo *font) const {
  std::string result;
  int len = str.size();
  int step = 0;
  int i = 0;
  for (i = 0; i < len - min_norm_length_ + 1; i += step) {
    step = 0;
    for (int liglen = max_norm_length_; liglen >= min_norm_length_; --liglen) {
      if (i + liglen <= len) {
        std::string lig_cand = str.substr(i, liglen);
        auto it = norm_to_lig_table_.find(lig_cand);
        if (it != norm_to_lig_table_.end()) {
          tlog(3, "Considering %s -> %s\n", lig_cand.c_str(), it->second.c_str());
          if (font) {
            // Test for renderability.
            if (!font->CanRenderString(it->second.data(), it->second.length())) {
              continue; // Not renderable
            }
          }
          // Found a match so convert it.
          step = liglen;
          result += it->second;
          tlog(2, "Substituted %s -> %s\n", lig_cand.c_str(), it->second.c_str());
          break;
        }
      }
    }
    if (step == 0) {
      result += str[i];
      step = 1;
    }
  }
  result += str.substr(i, len - i);
  return result;
}

} // namespace tesseract
