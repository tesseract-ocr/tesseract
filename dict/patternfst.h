// Copyright 2008 Google Inc. All Rights Reserved.

#ifndef THIRD_PARTY_TESSERACT_DICT_PATTERNFST_H_
#define THIRD_PARTY_TESSERACT_DICT_PATTERNFST_H_

#include <string>
#include <map>
#include <list>
#include <vector>
#include "wordfst.h"

class PatternFst {
 public :
  PatternFst();
  ~PatternFst();

  bool AddPattern();

  void Output(const string& filename);

  bool PatternMatches();
  bool WordMatches();

  int GetPatternId(const String& p);

  bool IdMatchString(int, const String& p);

  bool AddWord();

  bool SubMatches(unsigned int state,
                  unsigned int vector_pos);

  bool SubWMatches(unsigned int state,
                  unsigned int vector_pos);

  void Simplify(int threshold);

  void SetCurrentWord(const String& w, const String& p);

  void Update(WordFst* old, WordFst* update);

  void Matched(list<String>* results, String prefix, int current_node);

  void SubOutput(const string& filename, int state);

  void UnderLoad(const string& filename, int current_node);
  void LoadFromFile(const string& filename);

 private :
  // the id that will be assigned to the next state that will be added
  unsigned int next_node_id_;

  // the fst that contains the patterns
  nlp_fst::StdVectorFst fst_;

  // a map to store the patterns only once each, and assign an id to
  // each of them
  map<String, int> patterns_ids_;

  // number of unique patterns (used to assign an id to the next
  // pattern to add
  unsigned int patterns_number_;

  // a map that contains pointers to the WordFsts associated to the id
  // of the corresponding state into the pattern fst.
  map<int, WordFst*> words_fst_;

  // The different parts of the current pattern
  // Example : if the current pattern is aaa.aa, then the vector
  // contains <aaa> <.> <aa> <,>
  vector<String> patterns_parts_;

  // Same as patterns_parts_ but will the real letters
  vector<String> words_parts_;

  // The current pattern
  String pattern_;

  // The current word
  String word_;

  // All the states id from the patterns fst that matches the current
  // pattern
  vector<int> path_;
};

string ConvertStringToStdString(const String& s);
String ConvertStdStringToString(const string& word);

#endif  // THIRD_PARTY_TESSERACT_DICT_PATTERNFST_H_
