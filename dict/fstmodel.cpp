// Copyright 2008 Google Inc. All Rights Reserved.
// Author: awiggenhauser@google.com (Amy Wiggenhauser)

#include <string>
#include "fstmodel.h"
#include "patternfst.h"
#include "wordfst.h"
#include "dict.h"

namespace tesseract {
int Dict::fst_letter_is_okay(void *dawg,
                             void* node,
                             int char_index,
                             char prevchar,
                             const char *word,
                             int word_end) {
  if (word_end == 0)
    return 1;
  string current_word = string(word);
  String uchar_word = ConvertStdStringToString(current_word);
  return LanguageModel::Instance()->CheckWord(uchar_word);
}
}  // namespace Tesseract

// Used to get the pattern of a word
enum ECharType GetType(uint32 c) {
  if (UnicodeProps::IsWhitespace(c))
    return EMPTY;
  if (UnicodeProps::IsUpper(c))
    return UPPER_CASE;
  if (UnicodeProps::IsLower(c))
    return LOWER_CASE;
  if (u_isdigit(c))
    return DIGIT;
  if (is_start_sentence_punc(c))
    return START_SENTENCE;
  if (is_end_sentence_punc(c))
    return END_SENTENCE;
  if (is_open_expr_punc(c))
    return OPEN_EXPR;
  if (is_close_expr_punc(c))
    return CLOSE_EXPR;
  if (is_open_quote(c))
    return OPEN_QUOTE;
  if (is_close_quote(c))
    return CLOSE_QUOTE;
  if (is_other_punc(c))
    return OTHER;
  if (is_currency_symbol(c))
    return CURRENCY;
  if (is_dash_punc(c))
    return DASH;
  return UNKNOWN;
}


uint32 GetName(enum ECharType e) {
  char type_array[15] = {'a',  // lower_case
                         'a',  // upper_case
                         '1',  // digit
                         '.',  // end sentence
                         '#',  // start sentence
                         '(',  // open expression
                         ')',  // close expression
                         '<',  // open quote
                         '>',  // close quote
                         '$',  // currency symbol
                         '-',  // dashes
                         ',',  // other
                         '0',  // empty
                         '?'};  // unknown
  // We regroup lower and upper case together to avoid
  // a lot of bad combinaisons like in this example :
  // Garden and Kitten would create
  // Garden Gitten Karten Kitten
  return type_array[e];
}

String PatternizeWord(const String& s) {
  String res = String();
  for (unsigned int i = 0; i < s.size(); i++)
    res.push_back(GetName(GetType(s[i])));
  return res;
}


LanguageModel* LanguageModel::instance_ = NULL;

LanguageModel* LanguageModel::Instance() {
  if (instance_ == 0) {
    instance_ = new LanguageModel;
  }
  return instance_;
}

LanguageModel::LanguageModel() {
}

void LanguageModel::InitWithLanguage(const string& langid) {
  fst_.LoadFromFile(langid);
}

bool LanguageModel::CheckWord(const String& word) {
  fst_.SetCurrentWord(word, PatternizeWord(word));
  return fst_.WordMatches();
}

LanguageModel::~LanguageModel() {
}
