///////////////////////////////////////////////////////////////////////
// File:        dict.h
// Description: dict class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#ifndef TESSERACT_DICT_DICT_H_
#define TESSERACT_DICT_DICT_H_

#include "choices.h"
#include "choicearr.h"
#include "dawg.h"
#include "image.h"
#include "ratngs.h"
#include "stopper.h"
#include "unicharset.h"

typedef LIST AMBIG_TABLE;

// Struct used to hold temprorary information about fragments.
struct CHAR_FRAGMENT_INFO {
  UNICHAR_ID unichar_id;
  const CHAR_FRAGMENT *fragment;
  int num_fragments;
  float rating;
  float certainty;
};

namespace tesseract {
class Dict
{
 public:
  Dict(Image* image_ptr);
  ~Dict();
  Image* getImage() {
    return image_ptr_;
  }
  UNICHARSET& getUnicharset() {
    return getImage()->getCCUtil()->unicharset;
  }

  /* permdawg.cpp ************************************************************/
  void init_permdawg();
  void end_permdawg();
  void append_next_choice(  /*previous option */
                          EDGE_ARRAY dawg,
                          NODE_REF node,
                          char permuter,
                          char *word,
                          char unichar_lengths[],
                          int unichar_offsets[],
                          CHOICES_LIST choices,
                          int char_choice_index,
                          int word_index,
                          A_CHOICE *this_choice,
                          const char *prevchar,
                          float *limit,
                          float rating,
                          float certainty,
                          float *rating_array,
                          float *certainty_array,
                          int word_ending,
                          int last_word,
                          const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                          char fragment_lengths[],
                          CHOICES *result);
  int test_freq_words(const char *word);
  CHOICES dawg_permute(EDGE_ARRAY dawg,
                       NODE_REF node,
                       char permuter,
                       CHOICES_LIST choices,
                       int char_choice_index,
                       int word_index,
                       float *limit,
                       char *word,
                       char unichar_lengths[],
                       int unichar_offsets[],
                       float rating,
                       float certainty,
                       float *rating_array,
                       float *certainty_array,
                       int last_word,
                       const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                       char fragment_lengths[]);
  void dawg_permute_and_select(const char *string,
                               EDGE_ARRAY dawg,
                               char permuter,
                               CHOICES_LIST character_choices,
                               A_CHOICE *best_choice,
                               inT16 system_words);
  void adjust_word(A_CHOICE *best_choice, float *certainty_array);

  /* permute.cpp *************************************************************/
  void add_document_word(const WERD_CHOICE &best_choice);
  void init_permute();
  WERD_CHOICE *permute_top_choice(
    const BLOB_CHOICE_LIST_VECTOR &char_choices,
    float* rating_limit,
    WERD_CHOICE *raw_choice,
    BOOL8 *any_alpha);
  const char* choose_il1(const char *first_char,       //first choice
                         const char *second_char,      //second choice
                         const char *third_char,       //third choice
                         const char *prev_char,        //prev in word
                         const char *next_char,        //next in word
                         const char *next_next_char);  //after next next in word
  int valid_word(const char *string);
  A_CHOICE *permute_words(CHOICES_LIST char_choices,
                          float rating_limit);
  WERD_CHOICE *permute_all(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                           float rating_limit,
                           WERD_CHOICE *raw_choice);
  void end_permute();
  void adjust_non_word(const char *word, const char *word_lengths,
                       float rating, float *new_rating,
                       float *adjust_factor);
  void permute_subword(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                       float rating_limit,
                       int start,
                       int end,
                       WERD_CHOICE *current_word);
  void permute_characters(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                          float limit,
                          WERD_CHOICE *best_choice,
                          WERD_CHOICE *raw_choice);
  WERD_CHOICE *permute_compound_words(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);
  bool fragment_state_okay(UNICHAR_ID curr_unichar_id,
                           float curr_rating, float curr_certainty,
                           const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                           const char *debug, int word_ending,
                           CHAR_FRAGMENT_INFO *char_frag_info);
  WERD_CHOICE *top_fragments_permute_and_select(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      float rating_limit);

  void top_fragments_permute(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      int char_choice_index,
      float min_rating,
      const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      WERD_CHOICE *word,
      float certainties[],
      float *limit,
      WERD_CHOICE *best_choice);

  void append_top_fragments_choices(
      const BLOB_CHOICE_LIST_VECTOR &char_choices,
      const BLOB_CHOICE &blob_choice,
      int char_choice_index,
      float min_rating,
      const CHAR_FRAGMENT_INFO *prev_char_frag_info,
      WERD_CHOICE *word,
      float certainties[],
      float *limit,
      WERD_CHOICE *best_choice);

