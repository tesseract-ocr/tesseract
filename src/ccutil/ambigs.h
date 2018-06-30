///////////////////////////////////////////////////////////////////////
// File:        ambigs.h
// Description: Constants, flags, functions for dealing with
//              ambiguities (training and recognition).
// Author:      Daria Antonova
// Created:     Mon Aug 23 11:26:43 PDT 2008
//
// (C) Copyright 2008, Google Inc.
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

#ifndef TESSERACT_CCUTIL_AMBIGS_H_
#define TESSERACT_CCUTIL_AMBIGS_H_

#include "elst.h"
#include "tprintf.h"
#include "unichar.h"
#include "unicharset.h"
#include "genericvector.h"

#define MAX_AMBIG_SIZE    10

namespace tesseract {

using UnicharIdVector = GenericVector<UNICHAR_ID>;

static const int kUnigramAmbigsBufferSize = 1000;
static const char kAmbigNgramSeparator[] = { ' ', '\0' };
static const char kAmbigDelimiters[] = "\t ";
static const char kIllegalMsg[] =
  "Illegal ambiguity specification on line %d\n";
static const char kIllegalUnicharMsg[] =
  "Illegal unichar %s in ambiguity specification\n";

enum AmbigType {
  NOT_AMBIG,        // the ngram pair is not ambiguous
  REPLACE_AMBIG,    // ocred ngram should always be substituted with correct
  DEFINITE_AMBIG,   // add correct ngram to the classifier results (1-1)
  SIMILAR_AMBIG,    // use pairwise classifier for ocred/correct pair (1-1)
  CASE_AMBIG,       // this is a case ambiguity (1-1)

  AMBIG_TYPE_COUNT  // number of enum entries
};

// A collection of utility functions for arrays of UNICHAR_IDs that are
// terminated by INVALID_UNICHAR_ID.
class UnicharIdArrayUtils {
 public:
  // Compares two arrays of unichar ids. Returns -1 if the length of array1 is
  // less than length of array2, if any array1[i] is less than array2[i].
  // Returns 0 if the arrays are equal, 1 otherwise.
  // The function assumes that the arrays are terminated by INVALID_UNICHAR_ID.
  static inline int compare(const UNICHAR_ID *ptr1, const UNICHAR_ID *ptr2) {
    for (;;) {
      const UNICHAR_ID val1 = *ptr1++;
      const UNICHAR_ID val2 = *ptr2++;
      if (val1 != val2) {
        if (val1 == INVALID_UNICHAR_ID) return -1;
        if (val2 == INVALID_UNICHAR_ID) return 1;
        if (val1 < val2) return -1;
        return 1;
      }
      if (val1 == INVALID_UNICHAR_ID) return 0;
    }
  }

  // Look uid in the vector of uids.  If found, the index of the matched
  // element is returned.  Otherwise, it returns -1.
  static inline int find_in(const UnicharIdVector& uid_vec,
                            const UNICHAR_ID uid) {
    for (int i = 0; i < uid_vec.size(); ++i)
      if (uid_vec[i] == uid) return i;
    return -1;
  }

  // Copies UNICHAR_IDs from dst to src. Returns the number of ids copied.
  // The function assumes that the arrays are terminated by INVALID_UNICHAR_ID
  // and that dst has enough space for all the elements from src.
  static inline int copy(const UNICHAR_ID src[], UNICHAR_ID dst[]) {
    int i = 0;
    do {
      dst[i] = src[i];
    } while (dst[i++] != INVALID_UNICHAR_ID);
    return i - 1;
  }

  // Prints unichars corresponding to the unichar_ids in the given array.
  // The function assumes that array is terminated by INVALID_UNICHAR_ID.
  static inline void print(const UNICHAR_ID array[],
                           const UNICHARSET &unicharset) {
    const UNICHAR_ID *ptr = array;
    if (*ptr == INVALID_UNICHAR_ID) tprintf("[Empty]");
    while (*ptr != INVALID_UNICHAR_ID) {
      tprintf("%s ", unicharset.id_to_unichar(*ptr++));
    }
    tprintf("( ");
    ptr = array;
    while (*ptr != INVALID_UNICHAR_ID) tprintf("%d ", *ptr++);
    tprintf(")\n");
  }
};

// AMBIG_SPEC_LIST stores a list of dangerous ambigs that
// start with the same unichar (e.g. r->t rn->m rr1->m).
class AmbigSpec : public ELIST_LINK {
 public:
  AmbigSpec();
  ~AmbigSpec() = default;

  // Comparator function for sorting AmbigSpec_LISTs. The lists will
  // be sorted by their wrong_ngram arrays. Example of wrong_ngram vectors
  // in a a sorted AmbigSpec_LIST: [9 1 3], [9 3 4], [9 8], [9, 8 1].
  static int compare_ambig_specs(const void *spec1, const void *spec2) {
    const AmbigSpec *s1 = *static_cast<const AmbigSpec *const *>(spec1);
    const AmbigSpec *s2 = *static_cast<const AmbigSpec *const *>(spec2);
    int result = UnicharIdArrayUtils::compare(s1->wrong_ngram, s2->wrong_ngram);
    if (result != 0) return result;
    return UnicharIdArrayUtils::compare(s1->correct_fragments,
                                        s2->correct_fragments);
  }

