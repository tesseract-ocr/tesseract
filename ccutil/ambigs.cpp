///////////////////////////////////////////////////////////////////////
// File:        ambigs.cc
// Description: Functions for dealing with ambiguities
//              (training and recognition).
// Author:      Daria Antonova
// Created:     Mon Feb 5 11:26:43 PDT 2009
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

#include "ambigs.h"
#include "helpers.h"

#ifdef _WIN32
#ifndef __GNUC__
#define strtok_r strtok_s
#else
#include "strtok_r.h"
#endif  /* __GNUC__ */
#endif  /* _WIN32 */

namespace tesseract {

AmbigSpec::AmbigSpec() {
  wrong_ngram[0] = INVALID_UNICHAR_ID;
  correct_fragments[0] = INVALID_UNICHAR_ID;
  correct_ngram_id = INVALID_UNICHAR_ID;
  type = NOT_AMBIG;
  wrong_ngram_size = 0;
}

ELISTIZE(AmbigSpec);

void UnicharAmbigs::LoadUnicharAmbigs(FILE *AmbigFile,
                                      inT64 end_offset,
                                      int debug_level,
                                      bool use_ambigs_for_adaption,
                                      UNICHARSET *unicharset) {
  int i, j;
  UnicharIdVector *adaption_ambigs_entry;
  for (i = 0; i < unicharset->size(); ++i) {
    replace_ambigs_.push_back(NULL);
    dang_ambigs_.push_back(NULL);
    one_to_one_definite_ambigs_.push_back(NULL);
    if (use_ambigs_for_adaption) {
      ambigs_for_adaption_.push_back(NULL);
      reverse_ambigs_for_adaption_.push_back(NULL);
    }
  }
  if (debug_level) tprintf("Reading ambiguities\n");

  int TestAmbigPartSize;
  int ReplacementAmbigPartSize;
  // Maximum line size:
  //   10 for sizes of ambigs, tabs, abmig type and newline
  //   UNICHAR_LEN * (MAX_AMBIG_SIZE + 1) for each part of the ambig
  // The space for buffer is allocated on the heap to avoid
  // GCC frame size warning.
  const int kMaxAmbigStringSize = UNICHAR_LEN * (MAX_AMBIG_SIZE + 1);
  const int kBufferSize = 10 + 2 * kMaxAmbigStringSize;
  char *buffer = new char[kBufferSize];
  char ReplacementString[kMaxAmbigStringSize];
  UNICHAR_ID TestUnicharIds[MAX_AMBIG_SIZE + 1];
  int line_num = 0;
  int type = NOT_AMBIG;

  // Determine the version of the ambigs file.
  int version = 0;
  ASSERT_HOST(fgets(buffer, kBufferSize, AmbigFile) != NULL &&
              strlen(buffer) > 0);
  if (*buffer == 'v') {
    version = static_cast<int>(strtol(buffer+1, NULL, 10));
    ++line_num;
  } else {
    rewind(AmbigFile);
  }
  while ((end_offset < 0 || ftell(AmbigFile) < end_offset) &&
         fgets(buffer, kBufferSize, AmbigFile) != NULL) {
    chomp_string(buffer);
    if (debug_level > 2) tprintf("read line %s\n", buffer);
    ++line_num;
    if (!ParseAmbiguityLine(line_num, version, debug_level, *unicharset,
                            buffer, &TestAmbigPartSize, TestUnicharIds,
                            &ReplacementAmbigPartSize,
                            ReplacementString, &type)) continue;
    // Construct AmbigSpec and add it to the appropriate AmbigSpec_LIST.
    AmbigSpec *ambig_spec = new AmbigSpec();
    InsertIntoTable((type == REPLACE_AMBIG) ? replace_ambigs_ : dang_ambigs_,
                    TestAmbigPartSize, TestUnicharIds,
                    ReplacementAmbigPartSize, ReplacementString, type,
                    ambig_spec, unicharset);

    // Update one_to_one_definite_ambigs_.
    if (TestAmbigPartSize == 1 &&
        ReplacementAmbigPartSize == 1 && type == DEFINITE_AMBIG) {
      if (one_to_one_definite_ambigs_[TestUnicharIds[0]] == NULL) {
        one_to_one_definite_ambigs_[TestUnicharIds[0]] = new UnicharIdVector();
      }
      one_to_one_definite_ambigs_[TestUnicharIds[0]]->push_back(
          ambig_spec->correct_ngram_id);
    }
    // Update ambigs_for_adaption_.
    if (use_ambigs_for_adaption) {
      for (i = 0; i < TestAmbigPartSize; ++i) {
        if (ambigs_for_adaption_[TestUnicharIds[i]] == NULL) {
          ambigs_for_adaption_[TestUnicharIds[i]] = new UnicharIdVector();
        }
        adaption_ambigs_entry = ambigs_for_adaption_[TestUnicharIds[i]];
        const char *tmp_ptr = ReplacementString;
        const char *tmp_ptr_end = ReplacementString + strlen(ReplacementString);
        int step = unicharset->step(tmp_ptr);
        while (step > 0) {
          UNICHAR_ID id_to_insert = unicharset->unichar_to_id(tmp_ptr, step);
          ASSERT_HOST(id_to_insert != INVALID_UNICHAR_ID);
          // Add the new unichar id to adaption_ambigs_entry (only if the
          // vector does not already contain it) keeping it in sorted order.
          for (j = 0; j < adaption_ambigs_entry->size() &&
               (*adaption_ambigs_entry)[j] > id_to_insert; ++j);
          if (j < adaption_ambigs_entry->size()) {
            if ((*adaption_ambigs_entry)[j] != id_to_insert) {
              adaption_ambigs_entry->insert(id_to_insert, j);
            }
          } else {
            adaption_ambigs_entry->push_back(id_to_insert);
          }
          // Update tmp_ptr and step.
          tmp_ptr += step;
          step = tmp_ptr < tmp_ptr_end ? unicharset->step(tmp_ptr) : 0;
        }
      }
    }
  }
  delete[] buffer;

  // Fill in reverse_ambigs_for_adaption from ambigs_for_adaption vector.
  if (use_ambigs_for_adaption) {
    for (i = 0; i < ambigs_for_adaption_.size(); ++i) {
      adaption_ambigs_entry = ambigs_for_adaption_[i];
      if (adaption_ambigs_entry == NULL) continue;
      for (j = 0; j < adaption_ambigs_entry->size(); ++j) {
        UNICHAR_ID ambig_id = (*adaption_ambigs_entry)[j];
        if (reverse_ambigs_for_adaption_[ambig_id] == NULL) {
          reverse_ambigs_for_adaption_[ambig_id] = new UnicharIdVector();
        }
        reverse_ambigs_for_adaption_[ambig_id]->push_back(i);
      }
    }
  }

  // Print what was read from the input file.
  if (debug_level > 1) {
    for (int tbl = 0; tbl < 2; ++tbl) {
      const UnicharAmbigsVector &print_table =
        (tbl == 0) ? replace_ambigs_ : dang_ambigs_;
      for (i = 0; i < print_table.size(); ++i) {
        AmbigSpec_LIST *lst = print_table[i];
        if (lst == NULL) continue;
        if (!lst->empty()) {
          tprintf("%s Ambiguities for %s:\n",
                  (tbl == 0) ? "Replaceable" : "Dangerous",
                  unicharset->debug_str(i).string());
        }
        AmbigSpec_IT lst_it(lst);
        for (lst_it.mark_cycle_pt(); !lst_it.cycled_list(); lst_it.forward()) {
          AmbigSpec *ambig_spec = lst_it.data();
          tprintf("wrong_ngram:");
          UnicharIdArrayUtils::print(ambig_spec->wrong_ngram, *unicharset);
          tprintf("correct_fragments:");
          UnicharIdArrayUtils::print(ambig_spec->correct_fragments, *unicharset);
        }
      }
    }
    if (use_ambigs_for_adaption) {
      for (int vec_id = 0; vec_id < 2; ++vec_id) {
        const GenericVector<UnicharIdVector *> &vec = (vec_id == 0) ?
          ambigs_for_adaption_ : reverse_ambigs_for_adaption_;
        for (i = 0; i < vec.size(); ++i) {
          adaption_ambigs_entry = vec[i];
          if (adaption_ambigs_entry != NULL) {
            tprintf("%sAmbigs for adaption for %s:\n",
                    (vec_id == 0) ? "" : "Reverse ",
                    unicharset->debug_str(i).string());
            for (j = 0; j < adaption_ambigs_entry->size(); ++j) {
              tprintf("%s ", unicharset->debug_str(
                  (*adaption_ambigs_entry)[j]).string());
            }
            tprintf("\n");
          }
        }
      }
    }
  }
}

bool UnicharAmbigs::ParseAmbiguityLine(
    int line_num, int version, int debug_level, const UNICHARSET &unicharset,
    char *buffer, int *TestAmbigPartSize, UNICHAR_ID *TestUnicharIds,
    int *ReplacementAmbigPartSize, char *ReplacementString, int *type) {
  int i;
  char *token;
  char *next_token;
  if (!(token = strtok_r(buffer, kAmbigDelimiters, &next_token)) ||
      !sscanf(token, "%d", TestAmbigPartSize) || TestAmbigPartSize <= 0) {
    if (debug_level) tprintf(kIllegalMsg, line_num);
    return false;
  }
  if (*TestAmbigPartSize > MAX_AMBIG_SIZE) {
    tprintf("Too many unichars in ambiguity on line %d\n");
    return false;
  }
  for (i = 0; i < *TestAmbigPartSize; ++i) {
    if (!(token = strtok_r(NULL, kAmbigDelimiters, &next_token))) break;
    if (!unicharset.contains_unichar(token)) {
      if (debug_level) tprintf(kIllegalUnicharMsg, token);
      break;
    }
    TestUnicharIds[i] = unicharset.unichar_to_id(token);
  }
  TestUnicharIds[i] = INVALID_UNICHAR_ID;

  if (i != *TestAmbigPartSize ||
      !(token = strtok_r(NULL, kAmbigDelimiters, &next_token)) ||
      !sscanf(token, "%d", ReplacementAmbigPartSize) ||
        *ReplacementAmbigPartSize <= 0) {
    if (debug_level) tprintf(kIllegalMsg, line_num);
    return false;
  }
  if (*ReplacementAmbigPartSize > MAX_AMBIG_SIZE) {
    tprintf("Too many unichars in ambiguity on line %d\n");
    return false;
  }
  ReplacementString[0] = '\0';
  for (i = 0; i < *ReplacementAmbigPartSize; ++i) {
    if (!(token = strtok_r(NULL, kAmbigDelimiters, &next_token))) break;
    strcat(ReplacementString, token);
    if (!unicharset.contains_unichar(token)) {
      if (debug_level) tprintf(kIllegalUnicharMsg, token);
      break;
    }
  }
  if (i != *ReplacementAmbigPartSize) {
    if (debug_level) tprintf(kIllegalMsg, line_num);
    return false;
  }
  if (version > 0) {
    // The next field being true indicates that the abiguity should
    // always be substituted (e.g. '' should always be changed to ").
    // For such "certain" n -> m ambigs tesseract will insert character
    // fragments for the n pieces in the unicharset. AmbigsFound()
    // will then replace the incorrect ngram with the character
    // fragments of the correct character (or ngram if m > 1).
    // Note that if m > 1, an ngram will be inserted into the
    // modified word, not the individual unigrams. Tesseract
    // has limited support for ngram unichar (e.g. dawg permuter).
    if (!(token = strtok_r(NULL, kAmbigDelimiters, &next_token)) ||
        !sscanf(token, "%d", type)) {
      if (debug_level) tprintf(kIllegalMsg, line_num);
      return false;
    }
  }
  return true;
}

void UnicharAmbigs::InsertIntoTable(
    UnicharAmbigsVector &table, int TestAmbigPartSize,
    UNICHAR_ID *TestUnicharIds, int ReplacementAmbigPartSize,
    const char *ReplacementString, int type,
    AmbigSpec *ambig_spec, UNICHARSET *unicharset) {
  ambig_spec->type = static_cast<AmbigType>(type);
  if (TestAmbigPartSize == 1 && ReplacementAmbigPartSize == 1 &&
      unicharset->to_lower(TestUnicharIds[0]) ==
      unicharset->to_lower(unicharset->unichar_to_id(ReplacementString))) {
    ambig_spec->type = CASE_AMBIG;
  }

  ambig_spec->wrong_ngram_size =
    UnicharIdArrayUtils::copy(TestUnicharIds, ambig_spec->wrong_ngram);

  // Since we need to maintain a constant number of unichar positions in
  // order to construct ambig_blob_choices vector in NoDangerousAmbig(), for
  // each n->m ambiguity we will have to place n character fragments of the
  // correct ngram into the corresponding positions in the vector (e.g. given
  // "vvvvw" and vvvv->ww we will place v and |ww|0|4 into position 0, v and
  // |ww|1|4 into position 1 and so on. The correct ngram is reconstructed
  // from fragments by dawg_permute_and_select().

  // Insert the corresponding correct ngram into the unicharset.
  // Unicharset code assumes that the "base" ngram is inserted into
  // the unicharset before fragments of this ngram are inserted.
  unicharset->unichar_insert(ReplacementString);
  ambig_spec->correct_ngram_id =
    unicharset->unichar_to_id(ReplacementString);
  if (ReplacementAmbigPartSize > 1) {
    unicharset->set_isngram(ambig_spec->correct_ngram_id, true);
  }
  // Add the corresponding fragments of the wrong ngram to unicharset.
  int i;
  for (i = 0; i < TestAmbigPartSize; ++i) {
    UNICHAR_ID unichar_id;
    if (TestAmbigPartSize == 1) {
      unichar_id = ambig_spec->correct_ngram_id;
    } else {
      STRING frag_str = CHAR_FRAGMENT::to_string(
          ReplacementString, i, TestAmbigPartSize, false);
      unicharset->unichar_insert(frag_str.string());
      unichar_id = unicharset->unichar_to_id(frag_str.string());
    }
    ambig_spec->correct_fragments[i] = unichar_id;
  }
  ambig_spec->correct_fragments[i] = INVALID_UNICHAR_ID;

  // Add AmbigSpec for this ambiguity to the corresponding AmbigSpec_LIST.
  // Keep AmbigSpec_LISTs sorted by AmbigSpec.wrong_ngram.
  if (table[TestUnicharIds[0]] == NULL) {
    table[TestUnicharIds[0]] = new AmbigSpec_LIST();
  }
  table[TestUnicharIds[0]]->add_sorted(
      AmbigSpec::compare_ambig_specs, false, ambig_spec);
}

}  // namespace tesseract
