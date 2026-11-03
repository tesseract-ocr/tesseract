// SPDX-License-Identifier: Apache-2.0
// File:        sha256.h
// Description: Self-contained SHA-256 hash (FIPS 180-2).
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

#ifndef TESSERACT_CCUTIL_SHA256_H_
#define TESSERACT_CCUTIL_SHA256_H_

#include <cstddef>
#include <cstdint>
#include <string>

namespace tesseract {

// Incremental SHA-256 hasher.
//
//   SHA256 h;
//   h.Update(data1, len1);
//   h.Update(data2, len2);
//   std::string hex = h.FinalHex();   // 64 lowercase hex chars
//
// After FinalHex() the object is consumed; do not call Update() again.
class SHA256 {
public:
  SHA256();

  // Feed data in arbitrary-sized chunks.
  void Update(const uint8_t *data, size_t len);

  // Finalize and return the 64-character lowercase hex digest.
  std::string FinalHex();

private:
  void ProcessBlock(const uint8_t block[64]);

  uint32_t state_[8];
  uint64_t total_bytes_;
  uint8_t buf_[64];
  size_t buf_len_;
};

// Hash an entire file.  Returns a 64-char lowercase hex string, or ""
// if the file cannot be opened.  A null path also returns "".
std::string SHA256File(const char *path);

} // namespace tesseract

#endif // TESSERACT_CCUTIL_SHA256_H_
