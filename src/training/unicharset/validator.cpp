#include "validator.h"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <iterator>

#include "icuerrorcode.h"
#include "unicode/uchar.h"    // From libicu
#include "unicode/uscript.h"  // From libicu
#include "validate_grapheme.h"
#include "validate_indic.h"
#include "validate_javanese.h"
#include "validate_khmer.h"
#include "validate_myanmar.h"

namespace tesseract {

// Some specific but universally useful unicodes.
const char32 Validator::kZeroWidthSpace = 0x200B;
const char32 Validator::kZeroWidthNonJoiner = 0x200C;
const char32 Validator::kZeroWidthJoiner = 0x200D;
const char32 Validator::kLeftToRightMark = 0x200E;
const char32 Validator::kRightToLeftMark = 0x200F;
const char32 Validator::kInvalid = 0xfffd;

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
Validator::~Validator() = default;

// Validates and cleans the src vector of unicodes to the *dest, according to
// g_mode. In the case of kSingleString, a single vector containing the whole
// result is added to *dest. With kCombined, multiple vectors are added to
// *dest with one grapheme in each. With kGlyphSplit, multiple vectors are
// added to *dest with a smaller unit representing a glyph in each.
// In case of validation error, returns false and as much as possible of the
// input, without discarding invalid text.
/* static */
bool Validator::ValidateCleanAndSegment(
    GraphemeNormMode g_mode, bool report_errors, const std::vector<char32>& src,
    std::vector<std::vector<char32>>* dest) {
  ValidateGrapheme g_validator(ViramaScript::kNonVirama, report_errors);
  std::vector<std::vector<char32>> graphemes;
  ViramaScript script = MostFrequentViramaScript(src);
  bool success = true;
  if (script == ViramaScript::kNonVirama) {
    // The grapheme segmenter's maximum segmentation is the grapheme unit, so
    // up the mode by 1 to get the desired effect.
    if (g_mode == GraphemeNormMode::kCombined)
      g_mode = GraphemeNormMode::kGlyphSplit;
    else if (g_mode == GraphemeNormMode::kGlyphSplit)
      g_mode = GraphemeNormMode::kIndividualUnicodes;
    // Just do grapheme segmentation.
    success = g_validator.ValidateCleanAndSegmentInternal(g_mode, src, dest);
  } else {
    success = g_validator.ValidateCleanAndSegmentInternal(
        GraphemeNormMode::kGlyphSplit, src, &graphemes);
    std::unique_ptr<Validator> validator(
        ScriptValidator(script, report_errors));
    for (const auto& grapheme : graphemes) {
      if (!validator->ValidateCleanAndSegmentInternal(g_mode, grapheme, dest)) {
        success = false;
      }
    }
  }
  return success;
}

// Factory method that understands how to map script to the right subclass.
std::unique_ptr<Validator> Validator::ScriptValidator(ViramaScript script,
                                                      bool report_errors) {
  switch (script) {
    case ViramaScript::kNonVirama:
      return std::unique_ptr<Validator>(
          new ValidateGrapheme(script, report_errors));
    case ViramaScript::kJavanese:
      return std::unique_ptr<Validator>(
          new ValidateJavanese(script, report_errors));
    case ViramaScript::kMyanmar:
      return std::unique_ptr<Validator>(
          new ValidateMyanmar(script, report_errors));
    case ViramaScript::kKhmer:
      return std::unique_ptr<Validator>(
          new ValidateKhmer(script, report_errors));
    default:
      return std::unique_ptr<Validator>(
          new ValidateIndic(script, report_errors));
  }
}

// Internal version of the public static ValidateCleanAndSegment.
// Validates and cleans the src vector of unicodes to the *dest, according to
// its type and the given g_mode.
// In case of validation error, returns false and returns as much as possible
// of the input, without discarding invalid text.
bool Validator::ValidateCleanAndSegmentInternal(
    GraphemeNormMode g_mode, const std::vector<char32>& src,
    std::vector<std::vector<char32>>* dest) {
  Clear();
  ComputeClassCodes(src);
  bool success = true;
  for (codes_used_ = 0; codes_used_ < codes_.size();) {
    if (!ConsumeGraphemeIfValid()) {
      success = false;
      ++codes_used_;
    }
  }
  MoveResultsToDest(g_mode, dest);
  return success;
}

// Moves the results from parts_ or output_ to dest according to g_mode.
void Validator::MoveResultsToDest(GraphemeNormMode g_mode,
                                  std::vector<std::vector<char32>>* dest) {
  if (g_mode == GraphemeNormMode::kIndividualUnicodes) {
    // Append each element of the combined output_ that we made as a new vector
    // in dest.
    dest->reserve(dest->size() + output_.size());
    for (char32 ch : output_) dest->push_back({ch});
  } else if (g_mode == GraphemeNormMode::kGlyphSplit) {
    // Append all the parts_ that we made onto dest.
    std::move(parts_.begin(), parts_.end(), std::back_inserter(*dest));
  } else if (g_mode == GraphemeNormMode::kCombined || dest->empty()) {
    // Append the combined output_ that we made onto dest as one new vector.
    dest->push_back(std::vector<char32>());
    output_.swap(dest->back());
  } else {  // kNone.
    // Append the combined output_ that we made onto the last existing element
    // of dest.
    dest->back().insert(dest->back().end(), output_.begin(), output_.end());
  }
}

static bool CmpPairSecond(const std::pair<int, int>& p1,
                          const std::pair<int, int>& p2) {
  return p1.second < p2.second;
}

// Computes and returns the ViramaScript corresponding to the most frequent
// virama-using script in the input, or kNonVirama if none are present.
/* static */
ViramaScript Validator::MostFrequentViramaScript(
    const std::vector<char32>& utf32) {
  std::unordered_map<int, int> histogram;
  for (char32 ch : utf32) {
    // Determine the codepage base. For the Indic scripts, Khmer and Javanese, it is
    // sufficient to divide by kIndicCodePageSize but Myanmar is all over the
    // unicode code space, so use its script id.
    int base = ch / kIndicCodePageSize;
    IcuErrorCode err;
    UScriptCode script_code = uscript_getScript(ch, err);
    if ((kMinIndicUnicode <= ch && ch <= kMaxJavaneseUnicode &&
         script_code != USCRIPT_COMMON) ||
        script_code == USCRIPT_MYANMAR) {
      if (script_code == USCRIPT_MYANMAR)
        base = static_cast<char32>(ViramaScript::kMyanmar) / kIndicCodePageSize;
      ++histogram[base];
    }
  }
  if (!histogram.empty()) {
    int base =
        std::max_element(histogram.begin(), histogram.end(), CmpPairSecond)
            ->first;
    char32 codebase = static_cast<char32>(base * kIndicCodePageSize);
    // Check for validity.
    if (codebase == static_cast<char32>(ViramaScript::kMyanmar) ||
        codebase == static_cast<char32>(ViramaScript::kJavanese) ||
        codebase == static_cast<char32>(ViramaScript::kKhmer) ||
        (static_cast<char32>(ViramaScript::kDevanagari) <= codebase &&
         codebase <= static_cast<char32>(ViramaScript::kSinhala))) {
      return static_cast<ViramaScript>(codebase);
    }
  }
  return ViramaScript::kNonVirama;
}

// Returns true if the given UTF-32 unicode is a "virama" character.
/* static */
bool Validator::IsVirama(char32 unicode) {
  return (kMinIndicUnicode <= unicode && unicode <= kMaxSinhalaUnicode &&
          (unicode & 0x7f) == 0x4d) ||
         unicode == kSinhalaVirama ||
         unicode == kJavaneseVirama ||
         unicode == kMyanmarVirama ||
         unicode == kKhmerVirama;
}

// Returns true if the given UTF-32 unicode is a vedic accent.
/* static */
bool Validator::IsVedicAccent(char32 unicode) {
  return (0x1cd0 <= unicode && unicode < 0x1d00) ||
          (0xa8e0 <= unicode && unicode <= 0xa8f7) ||
          (0x951  <= unicode && unicode <= 0x954);
}

// Returns true if the script is one that uses subscripts for conjuncts.
bool Validator::IsSubscriptScript() const {
  return script_ == ViramaScript::kTelugu ||
         script_ == ViramaScript::kKannada ||
         script_ == ViramaScript::kJavanese ||
         script_ == ViramaScript::kMyanmar ||
         script_ == ViramaScript::kKhmer;
}

void Validator::ComputeClassCodes(const std::vector<char32>& text) {
  codes_.reserve(text.size());
  for (char32 c : text) {
    codes_.push_back(std::make_pair(UnicodeToCharClass(c), c));
  }
}

// Resets to the initial state.
void Validator::Clear() {
  codes_.clear();
  parts_.clear();
  output_.clear();
  codes_used_ = 0;
  output_used_ = 0;
}

}  // namespace tesseract
