// SPDX-License-Identifier: Apache-2.0
// File:        hocrrenderer_internal.h
// Description: Internal helpers for hOCR rendering.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_API_HOCRRENDERER_INTERNAL_H_
#define TESSERACT_API_HOCRRENDERER_INTERNAL_H_

#include <tesseract/baseapi.h> // for HOcrEscape

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tesseract {

static const std::unordered_map<std::string, std::string> &Iso639Map() {
  static const std::unordered_map<std::string, std::string> map{
      {"afr", "af"},
      {"amh", "am"},
      {"ara", "ar"},
      {"asm", "as"},
      {"aze", "az"},
      {"aze_cyrl", "az"},
      {"bel", "be"},
      {"ben", "bn"},
      {"bod", "bo"},
      {"bos", "bs"},
      {"bre", "br"},
      {"bul", "bg"},
      {"cat", "ca"},
      {"ceb", "ceb"},
      {"ces", "cs"},
      {"chi_sim", "zh-Hans"},
      {"chi_sim_vert", "zh-Hans"},
      {"chi_tra", "zh-Hant"},
      {"chi_tra_vert", "zh-Hant"},
      {"chr", "chr"},
      {"cos", "co"},
      {"cym", "cy"},
      {"dan", "da"},
      {"deu", "de"},
      {"deu_latf", "de"},
      {"div", "dv"},
      {"dzo", "dz"},
      {"ell", "el"},
      {"eng", "en"},
      {"enm", "enm"},
      {"epo", "eo"},
      {"est", "et"},
      {"eus", "eu"},
      {"fao", "fo"},
      {"fas", "fa"},
      {"fil", "fil"},
      {"fin", "fi"},
      {"fra", "fr"},
      {"frk", "de"},
      {"frm", "frm"},
      {"fry", "fy"},
      {"gla", "gd"},
      {"gle", "ga"},
      {"glg", "gl"},
      {"grc", "grc"},
      {"guj", "gu"},
      {"hat", "ht"},
      {"heb", "he"},
      {"hin", "hi"},
      {"hrv", "hr"},
      {"hun", "hu"},
      {"hye", "hy"},
      {"iku", "iu"},
      {"ind", "id"},
      {"isl", "is"},
      {"ita", "it"},
      {"ita_old", "it"},
      {"jav", "jv"},
      {"jpn", "ja"},
      {"jpn_vert", "ja"},
      {"kan", "kn"},
      {"kat", "ka"},
      {"kat_old", "ka"},
      {"kaz", "kk"},
      {"khm", "km"},
      {"kir", "ky"},
      {"kmr", "ku"},
      {"kor", "ko"},
      {"kor_vert", "ko"},
      {"kur_ara", "ku"},
      {"lao", "lo"},
      {"lat", "la"},
      {"lav", "lv"},
      {"lit", "lt"},
      {"ltz", "lb"},
      {"mal", "ml"},
      {"mar", "mr"},
      {"mkd", "mk"},
      {"mlt", "mt"},
      {"mon", "mn"},
      {"mri", "mi"},
      {"msa", "ms"},
      {"mya", "my"},
      {"nep", "ne"},
      {"nld", "nl"},
      {"nor", "no"},
      {"oci", "oc"},
      {"ori", "or"},
      {"pan", "pa"},
      {"pol", "pl"},
      {"por", "pt"},
      {"pus", "ps"},
      {"que", "qu"},
      {"ron", "ro"},
      {"rus", "ru"},
      {"san", "sa"},
      {"sin", "si"},
      {"slk", "sk"},
      {"slv", "sl"},
      {"snd", "sd"},
      {"spa", "es"},
      {"spa_old", "es"},
      {"sqi", "sq"},
      {"srp", "sr"},
      {"srp_latn", "sr"},
      {"sun", "su"},
      {"swa", "sw"},
      {"swe", "sv"},
      {"syr", "syr"},
      {"tam", "ta"},
      {"tat", "tt"},
      {"tel", "te"},
      {"tgk", "tg"},
      {"tgl", "tl"},
      {"tha", "th"},
      {"tir", "ti"},
      {"ton", "to"},
      {"tur", "tr"},
      {"uig", "ug"},
      {"ukr", "uk"},
      {"urd", "ur"},
      {"uzb", "uz"},
      {"uzb_cyrl", "uz"},
      {"vie", "vi"},
      {"yid", "yi"},
      {"yor", "yo"},
      {"zho", "zh"},
  };
  return map;
}

static const std::unordered_map<std::string, std::string> &ScriptIso15924Map() {
  static const std::unordered_map<std::string, std::string> map{
      {"Arabic", "Arab"},      {"Armenian", "Armn"},
      {"Bengali", "Beng"},     {"Canadian_Aboriginal", "Cans"},
      {"Cherokee", "Cher"},    {"Cyrillic", "Cyrl"},
      {"Devanagari", "Deva"},  {"Ethiopic", "Ethi"},
      {"Fraktur", "Latf"},     {"Georgian", "Geor"},
      {"Greek", "Grek"},       {"Gujarati", "Gujr"},
      {"Gurmukhi", "Guru"},    {"HanS", "Hans"},
      {"HanS_vert", "Hans"},   {"HanT", "Hant"},
      {"HanT_vert", "Hant"},   {"Hangul", "Hang"},
      {"Hangul_vert", "Hang"}, {"Hebrew", "Hebr"},
      {"Japanese", "Jpan"},    {"Japanese_vert", "Jpan"},
      {"Kannada", "Knda"},     {"Khmer", "Khmr"},
      {"Lao", "Laoo"},         {"Latin", "Latn"},
      {"Malayalam", "Mlym"},   {"Myanmar", "Mymr"},
      {"Oriya", "Orya"},       {"Sinhala", "Sinh"},
      {"Syriac", "Syrc"},      {"Tamil", "Taml"},
      {"Telugu", "Telu"},      {"Thaana", "Thaa"},
      {"Thai", "Thai"},        {"Tibetan", "Tibt"},
      {"Vietnamese", "Latn"},
  };
  return map;
}

static std::string FormatHOcrLanguageMetaTags(
    const std::vector<std::string> &models) {
  std::string langs;
  std::string scripts;
  std::unordered_set<std::string> seen_langs;
  std::unordered_set<std::string> seen_scripts;
  const auto &iso_map = Iso639Map();
  const auto &script_map = ScriptIso15924Map();

  auto append_unique = [](std::string &out,
                          std::unordered_set<std::string> &seen,
                          const std::string &code) {
    if (seen.insert(code).second) {
      if (!out.empty()) {
        out += ' ';
      }
      out += code;
    }
  };

  for (const std::string &model : models) {
    std::string name = model;
    const auto slash = name.rfind('/');
    if (slash != std::string::npos) {
      name = name.substr(slash + 1);
    }
    if (name.empty() || name == "osd" || name == "equ") {
      continue;
    }
    auto script_it = script_map.find(name);
    if (script_it != script_map.end()) {
      append_unique(scripts, seen_scripts, script_it->second);
      continue;
    }
    auto lang_it = iso_map.find(name);
    if (lang_it != iso_map.end()) {
      append_unique(langs, seen_langs, lang_it->second);
    } else {
      append_unique(langs, seen_langs, "unknown");
    }
  }

  std::string result;
  if (!langs.empty()) {
    result += "  <meta name='ocr-langs' content='";
    result += HOcrEscape(langs.c_str());
    result += "' />\n";
  }
  if (!scripts.empty()) {
    result += "  <meta name='ocr-scripts' content='";
    result += HOcrEscape(scripts.c_str());
    result += "' />\n";
  }
  return result;
}

} // namespace tesseract

#endif // TESSERACT_API_HOCRRENDERER_INTERNAL_H_
