/******************************************************************************
 ** Filename:    stopper.c
 ** Purpose:     Stopping criteria for word classifier.
 ** Author:      Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cmath>

#include "stopper.h"
#ifndef DISABLED_LEGACY_ENGINE
#include "ambigs.h"
#endif
#include "ccutil.h"
#include "dict.h"
#include <tesseract/helpers.h>
#include "matchdefs.h"
#include "pageres.h"
#include "params.h"
#include "ratngs.h"
#include <tesseract/unichar.h>

/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/

namespace tesseract {

bool Dict::AcceptableChoice(const WERD_CHOICE& best_choice,
                            XHeightConsistencyEnum xheight_consistency) {
  float CertaintyThreshold = stopper_nondict_certainty_base;
  int WordSize;

  if (stopper_no_acceptable_choices) return false;

  if (best_choice.length() == 0) return false;

  bool no_dang_ambigs = !best_choice.dangerous_ambig_found();
  bool is_valid_word = valid_word_permuter(best_choice.permuter(), false);
  bool is_case_ok = case_ok(best_choice);

  if (stopper_debug_level >= 1) {
    const char *xht = "UNKNOWN";
    switch (xheight_consistency) {
      case XH_GOOD:  xht = "NORMAL"; break;
      case XH_SUBNORMAL:  xht = "SUBNORMAL"; break;
      case XH_INCONSISTENT:  xht = "INCONSISTENT"; break;
      default: xht = "UNKNOWN";
    }
    tprintf("\nStopper:  %s (word=%c, case=%c, xht_ok=%s=[%g,%g])\n",
            best_choice.unichar_string().c_str(),
            (is_valid_word ? 'y' : 'n'),
            (is_case_ok ? 'y' : 'n'),
            xht,
            best_choice.min_x_height(),
            best_choice.max_x_height());
  }
  // Do not accept invalid words in PASS1.
  if (reject_offset_ <= 0.0f && !is_valid_word) return false;
  if (is_valid_word && is_case_ok) {
    WordSize = LengthOfShortestAlphaRun(best_choice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    tprintf("Stopper:  Rating = %4.1f, Certainty = %4.1f, Threshold = %4.1f\n",
            best_choice.rating(), best_choice.certainty(), CertaintyThreshold);

  if (no_dang_ambigs &&
      best_choice.certainty() > CertaintyThreshold &&
      xheight_consistency < XH_INCONSISTENT &&
      UniformCertainties(best_choice)) {
    return true;
  } else {
    if (stopper_debug_level >= 1) {
      tprintf("AcceptableChoice() returned false"
              " (no_dang_ambig:%d cert:%.4g thresh:%g uniform:%d)\n",
              no_dang_ambigs, best_choice.certainty(),
              CertaintyThreshold,
              UniformCertainties(best_choice));
    }
    return false;
  }
}

bool Dict::AcceptableResult(WERD_RES *word) const {
  if (word->best_choice == nullptr) return false;
  float CertaintyThreshold = stopper_nondict_certainty_base - reject_offset_;
  int WordSize;

  if (stopper_debug_level >= 1) {
    tprintf("\nRejecter: %s (word=%c, case=%c, unambig=%c, multiple=%c)\n",
            word->best_choice->debug_string().c_str(),
            (valid_word(*word->best_choice) ? 'y' : 'n'),
            (case_ok(*word->best_choice) ? 'y' : 'n'),
            word->best_choice->dangerous_ambig_found() ? 'n' : 'y',
            word->best_choices.singleton() ? 'n' : 'y');
  }

  if (word->best_choice->length() == 0 || !word->best_choices.singleton())
    return false;
  if (valid_word(*word->best_choice) && case_ok(*word->best_choice)) {
    WordSize = LengthOfShortestAlphaRun(*word->best_choice);
    WordSize -= stopper_smallword_size;
    if (WordSize < 0)
      WordSize = 0;
    CertaintyThreshold += WordSize * stopper_certainty_per_char;
  }

  if (stopper_debug_level >= 1)
    tprintf("Rejecter: Certainty = %4.1f, Threshold = %4.1f   ",
            word->best_choice->certainty(), CertaintyThreshold);

  if (word->best_choice->certainty() > CertaintyThreshold &&
      !stopper_no_acceptable_choices) {
    if (stopper_debug_level >= 1)
      tprintf("ACCEPTED\n");
    return true;
  } else {
    if (stopper_debug_level >= 1)
      tprintf("REJECTED\n");
    return false;
  }
}

#if !defined(DISABLED_LEGACY_ENGINE)

bool Dict::NoDangerousAmbig(WERD_CHOICE *best_choice,
                            DANGERR *fixpt,
                            bool fix_replaceable,
                            MATRIX *ratings) {
  if (stopper_debug_level > 2) {
    tprintf("\nRunning NoDangerousAmbig() for %s\n",
            best_choice->debug_string().c_str());
  }

  // Construct BLOB_CHOICE_LIST_VECTOR with ambiguities
  // for each unichar id in BestChoice.
  BLOB_CHOICE_LIST_VECTOR ambig_blob_choices;
  int i;
  bool ambigs_found = false;
  // For each position in best_choice:
  // -- choose AMBIG_SPEC_LIST that corresponds to unichar_id at best_choice[i]
  // -- initialize wrong_ngram with a single unichar_id at best_choice[i]
  // -- look for ambiguities corresponding to wrong_ngram in the list while
  //    adding the following unichar_ids from best_choice to wrong_ngram
  //
  // Repeat the above procedure twice: first time look through
  // ambigs to be replaced and replace all the ambiguities found;
  // second time look through dangerous ambiguities and construct
  // ambig_blob_choices with fake a blob choice for each ambiguity
  // and pass them to dawg_permute_and_select() to search for
  // ambiguous words in the dictionaries.
  //
  // Note that during the execution of the for loop (on the first pass)
  // if replacements are made the length of best_choice might change.
  for (int pass = 0; pass < (fix_replaceable ? 2 : 1); ++pass) {
    bool replace = (fix_replaceable && pass == 0);
    const UnicharAmbigsVector &table = replace ?
      getUnicharAmbigs().replace_ambigs() : getUnicharAmbigs().dang_ambigs();
    if (!replace) {
      // Initialize ambig_blob_choices with lists containing a single
      // unichar id for the corresponding position in best_choice.
      // best_choice consisting from only the original letters will
      // have a rating of 0.0.
      for (i = 0; i < best_choice->length(); ++i) {
        auto *lst = new BLOB_CHOICE_LIST();
        BLOB_CHOICE_IT lst_it(lst);
        // TODO(rays/antonova) Put real xheights and y shifts here.
        lst_it.add_to_end(new BLOB_CHOICE(best_choice->unichar_id(i),
                                          0.0, 0.0, -1, 0, 1, 0, BCC_AMBIG));
        ambig_blob_choices.push_back(lst);
      }
    }
    UNICHAR_ID wrong_ngram[MAX_AMBIG_SIZE + 1];
    int wrong_ngram_index;
    int next_index;
    int blob_index = 0;
    for (i = 0; i < best_choice->length(); blob_index += best_choice->state(i),
         ++i) {
      UNICHAR_ID curr_unichar_id = best_choice->unichar_id(i);
      if (stopper_debug_level > 2) {
        tprintf("Looking for %s ngrams starting with %s:\n",
                replace ? "replaceable" : "ambiguous",
                getUnicharset().debug_str(curr_unichar_id).c_str());
      }
      int num_wrong_blobs = best_choice->state(i);
      wrong_ngram_index = 0;
      wrong_ngram[wrong_ngram_index] = curr_unichar_id;
      if (curr_unichar_id == INVALID_UNICHAR_ID ||
          curr_unichar_id >= table.size() ||
          table[curr_unichar_id] == nullptr) {
        continue;  // there is no ambig spec for this unichar id
      }
      AmbigSpec_IT spec_it(table[curr_unichar_id]);
      for (spec_it.mark_cycle_pt(); !spec_it.cycled_list();) {
        const AmbigSpec *ambig_spec = spec_it.data();
        wrong_ngram[wrong_ngram_index+1] = INVALID_UNICHAR_ID;
        int compare = UnicharIdArrayUtils::compare(wrong_ngram,
                                                   ambig_spec->wrong_ngram);
        if (stopper_debug_level > 2) {
          tprintf("candidate ngram: ");
          UnicharIdArrayUtils::print(wrong_ngram, getUnicharset());
          tprintf("current ngram from spec: ");
          UnicharIdArrayUtils::print(ambig_spec->wrong_ngram, getUnicharset());
          tprintf("comparison result: %d\n", compare);
        }
        if (compare == 0) {
          // Record the place where we found an ambiguity.
          if (fixpt != nullptr) {
            UNICHAR_ID leftmost_id = ambig_spec->correct_fragments[0];
            fixpt->push_back(DANGERR_INFO(
                blob_index, blob_index + num_wrong_blobs, replace,
                getUnicharset().get_isngram(ambig_spec->correct_ngram_id),
                leftmost_id));
            if (stopper_debug_level > 1) {
              tprintf("fixpt+=(%d %d %d %d %s)\n", blob_index,
                      blob_index + num_wrong_blobs, false,
                      getUnicharset().get_isngram(
                          ambig_spec->correct_ngram_id),
                      getUnicharset().id_to_unichar(leftmost_id));
            }
          }

          if (replace) {
            if (stopper_debug_level > 2) {
              tprintf("replace ambiguity with %s : ",
                      getUnicharset().id_to_unichar(
                          ambig_spec->correct_ngram_id));
              UnicharIdArrayUtils::print(
                  ambig_spec->correct_fragments, getUnicharset());
            }
            ReplaceAmbig(i, ambig_spec->wrong_ngram_size,
                         ambig_spec->correct_ngram_id,
                         best_choice, ratings);
          } else if (i > 0 || ambig_spec->type != CASE_AMBIG) {
            // We found dang ambig - update ambig_blob_choices.
            if (stopper_debug_level > 2) {
              tprintf("found ambiguity: ");
              UnicharIdArrayUtils::print(
                  ambig_spec->correct_fragments, getUnicharset());
            }
            ambigs_found = true;
            for (int tmp_index = 0; tmp_index <= wrong_ngram_index;
                 ++tmp_index) {
              // Add a blob choice for the corresponding fragment of the
              // ambiguity. These fake blob choices are initialized with
              // negative ratings (which are not possible for real blob
              // choices), so that dawg_permute_and_select() considers any
              // word not consisting of only the original letters a better
              // choice and stops searching for alternatives once such a
              // choice is found.
              BLOB_CHOICE_IT bc_it(ambig_blob_choices[i+tmp_index]);
              bc_it.add_to_end(new BLOB_CHOICE(
                  ambig_spec->correct_fragments[tmp_index], -1.0, 0.0,
                  -1, 0, 1, 0, BCC_AMBIG));
            }
          }
          spec_it.forward();
        } else if (compare == -1) {
          if (wrong_ngram_index+1 < ambig_spec->wrong_ngram_size &&
              ((next_index = wrong_ngram_index+1+i) < best_choice->length())) {
            // Add the next unichar id to wrong_ngram and keep looking for
            // more ambigs starting with curr_unichar_id in AMBIG_SPEC_LIST.
            wrong_ngram[++wrong_ngram_index] =
              best_choice->unichar_id(next_index);
            num_wrong_blobs += best_choice->state(next_index);
          } else {
            break;  // no more matching ambigs in this AMBIG_SPEC_LIST
          }
        } else {
          spec_it.forward();
        }
      }  // end searching AmbigSpec_LIST
    }  // end searching best_choice
  }  // end searching replace and dangerous ambigs

  // If any ambiguities were found permute the constructed ambig_blob_choices
  // to see if an alternative dictionary word can be found.
  if (ambigs_found) {
    if (stopper_debug_level > 2) {
      tprintf("\nResulting ambig_blob_choices:\n");
      for (i = 0; i < ambig_blob_choices.size(); ++i) {
        print_ratings_list("", ambig_blob_choices.get(i), getUnicharset());
        tprintf("\n");
      }
    }
    WERD_CHOICE *alt_word = dawg_permute_and_select(ambig_blob_choices, 0.0);
    ambigs_found = (alt_word->rating() < 0.0);
    if (ambigs_found) {
      if (stopper_debug_level >= 1) {
        tprintf ("Stopper: Possible ambiguous word = %s\n",
                 alt_word->debug_string().c_str());
      }
      if (fixpt != nullptr) {
        // Note: Currently character choices combined from fragments can only
        // be generated by NoDangrousAmbigs(). This code should be updated if
        // the capability to produce classifications combined from character
        // fragments is added to other functions.
        int orig_i = 0;
        for (i = 0; i < alt_word->length(); ++i) {
          const UNICHARSET &uchset = getUnicharset();
          bool replacement_is_ngram =
              uchset.get_isngram(alt_word->unichar_id(i));
          UNICHAR_ID leftmost_id = alt_word->unichar_id(i);
          if (replacement_is_ngram) {
            // we have to extract the leftmost unichar from the ngram.
            const char *str = uchset.id_to_unichar(leftmost_id);
            int step = uchset.step(str);
            if (step) leftmost_id = uchset.unichar_to_id(str, step);
          }
          int end_i = orig_i + alt_word->state(i);
          if (alt_word->state(i) > 1 ||
              (orig_i + 1 == end_i && replacement_is_ngram)) {
            // Compute proper blob indices.
            int blob_start = 0;
            for (int j = 0; j < orig_i; ++j)
              blob_start += best_choice->state(j);
            int blob_end = blob_start;
            for (int j = orig_i; j < end_i; ++j)
              blob_end += best_choice->state(j);
            fixpt->push_back(DANGERR_INFO(blob_start, blob_end, true,
                                          replacement_is_ngram, leftmost_id));
            if (stopper_debug_level > 1) {
              tprintf("fixpt->dangerous+=(%d %d %d %d %s)\n", orig_i, end_i,
                      true, replacement_is_ngram,
                      uchset.id_to_unichar(leftmost_id));
            }
          }
          orig_i += alt_word->state(i);
        }
      }
    }
    delete alt_word;
  }
  if (output_ambig_words_file_ != nullptr) {
    fprintf(output_ambig_words_file_, "\n");
  }

  ambig_blob_choices.delete_data_pointers();
  return !ambigs_found;
}

void Dict::EndDangerousAmbigs() {}

#endif   // !defined(DISABLED_LEGACY_ENGINE)

void Dict::SettupStopperPass1() {
  reject_offset_ = 0.0;
}

void Dict::SettupStopperPass2() {
  reject_offset_ = stopper_phase2_certainty_rejection_offset;
}

void Dict::ReplaceAmbig(int wrong_ngram_begin_index, int wrong_ngram_size,
                        UNICHAR_ID correct_ngram_id, WERD_CHOICE *werd_choice,
                        MATRIX *ratings) {
  int num_blobs_to_replace = 0;
  int begin_blob_index = 0;
  int i;
  // Rating and certainty for the new BLOB_CHOICE are derived from the
  // replaced choices.
  float new_rating = 0.0f;
  float new_certainty = 0.0f;
  BLOB_CHOICE* old_choice = nullptr;
  for (i = 0; i < wrong_ngram_begin_index + wrong_ngram_size; ++i) {
    if (i >= wrong_ngram_begin_index) {
      int num_blobs = werd_choice->state(i);
      int col = begin_blob_index + num_blobs_to_replace;
      int row = col + num_blobs - 1;
      BLOB_CHOICE_LIST* choices = ratings->get(col, row);
      ASSERT_HOST(choices != nullptr);
      old_choice = FindMatchingChoice(werd_choice->unichar_id(i), choices);
      ASSERT_HOST(old_choice != nullptr);
      new_rating += old_choice->rating();
      new_certainty += old_choice->certainty();
      num_blobs_to_replace += num_blobs;
    } else {
      begin_blob_index += werd_choice->state(i);
    }
  }
  new_certainty /= wrong_ngram_size;
  // If there is no entry in the ratings matrix, add it.
  MATRIX_COORD coord(begin_blob_index,
                     begin_blob_index + num_blobs_to_replace - 1);
  if (!coord.Valid(*ratings)) {
    ratings->IncreaseBandSize(coord.row - coord.col + 1);
  }
  if (ratings->get(coord.col, coord.row) == nullptr)
    ratings->put(coord.col, coord.row, new BLOB_CHOICE_LIST);
  BLOB_CHOICE_LIST* new_choices = ratings->get(coord.col, coord.row);
  BLOB_CHOICE* choice = FindMatchingChoice(correct_ngram_id, new_choices);
  if (choice != nullptr) {
    // Already there. Upgrade if new rating better.
    if (new_rating < choice->rating())
      choice->set_rating(new_rating);
    if (new_certainty < choice->certainty())
      choice->set_certainty(new_certainty);
    // DO NOT SORT!! It will mess up the iterator in LanguageModel::UpdateState.
  } else {
    // Need a new choice with the correct_ngram_id.
    choice = new BLOB_CHOICE(*old_choice);
    choice->set_unichar_id(correct_ngram_id);
    choice->set_rating(new_rating);
    choice->set_certainty(new_certainty);
    choice->set_classifier(BCC_AMBIG);
    choice->set_matrix_cell(coord.col, coord.row);
    BLOB_CHOICE_IT it (new_choices);
    it.add_to_end(choice);
  }
  // Remove current unichar from werd_choice. On the last iteration
  // set the correct replacement unichar instead of removing a unichar.
  for (int replaced_count = 0; replaced_count < wrong_ngram_size;
       ++replaced_count) {
    if (replaced_count + 1 == wrong_ngram_size) {
      werd_choice->set_blob_choice(wrong_ngram_begin_index,
                                   num_blobs_to_replace, choice);
    } else {
      werd_choice->remove_unichar_id(wrong_ngram_begin_index + 1);
    }
  }
  if (stopper_debug_level >= 1) {
      werd_choice->print("ReplaceAmbig() ");
      tprintf("Modified blob_choices: ");
      print_ratings_list("\n", new_choices, getUnicharset());
  }
}

int Dict::LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice) const {
  int shortest = INT32_MAX;
  int curr_len = 0;
  for (int w = 0; w < WordChoice.length(); ++w) {
    if (WordChoice.unicharset()->get_isalpha(WordChoice.unichar_id(w))) {
      curr_len++;
    } else if (curr_len > 0) {
      if (curr_len < shortest) shortest = curr_len;
      curr_len = 0;
    }
  }
  if (curr_len > 0 && curr_len < shortest) {
    shortest = curr_len;
  } else if (shortest == INT32_MAX) {
    shortest = 0;
  }
  return shortest;
}

int Dict::UniformCertainties(const WERD_CHOICE& word) {
  float Certainty;
  float WorstCertainty = FLT_MAX;
  float CertaintyThreshold;
  double TotalCertainty;
  double TotalCertaintySquared;
  double Variance;
  float Mean, StdDev;
  int word_length = word.length();

  if (word_length < 3)
    return true;

  TotalCertainty = TotalCertaintySquared = 0.0;
  for (int i = 0; i < word_length; ++i) {
    Certainty = word.certainty(i);
    TotalCertainty += Certainty;
    TotalCertaintySquared += static_cast<double>(Certainty) * Certainty;
    if (Certainty < WorstCertainty)
      WorstCertainty = Certainty;
  }

  // Subtract off worst certainty from statistics.
  word_length--;
  TotalCertainty -= WorstCertainty;
  TotalCertaintySquared -= static_cast<double>(WorstCertainty) * WorstCertainty;

  Mean = TotalCertainty / word_length;
  Variance = ((word_length * TotalCertaintySquared -
    TotalCertainty * TotalCertainty) /
    (word_length * (word_length - 1)));
  if (Variance < 0.0)
    Variance = 0.0;
  StdDev = sqrt(Variance);

  CertaintyThreshold = Mean - stopper_allowable_character_badness * StdDev;
  if (CertaintyThreshold > stopper_nondict_certainty_base)
    CertaintyThreshold = stopper_nondict_certainty_base;

  if (word.certainty() < CertaintyThreshold) {
    if (stopper_debug_level >= 1)
      tprintf("Stopper: Non-uniform certainty = %4.1f"
              " (m=%4.1f, s=%4.1f, t=%4.1f)\n",
              word.certainty(), Mean, StdDev, CertaintyThreshold);
    return false;
  } else {
    return true;
  }
}

} // namespace tesseract
