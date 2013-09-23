#include "normstrngs.h"

#include "icuerrorcode.h"
#include "unichar.h"
#include "unicode/normalizer2.h"  // From libicu
#include "unicode/unorm2.h"       // From libicu

namespace tesseract {

void UTF8ToUTF32(const char* utf8_str, GenericVector<char32>* str32) {
  str32->clear();
  str32->reserve(strlen(utf8_str));
  int len = strlen(utf8_str);
  int step = 0;
  for (int ch = 0; ch < len; ch += step) {
    step = UNICHAR::utf8_step(utf8_str + ch);
    if (step > 0) {
      UNICHAR uni_ch(utf8_str + ch, step);
      (*str32) += uni_ch.first_uni();
    }
  }
}

void UTF32ToUTF8(const GenericVector<char32>& str32, STRING* utf8_str) {
  utf8_str->ensure(str32.length());
  utf8_str->assign("", 0);
  for (int i = 0; i < str32.length(); ++i) {
    UNICHAR uni_ch(str32[i]);
    char *utf8 = uni_ch.utf8_str();
    if (utf8 != NULL) {
      (*utf8_str) += utf8;
      delete[] utf8;
    }
  }
}

bool is_hyphen_punc(const char32 ch) {
  static const int kNumHyphenPuncUnicodes = 13;
  static const char32 kHyphenPuncUnicodes[kNumHyphenPuncUnicodes] = {
    '-',
    0x2010, 0x2011, 0x2012, 0x2013, 0x2014, 0x2015,  // hyphen..horizontal bar
    0x207b,  // superscript minus
    0x208b,  // subscript minus
    0x2212,  // minus sign
    0xfe58,  // small em dash
    0xfe63,  // small hyphen-minus
    0xff0d,  // fullwidth hyphen-minus
  };
  for (int i = 0; i < kNumHyphenPuncUnicodes; ++i) {
    if (kHyphenPuncUnicodes[i] == ch)
      return true;
  }
  return false;
}

bool is_single_quote(const char32 ch) {
  static const int kNumSingleQuoteUnicodes = 8;
  static const char32 kSingleQuoteUnicodes[kNumSingleQuoteUnicodes] = {
    '\'',
    '`',
    0x2018,  // left single quotation mark (English, others)
    0x2019,  // right single quotation mark (Danish, Finnish, Swedish, Norw.)
             // We may have to introduce a comma set with 0x201a
    0x201B,  // single high-reveresed-9 quotation mark (PropList.txt)
    0x2032,  // prime
    0x300C,  // left corner bracket (East Asian languages)
    0xFF07,  // fullwidth apostrophe
  };
  for (int i = 0; i < kNumSingleQuoteUnicodes; ++i) {
    if (kSingleQuoteUnicodes[i] == ch)
      return true;
  }
  return false;
}

bool is_double_quote(const char32 ch) {
  static const int kNumDoubleQuoteUnicodes = 8;
  static const char32 kDoubleQuoteUnicodes[kNumDoubleQuoteUnicodes] = {
    '"',
    0x201C,  // left double quotation mark (English, others)
    0x201D,  // right double quotation mark (Danish, Finnish, Swedish, Norw.)
    0x201F,  // double high-reversed-9 quotation mark (PropList.txt)
    0x2033,  // double prime
    0x301D,  // reversed double prime quotation mark (East Asian langs, horiz.)
    0x301E,  // close double prime (East Asian languages written horizontally)
    0xFF02,  // fullwidth quotation mark
  };
  for (int i = 0; i < kNumDoubleQuoteUnicodes; ++i) {
    if (kDoubleQuoteUnicodes[i] == ch)
      return true;
  }
  return false;
}

STRING NormalizeUTF8String(const char* str8) {
  GenericVector<char32> str32, out_str32, norm_str;
  UTF8ToUTF32(str8, &str32);
  for (int i = 0; i < str32.length(); ++i) {
    norm_str.clear();
    NormalizeChar32(str32[i], &norm_str);
    for (int j = 0; j < norm_str.length(); ++j) {
      out_str32.push_back(norm_str[j]);
    }
  }
  STRING out_str8;
  UTF32ToUTF8(out_str32, &out_str8);
  return out_str8;
}

void NormalizeChar32(char32 ch, GenericVector<char32>* str) {
  IcuErrorCode error_code;
  const icu::Normalizer2* nfkc = icu::Normalizer2::getInstance(
      NULL, "nfkc", UNORM2_COMPOSE, error_code);
  error_code.assertSuccess();
  error_code.reset();

  icu::UnicodeString uch_str(static_cast<UChar32>(ch));
  icu::UnicodeString norm_str = nfkc->normalize(uch_str, error_code);
  error_code.assertSuccess();

  str->clear();
  for (int i = 0; i < norm_str.length(); ++i) {
    // If any spaces were added by NFKC, pretend normalization is a nop.
    if (norm_str[i] == ' ') {
      str->clear();
      str->push_back(ch);
      break;
    } else {
      str->push_back(OCRNormalize(static_cast<char32>(norm_str[i])));
    }
  }
}

// Apply just the OCR-specific normalizations and return the normalized char.
char32 OCRNormalize(char32 ch) {
  if (is_hyphen_punc(ch))
    return '-';
  else if (is_single_quote(ch))
    return '\'';
  else if (is_double_quote(ch))
    return '"';
  return ch;
}

bool IsOCREquivalent(char32 ch1, char32 ch2) {
  return OCRNormalize(ch1) == OCRNormalize(ch2);
}


}  // namespace tesseract
