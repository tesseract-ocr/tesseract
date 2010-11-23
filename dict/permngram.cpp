///////////////////////////////////////////////////////////////////////
// File:        permngram.cpp
// Description: Character n-gram permuter
// Author:      Thomas Kielbus, modified by Daria Antonova
// Created:     Wed Sep 12 11:26:43 PDT 2007
//
// (C) Copyright 2007, Google Inc.
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

#include "permngram.h"

#include "dict.h"
#include "elst.h"
#include "genericvector.h"
#include "host.h"
#include "strngs.h"
#include "unichar.h"
#include "params.h"

#include <math.h>
#include <ctype.h>

ELISTIZE(ViterbiEntry)

// Permutes the given char_choices using a character level n-gram model and
// returns the best word choice found.
// This is performed with the help of Viterbi algorithm for finding the most
// likely string of all the possible ones represented by char_choices.
// The transition probabilities are given by the ngram model and are obtained
// by calling ProbabilityInContext() function. The emission probabilities are
// given by the normalized certainties supplied by tesseract character
// classifier. They are not true probabilities, but more or less depict
// the probability that the character choice is correct.
// All the computation is performed in log-space to avoid underflow.

namespace tesseract {

static const float kSmallProbability = 0.0000001;

// Computes -(log(prob(classifier)) + log(prob(ngram model)))
// for the given string (str_ptr) in the given context.
// If there are multiple unichars at one position - take the average of their
// probabilities. UNICHAR::utf8_step() is used to separate out individual
// UTF8 characters, since probability_in_context() can only handle one at
// a time (while unicharset might contain ngrams and glyphs composed from
// multiple UTF8 characters).
float Dict::compute_ngram_cost(float certainty, float denom,
                               const char *str, const char *context,
                               const UNICHARSET &unicharset) {
  const char *context_ptr = context;
  char *modified_context = NULL;
  char *modified_context_end = NULL;
  const char *str_ptr = str;
  const char *str_end = str_ptr + strlen(str_ptr);
  float prob = 0.0;
  int num = 0;
  int step = 0;
  while (str_ptr < str_end && (step = UNICHAR::utf8_step(str_ptr))) {
    if (ngram_permuter_debug_level > 1) {
      tprintf("prob(%s | %s)=%g\n", str_ptr, context_ptr,
              (this->*probability_in_context_)(context_ptr, -1, str_ptr, step));
    }
    prob += (this->*probability_in_context_)(context_ptr, -1, str_ptr, step);
    ++num;
    if (use_only_first_uft8_step) break;
    str_ptr += step;
    // If there are multiple characters present in str, context is
    // updated to include the previously examined characters from str,
    // unless use_only_first_uft8_step is true.
    if (str_ptr < str_end) {
      if (modified_context == NULL) {
        int context_len = strlen(context);
        modified_context = new char[context_len + strlen(str_ptr) + step + 1];
        strcpy(modified_context, context);
        modified_context_end = modified_context + context_len;
        context_ptr = modified_context;
      }
      strncpy(modified_context_end, str_ptr - step, step);
      modified_context_end += step;
      *modified_context_end = '\0';
    }
  }
  if (modified_context != NULL) delete[] modified_context;
  if (prob < kSmallProbability) prob = kSmallProbability;
  float cost =
    -(log(-1.0 * certainty / denom) +
      ngram_model_scale_factor * log(prob / static_cast<float>(num)));
  if (ngram_permuter_debug_level > 1) {
    tprintf("-log p(%s | %s)=%g\n", str, context_ptr, cost);
  }
  return cost;
}

WERD_CHOICE *Dict::ngram_permute_and_select(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float rating_limit,
    float adjust_factor) {
  if (char_choices.length() > MAX_WERD_LENGTH ||
      char_choices.length() <= 1) return NULL;
  const UNICHARSET &unicharset = getUnicharset();
  if (ngram_permuter_debug_level > 1) {
    tprintf("Starting ngram permuter\n");
  }
  // Make sure there are no NULL choices and no empty choices lists.
  // Compute the size of the largest choices list.
  // Compute the normalization factors for the classifier confidences.
  int i;
  BLOB_CHOICE_IT bit;
  int max_num_choices = 0;
  float denoms[MAX_WERD_LENGTH];
  float denom;
  for (i = 0; i < char_choices.length(); ++i) {
    bit.set_to_list(char_choices[i]);
    int len = 0;
    denom = 0.0;
    for (bit.mark_cycle_pt(); !bit.cycled_list(); bit.forward()) {
      if (bit.data() == NULL) return NULL;
      ++len;
      denom += -1.0 / bit.data()->certainty();
    }
    if (len == 0) return NULL;
    // The ideal situation would be to have the classifier scores for
    // classifying each position as each of the characters in the unicharset.
    // Since we can not do this because of speed, we add a very crude estimate
    // of what these scores for the "missing" classifications would sum up to.
    denom += (unicharset.size() - len) * (-1.0 / ngram_permuter_nonmatch_score);
    denoms[i] = denom;
    if (max_num_choices < len) max_num_choices = len;
  }
  // Compute the best string for each position of the char_chocies.
  ViterbiEntry_LIST vlist1;
  ViterbiEntry_LIST vlist2;
  ViterbiEntry_LIST *tmp_vlist_ptr;
  ViterbiEntry_LIST *curr_vlist_ptr = &vlist1;
  ViterbiEntry_LIST *prev_vlist_ptr = &vlist2;
  ViterbiEntry *ve = NULL;
  for (i = 0; i < char_choices.length(); ++i) {
    bit.set_to_list(char_choices[i]);
    if (ngram_permuter_debug_level > 1) {
      tprintf("Exploring char list of size %d at position %d\n",
              char_choices[i]->length(), i);
    }
    denom = denoms[i];
    int curr_vlist_len = 0;
    float max_cost = 0;
    for (bit.mark_cycle_pt(); !bit.cycled_list(); bit.forward()) {
      BLOB_CHOICE *b = bit.data();
      const CHAR_FRAGMENT *frag = unicharset.get_fragment(b->unichar_id());
      const char *curr_unichar = (frag != NULL) ? frag->get_unichar() :
        unicharset.id_to_unichar(b->unichar_id());
      if (i == 0) {  // initialize Viterbi entries for the 1st position
        if (frag != NULL && !frag->is_beginning()) continue;
        ve = new ViterbiEntry();
        ve->string_so_far[0] = '\0';
        ve->num_unichars = 0;
        if (frag == NULL) ve->UpdateChars(curr_unichar, 1, b->unichar_id());
        ve->frag = frag;
        // TODO(daria): check if we need to pass " " instead of "".
        // TODO(daria): see if we can use the previous word.
        ve->cost = compute_ngram_cost(b->certainty(), denom,
                                      curr_unichar, "", unicharset);
        ve->ratings_sum = b->rating();
        curr_vlist_ptr->add_sorted(ViterbiEntry::compare, false, ve);
        if (ngram_permuter_debug_level > 1) ve->Print();
        curr_vlist_len++;
      } else {
        ViterbiEntry_IT prev_vlist_it = prev_vlist_ptr;
        for (prev_vlist_it.mark_cycle_pt(); !prev_vlist_it.cycled_list();
             prev_vlist_it.forward()) {
          const ViterbiEntry &prev_ve = *(prev_vlist_it.data());
          // Drop this entry if it's ratings_sum * adjust_factor will be
          // larger than rating_limit.
          if ((prev_ve.ratings_sum + b->rating()) * adjust_factor >
              rating_limit) {
            if (ngram_permuter_debug_level > 1) {
              tprintf("Discarded expensive ViterbiEntry with rating %g*%g"
                      ", last unichar %s, rating_limit=%g\n",
                      prev_ve.ratings_sum + b->rating(),
                      unicharset.id_to_unichar(b->unichar_id()),
                      adjust_factor, rating_limit);
            }
            continue;
          }

          // If we are dealing with fragments make sure that the previous
          // fragment matches the current one.
          // For character composed from n fragments probability_in_context()
          // is called n times total.
          if (prev_ve.frag != NULL || frag != NULL) {
            CHAR_FRAGMENT_INFO prev_char_frag_info;
            prev_char_frag_info.fragment = prev_ve.frag;
            CHAR_FRAGMENT_INFO char_frag_info;
            if (!fragment_state_okay(
                b->unichar_id(), 0.0, 0.0, &prev_char_frag_info,
                (ngram_permuter_debug_level > 1) ? "ngram_debug" : NULL,
                i+1 == char_choices.length(), &char_frag_info)) continue;
          }
          float cost = prev_ve.cost +
            compute_ngram_cost(b->certainty(), denom, curr_unichar,
                               prev_ve.string_so_far, unicharset);
          if (curr_vlist_len >= max_viterbi_list_size && cost >= max_cost) {
            if (ngram_permuter_debug_level > 1) {
              tprintf("Discarded ViterbiEntry with a high cost %g"
                      ", last unichar %s, max_cost=%g\n",
                      cost, unicharset.id_to_unichar(b->unichar_id()),
                      max_cost);
            }
            continue;
          }

          // Add a new entry to the current ViterbiEntry_CLIST.
          ve = new ViterbiEntry();
          ve->CopyChars(prev_ve);
          if (frag == NULL) {
            ve->UpdateChars(curr_unichar, 1, b->unichar_id());
          } else if (frag->is_ending()) {
            ve->UpdateChars("", frag->get_total(),
                            unicharset.unichar_to_id(frag->get_unichar()));
          }
          ve->frag = (frag != NULL && frag->is_ending()) ? NULL : frag;
          ve->cost = cost;
          ve->ratings_sum = prev_ve.ratings_sum + b->rating();
          curr_vlist_ptr->add_sorted(ViterbiEntry::compare, false, ve);
          if (cost > max_cost) max_cost = cost;
          if (ngram_permuter_debug_level > 1) ve->Print();
          curr_vlist_len++;
        }
      }
    }
    if (curr_vlist_len == 0) { return NULL; } // no entries stored
    // Leave only max_viterbi_list_size ViterbiEntries.
    if (curr_vlist_len > max_viterbi_list_size) {
      ViterbiEntry_IT curr_vlist_it = curr_vlist_ptr;
      curr_vlist_len = 0;
      for (curr_vlist_it.mark_cycle_pt(); !curr_vlist_it.cycled_list();
           curr_vlist_it.forward()) {
        if (curr_vlist_len < max_viterbi_list_size) {
          curr_vlist_len++;
        } else {
          delete curr_vlist_it.extract();
        }
      }
    }
    // Clear prev_vlist, swap prev_vlist_ptr and curr_vlist_ptr
    prev_vlist_ptr->clear();
    tmp_vlist_ptr = prev_vlist_ptr;
    prev_vlist_ptr = curr_vlist_ptr;
    curr_vlist_ptr = tmp_vlist_ptr;
  }

  float best_cost = MAX_FLOAT32;
  ViterbiEntry *best_ve = NULL;
  ViterbiEntry_IT prev_vlist_it = prev_vlist_ptr;
  for (prev_vlist_it.mark_cycle_pt(); !prev_vlist_it.cycled_list();
       prev_vlist_it.forward()) {
    ViterbiEntry *tmp_ve = prev_vlist_it.data();
    if (tmp_ve->cost < best_cost) {
      best_ve = tmp_ve;
      best_cost = tmp_ve->cost;
    }
  }

  // Populate ngram_choice with the unichar ids from best_ve->unichar_ids.
  // Get the certainties of the corresponding unichars from char_choices.
  WERD_CHOICE *ngram_choice =
    new WERD_CHOICE(best_ve->num_unichars);
  float certainties[MAX_WERD_LENGTH];
  float min_certainty = 0.0;
  int char_choices_index = 0;
  const char *frag_uch = "";
  int j;
  for (i = 0; i < best_ve->num_unichars; ++i) {
    ngram_choice->append_unichar_id_space_allocated(
        best_ve->unichar_ids[i], best_ve->fragment_lengths[i], 0.0, 0.0);
    if (best_ve->fragment_lengths[i] > 1) {
      frag_uch = unicharset.id_to_unichar(best_ve->fragment_lengths[i]);
    }
    for (j = 0; j < best_ve->fragment_lengths[i]; ++j) {
      UNICHAR_ID unichar_id_for_cert;
      if (best_ve->fragment_lengths[i] > 1) {
        unichar_id_for_cert = unicharset.unichar_to_id(
            CHAR_FRAGMENT::to_string(frag_uch, j,
                                     best_ve->fragment_lengths[i]).string());
      } else {
        unichar_id_for_cert = best_ve->unichar_ids[i];
      }
      bit.set_to_list(char_choices[char_choices_index]);
      for (bit.mark_cycle_pt(); !bit.cycled_list(); bit.forward()) {
        if (bit.data()->unichar_id() == unichar_id_for_cert) {
          certainties[char_choices_index] = bit.data()->certainty();
          if (certainties[char_choices_index] < min_certainty) {
            min_certainty = certainties[char_choices_index];
          }
          break;
        }
      }
      char_choices_index++;
    }
  }
  ngram_choice->set_certainty(min_certainty);
  ngram_choice->set_permuter(NGRAM_PERM);
  ngram_choice->set_rating(best_ve->ratings_sum * adjust_factor);
  LogNewChoice(adjust_factor, certainties, false, ngram_choice);
  if (ngram_permuter_debug_level > 1) {
    tprintf("Ngram model choice: %s\n",
            ngram_choice->debug_string(getUnicharset()).string());
    ngram_choice->print();
  }

  return ngram_choice;
}

}  // namespace tesseract