  UNICHAR_ID wrong_ngram[MAX_AMBIG_SIZE + 1];
  UNICHAR_ID correct_fragments[MAX_AMBIG_SIZE + 1];
  UNICHAR_ID correct_ngram_id;
  AmbigType type;
  int wrong_ngram_size;
};
ELISTIZEH(AmbigSpec)

// AMBIG_TABLE[i] stores a set of ambiguities whose
// wrong ngram starts with unichar id i.
using UnicharAmbigsVector = GenericVector<AmbigSpec_LIST *>;

class UnicharAmbigs {
 public:
  UnicharAmbigs() = default;
  ~UnicharAmbigs() {
    replace_ambigs_.delete_data_pointers();
    dang_ambigs_.delete_data_pointers();
    one_to_one_definite_ambigs_.delete_data_pointers();
  }

  const UnicharAmbigsVector &dang_ambigs() const { return dang_ambigs_; }
  const UnicharAmbigsVector &replace_ambigs() const { return replace_ambigs_; }

  // Initializes the ambigs by adding a nullptr pointer to each table.
  void InitUnicharAmbigs(const UNICHARSET& unicharset,
                         bool use_ambigs_for_adaption);

  // Loads the universal ambigs that are useful for any language.
  void LoadUniversal(const UNICHARSET& encoder_set, UNICHARSET* unicharset);

  // Fills in two ambiguity tables (replaceable and dangerous) with information
  // read from the ambigs file. An ambiguity table is an array of lists.
  // The array is indexed by a class id. Each entry in the table provides
  // a list of potential ambiguities which can start with the corresponding
  // character. For example the ambiguity "rn -> m", would be located in the
  // table at index of unicharset.unichar_to_id('r').
  // In 1-1 ambiguities (e.g. s -> S, 1 -> I) are recorded in
  // one_to_one_definite_ambigs_. This vector is also indexed by the class id
  // of the wrong part of the ambiguity and each entry contains a vector of
  // unichar ids that are ambiguous to it.
  // encoder_set is used to encode the ambiguity strings, undisturbed by new
  // unichar_ids that may be created by adding the ambigs.
  void LoadUnicharAmbigs(const UNICHARSET& encoder_set,
                         TFile *ambigs_file, int debug_level,
                         bool use_ambigs_for_adaption, UNICHARSET *unicharset);

  // Returns definite 1-1 ambigs for the given unichar id.
  inline const UnicharIdVector *OneToOneDefiniteAmbigs(
      UNICHAR_ID unichar_id) const {
    if (one_to_one_definite_ambigs_.empty()) return nullptr;
    return one_to_one_definite_ambigs_[unichar_id];
  }

  // Returns a pointer to the vector with all unichar ids that appear in the
  // 'correct' part of the ambiguity pair when the given unichar id appears
  // in the 'wrong' part of the ambiguity. E.g. if DangAmbigs file consist of
  // m->rn,rn->m,m->iii, UnicharAmbigsForAdaption() called with unichar id of
  // m will return a pointer to a vector with unichar ids of r,n,i.
  inline const UnicharIdVector *AmbigsForAdaption(
      UNICHAR_ID unichar_id) const {
    if (ambigs_for_adaption_.empty()) return nullptr;
    return ambigs_for_adaption_[unichar_id];
  }

  // Similar to the above, but return the vector of unichar ids for which
  // the given unichar_id is an ambiguity (appears in the 'wrong' part of
  // some ambiguity pair).
  inline const UnicharIdVector *ReverseAmbigsForAdaption(
      UNICHAR_ID unichar_id) const {
    if (reverse_ambigs_for_adaption_.empty()) return nullptr;
    return reverse_ambigs_for_adaption_[unichar_id];
  }

 private:
  bool ParseAmbiguityLine(int line_num, int version, int debug_level,
                          const UNICHARSET &unicharset, char *buffer,
                          int *test_ambig_part_size,
                          UNICHAR_ID *test_unichar_ids,
                          int *replacement_ambig_part_size,
                          char *replacement_string, int *type);
  bool InsertIntoTable(UnicharAmbigsVector &table,
                       int test_ambig_part_size, UNICHAR_ID *test_unichar_ids,
                       int replacement_ambig_part_size,
                       const char *replacement_string, int type,
                       AmbigSpec *ambig_spec, UNICHARSET *unicharset);

  UnicharAmbigsVector dang_ambigs_;
  UnicharAmbigsVector replace_ambigs_;
  GenericVector<UnicharIdVector *> one_to_one_definite_ambigs_;
  GenericVector<UnicharIdVector *> ambigs_for_adaption_;
  GenericVector<UnicharIdVector *> reverse_ambigs_for_adaption_;
};

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_AMBIGS_H_
