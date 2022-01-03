// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "fopenutf8.h"

#include "winutils.h"

namespace tesseract {

FILE* fopenUtf8(const char* path, const char* mode) {
#ifdef _WIN32
  return _wfopen(
    winutils::Utf8ToUtf16(path).c_str(), winutils::Utf8ToUtf16(mode).c_str());
#else
  return fopen(path, mode);
#endif
}

} // namespace tesseract
