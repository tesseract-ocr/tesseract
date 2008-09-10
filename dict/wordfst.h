// Copyright 2008 Google Inc. All Rights Reserved.

#ifndef THIRD_PARTY_TESSERACT_DICT_WORDFST_H_
#define THIRD_PARTY_TESSERACT_DICT_WORDFST_H_

#include <string>
#include <map>
#include <list>

#include "nlp/fst/lib/fstlib-inl.h"

#include "third_party/libidn/stringprep.h"
#include "util/utf8/unicodeprops.h"
#include "third_party/icu/current/unicode/uchar.h"
#include "i18n/utf8/char_properties.h"

// A string of unicode (utf32/ucs4) characters.
typedef basic_string<uint32> String;

// A Comparison functor for stl maps.
struct ltstr {
  bool operator()(const String& s1, const String& s2) const {
    return s1 < s2;
  }
};

enum ECharType {
  LOWER_CASE,
  UPPER_CASE,
  DIGIT,
  END_SENTENCE,
  START_SENTENCE,
  OPEN_EXPR,
  CLOSE_EXPR,
  OPEN_QUOTE,
  CLOSE_QUOTE,
  CURRENCY,
  DASH,
  OTHER,
  EMPTY,
  UNKNOWN
};

using nlp_fst::StdVectorFst;
using nlp_fst::StdFst;
using nlp_fst::StdArc;
using nlp_fst::ArcIterator;
using nlp_fst::StateIterator;
using nlp_fst::kNoStateId;
using nlp_fst::StdOLabelCompare;
using nlp_fst::StdILabelCompare;
using nlp_fst::kNoStateId;

class WordFst {
 public :
  WordFst();
  ~WordFst();

  bool AddWord(const String& word);

  void Output(FILE* filename);
  void LoadFromFile(FILE* filename);

  bool Matches(const String& word);

  StdVectorFst* fst();
  void SetFst(StdVectorFst);

  void WordsMatched(list<String>* results,
                    String prefix,
                    int current_node);

 private :
  unsigned int next_node_id_;
  nlp_fst::StdVectorFst fst_;
};

bool CompareWordsFsts(WordFst *a, WordFst *b);
bool MyCompareWordsFsts(WordFst *a, WordFst *b, list<String>*, int);

#endif  // THIRD_PARTY_TESSERACT_DICT_WORDFST_H_
