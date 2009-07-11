///////////////////////////////////////////////////////////////////////
// File:        permngram.cpp
// Description: Character n-gram permuter
// Author:      Thomas Kielbus
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

#include "const.h"
#include "permngram.h"
#include "permute.h"
#include "dawg.h"
#include "tordvars.h"
#include "stopper.h"
#include "globals.h"
#include "context.h"
#include "ndminx.h"
#include "dict.h"
#include "conversion.h"

#include <math.h>
#include <ctype.h>

// Ratio to control the relative importance of the classifier and the ngram
// in the final score of a classification unit. Must be >= 0 and <= 1.
// A value of 1.0 uses only the shape classifier score.
// A value of 0.0 uses only the ngram score.
double_VAR(classifier_score_ngram_score_ratio,
           0.7,
           "");

// Rating adjustment multiplier for words not in the DAWG. Must be >= 1.
double_VAR(non_dawg_prefix_rating_adjustment,
           1.5,
           "");

// HypothesisPrefix represents a word prefix during the search of the
// character-level n-gram model based permuter.
// It holds the data needed to create the corresponding A_CHOICE.
// Note that the string stored in the _word data member always begin with a
// space character. This is used by the n-gram model to score the word.
// HypothesisPrefix also contains the node in the DAWG that is reached when
// searching for the corresponding prefix.
class HypothesisPrefix {
 public:
  HypothesisPrefix();
  HypothesisPrefix(const HypothesisPrefix& prefix,
                   A_CHOICE* choice,
                   bool end_of_word,
                   const tesseract::Dawg *dawg,
                   tesseract::Dict* dict);

  double rating() const {return rating_;}
  double certainty() const {return certainty_;}
  const char* word() const {return word_;}
  const char* unichar_lengths() const {return unichar_lengths_;}
  const float* certainty_array() const {return certainty_array_;}
  bool is_dawg_prefix() const {return is_dawg_prefix_;}
  NODE_REF dawg_node() const {return dawg_node_;}

 private:
  double rating_;
  double certainty_;
  char word_[UNICHAR_LEN * MAX_WERD_LENGTH + 2];
  char unichar_lengths_[MAX_WERD_LENGTH + 1];
  float certainty_array_[MAX_WERD_LENGTH + 1];
  NODE_REF dawg_node_;
  bool is_dawg_prefix_;
};

// HypothesisPrefix is the class used as nodes in HypothesisPrefixLists
typedef HypothesisPrefix HypothesisPrefixListNode;

// HypothesisPrefixList maintains a sorted list of HypothesisPrefixes. The size
// is bounded by the argument given to the constructor.
// For the sake of simplicity, current implementation is not as efficient as it
// could be. The list is represented by a static array of pointers to its
// elements. All nodes are stored in positions from 0 to (size() - 1).
class HypothesisPrefixList {
 public:
  HypothesisPrefixList(int size_bound);
  ~HypothesisPrefixList();

  void add_node(HypothesisPrefix* node);
  int size() const {return _size;}
  void clear();
  const HypothesisPrefix& node(int index) {return *_list_nodes[index];}

 private:
  HypothesisPrefix** _list_nodes;
  int _size_bound;
  int _size;
};

// Return the classifier_score_ngram_score_ratio for a given choice string.
// The classification decision for characters like comma and period should
// be based only on shape rather than on shape and n-gram score.
// Return 1.0 for them, the default classifier_score_ngram_score_ratio
// otherwise.
static double get_classifier_score_ngram_score_ratio(const char* choice);

