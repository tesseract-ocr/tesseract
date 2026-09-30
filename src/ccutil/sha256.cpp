// SPDX-License-Identifier: Apache-2.0
// File:        sha256.cpp
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

#include "sha256.h"

#include <algorithm>
#include <fstream>

#include <cstdio>
#include <cstring>

namespace tesseract {

// ---------- helpers ---------------------------------------------------------

static inline uint32_t rotr(uint32_t x, unsigned n) {
  return (x >> n) | (x << (32 - n));
}

static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (~x & z);
}

static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint32_t Sigma0(uint32_t x) {
  return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

static inline uint32_t Sigma1(uint32_t x) {
  return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

static inline uint32_t sigma0(uint32_t x) {
  return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

static inline uint32_t sigma1(uint32_t x) {
  return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

// ---------- SHA256 ----------------------------------------------------------

SHA256::SHA256() : total_bytes_(0), buf_len_(0) {
  std::fill(std::begin(buf_), std::end(buf_), uint8_t{0});
  state_[0] = 0x6a09e667;
  state_[1] = 0xbb67ae85;
  state_[2] = 0x3c6ef372;
  state_[3] = 0xa54ff53a;
  state_[4] = 0x510e527f;
  state_[5] = 0x9b05688c;
  state_[6] = 0x1f83d9ab;
  state_[7] = 0x5be0cd19;
}

void SHA256::ProcessBlock(const uint8_t block[64]) {
  uint32_t W[64];
  for (int i = 0; i < 16; ++i) {
    W[i] = (static_cast<uint32_t>(block[4 * i]) << 24) |
           (static_cast<uint32_t>(block[4 * i + 1]) << 16) |
           (static_cast<uint32_t>(block[4 * i + 2]) << 8) |
           (static_cast<uint32_t>(block[4 * i + 3]));
  }
  for (int i = 16; i < 64; ++i) {
    W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
  }

  uint32_t a = state_[0], b = state_[1], c = state_[2], d = state_[3];
  uint32_t e = state_[4], f = state_[5], g = state_[6], h = state_[7];

  for (int i = 0; i < 64; ++i) {
    uint32_t T1 = h + Sigma1(e) + ch(e, f, g) + K[i] + W[i];
    uint32_t T2 = Sigma0(a) + maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
  }

  state_[0] += a;
  state_[1] += b;
  state_[2] += c;
  state_[3] += d;
  state_[4] += e;
  state_[5] += f;
  state_[6] += g;
  state_[7] += h;
}

void SHA256::Update(const uint8_t *data, size_t len) {
  total_bytes_ += len;
  // If there is leftover data in buf_, try to fill it to a full block.
  if (buf_len_ > 0) {
    size_t space = 64 - buf_len_;
    size_t copy = (len < space) ? len : space;
    std::copy_n(data, copy, buf_ + buf_len_);  // copy <= 64 - buf_len_
    buf_len_ += copy;
    data += copy;
    len -= copy;
    if (buf_len_ == 64) {
      ProcessBlock(buf_);
      buf_len_ = 0;
    }
  }
  // Process complete blocks directly from data.
  while (len >= 64) {
    ProcessBlock(data);
    data += 64;
    len -= 64;
  }
  // Buffer remaining bytes.
  if (len > 0) {
    std::copy_n(data, len, buf_);  // len < 64, guaranteed by the loop above
    buf_len_ = len;
  }
}

std::string SHA256::FinalHex() {
  // Pad: append 0x80, then zeros, then 8-byte big-endian bit length.
  uint64_t total_bits = total_bytes_ * 8;
  uint8_t pad_byte = 0x80;
  Update(&pad_byte, 1);

  // Need buf_len_ == 56 mod 64 after this padding (room for 8-byte length).
  uint8_t zero = 0;
  while (buf_len_ != 56) {
    Update(&zero, 1);
  }

  // Append bit length as big-endian 64-bit integer.
  uint8_t len_bytes[8];
  for (int i = 7; i >= 0; --i) {
    len_bytes[i] = static_cast<uint8_t>(total_bits);
    total_bits >>= 8;
  }
  Update(len_bytes, 8);

  // Format as 64-char lowercase hex.
  static const char hex_chars[] = "0123456789abcdef";
  std::string result;
  result.reserve(64);
  for (int i = 0; i < 8; ++i) {
    result += hex_chars[(state_[i] >> 28) & 0xf];
    result += hex_chars[(state_[i] >> 24) & 0xf];
    result += hex_chars[(state_[i] >> 20) & 0xf];
    result += hex_chars[(state_[i] >> 16) & 0xf];
    result += hex_chars[(state_[i] >> 12) & 0xf];
    result += hex_chars[(state_[i] >> 8) & 0xf];
    result += hex_chars[(state_[i] >> 4) & 0xf];
    result += hex_chars[(state_[i] >> 0) & 0xf];
  }
  return result;
}

// ---------- SHA256File ------------------------------------------------------

std::string SHA256File(const char *path) {
  if (path == nullptr) {
    return "";
  }
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    return "";
  }

  SHA256 h;
  char buf[8192];
  while (in.read(buf, sizeof(buf))) {
    h.Update(reinterpret_cast<const uint8_t *>(buf), sizeof(buf));
  }
  // The final read stops short of the buffer size; gcount() holds what it got.
  h.Update(reinterpret_cast<const uint8_t *>(buf),
           static_cast<size_t>(in.gcount()));

  // A read error (e.g. the path is a directory, which opens but cannot be
  // read) must not yield the digest of the bytes read so far, which would
  // look like a valid hash of the file.
  if (in.bad()) {
    return "";
  }
  return h.FinalHex();
}

} // namespace tesseract
