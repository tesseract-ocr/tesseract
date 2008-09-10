// Copyright 2008 Google Inc. All Rights Reserved.

#include <string>
#include <map>
#include <list>
#include "wordfst.h"

WordFst::WordFst() : next_node_id_(1),
                     fst_(StdVectorFst()) {
  fst_.AddState();
  fst_.SetStart(0);
}

WordFst::~WordFst() {
}

// Add the given word into the fst
bool WordFst::AddWord(const String& w) {
  int current_node = 0;
  unsigned int size = w.size();

  for (unsigned int String_iter = 0;
       String_iter < size;
       String_iter++) {
    ArcIterator<StdFst> arc_iter(fst_, current_node);
    for (;
         !arc_iter.Done(); arc_iter.Next()) {
      const StdArc &arc = arc_iter.Value();
      if (arc.ilabel == w[String_iter]) {
        current_node = arc.nextstate;
        break;
      }
    }
    if (arc_iter.Done()) {
      fst_.AddState();
      fst_.AddArc(current_node, StdArc(w[String_iter],
                                       w[String_iter],
                                       0,
                                       next_node_id_));
      current_node = next_node_id_;
      next_node_id_++;
    }
  }
  fst_.SetFinal(current_node, 0);
  return true;
}

// Return true if the given word is matched by the fst
bool WordFst::Matches(const String& w) {
  int current_node = 0;
  unsigned int size = w.size();
  unsigned int String_iter = 0;

  for (;
       String_iter < size;
       String_iter++) {
    ArcIterator<StdFst> arc_iter(fst_, current_node);
    for (;
         !arc_iter.Done(); arc_iter.Next()) {
      const StdArc &arc = arc_iter.Value();
      if (arc.ilabel == w[String_iter]) {
        current_node = arc.nextstate;
        break;
      }
    }
    if (arc_iter.Done() && !String_iter == size - 1)
      return false;
  }
  // Not necessary to test if the state is final because all the
  // words into the same WordFst have the same length
  return true;
}

void WordFst::LoadFromFile(FILE *f) {
  char buffer[100];
  unsigned int nb_of_states = 0;
  fscanf(f, "%s %d\n", buffer, &nb_of_states);
  for (unsigned int i = 0; i < nb_of_states; i++)
    fst_.AddState();
  fst_.SetStart(0);
  unsigned int num = 0;
  unsigned int current_state = 0;
  while (fscanf(f, "%s %d\n", buffer, &num) != EOF) {
    if (string(buffer) == "ENDWORDFST")
      return;
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
}

// Store the fst into a file
void WordFst::Output(FILE *f) {
  fprintf(f, "WORDFST %d\n", next_node_id_);
  for (StateIterator<StdFst> siter(fst_); !siter.Done(); siter.Next()) {
    fprintf(f, "State %d\n", siter.Value());
    ArcIterator<StdFst> arc_iter(fst_, siter.Value());
    for (; !arc_iter.Done(); arc_iter.Next()) {
      const StdArc &arc = arc_iter.Value();
      fprintf(f, "%d %d\n", arc.ilabel, arc.nextstate);
    }
    fprintf(f, "EndState %d\n", siter.Value());
  }
  fprintf(f, "ENDWORDFST 0\n");
}

StdVectorFst* WordFst::fst() {
  return &fst_;
}

void WordFst::SetFst(StdVectorFst svf) {
  fst_ = svf;
}

// Get all the words matched by the fst starting at the node <current_node>
void WordFst::WordsMatched(list<String>* results,
                           String prefix,
                           int current_node) {
  ArcIterator<StdFst> arc_iter(fst_, current_node);
  if (arc_iter.Done()) {
    results->push_back(prefix);
    return;
  }
  for (; !arc_iter.Done(); arc_iter.Next()) {
    const StdArc &arc = arc_iter.Value();
    String tmp = prefix;
    tmp.push_back(arc.ilabel);
    WordsMatched(results, tmp, arc.nextstate);
  }
}

// Compare two fst (fast way)
// Can be modified to accept small differencies between 2 wordfst
bool MyCompareWordsFsts(WordFst *a, WordFst *b,
                        list<String>* la,
                        int max_size) {
  la->clear();
  a->WordsMatched(la, String(), 0);
  b->WordsMatched(la, String(), 0);

  la->sort();
  unsigned int size1 = la->size();
  la->unique();
  unsigned int size2 = la->size();
  return 2 * size2 <= (max_size + size1);
}

// Compare two fst (slow because of the Difference call)
bool CompareWordsFsts(WordFst *a, WordFst *b) {
  StdVectorFst* afst = a->fst();
  StdVectorFst* bfst = b->fst();

  if (a == b || afst->Start() == kNoStateId || bfst->Start() == kNoStateId)
    return false;
  StdVectorFst c;

  StdVectorFst d;
  RmEpsilon(bfst);
  Determinize(*bfst, &d);
  ArcSort(afst, StdOLabelCompare());
  ArcSort(&d, StdILabelCompare());
  Difference(*afst, d, &c);
  return (c.Start() == kNoStateId);
}
