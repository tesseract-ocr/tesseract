// SPDX-License-Identifier: Apache-2.0
// File:        runtimeparams.cpp
// Description: Build a JSON string of runtime parameters for hOCR / ALTO / PAGE
//              output metadata.
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

#include "runtimeparams.h"

#include <tesseract/baseapi.h>

#include "sha256.h"

#include <string>
#include <vector>

namespace tesseract {

// ---------- EscapeForXmlAttribute -------------------------------------------

// Escape for a single-quoted XML attribute.  Inside '…', only these four
// characters need escaping.  Double quotes are left literal so embedded JSON
// remains directly parseable.
std::string EscapeForXmlAttribute(const std::string &value) {
  std::string out;
  out.reserve(value.size() + value.size() / 8); // minor over-allocation
  for (char c : value) {
    switch (c) {
      case '&':
        out += "&amp;";
        break;
      case '\'':
        out += "&apos;";
        break;
      case '<':
        out += "&lt;";
        break;
      case '>':
        out += "&gt;";
        break;
      default:
        out += c;
    }
  }
  return out;
}

// ---------- JSON helpers (no dependency on a JSON library) ------------------

// Escape a string for JSON.  Only characters that JSON requires to be escaped
// are handled: backslash, double-quote, and control characters.
static std::string JsonEscape(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          // Generic \u00xx escape for other control characters.
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
          out += buf;
        } else {
          out += c;
        }
    }
  }
  return out;
}

// ---------- BuildRuntimeParametersJSON --------------------------------------

std::string BuildRuntimeParametersJSON(TessBaseAPI &api) {
  // 1. Language string — verbatim, never reduced to ISO codes.
  const char *langs = api.GetInitLanguagesAsString();
  std::string langs_str = (langs != nullptr) ? langs : "";

  // 2. Traineddata hashes.
  std::vector<std::string> loaded;
  api.GetLoadedLanguagesAsVector(&loaded);
  const char *datapath = api.GetDatapath();
  std::string dp = (datapath != nullptr) ? datapath : "";

  // Build the tessdata-hashes object entries.
  std::string hashes_json;
  bool first_hash = true;
  for (const std::string &lang : loaded) {
    std::string path = dp + lang + ".traineddata";
    std::string digest = SHA256File(path.c_str());
    if (digest.empty()) {
      continue; // unreadable file → omit, do NOT emit a placeholder
    }
    if (!first_hash) {
      hashes_json += ',';
    }
    hashes_json += "\"";
    hashes_json += JsonEscape(lang);
    hashes_json += "\":\"sha256:";
    hashes_json += digest;
    hashes_json += "\"";
    first_hash = false;
  }

  // 3. Assemble the top-level JSON object.
  std::string json = "{\"langs\":\"";
  json += JsonEscape(langs_str);
  json += "\",\"tessdata-hashes\":{";
  json += hashes_json;
  json += "}}";

  return json;
}

} // namespace tesseract