  /* permnum.cpp *************************************************************/
  int pure_number(const char *string, const char *lengths);
  void append_number_choices(int state,
                             char *word,
                             char unichar_lengths[],
                             int unichar_offsets[],
                             CHOICES_LIST choices,
                             int char_index,
                             A_CHOICE *this_choice,
                             float *limit,
                             float rating,
                             float certainty,
                             float *certainty_array,
                             CHOICES *result);
  void append_number_choices(int state,
                             char *word,
                             char unichar_lengths[],
                             int unichar_offsets[],
                             CHOICES_LIST choices,
                             int char_choice_index,
                             int word_index,
                             A_CHOICE *this_choice,
                             float *limit,
                             float rating,
                             float certainty,
                             float *certainty_array,
                             const CHAR_FRAGMENT_INFO *prev_char_frag_info,
                             char fragment_lengths[],
                             CHOICES *result);
  int number_character_type(  //current state
                              const char* ch,
                              int length,
                              int state);
  int valid_number(const char *string, const char *lengths);
  int number_state_change(int state,             //current state
                          const char *word,      //current char
                          const char *lengths);  //length of current char
  A_CHOICE *number_permute_and_select(CHOICES_LIST char_choices,
                                      float rating_limit);
  CHOICES number_permute(int state,
                         CHOICES_LIST choices,
                         int char_choice_index,
                         int word_index,
                         float *limit,
                         char *word,
                         char unichar_lengths[],
                         int unichar_offsets[],
                         float rating,
                         float certainty,
                         float *certainty_array,
                         const CHAR_FRAGMENT_INFO
                           *prev_char_frag_info,
                         char fragment_lengths[]);
  void adjust_number(A_CHOICE *best_choice, float *certainty_array);
  /* context.cpp *************************************************************/
  int punctuation_ok(const char *word, const char *lengths);
  int case_ok(const char *word, const char *lengths);
  /* stopper.cpp *************************************************************/
  int NoDangerousAmbig(const char *Word,
                       const char *Word_lengths,
                       DANGERR *fixpt);
  AMBIG_TABLE *FillAmbigTable();
  int LengthOfShortestAlphaRun(const WERD_CHOICE &WordChoice);
  VIABLE_CHOICE NewViableChoice(const WERD_CHOICE &WordChoice,
                                FLOAT32 AdjustFactor,
                                const float Certainties[]);
  void PrintViableChoice(FILE *File, const char *Label, VIABLE_CHOICE Choice);
  int StringSameAs(const char *String,
                   const char *String_lengths,
                   VIABLE_CHOICE ViableChoice);
  bool StringSameAs(const WERD_CHOICE &WordChoice,
                    VIABLE_CHOICE ViableChoice);
  int AcceptableChoice(const BLOB_CHOICE_LIST_VECTOR &Choices,
                       const WERD_CHOICE &BestChoice,
                       const WERD_CHOICE &RawChoice,
                       DANGERR *fixpt,
                       ACCEPTABLE_CHOICE_CALLER caller);
  int AcceptableResult(const WERD_CHOICE &BestChoice,
                       const WERD_CHOICE &RawChoice);
  int AmbigsFound(char *Word,
                  char *CurrentChar,
                  const char *Tail,
                  const char *Tail_lengths,
                  LIST Ambigs,
                  DANGERR *fixpt);
  int ChoiceSameAs(const WERD_CHOICE &WordChoice, VIABLE_CHOICE ViableChoice);
  void LogNewRawChoice(const WERD_CHOICE &WordChoice, FLOAT32 AdjustFactor,
                       const float Certainties[]);
  void LogNewWordChoice(const WERD_CHOICE &WordChoice, FLOAT32 AdjustFactor,
                        const float Certainties[]);
  void EndDangerousAmbigs();
  int CurrentBestChoiceIs(const WERD_CHOICE &WordChoice);
  FLOAT32 CurrentBestChoiceAdjustFactor();
  int CurrentWordAmbig();
  void DebugWordChoices();
  void FillViableChoice(const WERD_CHOICE &WordChoice,
                        FLOAT32 AdjustFactor, const float Certainties[],
                        bool SameString, VIABLE_CHOICE ViableChoice);
  /* choices.cpp *************************************************************/
  void print_word_string(const char* str);
  /* dawg.cpp ****************************************************************/
  int def_letter_is_okay(void* dawg, void* node, int char_index,
                         char prevchar, const char* word, int word_end);
  int new_letter_is_okay(void* dawg, void* node, int char_index,
                         char prevchar, const char* word, int word_end);
  int fst_letter_is_okay(void* dawg, void* node, int char_index,
                         char prevchar, const char* word, int word_end);
  inT32 word_in_dawg(EDGE_ARRAY dawg, const char *string);
  inT32 verify_trailing_punct(EDGE_ARRAY dawg, char *word, inT32 char_index);
  void print_word_choice(const char *label, A_CHOICE* choice);
  void print_choices(const char *label,
                     CHOICES rating);   // List of (A_CHOICE*).
  /* permngram.cpp ***********************************************************/
  A_CHOICE *ngram_permute_and_select(CHOICES_LIST char_choices,
                                     float rating_limit,
                                     EDGE_ARRAY dawg);
  /* lookdawg.cpp ************************************************************/
  void print_lost_words (EDGE_ARRAY dawg,
                         char       *filename);
  void check_for_words (EDGE_ARRAY dawg,
                        char       *filename);
  /* trie.cpp ****************************************************************/
  void read_word_list(const char *filename,
                      EDGE_ARRAY dawg,
                      inT32 max_num_edges,
                      inT32 reserved_edges);

  // pointer to function to allow it to be modified.
  int (Dict::*letter_is_okay_)(void* dawg, void* node, int char_index,
                               char prevchar, const char *word, int word_end);
  /* conversion.cpp **********************************************************/
  // TODO(daria): remove this declaration when conversion.cpp is deprecated.
  void LogNewWordChoice(A_CHOICE *a_choice,
                        FLOAT32 adjust_factor,
                        const float certainties[],
                        const UNICHARSET &unicharset);

 private:
  Image* image_ptr_;
};
}  // namespace tesseract

#endif  // THIRD_PARTY_TESSERACT_DICT_DICT_H_
