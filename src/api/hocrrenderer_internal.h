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

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tesseract {

static const std::unordered_map<std::string, std::string> &Iso639Map() {
  static const std::unordered_map<std::string, std::string> map{
      {"afr", "af"},  {"amh", "am"},  {"ara", "ar"},     {"asm", "as"},
      {"aze", "az"},  {"bel", "be"},  {"ben", "bn"},     {"bod", "bo"},
      {"bos", "bs"},  {"bre", "br"},  {"bul", "bg"},     {"cat", "ca"},
      {"ceb", "ceb"}, {"ces", "cs"},  {"chi_sim", "zh"}, {"chi_tra", "zh"},
      {"chr", "chr"}, {"cos", "co"},  {"cym", "cy"},     {"dan", "da"},
      {"deu", "de"},  {"div", "dv"},  {"dzo", "dz"},     {"ell", "el"},
      {"eng", "en"},  {"enm", "en"},  {"epo", "eo"},     {"est", "et"},
      {"eus", "eu"},  {"fao", "fo"},  {"fas", "fa"},     {"fil", "fil"},
      {"fin", "fi"},  {"fra", "fr"},  {"frk", "de"},     {"frm", "fr"},
      {"fry", "fy"},  {"gla", "gd"},  {"gle", "ga"},     {"glg", "gl"},
      {"grc", "el"},  {"guj", "gu"},  {"hat", "ht"},     {"heb", "he"},
      {"hin", "hi"},  {"hrv", "hr"},  {"hun", "hu"},     {"hye", "hy"},
      {"iku", "iu"},  {"ind", "id"},  {"isl", "is"},     {"ita", "it"},
      {"jav", "jv"},  {"jpn", "ja"},  {"kan", "kn"},     {"kat", "ka"},
      {"kaz", "kk"},  {"khm", "km"},  {"kir", "ky"},     {"kmr", "ku"},
      {"kor", "ko"},  {"lao", "lo"},  {"lat", "la"},     {"lav", "lv"},
      {"lit", "lt"},  {"ltz", "lb"},  {"mal", "ml"},     {"mar", "mr"},
      {"mkd", "mk"},  {"mlt", "mt"},  {"mon", "mn"},     {"mri", "mi"},
      {"msa", "ms"},  {"mya", "my"},  {"nep", "ne"},     {"nld", "nl"},
      {"nor", "no"},  {"oci", "oc"},  {"ori", "or"},     {"pan", "pa"},
      {"pol", "pl"},  {"por", "pt"},  {"pus", "ps"},     {"que", "qu"},
      {"ron", "ro"},  {"rus", "ru"},  {"san", "sa"},     {"sin", "si"},
      {"slk", "sk"},  {"slv", "sl"},  {"snd", "sd"},     {"spa", "es"},
      {"sqi", "sq"},  {"srp", "sr"},  {"sun", "su"},     {"swa", "sw"},
      {"swe", "sv"},  {"syr", "syr"}, {"tam", "ta"},     {"tat", "tt"},
      {"tel", "te"},  {"tgk", "tg"},  {"tha", "th"},     {"tir", "ti"},
      {"ton", "to"},  {"tur", "tr"},  {"uig", "ug"},     {"ukr", "uk"},
      {"urd", "ur"},  {"uzb", "uz"},  {"vie", "vi"},     {"yid", "yi"},
      {"yor", "yo"},  {"zho", "zh"},
  };
  return map;
}

// Look up a model name in the ISO 639 map, trying progressively shorter
// prefixes by stripping trailing _suffix parts. This handles compound
// model names like deu_latf -> deu -> "de", chi_tra_vert -> chi_tra -> "zh".
static std::string LookupIso639(const std::string &model) {
  const auto &iso_map = Iso639Map();
  auto it = iso_map.find(model);
  if (it != iso_map.end()) {
    return it->second;
  }
  std::string prefix = model;
  while (true) {
    auto pos = prefix.rfind('_');
    if (pos == std::string::npos) {
      break;
    }
    prefix = prefix.substr(0, pos);
    it = iso_map.find(prefix);
    if (it != iso_map.end()) {
      return it->second;
    }
  }
  return model;
}

static const std::unordered_set<std::string> &ScriptNames() {
  static const std::unordered_set<std::string> set{
      "Arabic",     "Armenian",      "Bengali",     "Canadian_Aboriginal",
      "Cherokee",   "Cyrillic",      "Devanagari",  "Ethiopic",
      "Fraktur",    "Georgian",      "Greek",       "Gujarati",
      "Gurmukhi",   "HanS",          "HanS_vert",   "HanT",
      "HanT_vert",  "Hangul",        "Hangul_vert", "Hebrew",
      "Japanese",   "Japanese_vert", "Kannada",     "Khmer",
      "Lao",        "Latin",         "Malayalam",   "Myanmar",
      "Oriya",      "Sinhala",       "Syriac",      "Tamil",
      "Telugu",     "Thaana",        "Thai",        "Tibetan",
      "Vietnamese",
  };
  return set;
}

static std::string FormatHOcrLanguageMetaTags(const char *languages) {
  if (languages == nullptr || languages[0] == '\0') {
    return {};
  }
  std::string langs;
  std::string scripts;
  const auto &script_set = ScriptNames();
  std::istringstream stream(languages);
  std::string token;
  while (std::getline(stream, token, '+')) {
    if (token.empty()) {
      continue;
    }
    if (script_set.count(token)) {
      if (!scripts.empty()) {
        scripts += ' ';
      }
      scripts += token;
    } else {
      if (!langs.empty()) {
        langs += ' ';
      }
      langs += LookupIso639(token);
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
