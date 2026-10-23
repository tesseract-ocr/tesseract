// SPDX-License-Identifier: Apache-2.0
// File:        runtimeparams.h
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

#ifndef TESSERACT_API_RUNTIMEPARAMS_H_
#define TESSERACT_API_RUNTIMEPARAMS_H_

#include <string>

namespace tesseract {

class TessBaseAPI;

// Build a JSON string recording the runtime parameters that are relevant for
// reproducing an OCR run:
//
//   {"langs":"eng+fra","tessdata-hashes":{"eng":"sha256:...","fra":"sha256:..."}}
//
// The language string is taken verbatim from GetInitLanguagesAsString(),
// preserving compound model names like "deu_latf".  Traineddata file hashes
// are SHA-256 digests of the files loaded from disk; if a file cannot be read
// (e.g. loaded from an archive), its entry is silently omitted.
//
// The returned string contains no newlines — it is intended to be embedded in
// an XML attribute.
std::string BuildRuntimeParametersJSON(TessBaseAPI &api);

// Escape a string for use inside a single-quoted XML attribute value.
// Only ', &, <, > are escaped.  Double quotes are left literal because
// they are legal inside single-quoted attributes and keeping them makes the
// embedded JSON directly parseable.
std::string EscapeForXmlAttribute(const std::string &value);

} // namespace tesseract

#endif // TESSERACT_API_RUNTIMEPARAMS_H_
