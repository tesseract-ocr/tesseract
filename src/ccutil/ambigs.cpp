///////////////////////////////////////////////////////////////////////
// File:        ambigs.cpp
// Description: Functions for dealing with ambiguities
//              (training and recognition).
// Author:      Daria Antonova
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
#include "universalambigs.h"

#include <cstdio>

#if defined(_WIN32) && !defined(__GNUC__)
#  define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif /* _WIN32 && !__GNUC__ */

namespace tesseract {

static const char kAmbigDelimiters[] = "\t ";
static const char kIllegalMsg[] = "Illegal ambiguity specification on line %d\n";
static const char kIllegalUnicharMsg[] = "Illegal unichar %s in ambiguity specification\n";

// Maximum line size:
//   10 for sizes of ambigs, tabs, abmig type and newline
//   UNICHAR_LEN * (MAX_AMBIG_SIZE + 1) for each part of the ambig
const int kMaxAmbigStringSize = UNICHAR_LEN * (MAX_AMBIG_SIZE + 1);

AmbigSpec::AmbigSpec() {
  wrong_ngram[0] = INVALID_UNICHAR_ID;
  correct_fragments[0] = INVALID_UNICHAR_ID;
  correct_ngram_id = INVALID_UNICHAR_ID;
  type = NOT_AMBIG;
  wrong_ngram_size = 0;
}

// Initializes the ambigs by adding a nullptr pointer to each table.
void UnicharAmbigs::InitUnicharAmbigs(const UNICHARSET &unicharset, bool use_ambigs_for_adaption) {
  for (unsigned i = 0; i < unicharset.size(); ++i) {
    replace_ambigs_.push_back(nullptr);
    dang_ambigs_.push_back(nullptr);
    one_to_one_definite_ambigs_.push_back(nullptr);
    if (use_ambigs_for_adaption) {
      ambigs_for_adaption_.push_back(nullptr);
      reverse_ambigs_for_adaption_.push_back(nullptr);
    }
  }
}

// Loads the universal ambigs that are useful for any language.
void UnicharAmbigs::LoadUniversal(const UNICHARSET &encoder_set, UNICHARSET *unicharset) {
  TFile file;
  if (!file.Open(kUniversalAmbigsFile, ksizeofUniversalAmbigsFile)) {
    return;
  }
  LoadUnicharAmbigs(encoder_set, &file, 0, false, unicharset);
}

void UnicharAmbigs::LoadUnicharAmbigs(const UNICHARSET &encoder_set, TFile *ambig_file,
                                      int debug_level, bool use_ambigs_for_adaption,
                                      UNICHARSET *unicharset) {
  UnicharIdVector *adaption_ambigs_entry;
  if (debug_level) {
    tprintf("Reading ambiguities\n");
  }

  int test_ambig_part_size;
  int replacement_ambig_part_size;
  // The space for buffer is allocated on the heap to avoid
  // GCC frame size warning.
  const int kBufferSize = 10 + 2 * kMaxAmbigStringSize;
  char *buffer = new char[kBufferSize];
  char replacement_string[kMaxAmbigStringSize];
  UNICHAR_ID test_unichar_ids[MAX_AMBIG_SIZE + 1];
  int line_num = 0;
  int type = NOT_AMBIG;

  // Determine the version of the ambigs file.
  int version = 0;
  ASSERT_HOST(ambig_file->FGets(buffer, kBufferSize) != nullptr && buffer[0] != '\0');
  if (*buffer == 'v') {
    version = static_cast<int>(strtol(buffer + 1, nullptr, 10));
    ++line_num;
  } else {
    ambig_file->Rewind();
  }
  while (ambig_file->FGets(buffer, kBufferSize) != nullptr) {
    chomp_string(buffer);
    if (debug_level > 2) {
      tprintf("read line %s\n", buffer);
    }
    ++line_num;
    if (!ParseAmbiguityLine(line_num, version, debug_level, encoder_set, buffer,
                            &test_ambig_part_size, test_unichar_ids, &replacement_ambig_part_size,
                            replacement_string, &type)) {
      continue;
    }
    // Construct AmbigSpec and add it to the appropriate AmbigSpec_LIST.
    auto *ambig_spec = new AmbigSpec();
    if (!InsertIntoTable((type == REPLACE_AMBIG) ? replace_ambigs_ : dang_ambigs_,
                         test_ambig_part_size, test_unichar_ids, replacement_ambig_part_size,
                         replacement_string, type, ambig_spec, unicharset)) {
      continue;
    }

    // Update one_to_one_definite_ambigs_.
    if (test_ambig_part_size == 1 && replacement_ambig_part_size == 1 && type == DEFINITE_AMBIG) {
      if (one_to_one_definite_ambigs_[test_unichar_ids[0]] == nullptr) {
        one_to_one_definite_ambigs_[test_unichar_ids[0]] = new UnicharIdVector();
      }
      one_to_one_definite_ambigs_[test_unichar_ids[0]]->push_back(ambig_spec->correct_ngram_id);
    }
    // Update ambigs_for_adaption_.
    if (use_ambigs_for_adaption) {
      std::vector<UNICHAR_ID> encoding;
      // Silently ignore invalid strings, as before, so it is safe to use a
      // universal ambigs file.
      if (unicharset->encode_string(replacement_string, true, &encoding, nullptr, nullptr)) {
        for (int i = 0; i < test_ambig_part_size; ++i) {
          if (ambigs_for_adaption_[test_unichar_ids[i]] == nullptr) {
            ambigs_for_adaption_[test_unichar_ids[i]] = new UnicharIdVector();
          }
          adaption_ambigs_entry = ambigs_for_adaption_[test_unichar_ids[i]];
          for (int id_to_insert : encoding) {
            ASSERT_HOST(id_to_insert != INVALID_UNICHAR_ID);
            // Add the new unichar id to adaption_ambigs_entry (only if the
            // vector does not already contain it) keeping it in sorted order.
            size_t j;
            for (j = 0;
                 j < adaption_ambigs_entry->size() && (*adaption_ambigs_entry)[j] > id_to_insert;
                 ++j) {
            }
            if (j < adaption_ambigs_entry->size()) {
              if ((*adaption_ambigs_entry)[j] != id_to_insert) {
                adaption_ambigs_entry->insert(adaption_ambigs_entry->begin() + j, id_to_insert);
              }
            } else {
              adaption_ambigs_entry->push_back(id_to_insert);
            }
          }
        }
      }
    }
  }
  delete[] buffer;

  // Fill in reverse_ambigs_for_adaption from ambigs_for_adaption vector.
  if (use_ambigs_for_adaption) {
    for (size_t i = 0; i < ambigs_for_adaption_.size(); ++i) {
      adaption_ambigs_entry = ambigs_for_adaption_[i];
      if (adaption_ambigs_entry == nullptr) {
        continue;
      }
      for (size_t j = 0; j < adaption_ambigs_entry->size(); ++j) {
        UNICHAR_ID ambig_id = (*adaption_ambigs_entry)[j];
        if (reverse_ambigs_for_adaption_[ambig_id] == nullptr) {
          reverse_ambigs_for_adaption_[ambig_id] = new UnicharIdVector();
        }
        reverse_ambigs_for_adaption_[ambig_id]->push_back(i);
      }
    }
  }

  // Print what was read from the input file.
  if (debug_level > 1) {
    for (int tbl = 0; tbl < 2; ++tbl) {
      const UnicharAmbigsVector &print_table = (tbl == 0) ? replace_ambigs_ : dang_ambigs_;
      for (size_t i = 0; i < print_table.size(); ++i) {
        AmbigSpec_LIST *lst = print_table[i];
        if (lst == nullptr) {
          continue;
        }
        if (!lst->empty()) {
          tprintf("%s Ambiguities for %s:\n", (tbl == 0) ? "Replaceable" : "Dangerous",
                  unicharset->debug_str(i).c_str());
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
        const std::vector<UnicharIdVector *> &vec =
            (vec_id == 0) ? ambigs_for_adaption_ : reverse_ambigs_for_adaption_;
        for (size_t i = 0; i < vec.size(); ++i) {
          adaption_ambigs_entry = vec[i];
          if (adaption_ambigs_entry != nullptr) {
            tprintf("%sAmbigs for adaption for %s:\n", (vec_id == 0) ? "" : "Reverse ",
                    unicharset->debug_str(i).c_str());
            for (size_t j = 0; j < adaption_ambigs_entry->size(); ++j) {
              tprintf("%s ", unicharset->debug_str((*adaption_ambigs_entry)[j]).c_str());
            }
            tprintf("\n");
          }
        }
      }
    }
  }
}

bool UnicharAmbigs::ParseAmbiguityLine(int line_num, int version, int debug_level,
                                       const UNICHARSET &unicharset, char *buffer,
                                       int *test_ambig_part_size, UNICHAR_ID *test_unichar_ids,
                                       int *replacement_ambig_part_size, char *replacement_string,
                                       int *type) {
  if (version > 1) {
    // Simpler format is just wrong-string correct-string type\n.
    std::string input(buffer);
    std::vector<std::string> fields = split(input, ' ');
    if (fields.size() != 3) {
      if (debug_level) {
        tprintf(kIllegalMsg, line_num);
      }
      return false;
    }
    // Encode wrong-string.
    std::vector<UNICHAR_ID> unichars;
    if (!unicharset.encode_string(fields[0].c_str(), true, &unichars, nullptr, nullptr)) {
      return false;
    }
    *test_ambig_part_size = unichars.size();
    if (*test_ambig_part_size > MAX_AMBIG_SIZE) {
      if (debug_level) {
        tprintf("Too many unichars in ambiguity on line %d\n", line_num);
      }
      return false;
    }
    // Copy encoded string to output.
    for (size_t i = 0; i < unichars.size(); ++i) {
      test_unichar_ids[i] = unichars[i];
    }
    test_unichar_ids[unichars.size()] = INVALID_UNICHAR_ID;
    // Encode replacement-string to check validity.
    if (!unicharset.encode_string(fields[1].c_str(), true, &unichars, nullptr, nullptr)) {
      return false;
    }
    *replacement_ambig_part_size = unichars.size();
    if (*replacement_ambig_part_size > MAX_AMBIG_SIZE) {
      if (debug_level) {
        tprintf("Too many unichars in ambiguity on line %d\n", line_num);
      }
      return false;
    }
    if (sscanf(fields[2].c_str(), "%d", type) != 1) {
      if (debug_level) {
        tprintf(kIllegalMsg, line_num);
      }
      return false;
    }
    snprintf(replacement_string, kMaxAmbigStringSize, "%s", fields[1].c_str());
    return true;
  }
  int i;
  char *token;
  char *next_token;
  if (!(token = strtok_r(buffer, kAmbigDelimiters, &next_token)) ||
      !sscanf(token, "%d", test_ambig_part_size) || *test_ambig_part_size <= 0) {
    if (debug_level) {
      tprintf(kIllegalMsg, line_num);
    }
    return false;
  }
  if (*test_ambig_part_size > MAX_AMBIG_SIZE) {
    if (debug_level) {
      tprintf("Too many unichars in ambiguity on line %d\n", line_num);
    }
    return false;
  }
  for (i = 0; i < *test_ambig_part_size; ++i) {
    if (!(token = strtok_r(nullptr, kAmbigDelimiters, &next_token))) {
      break;
    }
    if (!unicharset.contains_unichar(token)) {
      if (debug_level) {
        tprintf(kIllegalUnicharMsg, token);
      }
      break;
    }
    test_unichar_ids[i] = unicharset.unichar_to_id(token);
  }
  test_unichar_ids[i] = INVALID_UNICHAR_ID;

  if (i != *test_ambig_part_size || !(token = strtok_r(nullptr, kAmbigDelimiters, &next_token)) ||
      !sscanf(token, "%d", replacement_ambig_part_size) || *replacement_ambig_part_size <= 0) {
    if (debug_level) {
      tprintf(kIllegalMsg, line_num);
    }
    return false;
  }
  if (*replacement_ambig_part_size > MAX_AMBIG_SIZE) {
    if (debug_level) {
      tprintf("Too many unichars in ambiguity on line %d\n", line_num);
    }
    return false;
  }
  replacement_string[0] = '\0';
  for (i = 0; i < *replacement_ambig_part_size; ++i) {
    if (!(token = strtok_r(nullptr, kAmbigDelimiters, &next_token))) {
      break;
    }
    strcat(replacement_string, token);
    if (!unicharset.contains_unichar(token)) {
      if (debug_level) {
        tprintf(kIllegalUnicharMsg, token);
      }
      break;
    }
  }
  if (i != *replacement_ambig_part_size) {
    if (debug_level) {
      tprintf(kIllegalMsg, line_num);
    }
    return false;
  }
  if (version > 0) {
    // The next field being true indicates that the ambiguity should
    // always be substituted (e.g. '' should always be changed to ").
    // For such "certain" n -> m ambigs tesseract will insert character
    // fragments for the n pieces in the unicharset. AmbigsFound()
    // will then replace the incorrect ngram with the character
    // fragments of the correct character (or ngram if m > 1).
    // Note that if m > 1, an ngram will be inserted into the
    // modified word, not the individual unigrams. Tesseract
    // has limited support for ngram unichar (e.g. dawg permuter).
    if (!(token = strtok_r(nullptr, kAmbigDelimiters, &next_token)) || !sscanf(token, "%d", type)) {
      if (debug_level) {
        tprintf(kIllegalMsg, line_num);
      }
      return false;
    }
  }
  return true;
}

bool UnicharAmbigs::InsertIntoTable(UnicharAmbigsVector &table, int test_ambig_part_size,
                                    UNICHAR_ID *test_unichar_ids, int replacement_ambig_part_size,
                                    const char *replacement_string, int type, AmbigSpec *ambig_spec,
                                    UNICHARSET *unicharset) {
  ambig_spec->type = static_cast<AmbigType>(type);
  if (test_ambig_part_size == 1 && replacement_ambig_part_size == 1 &&
      unicharset->to_lower(test_unichar_ids[0]) ==
          unicharset->to_lower(unicharset->unichar_to_id(replacement_string))) {
    ambig_spec->type = CASE_AMBIG;
  }

  ambig_spec->wrong_ngram_size =
      UnicharIdArrayUtils::copy(test_unichar_ids, ambig_spec->wrong_ngram);

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
  unicharset->unichar_insert(replacement_string, OldUncleanUnichars::kTrue);
  ambig_spec->correct_ngram_id = unicharset->unichar_to_id(replacement_string);
  if (replacement_ambig_part_size > 1) {
    unicharset->set_isngram(ambig_spec->correct_ngram_id, true);
  }
  // Add the corresponding fragments of the wrong ngram to unicharset.
  int i;
  for (i = 0; i < test_ambig_part_size; ++i) {
    UNICHAR_ID unichar_id;
    if (test_ambig_part_size == 1) {
      unichar_id = ambig_spec->correct_ngram_id;
    } else {
      std::string frag_str =
          CHAR_FRAGMENT::to_string(replacement_string, i, test_ambig_part_size, false);
      unicharset->unichar_insert(frag_str.c_str(), OldUncleanUnichars::kTrue);
      unichar_id = unicharset->unichar_to_id(frag_str.c_str());
    }
    ambig_spec->correct_fragments[i] = unichar_id;
  }
  ambig_spec->correct_fragments[i] = INVALID_UNICHAR_ID;

  // Add AmbigSpec for this ambiguity to the corresponding AmbigSpec_LIST.
  // Keep AmbigSpec_LISTs sorted by AmbigSpec.wrong_ngram.
  if (table[test_unichar_ids[0]] == nullptr) {
    table[test_unichar_ids[0]] = new AmbigSpec_LIST();
  }
  if (table[test_unichar_ids[0]]->add_sorted(AmbigSpec::compare_ambig_specs, true, ambig_spec)) {
    return true;
  }
  delete ambig_spec;
  return false;
}

} // namespace tesseract
