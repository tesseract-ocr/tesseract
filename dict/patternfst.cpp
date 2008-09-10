// Copyright 2008 Google Inc. All Rights Reserved.

#include <string>
#include <list>
#include <map>
#include <vector>
#include "patternfst.h"

// Utility function to print a String down into the LOG(INFO)
string ConvertStringToStdString(const String& s) {
  size_t written = 0;
  char* utf8_str = stringprep_ucs4_to_utf8(s.data(),
                                           s.length(),
                                           NULL, &written);
  string res = utf8_str;
  free(utf8_str);
  return res;
}

// Utility function to turn a std::string into a String
String ConvertStdStringToString(const string& word) {
  char* norm_word = stringprep_utf8_nfkc_normalize(word.data(),
                                                   word.length());
  uint32* uni_word = stringprep_utf8_to_ucs4(norm_word, -1, NULL);
  String uni_str(uni_word);
  free(norm_word);
  free(uni_word);
  return uni_str;
}


PatternFst::PatternFst() : next_node_id_(1),
                           fst_(StdVectorFst()),
                           patterns_ids_(map<String, int>()),
                           patterns_number_(1),
                           words_fst_(map<int, WordFst*>()) {
  fst_.AddState();
  fst_.SetStart(0);
}

PatternFst::~PatternFst() {
  for (map<int, WordFst*>::iterator mapi = words_fst_.begin();
       mapi != words_fst_.end();
       mapi++) {
    if (mapi->second != NULL)
      delete mapi->second;
  }
}

// Set the current word to treat.
// Usefull to avoid computing the pattern and the words parts several times.
// Here we compute it once for all.
//
// Call this function before AddPattern, AddWord, PatternMatches or WordMatches
void PatternFst::SetCurrentWord(const String& w, const String& p) {
  word_ = w;
  pattern_ = p;
  unsigned int size = w.size();
  patterns_parts_ = vector<String>();
  words_parts_ = vector<String>();
  unsigned int i = 0;
  while (i < size) {
    char current_char = p[i];
    String tmp = String();
    String tmp2 = String();
    while (p[i] == current_char && i < size) {
      tmp.push_back(w[i]);
      tmp2.push_back(p[i]);
      i++;
    }
    patterns_parts_.push_back(tmp2);
    words_parts_.push_back(tmp);
  }
  path_ = vector<int>(patterns_parts_.size(), 0);
}

int PatternFst::GetPatternId(const String& p) {
  if (patterns_ids_[p] == 0) {
    patterns_ids_[p] = patterns_number_;
    patterns_number_++;
  }
  return patterns_ids_[p];
}

// Add the current pattern (from the patterns_parts_ variable)
// into the fst
bool PatternFst::AddPattern() {
  if (PatternMatches())
    return true;

  int current_node = 0;
  int path_iter = 0;

  for (vector<String>::iterator ivect = patterns_parts_.begin();
       ivect != patterns_parts_.end();
       ivect++) {
    fst_.AddState();
    fst_.AddArc(current_node, StdArc(GetPatternId(*ivect),
                                     GetPatternId(*ivect),
                                     1,
                                     next_node_id_));
    if (!words_fst_[next_node_id_])
      words_fst_[next_node_id_] = new WordFst();
    path_[path_iter] = next_node_id_;
    current_node = next_node_id_;
    next_node_id_++;
    path_iter++;
  }
  fst_.SetFinal(current_node, 1);
  return true;
}

// Add the current pattern into the fst and then
// add the current word (from the words_parts_ variable)
bool PatternFst::AddWord() {
  if (!AddPattern())
    return false;
  for (int i = 0; i < words_parts_.size(); i++) {
    words_fst_[path_[i]]->AddWord(words_parts_[i]);
  }
  return true;
}

// Return true if the given <id> correspond to the given string <pat>
bool PatternFst::IdMatchString(int id, const String& pat) {
  if (id == 0)
    return false;
  int tst = patterns_ids_[pat];
  if (tst == 0)
    patterns_ids_.erase(pat);
  return tst == id;
}

// Recursive call used by the PatternMatches() function below
// Check if the current pattern is matched by the fst
bool PatternFst::SubMatches(unsigned int state,
                       unsigned int vector_pos) {
  ArcIterator<StdFst> arc_iter(fst_, state);
  if (vector_pos == patterns_parts_.size() && arc_iter.Done())
    return true;
  for (; !arc_iter.Done(); arc_iter.Next()) {
    const StdArc &arc = arc_iter.Value();
    if (IdMatchString(arc.ilabel, patterns_parts_[vector_pos]) &&
        SubMatches(arc.nextstate, vector_pos + 1)) {
      path_[vector_pos] = arc.nextstate;
      return true;
    }
  }
  return false;
}

// Wrapper function for the SubMatches function above
bool PatternFst::PatternMatches() {
  return SubMatches(0, 0);
}

// Recursive call used by the WordMatches() function below
// Check if the current word is matched by the fst
bool PatternFst::SubWMatches(unsigned int state,
                             unsigned int vector_pos) {
  ArcIterator<StdFst> arc_iter(fst_, state);
  if (vector_pos == patterns_parts_.size() && arc_iter.Done())
    return true;
  for (; !arc_iter.Done(); arc_iter.Next()) {
    const StdArc &arc = arc_iter.Value();
    if (IdMatchString(arc.ilabel, patterns_parts_[vector_pos]) &&
        words_fst_[arc.nextstate]->Matches(words_parts_[vector_pos]) &&
        SubWMatches(arc.nextstate, vector_pos + 1)) {
      path_[vector_pos] = arc.nextstate;
      return true;
    }
  }
  return false;
}

