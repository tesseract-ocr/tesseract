#include "tesseract/training/validator.h"

namespace tesseract {
namespace {

class TestableValidator : public Validator {
 public:
  static ViramaScript TestableMostFrequentViramaScript(
      const std::vector<char32>& utf32) {
    return MostFrequentViramaScript(utf32);
  }
};

// The majority of Validator is tested by the script-specific tests of its
// subclasses, but the MostFrequentViramaScript function is worth a unittest.
TEST(ValidatorTest, MostFrequentViramaScript) {
  // The most frequent virama script should come out correct, despite
  // distractions from other scripts.
  EXPECT_EQ(ViramaScript::kTelugu,
            TestableValidator::TestableMostFrequentViramaScript({0xc05}));
  // It is still Telugu surrounded by Latin.
  EXPECT_EQ(ViramaScript::kTelugu,
            TestableValidator::TestableMostFrequentViramaScript(
                {'a', 0xc05, 'b', 'c'}));
  // But not still Telugu surrounded by Devanagari.
  EXPECT_EQ(ViramaScript::kDevanagari,
            TestableValidator::TestableMostFrequentViramaScript(
                {0x905, 0xc05, 0x906, 0x907}));
  EXPECT_EQ(ViramaScript::kKannada,
            TestableValidator::TestableMostFrequentViramaScript(
                {0xc85, 0xc05, 0xc86, 0xc87}));
  EXPECT_EQ(ViramaScript::kBengali,
            TestableValidator::TestableMostFrequentViramaScript(
                {0x985, 0xc05, 0x986, 0x987}));
  // Danda and double Danda don't count as Devanagari, as they are common.
  EXPECT_EQ(ViramaScript::kTelugu,
            TestableValidator::TestableMostFrequentViramaScript(
                {0x964, 0xc05, 0x965, 0x965}));
}

// ValidateCleanAndSegment doesn't modify the input by much, but its
// transformation should be idempotent. (Doesn't change again if re-applied.)
TEST(ValidatorTest, Idempotency) {
  std::vector<char32> str1(
      {0xd24, 0xd23, 0xd32, 0xd4d, '\'', 0x200d, 0x200c, 0x200d, 0x200c});
  std::vector<char32> str2(
      {0xd24, 0xd23, 0xd32, 0xd4d, 0x200c, 0x200d, 0x200c, 0x200d, '\''});
  std::vector<std::vector<char32>> result1, result2, result3, result4;
  EXPECT_TRUE(Validator::ValidateCleanAndSegment(
      GraphemeNormMode::kSingleString, true, str1, &result1));
  EXPECT_TRUE(Validator::ValidateCleanAndSegment(
      GraphemeNormMode::kSingleString, true, result1[0], &result2));
  EXPECT_EQ(result1.size(), result2.size());
  EXPECT_THAT(result2[0], testing::ElementsAreArray(result1[0]));
  EXPECT_TRUE(Validator::ValidateCleanAndSegment(
      GraphemeNormMode::kSingleString, true, str2, &result3));
  EXPECT_TRUE(Validator::ValidateCleanAndSegment(
      GraphemeNormMode::kSingleString, true, result3[0], &result4));
  EXPECT_EQ(result3.size(), result4.size());
  EXPECT_THAT(result4[0], testing::ElementsAreArray(result3[0]));
}

}  // namespace
}  // namespace tesseract