// Permute the given char_choices using a character level n-gram model and
// return the best word choice found.
// This is performed by maintaining a HypothesisPrefixList of HypothesisPrefixes.
// For each character position, each possible character choice is appended to
// the best current prefixes to create the list of best prefixes at the next
// character position.
namespace tesseract {
A_CHOICE *Dict::ngram_permute_and_select(CHOICES_LIST char_choices,
                                         float rating_limit,
                                         const Dawg *dawg) {
  if (array_count (char_choices) <= MAX_WERD_LENGTH) {
    CHOICES choices;
    int char_index_max = array_count(char_choices);
    HypothesisPrefixList list_1(20);
    HypothesisPrefixList list_2(20);
    HypothesisPrefixList* current_list = &list_1;
    HypothesisPrefixList* next_list = &list_2;
    HypothesisPrefix* initial_node = new HypothesisPrefix();
    current_list->add_node(initial_node);
    for (int char_index = 0; char_index < char_index_max; ++char_index) {
      iterate_list(choices, (CHOICES) array_index(char_choices, char_index)) {
        A_CHOICE* choice = (A_CHOICE *) first_node(choices);
        for (int node_index = 0;
             node_index < current_list->size();
             ++node_index) {
          // Append this choice to the current node
          HypothesisPrefix* new_node = new HypothesisPrefix(
              current_list->node(node_index),
              choice,
              char_index == char_index_max - 1,
              dawg, this);
          next_list->add_node(new_node);
        }
      }
      // Clear current list and switch lists
      current_list->clear();
      HypothesisPrefixList* temp_list = current_list;
      current_list = next_list;
      next_list = temp_list;

      // Give up if the current best rating is worse than rating_limit
      if (current_list->node(0).rating() > rating_limit)
        return new_choice (NULL, NULL, MAXFLOAT, -MAXFLOAT, -1, NO_PERM);
    }
    const HypothesisPrefix& best_word = current_list->node(0);
    A_CHOICE* best_choice = new_choice (best_word.word() + 1,
                                        best_word.unichar_lengths(),
                                        best_word.rating(),
                                        best_word.certainty(), -1,
                                        valid_word(best_word.word() + 1) ?
                                        SYSTEM_DAWG_PERM : TOP_CHOICE_PERM);
    LogNewWordChoice(best_choice, best_word.is_dawg_prefix() ?
                     1.0 : non_dawg_prefix_rating_adjustment,
                     const_cast<float*>(best_word.certainty_array()),
                     getUnicharset());
    return best_choice;
  } else {
    return new_choice (NULL, NULL, MAXFLOAT, -MAXFLOAT, -1, NO_PERM);
  }
}
}  // namespace tesseract

double get_classifier_score_ngram_score_ratio(const char* choice) {
  if (!strcmp(",", choice) ||
      !strcmp(".", choice))
    return 1.0;
  else
    return classifier_score_ngram_score_ratio;
}

// Initial HypothesisPrefix constructor used to create the first state of the
// search.
HypothesisPrefix::HypothesisPrefix() {
  rating_ = 0;
  certainty_ = MAXFLOAT;
  strcpy(word_, " ");
  unichar_lengths_[0] = '\0';
  dawg_node_ = 0;
  is_dawg_prefix_ = true;
}

// Main constructor to create a new HypothesisPrefix by appending a character
// choice (A_CHOICE) to an existing HypothesisPrefix. This constructor takes
// care of copying the original prefix's data members, appends the character
// choice to the word and updates its rating using a character-level n-gram
// model. The state in the DAWG is also updated.
HypothesisPrefix::HypothesisPrefix(const HypothesisPrefix& prefix,
                                   A_CHOICE* choice,
                                   bool end_of_word,
                                   const tesseract::Dawg *dawg,
                                   tesseract::Dict* dict) {
  char* word_ptr = word_;
  const char* prefix_word_ptr = prefix.word_;

  // Copy first space character
  *(word_ptr++) = *(prefix_word_ptr++);

  // Copy existing word, unichar_lengths, certainty_array
  int char_index;
  for (char_index = 0;
       prefix.unichar_lengths_[char_index] != '\0';
       ++char_index) {
    for (int char_subindex = 0;
         char_subindex < prefix.unichar_lengths_[char_index];
         ++char_subindex) {
      *(word_ptr++) = *(prefix_word_ptr++);
    }
    unichar_lengths_[char_index] = prefix.unichar_lengths_[char_index];
    certainty_array_[char_index] = prefix.certainty_array_[char_index];
  }

  // If choice is empty, use a space character instead
  const char* class_string_choice = *class_string(choice) == '\0' ?
      " " : class_string(choice);

  // Update certainty
  certainty_ = MIN(prefix.certainty_, class_certainty(choice));

  // Apprend choice to the word
  strcpy(word_ptr, class_string_choice);
  unichar_lengths_[char_index] = strlen(class_string_choice);
  unichar_lengths_[char_index + 1] = '\0';

  // Append choice certainty to the certainty array
  certainty_array_[char_index] = class_certainty(choice);

  // Copy DAWG node state
  dawg_node_ = prefix.dawg_node_;
  is_dawg_prefix_ = prefix.is_dawg_prefix_;

  // Verify DAWG and update dawg_node_ if the current prefix is already valid
  if (is_dawg_prefix_) {
    for (int char_subindex = 0;
         class_string_choice[char_subindex] != '\0';
         ++char_subindex) {

      // TODO(daria): update this code (and the rest of ngram permuter code
      // to deal with unichar ids, make use of the new parallel dawg search
      // and use WERD_CHOICE, BLOB_CHOICE_LIST_VECTOR instead of the deprecated
      // A_CHOICE.
      tprintf("Error: ngram permuter functionality is not available\n");
      exit(1);

      // Verify each byte of the appended character. Note that word_ptr points
      // to the first byte so (word_ptr - (word_ + 1)) is the index of the first
      // new byte in the string that starts at (word_ + 1).
      /*
      int current_byte_index = word_ptr - (word_ + 1) + char_subindex;
      if (!(dict->*dict->letter_is_okay_)(
         dawg, &dawg_node_, current_byte_index, word_ + 1,
         end_of_word && class_string_choice[char_subindex + 1] == '\0')) {
        dawg_node_ = NO_EDGE;
        is_dawg_prefix_ = false;
        break;
      }
      */
    }
  }

  // Copy the prefix rating
  rating_ = prefix.rating_;

  // Compute rating of current character
  double probability = probability_in_context(prefix.word_, -1,
                                              class_string_choice, -1);

  // If last character of the word, take the following space into account
  if (end_of_word)
    probability *= probability_in_context(word_, -1, " ", -1);

  double local_classifier_score_ngram_score_ratio =
      get_classifier_score_ngram_score_ratio(class_string_choice);

  double classifier_rating = class_rating(choice);
  double ngram_rating = -log(probability) / log(2.0);
  double mixed_rating =
      local_classifier_score_ngram_score_ratio * classifier_rating +
      (1 - local_classifier_score_ngram_score_ratio) * ngram_rating;

  // If the current word is not a valid prefix, adjust the rating of the
  // character being appended. If it used to be a valid prefix, compensate for
  // previous adjustments.
  if (!is_dawg_prefix_) {
    if (prefix.is_dawg_prefix_)
      rating_ *= non_dawg_prefix_rating_adjustment;
    mixed_rating *= non_dawg_prefix_rating_adjustment;
  }

  // Update rating by adding the rating of the character being appended.
  rating_ += mixed_rating;
}

