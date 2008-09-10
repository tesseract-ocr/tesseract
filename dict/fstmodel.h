// Copyright 2008 Google Inc. All Rights Reserved.
// Author: awiggenhauser@google.com (Amy Wiggenhauser)

#ifndef THIRD_PARTY_TESSERACT_DICT_FSTMODEL_H_
#define THIRD_PARTY_TESSERACT_DICT_FSTMODEL_H_

# include <string>
# include "patternfst.h"

enum ECharType GetType(uint32 c);
uint32 GetName(enum ECharType e);

class LanguageModel {
 public:
  static LanguageModel* Instance();
  void InitWithLanguage(const string& langid);
  ~LanguageModel();

  bool CheckWord(const String& word);

 protected:
  LanguageModel();

 private:
  static LanguageModel* instance_;
  PatternFst fst_;
};

#endif  // THIRD_PARTY_TESSERACT_DICT_FSTMODEL_H_
