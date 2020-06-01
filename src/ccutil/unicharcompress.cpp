///////////////////////////////////////////////////////////////////////
// File:        unicharcompress.cpp
// Description: Unicode re-encoding using a sequence of smaller numbers in
//              place of a single large code for CJK, similarly for Indic,
//              and dissection of ligatures for other scripts.
// Author:      Ray Smith
//
// (C) Copyright 2015, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#include "unicharcompress.h"
#include <algorithm>
#include <memory>
#include "tprintf.h"

namespace tesseract {

// String used to represent the null_id in direct_set.
static const char* kNullChar = "<nul>";
// Radix to make unique values from the stored radical codes.
const int kRadicalRadix = 29;

// "Hash" function for const std::vector<int> computes the sum of elements.
// Build a unique number for each code sequence that we can use as the index in
// a hash map of ints instead of trying to hash the vectors.
static int RadicalPreHash(const std::vector<int>& rs) {
  size_t result = 0;
  for (int radical : rs) {
    result *= kRadicalRadix;
    result += radical;
  }
  return result;
}

// A hash map to convert unicodes to radical encoding.
using RSMap = std::unordered_map<int, std::unique_ptr<std::vector<int>>>;
// A hash map to count occurrences of each radical encoding.
using RSCounts = std::unordered_map<int, int>;

static bool DecodeRadicalLine(STRING* radical_data_line, RSMap* radical_map) {
  if (radical_data_line->length() == 0 || (*radical_data_line)[0] == '#')
    return true;
  GenericVector<STRING> entries;
  radical_data_line->split(' ', &entries);
  if (entries.size() < 2) return false;
  char* end = nullptr;
  int unicode = strtol(&entries[0][0], &end, 10);
  if (*end != '\0') return false;
  std::unique_ptr<std::vector<int>> radicals(new std::vector<int>);
  for (int i = 1; i < entries.size(); ++i) {
    int radical = strtol(&entries[i][0], &end, 10);
    if (*end != '\0') return false;
    radicals->push_back(radical);
  }
  (*radical_map)[unicode] = std::move(radicals);
  return true;
}

// Helper function builds the RSMap from the radical-stroke file, which has
// already been read into a STRING. Returns false on error.
// The radical_stroke_table is non-const because it gets split and the caller
// is unlikely to want to use it again.
static bool DecodeRadicalTable(STRING* radical_data, RSMap* radical_map) {
  GenericVector<STRING> lines;
  radical_data->split('\n', &lines);
  for (int i = 0; i < lines.size(); ++i) {
    if (!DecodeRadicalLine(&lines[i], radical_map)) {
      tprintf("Invalid format in radical table at line %d: %s\n", i,
              lines[i].c_str());
      return false;
    }
  }
  return true;
}

UnicharCompress::UnicharCompress() : code_range_(0) {}
UnicharCompress::UnicharCompress(const UnicharCompress& src) { *this = src; }
UnicharCompress::~UnicharCompress() { Cleanup(); }
UnicharCompress& UnicharCompress::operator=(const UnicharCompress& src) {
  Cleanup();
  encoder_ = src.encoder_;
  code_range_ = src.code_range_;
  SetupDecoder();
  return *this;
}

// Computes the encoding for the given unicharset. It is a requirement that
// the file training/langdata/radical-stroke.txt have been read into the
// input string radical_stroke_table.
// Returns false if the encoding cannot be constructed.
bool UnicharCompress::ComputeEncoding(const UNICHARSET& unicharset, int null_id,
                                      STRING* radical_stroke_table) {
  RSMap radical_map;
  if (radical_stroke_table != nullptr &&
      !DecodeRadicalTable(radical_stroke_table, &radical_map))
    return false;
  encoder_.clear();
  UNICHARSET direct_set;
  // To avoid unused codes, clear the special codes from the direct_set.
  direct_set.clear();
  // Always keep space as 0;
  direct_set.unichar_insert(" ", OldUncleanUnichars::kTrue);
  // Null char is next if we have one.
  if (null_id >= 0) {
    direct_set.unichar_insert(kNullChar);
  }
  RSCounts radical_counts;
  // In the initial map, codes [0, unicharset.size()) are
  // reserved for non-han/hangul sequences of 1 or more unicodes.
  int hangul_offset = unicharset.size();
  // Hangul takes the next range [hangul_offset, hangul_offset + kTotalJamos).
  const int kTotalJamos = kLCount + kVCount + kTCount;
  // Han takes the codes beyond hangul_offset + kTotalJamos. Since it is hard
  // to measure the number of radicals and strokes, initially we use the same
  // code range for all 3 Han code positions, and fix them after.
  int han_offset = hangul_offset + kTotalJamos;
  for (int u = 0; u <= unicharset.size(); ++u) {
    // We special-case allow null_id to be equal to unicharset.size() in case
    // there is no space in unicharset for it.
    if (u == unicharset.size() && u != null_id) break;  // Finished
    RecodedCharID code;
    // Convert to unicodes.
    std::vector<char32> unicodes;
    std::string cleaned;
    if (u < unicharset.size())
      cleaned = UNICHARSET::CleanupString(unicharset.id_to_unichar(u));
    if (u < unicharset.size() &&
        (unicodes = UNICHAR::UTF8ToUTF32(cleaned.c_str())).size() == 1) {
      // Check single unicodes for Hangul/Han and encode if so.
      int unicode = unicodes[0];
      int leading, vowel, trailing;
      auto it = radical_map.find(unicode);
      if (it != radical_map.end()) {
        // This is Han. Use the radical codes directly.
        int num_radicals = it->second->size();
        for (int c = 0; c < num_radicals; ++c) {
          code.Set(c, han_offset + (*it->second)[c]);
        }
        int pre_hash = RadicalPreHash(*it->second);
        int num_samples = radical_counts[pre_hash]++;
        if (num_samples > 0)
          code.Set(num_radicals, han_offset + num_samples + kRadicalRadix);
      } else if (DecomposeHangul(unicode, &leading, &vowel, &trailing)) {
        // This is Hangul. Since we know the exact size of each part at compile
        // time, it gets the bottom set of codes.
        code.Set3(leading + hangul_offset, vowel + kLCount + hangul_offset,
                  trailing + kLCount + kVCount + hangul_offset);
      }
    }
    // If the code is still empty, it wasn't Han or Hangul.
    if (code.length() == 0) {
      // Special cases.
      if (u == UNICHAR_SPACE) {
        code.Set(0, 0);  // Space.
      } else if (u == null_id || (unicharset.has_special_codes() &&
                                  u < SPECIAL_UNICHAR_CODES_COUNT)) {
        code.Set(0, direct_set.unichar_to_id(kNullChar));
      } else {
        // Add the direct_set unichar-ids of the unicodes in sequence to the
        // code.
        for (int uni : unicodes) {
          int position = code.length();
          if (position >= RecodedCharID::kMaxCodeLen) {
            tprintf("Unichar %d=%s is too long to encode!!\n", u,
                    unicharset.id_to_unichar(u));
            return false;
          }
          UNICHAR unichar(uni);
          char* utf8 = unichar.utf8_str();
          if (!direct_set.contains_unichar(utf8))
            direct_set.unichar_insert(utf8);
          code.Set(position, direct_set.unichar_to_id(utf8));
          delete[] utf8;
          if (direct_set.size() >
              unicharset.size() + !unicharset.has_special_codes()) {
            // Code space got bigger!
            tprintf("Code space expanded from original unicharset!!\n");
            return false;
          }
        }
      }
    }
    encoder_.push_back(code);
  }
  // Now renumber Han to make all codes unique. We already added han_offset to
  // all Han. Now separate out the radical, stroke, and count codes for Han.
  int code_offset = 0;
  for (int i = 0; i < RecodedCharID::kMaxCodeLen; ++i) {
    int max_offset = 0;
    for (int u = 0; u < unicharset.size(); ++u) {
      RecodedCharID* code = &encoder_[u];
      if (code->length() <= i) continue;
      max_offset = std::max(max_offset, (*code)(i)-han_offset);
      code->Set(i, (*code)(i) + code_offset);
    }
    if (max_offset == 0) break;
    code_offset += max_offset + 1;
  }
  DefragmentCodeValues(null_id >= 0 ? 1 : -1);
  SetupDecoder();
  return true;
}

// Sets up an encoder that doesn't change the unichars at all, so it just
// passes them through unchanged.
void UnicharCompress::SetupPassThrough(const UNICHARSET& unicharset) {
  GenericVector<RecodedCharID> codes;
  for (int u = 0; u < unicharset.size(); ++u) {
    RecodedCharID code;
    code.Set(0, u);
    codes.push_back(code);
  }
  if (!unicharset.has_special_codes()) {
    RecodedCharID code;
    code.Set(0, unicharset.size());
    codes.push_back(code);
  }
  SetupDirect(codes);
}

// Sets up an encoder directly using the given encoding vector, which maps
// unichar_ids to the given codes.
void UnicharCompress::SetupDirect(const GenericVector<RecodedCharID>& codes) {
  encoder_ = codes;
  ComputeCodeRange();
  SetupDecoder();
}

// Renumbers codes to eliminate unused values.
void UnicharCompress::DefragmentCodeValues(int encoded_null) {
  // There may not be any Hangul, but even if there is, it is possible that not
  // all codes are used. Likewise with the Han encoding, it is possible that not
  // all numbers of strokes are used.
  ComputeCodeRange();
  GenericVector<int> offsets;
  offsets.init_to_size(code_range_, 0);
  // Find which codes are used
  for (int c = 0; c < encoder_.size(); ++c) {
    const RecodedCharID& code = encoder_[c];
    for (int i = 0; i < code.length(); ++i) {
      offsets[code(i)] = 1;
    }
  }
  // Compute offsets based on code use.
  int offset = 0;
  for (int i = 0; i < offsets.size(); ++i) {
    // If not used, decrement everything above here.
    // We are moving encoded_null to the end, so it is not "used".
    if (offsets[i] == 0 || i == encoded_null) {
      --offset;
    } else {
      offsets[i] = offset;
    }
  }
  if (encoded_null >= 0) {
    // The encoded_null is moving to the end, for the benefit of TensorFlow,
    // which is offsets.size() + offsets.back().
    offsets[encoded_null] = offsets.size() + offsets.back() - encoded_null;
  }
  // Now apply the offsets.
  for (int c = 0; c < encoder_.size(); ++c) {
    RecodedCharID* code = &encoder_[c];
    for (int i = 0; i < code->length(); ++i) {
      int value = (*code)(i);
      code->Set(i, value + offsets[value]);
    }
  }
  ComputeCodeRange();
}

// Encodes a single unichar_id. Returns the length of the code, or zero if
// invalid input, and the encoding itself
int UnicharCompress::EncodeUnichar(int unichar_id, RecodedCharID* code) const {
  if (unichar_id < 0 || unichar_id >= encoder_.size()) return 0;
  *code = encoder_[unichar_id];
  return code->length();
}

// Decodes code, returning the original unichar-id, or
// INVALID_UNICHAR_ID if the input is invalid.
int UnicharCompress::DecodeUnichar(const RecodedCharID& code) const {
  int len = code.length();
  if (len <= 0 || len > RecodedCharID::kMaxCodeLen) return INVALID_UNICHAR_ID;
  auto it = decoder_.find(code);
  if (it == decoder_.end()) return INVALID_UNICHAR_ID;
  return it->second;
}

// Writes to the given file. Returns false in case of error.
bool UnicharCompress::Serialize(TFile* fp) const {
  return encoder_.SerializeClasses(fp);
}

// Reads from the given file. Returns false in case of error.
bool UnicharCompress::DeSerialize(TFile* fp) {
  if (!encoder_.DeSerializeClasses(fp)) return false;
  ComputeCodeRange();
  SetupDecoder();
  return true;
}

// Returns a STRING containing a text file that describes the encoding thus:
// <index>[,<index>]*<tab><UTF8-str><newline>
// In words, a comma-separated list of one or more indices, followed by a tab
// and the UTF-8 string that the code represents per line. Most simple scripts
// will encode a single index to a UTF8-string, but Chinese, Japanese, Korean
// and the Indic scripts will contain a many-to-many mapping.
// See the class comment above for details.
STRING UnicharCompress::GetEncodingAsString(
    const UNICHARSET& unicharset) const {
  STRING encoding;
  for (int c = 0; c < encoder_.size(); ++c) {
    const RecodedCharID& code = encoder_[c];
    if (0 < c && c < SPECIAL_UNICHAR_CODES_COUNT && code == encoder_[c - 1]) {
      // Don't show the duplicate entry.
      continue;
    }
    encoding.add_str_int("", code(0));
    for (int i = 1; i < code.length(); ++i) {
      encoding.add_str_int(",", code(i));
    }
    encoding += "\t";
    if (c >= unicharset.size() || (0 < c && c < SPECIAL_UNICHAR_CODES_COUNT &&
                                   unicharset.has_special_codes())) {
      encoding += kNullChar;
    } else {
      encoding += unicharset.id_to_unichar(c);
    }
    encoding += "\n";
  }
  return encoding;
}

// Helper decomposes a Hangul unicode to 3 parts, leading, vowel, trailing.
// Note that the returned values are 0-based indices, NOT unicode Jamo.
// Returns false if the input is not in the Hangul unicode range.
/* static */
bool UnicharCompress::DecomposeHangul(int unicode, int* leading, int* vowel,
                                      int* trailing) {
  if (unicode < kFirstHangul) return false;
  int offset = unicode - kFirstHangul;
  if (offset >= kNumHangul) return false;
  const int kNCount = kVCount * kTCount;
  *leading = offset / kNCount;
  *vowel = (offset % kNCount) / kTCount;
  *trailing = offset % kTCount;
  return true;
}

// Computes the value of code_range_ from the encoder_.
void UnicharCompress::ComputeCodeRange() {
  code_range_ = -1;
  for (int c = 0; c < encoder_.size(); ++c) {
    const RecodedCharID& code = encoder_[c];
    for (int i = 0; i < code.length(); ++i) {
      if (code(i) > code_range_) code_range_ = code(i);
    }
  }
  ++code_range_;
}

// Initializes the decoding hash_map from the encoding array.
void UnicharCompress::SetupDecoder() {
  Cleanup();
  is_valid_start_.init_to_size(code_range_, false);
  for (int c = 0; c < encoder_.size(); ++c) {
    const RecodedCharID& code = encoder_[c];
    decoder_[code] = c;
    is_valid_start_[code(0)] = true;
    RecodedCharID prefix = code;
    int len = code.length() - 1;
    prefix.Truncate(len);
    auto final_it = final_codes_.find(prefix);
    if (final_it == final_codes_.end()) {
      auto* code_list = new GenericVectorEqEq<int>;
      code_list->push_back(code(len));
      final_codes_[prefix] = code_list;
      while (--len >= 0) {
        prefix.Truncate(len);
        auto next_it = next_codes_.find(prefix);
        if (next_it == next_codes_.end()) {
          auto* code_list = new GenericVectorEqEq<int>;
          code_list->push_back(code(len));
          next_codes_[prefix] = code_list;
        } else {
          // We still have to search the list as we may get here via multiple
          // lengths of code.
          if (!next_it->second->contains(code(len)))
            next_it->second->push_back(code(len));
          break;  // This prefix has been processed.
        }
      }
    } else {
      if (!final_it->second->contains(code(len)))
        final_it->second->push_back(code(len));
    }
  }
}

// Frees allocated memory.
void UnicharCompress::Cleanup() {
  decoder_.clear();
  is_valid_start_.clear();
  for (auto& next_code : next_codes_) {
    delete next_code.second;
  }
  for (auto& final_code : final_codes_) {
    delete final_code.second;
  }
  next_codes_.clear();
  final_codes_.clear();
}

}  // namespace tesseract.
