/**********************************************************************
 * File:        ligature_table.h
 * Description: Class for adding and removing optional latin ligatures,
 *              conditional on codepoint support by a specified font
 *              (if specified).
 * Author:      Ranjith Unnikrishnan
 * Created:     Mon Nov 18 2013
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

#ifndef TRAININGDATA_LIGATURE_TABLE_H_
#define TRAININGDATA_LIGATURE_TABLE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "util.h"

namespace tesseract {

class PangoFontInfo;  // defined in pango_font_info.h

// Map to substitute strings for ligatures.
using LigHash = std::unordered_map<std::string, std::string, StringHash>;

class LigatureTable {
 public:
  // Get a static instance of this class.
  static LigatureTable* Get();

  // Convert the utf8 string so that ligaturizable sequences, such as "fi" get
  // replaced by the (utf8 code for) appropriate ligature characters. Only do so
  // if the corresponding ligature character is renderable in the current font.
  std::string AddLigatures(const std::string& str, const PangoFontInfo* font) const;
  // Remove all ligatures.
  std::string RemoveLigatures(const std::string& str) const;
  // Remove only custom ligatures (eg. "ct") encoded in the private-use-area.
  std::string RemoveCustomLigatures(const std::string& str) const;

  const LigHash& norm_to_lig_table() const {
    return norm_to_lig_table_;
  }
  const LigHash& lig_to_norm_table() const {
    return lig_to_norm_table_;
  }

 protected:
  LigatureTable();
  // Initialize the hash tables mapping between ligature strings and the
  // corresponding ligature characters.
  void Init();

  static std::unique_ptr<LigatureTable> instance_;
  LigHash norm_to_lig_table_;
  LigHash lig_to_norm_table_;
  int min_lig_length_;
  int max_lig_length_;
  int min_norm_length_;
  int max_norm_length_;

 private:
  LigatureTable(const LigatureTable&);
  void operator=(const LigatureTable&);
};

}  // namespace tesseract

#endif  // OCR_TRAININGDATA_TYPESETTING_LIGATURE_TABLE_H_