// Wrapper function for the SubWMatches function above
bool PatternFst::WordMatches() {
  return SubWMatches(0, 0);
}

// Get all the words matched by the fst into the <results> vector
// from the state <current_node> and add the prefix <prefix> to them
void PatternFst::Matched(list<String>* results,
                         String prefix, int current_node) {
  ArcIterator<StdFst> arc_iter(fst_, current_node);
  if (arc_iter.Done()) {
    results->push_back(prefix);
    return;
  }
  for (; !arc_iter.Done(); arc_iter.Next()) {
    const StdArc &arc = arc_iter.Value();
    list<String> words;
    words_fst_[arc.nextstate]->WordsMatched(&words, String(), 0);
    for (list<String>::iterator i = words.begin();
         i != words.end();
         i++) {
      String tmp = prefix;
      tmp.append(*i);
      Matched(results, tmp, arc.nextstate);
    }
  }
}

void PatternFst::LoadFromFile(const string& filename) {
  FILE *f = fopen(filename.c_str(), "r");
  if (!f) {
    LOG(ERROR) << "Could not read file " << filename;
    return;
  }
  char buffer[100];
  int num = 0;
  fscanf(f, "%s %d\n", buffer, &patterns_number_);
  fscanf(f, "%s %d\n", buffer, &next_node_id_);
  fscanf(f, "%s %d\n", buffer, &num);
  while (string(buffer) != "PATTERNFST") {
    patterns_ids_[ConvertStdStringToString(buffer)] = num;
    fscanf(f, "%s %d\n", buffer, &num);
  }
  for (unsigned int i = 0; i < num; i++)
    fst_.AddState();
  fst_.SetStart(0);
  unsigned int current_state = 0;
  while (fscanf(f, "%s %d\n", buffer, &num) != EOF) {
    if (string(buffer) == "ENDPATTERNFST")
      break;
    if (string(buffer) == "EndState")
      continue;
    if (string(buffer) == "State")
      current_state = num;
    else
      fst_.AddArc(current_state, StdArc(atoi(buffer),
                                        atoi(buffer),
                                        0,
                                        num));
  }
  for (StateIterator<StdFst> siter(fst_); !siter.Done(); siter.Next()) {
    fscanf(f, "%s\n", buffer);
    if (string(buffer) == "Ok") {
      if (!words_fst_[siter.Value()])
        words_fst_[siter.Value()] = new WordFst();
      words_fst_[siter.Value()]->LoadFromFile(f);
    }
  }
  fclose(f);
}

void PatternFst::Output(const string& filename) {
  FILE *f = fopen(filename.c_str(), "w");
  if (!f) {
    LOG(ERROR) << "Could not open file " << filename;
    return;
  }
  fprintf(f, "PATTERNSNB %d\n", patterns_number_);
  fprintf(f, "NEXTID %d\n", next_node_id_);
  for (map<String, int>::iterator mapi = patterns_ids_.begin();
       mapi != patterns_ids_.end();
       mapi++) {
    if (mapi->second != 0)
      fprintf(f, "%s %d\n",
              ConvertStringToStdString(mapi->first).c_str(), mapi->second);
  }
  fprintf(f, "PATTERNFST %d\n", next_node_id_);
  for (StateIterator<StdFst> siter(fst_); !siter.Done(); siter.Next()) {
    fprintf(f, "State %d\n", siter.Value());
    ArcIterator<StdFst> arc_iter(fst_, siter.Value());
    for (; !arc_iter.Done(); arc_iter.Next()) {
      const StdArc &arc = arc_iter.Value();
      fprintf(f, "%d %d\n", arc.ilabel, arc.nextstate);
    }
    fprintf(f, "EndState %d\n", siter.Value());
  }
  fprintf(f, "ENDPATTERNFST 0\n");
  for (StateIterator<StdFst> siter(fst_); !siter.Done(); siter.Next()) {
    if (words_fst_[siter.Value()]) {
      fprintf(f, "Ok\n");
      words_fst_[siter.Value()]->Output(f);
    } else {
      fprintf(f, "Ko\n");
    }
  }
  fclose(f);
}

// Regroup two WordFst by replacing the entries of <old> by <update>
// into the patterns_ids_ map
void PatternFst::Update(WordFst* old, WordFst* update) {
  for (map<int, WordFst*>::iterator iw = words_fst_.begin();
       iw != words_fst_.end();
       iw++)
    if (iw->second == old)
      words_fst_[iw->first] = update;
  delete old;
}

// Compare all the WordFsts and regroup them if necessary
void PatternFst::Simplify(int max) {
  for (map<int, WordFst*>::iterator iw = words_fst_.begin();
       iw != words_fst_.end();
       iw++) {
    for (map<int, WordFst*>::iterator iw2 = words_fst_.begin();
         iw2 != words_fst_.end();
         iw2++) {
      if (iw2 != iw) {
        list<String> diff;
        if (MyCompareWordsFsts(iw->second, iw2->second, &diff, max)) {
          if (iw->second != iw2->second) {
            Update(iw2->second, iw->second);
          }
        }
      }
    }
  }
}
