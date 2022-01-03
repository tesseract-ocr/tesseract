// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_CCUTIL_WINUTILS_H_
#define TESSERACT_CCUTIL_WINUTILS_H_

#ifdef _WIN32

#include <string>

namespace tesseract {
namespace winutils {

std::wstring Utf8ToUtf16(const char* utf8Str);
std::string Utf16ToUtf8(const wchar_t* utf16Str);

} // namespace winutils
} // namespace tesseract

#endif // _WIN32

#endif // TESSERACT_CCUTIL_WINUTILS_H_