// Create an empty HypothesisPrefixList. Its maximum size is set to the given
// bound.
HypothesisPrefixList::HypothesisPrefixList(int size_bound):
    _size_bound(size_bound),
    _size(0) {
  _list_nodes = new HypothesisPrefix*[_size_bound];
  for (int i = 0; i < _size_bound; ++i)
    _list_nodes[i] = NULL;
}

// Destroy a HypothesisPrefixList all contained nodes are deleted as well.
HypothesisPrefixList::~HypothesisPrefixList() {
  this->clear();
  delete[] _list_nodes;
}

// Add a node to the HypothesisPrefixList. Maintains the sorted list property.
// Note that the HypothesisPrefixList takes ownership of the given node and
// might delete it if needed. It must therefore have been allocated on the heap.
void HypothesisPrefixList::add_node(HypothesisPrefix* node) {
  // Detect nodes that have a worst rating that the current maximum and treat
  // them separately.
  if (_size > 0 && _list_nodes[_size - 1]->rating() < node->rating()) {
    if (_size == _size_bound) {
      // The list is already full. This node will not be added
      delete node;
    } else {
      // The list is not full. Add the node at the last position.
      _list_nodes[_size] = node;
      ++_size;
    }
    return;
  }
  // Find the correct position
  int node_index_target = 0;
  while (node_index_target < _size_bound &&
         _list_nodes[node_index_target] != NULL &&
         _list_nodes[node_index_target]->rating() < node->rating()) {
    ++node_index_target;
  }
  if (node_index_target >= _size_bound) {
    delete node;
  } else {
    // Move next states by 1. Starting from the last one.
    int node_index_move = _size - 1;
    while (node_index_move >= node_index_target) {
      if (node_index_move == _size_bound - 1)
        delete _list_nodes[node_index_move];
      else
        _list_nodes[node_index_move + 1] = _list_nodes[node_index_move];
      _list_nodes[node_index_move] = NULL;
      --node_index_move;
    }
    // Insert new node
    _list_nodes[node_index_target] = node;
    // Increment size if it has changed
    if (_size < _size_bound)
      ++_size;
  }
}

// Delete all contained nodes and set the size of the HypothesisPrefixList to 0
void HypothesisPrefixList::clear() {
  for (int i = 0; i < _size_bound && _list_nodes[i] != NULL; ++i) {
    delete _list_nodes[i];
    _list_nodes[i] = NULL;
  }
  _size = 0;
}
