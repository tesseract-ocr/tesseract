/* -*-C-*-
 ********************************************************************************
 *
 * File:         trie.cpp  (Formerly trie.c)
 * Description:  Functions to build a trie data structure.
 * Author:       Mark Seaman, OCR Technology
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/

#include "trie.h"

#include "callcpp.h"
#include "dawg.h"
#include "dict.h"
#include "genericvector.h"
#include "helpers.h"
#include "kdpair.h"

namespace tesseract {

const char kDoNotReverse[] = "RRP_DO_NO_REVERSE";
const char kReverseIfHasRTL[] = "RRP_REVERSE_IF_HAS_RTL";
const char kForceReverse[] = "RRP_FORCE_REVERSE";

const char * const RTLReversePolicyNames[] = {
  kDoNotReverse,
  kReverseIfHasRTL,
  kForceReverse
};

const char Trie::kAlphaPatternUnicode[] = "\u2000";
const char Trie::kDigitPatternUnicode[] = "\u2001";
const char Trie::kAlphanumPatternUnicode[] = "\u2002";
const char Trie::kPuncPatternUnicode[] = "\u2003";
const char Trie::kLowerPatternUnicode[] = "\u2004";
const char Trie::kUpperPatternUnicode[] = "\u2005";

const char *Trie::get_reverse_policy_name(RTLReversePolicy reverse_policy) {
  return RTLReversePolicyNames[reverse_policy];
}

// Reset the Trie to empty.
void Trie::clear() {
  nodes_.delete_data_pointers();
  nodes_.clear();
  root_back_freelist_.clear();
  num_edges_ = 0;
  new_dawg_node();  // Need to allocate node 0.
}

bool Trie::edge_char_of(NODE_REF node_ref, NODE_REF next_node,
                        int direction, bool word_end, UNICHAR_ID unichar_id,
                        EDGE_RECORD **edge_ptr, EDGE_INDEX *edge_index) const {
  if (debug_level_ == 3) {
    tprintf("edge_char_of() given node_ref " REFFORMAT " next_node " REFFORMAT
            " direction %d word_end %d unichar_id %d, exploring node:\n",
            node_ref, next_node, direction, word_end, unichar_id);
    if (node_ref != NO_EDGE) {
      print_node(node_ref, nodes_[node_ref]->forward_edges.size());
    }
  }
  if (node_ref == NO_EDGE) return false;
  assert(node_ref < nodes_.size());
  EDGE_VECTOR &vec = (direction == FORWARD_EDGE) ?
    nodes_[node_ref]->forward_edges : nodes_[node_ref]->backward_edges;
  int vec_size = vec.size();
  if (node_ref == 0 && direction == FORWARD_EDGE) {  // binary search
    EDGE_INDEX start = 0;
    EDGE_INDEX end = vec_size - 1;
    EDGE_INDEX k;
    int compare;
    while (start <= end) {
      k = (start + end) >> 1;  // (start + end) / 2
      compare = given_greater_than_edge_rec(next_node, word_end,
                                            unichar_id, vec[k]);
      if (compare == 0) {  // given == vec[k]
        *edge_ptr = &(vec[k]);
        *edge_index = k;
        return true;
      } else if (compare == 1) {  // given > vec[k]
        start = k + 1;
      } else {  // given < vec[k]
        end = k - 1;
      }
    }
  } else {  // linear search
    for (int i = 0; i < vec_size; ++i) {
      EDGE_RECORD &edge_rec = vec[i];
      if (edge_rec_match(next_node, word_end, unichar_id,
                         next_node_from_edge_rec(edge_rec),
                         end_of_word_from_edge_rec(edge_rec),
                         unichar_id_from_edge_rec(edge_rec))) {
        *edge_ptr = &(edge_rec);
        *edge_index = i;
        return true;
      }
    }
  }
  return false;  // not found
}

bool Trie::add_edge_linkage(NODE_REF node1, NODE_REF node2, bool marker_flag,
                            int direction, bool word_end,
                            UNICHAR_ID unichar_id) {
  EDGE_VECTOR *vec = (direction == FORWARD_EDGE) ?
    &(nodes_[node1]->forward_edges) : &(nodes_[node1]->backward_edges);
  int search_index;
  if (node1 == 0 && direction == FORWARD_EDGE) {
    search_index = 0;  // find the index to make the add sorted
    while (search_index < vec->size() &&
           given_greater_than_edge_rec(node2, word_end, unichar_id,
                                       (*vec)[search_index]) == 1) {
      search_index++;
    }
  } else {
    search_index = vec->size();  // add is unsorted, so index does not matter
  }
  EDGE_RECORD edge_rec;
  link_edge(&edge_rec, node2, marker_flag, direction, word_end, unichar_id);
  if (node1 == 0 && direction == BACKWARD_EDGE &&
      !root_back_freelist_.empty()) {
    EDGE_INDEX edge_index = root_back_freelist_.pop_back();
    (*vec)[edge_index] = edge_rec;
  } else if (search_index < vec->size()) {
    vec->insert(edge_rec, search_index);
  } else {
    vec->push_back(edge_rec);
  }
  if (debug_level_ > 1) {
    tprintf("new edge in nodes_[" REFFORMAT "]: ", node1);
    print_edge_rec(edge_rec);
    tprintf("\n");
  }
  num_edges_++;
  return true;
}

void Trie::add_word_ending(EDGE_RECORD *edge_ptr,
                           NODE_REF the_next_node,
                           bool marker_flag,
                           UNICHAR_ID unichar_id) {
  EDGE_RECORD *back_edge_ptr;
  EDGE_INDEX back_edge_index;
  ASSERT_HOST(edge_char_of(the_next_node, NO_EDGE, BACKWARD_EDGE, false,
                           unichar_id, &back_edge_ptr, &back_edge_index));
  if (marker_flag) {
    *back_edge_ptr |= (MARKER_FLAG << flag_start_bit_);
    *edge_ptr |= (MARKER_FLAG << flag_start_bit_);
  }
  // Mark both directions as end of word.
  *back_edge_ptr |= (WERD_END_FLAG << flag_start_bit_);
  *edge_ptr |= (WERD_END_FLAG << flag_start_bit_);
}

bool Trie::add_word_to_dawg(const WERD_CHOICE &word,
                            const GenericVector<bool> *repetitions) {
  if (word.length() <= 0) return false;  // can't add empty words
  if (repetitions != nullptr) ASSERT_HOST(repetitions->size() == word.length());
  // Make sure the word does not contain invalid unchar ids.
  for (int i = 0; i < word.length(); ++i) {
    if (word.unichar_id(i) < 0 ||
        word.unichar_id(i) >= unicharset_size_) return false;
  }

  EDGE_RECORD *edge_ptr;
  NODE_REF last_node = 0;
  NODE_REF the_next_node;
  bool marker_flag = false;
  EDGE_INDEX edge_index;
  int i;
  int32_t still_finding_chars = true;
  int32_t word_end = false;
  bool  add_failed = false;
  bool found;

  if (debug_level_ > 1) word.print("\nAdding word: ");

  UNICHAR_ID unichar_id;
  for (i = 0; i < word.length() - 1; ++i) {
    unichar_id = word.unichar_id(i);
    marker_flag = (repetitions != nullptr) ? (*repetitions)[i] : false;
    if (debug_level_ > 1) tprintf("Adding letter %d\n", unichar_id);
    if (still_finding_chars) {
      found = edge_char_of(last_node, NO_EDGE, FORWARD_EDGE, word_end,
                           unichar_id, &edge_ptr, &edge_index);
      if (found && debug_level_ > 1) {
        tprintf("exploring edge " REFFORMAT " in node " REFFORMAT "\n",
                edge_index, last_node);
      }
      if (!found) {
        still_finding_chars = false;
      } else if (next_node_from_edge_rec(*edge_ptr) == 0) {
        // We hit the end of an existing word, but the new word is longer.
        // In this case we have to disconnect the existing word from the
        // backwards root node, mark the current position as end-of-word
        // and add new nodes for the increased length. Disconnecting the
        // existing word from the backwards root node requires a linear
        // search, so it is much faster to add the longest words first,
        // to avoid having to come here.
        word_end = true;
        still_finding_chars = false;
        remove_edge(last_node, 0, word_end, unichar_id);
      } else {
        // We have to add a new branch here for the new word.
        if (marker_flag) set_marker_flag_in_edge_rec(edge_ptr);
        last_node = next_node_from_edge_rec(*edge_ptr);
      }
    }
    if (!still_finding_chars) {
      the_next_node = new_dawg_node();
      if (debug_level_ > 1)
        tprintf("adding node " REFFORMAT "\n", the_next_node);
      if (the_next_node == 0) {
        add_failed = true;
        break;
      }
      if (!add_new_edge(last_node, the_next_node,
                        marker_flag, word_end, unichar_id)) {
        add_failed = true;
        break;
      }
      word_end = false;
      last_node = the_next_node;
    }
  }
  the_next_node = 0;
  unichar_id = word.unichar_id(i);
  marker_flag = (repetitions != nullptr) ? (*repetitions)[i] : false;
  if (debug_level_ > 1) tprintf("Adding letter %d\n", unichar_id);
  if (still_finding_chars &&
      edge_char_of(last_node, NO_EDGE, FORWARD_EDGE, false,
                   unichar_id, &edge_ptr, &edge_index)) {
    // An extension of this word already exists in the trie, so we
    // only have to add the ending flags in both directions.
    add_word_ending(edge_ptr, next_node_from_edge_rec(*edge_ptr),
                    marker_flag, unichar_id);
  } else {
    // Add a link to node 0. All leaves connect to node 0 so the back links can
    // be used in reduction to a dawg. This root backward node has one edge
    // entry for every word, (except prefixes of longer words) so it is huge.
    if (!add_failed &&
        !add_new_edge(last_node, the_next_node, marker_flag, true, unichar_id))
      add_failed = true;
  }
  if (add_failed) {
    tprintf("Re-initializing document dictionary...\n");
    clear();
    return false;
  } else {
    return true;
  }
}

NODE_REF Trie::new_dawg_node() {
  auto *node = new TRIE_NODE_RECORD();
  nodes_.push_back(node);
  return nodes_.length() - 1;
}

// Sort function to sort words by decreasing order of length.
static int sort_strings_by_dec_length(const void* v1, const void* v2) {
  const auto *s1 = static_cast<const STRING *>(v1);
  const auto *s2 = static_cast<const STRING *>(v2);
  return s2->length() - s1->length();
}

bool Trie::read_and_add_word_list(const char *filename,
                                  const UNICHARSET &unicharset,
                                  Trie::RTLReversePolicy reverse_policy) {
  GenericVector<STRING> word_list;
  if (!read_word_list(filename, &word_list)) return false;
  word_list.sort(sort_strings_by_dec_length);
  return add_word_list(word_list, unicharset, reverse_policy);
}

bool Trie::read_word_list(const char *filename,
                          GenericVector<STRING>* words) {
  FILE *word_file;
  char line_str[CHARS_PER_LINE];
  int  word_count = 0;

  word_file = fopen(filename, "rb");
  if (word_file == nullptr) return false;

  while (fgets(line_str, sizeof(line_str), word_file) != nullptr) {
    chomp_string(line_str);  // remove newline
    STRING word_str(line_str);
    ++word_count;
    if (debug_level_ && word_count % 10000 == 0)
      tprintf("Read %d words so far\n", word_count);
    words->push_back(word_str);
  }
  if (debug_level_)
    tprintf("Read %d words total.\n", word_count);
  fclose(word_file);
  return true;
}

bool Trie::add_word_list(const GenericVector<STRING> &words,
                         const UNICHARSET &unicharset,
                         Trie::RTLReversePolicy reverse_policy) {
  for (int i = 0; i < words.size(); ++i) {
    WERD_CHOICE word(words[i].string(), unicharset);
    if (word.length() == 0 || word.contains_unichar_id(INVALID_UNICHAR_ID))
      continue;
    if ((reverse_policy == RRP_REVERSE_IF_HAS_RTL &&
         word.has_rtl_unichar_id()) ||
        reverse_policy == RRP_FORCE_REVERSE) {
      word.reverse_and_mirror_unichar_ids();
    }
    if (!word_in_dawg(word)) {
      add_word_to_dawg(word);
      if (!word_in_dawg(word)) {
        tprintf("Error: word '%s' not in DAWG after adding it\n",
                words[i].string());
        return false;
      }
    }
  }
  return true;
}

void Trie::initialize_patterns(UNICHARSET *unicharset) {
  unicharset->unichar_insert(kAlphaPatternUnicode);
  alpha_pattern_ = unicharset->unichar_to_id(kAlphaPatternUnicode);
  unicharset->unichar_insert(kDigitPatternUnicode);
  digit_pattern_ = unicharset->unichar_to_id(kDigitPatternUnicode);
  unicharset->unichar_insert(kAlphanumPatternUnicode);
  alphanum_pattern_ = unicharset->unichar_to_id(kAlphanumPatternUnicode);
  unicharset->unichar_insert(kPuncPatternUnicode);
  punc_pattern_ = unicharset->unichar_to_id(kPuncPatternUnicode);
  unicharset->unichar_insert(kLowerPatternUnicode);
  lower_pattern_ = unicharset->unichar_to_id(kLowerPatternUnicode);
  unicharset->unichar_insert(kUpperPatternUnicode);
  upper_pattern_ = unicharset->unichar_to_id(kUpperPatternUnicode);
  initialized_patterns_ = true;
  unicharset_size_ = unicharset->size();
}

void Trie::unichar_id_to_patterns(UNICHAR_ID unichar_id,
                                  const UNICHARSET &unicharset,
                                  GenericVector<UNICHAR_ID> *vec) const {
  bool is_alpha = unicharset.get_isalpha(unichar_id);
  if (is_alpha) {
    vec->push_back(alpha_pattern_);
    vec->push_back(alphanum_pattern_);
    if (unicharset.get_islower(unichar_id)) {
      vec->push_back(lower_pattern_);
    } else if (unicharset.get_isupper(unichar_id)) {
      vec->push_back(upper_pattern_);
    }
  }
  if (unicharset.get_isdigit(unichar_id)) {
    vec->push_back(digit_pattern_);
    if (!is_alpha) vec->push_back(alphanum_pattern_);
  }
  if (unicharset.get_ispunctuation(unichar_id)) {
    vec->push_back(punc_pattern_);
  }
}

UNICHAR_ID Trie::character_class_to_pattern(char ch) {
  if (ch == 'c') {
    return alpha_pattern_;
  } else if (ch == 'd') {
    return digit_pattern_;
  } else if (ch == 'n') {
    return alphanum_pattern_;
  } else if (ch == 'p') {
    return punc_pattern_;
  } else if (ch == 'a') {
    return lower_pattern_;
  } else if (ch == 'A') {
    return upper_pattern_;
  } else {
    return INVALID_UNICHAR_ID;
  }
}

bool Trie::read_pattern_list(const char *filename,
                             const UNICHARSET &unicharset) {
  if (!initialized_patterns_) {
    tprintf("please call initialize_patterns() before read_pattern_list()\n");
    return false;
  }

  FILE *pattern_file = fopen(filename, "rb");
  if (pattern_file == nullptr) {
    tprintf("Error opening pattern file %s\n", filename);
    return false;
  }

  int pattern_count = 0;
  char string[CHARS_PER_LINE];
  while (fgets(string, CHARS_PER_LINE, pattern_file) != nullptr) {
    chomp_string(string);  // remove newline
    // Parse the pattern and construct a unichar id vector.
    // Record the number of repetitions of each unichar in the parallel vector.
    WERD_CHOICE word(&unicharset);
    GenericVector<bool> repetitions_vec;
    const char *str_ptr = string;
    int step = unicharset.step(str_ptr);
    bool failed = false;
    while (step > 0) {
      UNICHAR_ID curr_unichar_id = INVALID_UNICHAR_ID;
      if (step == 1 && *str_ptr == '\\') {
        ++str_ptr;
        if (*str_ptr == '\\') {  // regular '\' unichar that was escaped
          curr_unichar_id = unicharset.unichar_to_id(str_ptr, step);
        } else {
          if (word.length() < kSaneNumConcreteChars) {
            tprintf("Please provide at least %d concrete characters at the"
                    " beginning of the pattern\n", kSaneNumConcreteChars);
            failed = true;
            break;
          }
          // Parse character class from expression.
          curr_unichar_id = character_class_to_pattern(*str_ptr);
        }
      } else {
        curr_unichar_id = unicharset.unichar_to_id(str_ptr, step);
      }
      if (curr_unichar_id ==  INVALID_UNICHAR_ID) {
        failed = true;
        break;  // failed to parse this pattern
      }
      word.append_unichar_id(curr_unichar_id, 1, 0.0, 0.0);
      repetitions_vec.push_back(false);
      str_ptr += step;
      step = unicharset.step(str_ptr);
      // Check if there is a repetition pattern specified after this unichar.
      if (step == 1 && *str_ptr == '\\' && *(str_ptr+1) == '*') {
        repetitions_vec[repetitions_vec.size()-1] = true;
        str_ptr += 2;
        step = unicharset.step(str_ptr);
      }
    }
    if (failed) {
      tprintf("Invalid user pattern %s\n", string);
      continue;
    }
    // Insert the pattern into the trie.
    if (debug_level_ > 2) {
      tprintf("Inserting expanded user pattern %s\n",
              word.debug_string().string());
    }
    if (!this->word_in_dawg(word)) {
      this->add_word_to_dawg(word, &repetitions_vec);
      if (!this->word_in_dawg(word)) {
        tprintf("Error: failed to insert pattern '%s'\n", string);
      }
    }
    ++pattern_count;
  }
  if (debug_level_) {
    tprintf("Read %d valid patterns from %s\n", pattern_count, filename);
  }
  fclose(pattern_file);
  return true;
}

void Trie::remove_edge_linkage(NODE_REF node1, NODE_REF node2, int direction,
                               bool word_end, UNICHAR_ID unichar_id) {
  EDGE_RECORD *edge_ptr = nullptr;
  EDGE_INDEX edge_index = 0;
  ASSERT_HOST(edge_char_of(node1, node2, direction, word_end,
                           unichar_id, &edge_ptr, &edge_index));
  if (debug_level_ > 1) {
    tprintf("removed edge in nodes_[" REFFORMAT "]: ", node1);
    print_edge_rec(*edge_ptr);
    tprintf("\n");
  }
  if (direction == FORWARD_EDGE) {
    nodes_[node1]->forward_edges.remove(edge_index);
  } else if (node1 == 0) {
    KillEdge(&nodes_[node1]->backward_edges[edge_index]);
    root_back_freelist_.push_back(edge_index);
  } else {
    nodes_[node1]->backward_edges.remove(edge_index);
  }
  --num_edges_;
}

// Some optimizations employed in add_word_to_dawg and trie_to_dawg:
// 1 Avoid insertion sorting or bubble sorting the tail root node
//   (back links on node 0, a list of all the leaves.). The node is
//   huge, and sorting it with n^2 time is terrible.
// 2 Avoid using GenericVector::remove on the tail root node.
//   (a) During add of words to the trie, zero-out the unichars and
//       keep a freelist of spaces to re-use.
//   (b) During reduction, just zero-out the unichars of deleted back
//       links, skipping zero entries while searching.
// 3 Avoid linear search of the tail root node. This has to be done when
//   a suffix is added to an existing word. Adding words by decreasing
//   length avoids this problem entirely. Words can still be added in
//   any order, but it is faster to add the longest first.
SquishedDawg *Trie::trie_to_dawg() {
  root_back_freelist_.clear();  // Will be invalided by trie_to_dawg.
  if (debug_level_ > 2) {
    print_all("Before reduction:", MAX_NODE_EDGES_DISPLAY);
  }
  auto reduced_nodes = new bool[nodes_.size()];
  for (int i = 0; i < nodes_.size(); i++) reduced_nodes[i] = false;
  this->reduce_node_input(0, reduced_nodes);
  delete[] reduced_nodes;

  if (debug_level_ > 2) {
    print_all("After reduction:", MAX_NODE_EDGES_DISPLAY);
  }
  // Build a translation map from node indices in nodes_ vector to
  // their target indices in EDGE_ARRAY.
  auto *node_ref_map = new NODE_REF[nodes_.size() + 1];
  int i, j;
  node_ref_map[0] = 0;
  for (i = 0; i < nodes_.size(); ++i) {
    node_ref_map[i+1] = node_ref_map[i] + nodes_[i]->forward_edges.size();
  }
  int num_forward_edges = node_ref_map[i];

  // Convert nodes_ vector into EDGE_ARRAY translating the next node references
  // in edges using node_ref_map. Empty nodes and backward edges are dropped.
  auto edge_array = new EDGE_RECORD[num_forward_edges];
  EDGE_ARRAY edge_array_ptr = edge_array;
  for (i = 0; i < nodes_.size(); ++i) {
    TRIE_NODE_RECORD *node_ptr = nodes_[i];
    int end = node_ptr->forward_edges.size();
    for (j = 0; j < end; ++j) {
      EDGE_RECORD &edge_rec = node_ptr->forward_edges[j];
      NODE_REF node_ref = next_node_from_edge_rec(edge_rec);
      ASSERT_HOST(node_ref < nodes_.size());
      UNICHAR_ID unichar_id = unichar_id_from_edge_rec(edge_rec);
      link_edge(edge_array_ptr, node_ref_map[node_ref], false, FORWARD_EDGE,
                end_of_word_from_edge_rec(edge_rec), unichar_id);
      if (j == end - 1) set_marker_flag_in_edge_rec(edge_array_ptr);
      ++edge_array_ptr;
    }
  }
  delete[] node_ref_map;

  return new SquishedDawg(edge_array, num_forward_edges, type_, lang_,
                          perm_, unicharset_size_, debug_level_);
}

bool Trie::eliminate_redundant_edges(NODE_REF node,
                                     const EDGE_RECORD &edge1,
                                     const EDGE_RECORD &edge2) {
  if (debug_level_ > 1) {
    tprintf("\nCollapsing node %" PRIi64 ":\n", node);
    print_node(node, MAX_NODE_EDGES_DISPLAY);
    tprintf("Candidate edges: ");
    print_edge_rec(edge1);
    tprintf(", ");
    print_edge_rec(edge2);
    tprintf("\n\n");
  }
  NODE_REF next_node1 = next_node_from_edge_rec(edge1);
  NODE_REF next_node2 = next_node_from_edge_rec(edge2);
  TRIE_NODE_RECORD *next_node2_ptr = nodes_[next_node2];
  // Translate all edges going to/from next_node2 to go to/from next_node1.
  EDGE_RECORD *edge_ptr = nullptr;
  EDGE_INDEX edge_index;
  int i;
  // The backward link in node to next_node2 will be zeroed out by the caller.
  // Copy all the backward links in next_node2 to node next_node1
  for (i = 0; i < next_node2_ptr->backward_edges.size(); ++i) {
    const EDGE_RECORD &bkw_edge = next_node2_ptr->backward_edges[i];
    NODE_REF curr_next_node = next_node_from_edge_rec(bkw_edge);
    UNICHAR_ID curr_unichar_id = unichar_id_from_edge_rec(bkw_edge);
    int curr_word_end = end_of_word_from_edge_rec(bkw_edge);
    bool marker_flag = marker_flag_from_edge_rec(bkw_edge);
    add_edge_linkage(next_node1, curr_next_node, marker_flag, BACKWARD_EDGE,
                     curr_word_end, curr_unichar_id);
    // Relocate the corresponding forward edge in curr_next_node
    ASSERT_HOST(edge_char_of(curr_next_node, next_node2, FORWARD_EDGE,
                             curr_word_end, curr_unichar_id,
                             &edge_ptr, &edge_index));
    set_next_node_in_edge_rec(edge_ptr, next_node1);
  }
  int next_node2_num_edges = (next_node2_ptr->forward_edges.size() +
                              next_node2_ptr->backward_edges.size());
  if (debug_level_ > 1) {
    tprintf("removed %d edges from node " REFFORMAT "\n",
            next_node2_num_edges, next_node2);
  }
  next_node2_ptr->forward_edges.clear();
  next_node2_ptr->backward_edges.clear();
  num_edges_ -= next_node2_num_edges;
  return true;
}

bool Trie::reduce_lettered_edges(EDGE_INDEX edge_index,
                                 UNICHAR_ID unichar_id,
                                 NODE_REF node,
                                 EDGE_VECTOR* backward_edges,
                                 NODE_MARKER reduced_nodes) {
  if (debug_level_ > 1)
    tprintf("reduce_lettered_edges(edge=" REFFORMAT ")\n", edge_index);
  // Compare each of the edge pairs with the given unichar_id.
  bool did_something = false;
  for (int i = edge_index; i < backward_edges->size() - 1; ++i) {
    // Find the first edge that can be eliminated.
    UNICHAR_ID curr_unichar_id = INVALID_UNICHAR_ID;
    while (i < backward_edges->size()) {
      if (!DeadEdge((*backward_edges)[i])) {
        curr_unichar_id = unichar_id_from_edge_rec((*backward_edges)[i]);
        if (curr_unichar_id != unichar_id) return did_something;
        if (can_be_eliminated((*backward_edges)[i])) break;
      }
      ++i;
    }
    if (i == backward_edges->size()) break;
    const EDGE_RECORD &edge_rec = (*backward_edges)[i];
    // Compare it to the rest of the edges with the given unichar_id.
    for (int j = i + 1; j < backward_edges->size(); ++j) {
      const EDGE_RECORD &next_edge_rec = (*backward_edges)[j];
      if (DeadEdge(next_edge_rec)) continue;
      UNICHAR_ID next_id = unichar_id_from_edge_rec(next_edge_rec);
      if (next_id != unichar_id) break;
      if (end_of_word_from_edge_rec(next_edge_rec) ==
          end_of_word_from_edge_rec(edge_rec) &&
          can_be_eliminated(next_edge_rec) &&
          eliminate_redundant_edges(node, edge_rec, next_edge_rec)) {
        reduced_nodes[next_node_from_edge_rec(edge_rec)] = false;
        did_something = true;
        KillEdge(&(*backward_edges)[j]);
      }
    }
  }
  return did_something;
}

void Trie::sort_edges(EDGE_VECTOR *edges) {
  int num_edges = edges->size();
  if (num_edges <= 1) return;
  GenericVector<KDPairInc<UNICHAR_ID, EDGE_RECORD> > sort_vec;
  sort_vec.reserve(num_edges);
  for (int i = 0; i < num_edges; ++i) {
    sort_vec.push_back(KDPairInc<UNICHAR_ID, EDGE_RECORD>(
        unichar_id_from_edge_rec((*edges)[i]), (*edges)[i]));
  }
  sort_vec.sort();
  for (int i = 0; i < num_edges; ++i)
    (*edges)[i] = sort_vec[i].data;
}

void Trie::reduce_node_input(NODE_REF node,
                             NODE_MARKER reduced_nodes) {
  EDGE_VECTOR &backward_edges = nodes_[node]->backward_edges;
  sort_edges(&backward_edges);
  if (debug_level_ > 1) {
    tprintf("reduce_node_input(node=" REFFORMAT ")\n", node);
    print_node(node, MAX_NODE_EDGES_DISPLAY);
  }

  EDGE_INDEX edge_index = 0;
  while (edge_index < backward_edges.size()) {
    if (DeadEdge(backward_edges[edge_index])) continue;
    UNICHAR_ID unichar_id =
      unichar_id_from_edge_rec(backward_edges[edge_index]);
    while (reduce_lettered_edges(edge_index, unichar_id, node,
                                 &backward_edges, reduced_nodes));
    while (++edge_index < backward_edges.size()) {
      UNICHAR_ID id = unichar_id_from_edge_rec(backward_edges[edge_index]);
      if (!DeadEdge(backward_edges[edge_index]) && id != unichar_id) break;
    }
  }
  reduced_nodes[node] = true;  // mark as reduced

  if (debug_level_ > 1) {
    tprintf("Node " REFFORMAT " after reduction:\n", node);
    print_node(node, MAX_NODE_EDGES_DISPLAY);
  }

  for (int i = 0; i < backward_edges.size(); ++i) {
    if (DeadEdge(backward_edges[i])) continue;
    NODE_REF next_node = next_node_from_edge_rec(backward_edges[i]);
    if (next_node != 0 && !reduced_nodes[next_node]) {
      reduce_node_input(next_node, reduced_nodes);
    }
  }
}

void Trie::print_node(NODE_REF node, int max_num_edges) const {
  if (node == NO_EDGE) return;  // nothing to print
  TRIE_NODE_RECORD *node_ptr = nodes_[node];
  int num_fwd = node_ptr->forward_edges.size();
  int num_bkw = node_ptr->backward_edges.size();
  EDGE_VECTOR *vec;
  for (int dir = 0; dir < 2; ++dir) {
    if (dir == 0) {
      vec = &(node_ptr->forward_edges);
      tprintf(REFFORMAT " (%d %d): ", node, num_fwd, num_bkw);
    } else {
      vec = &(node_ptr->backward_edges);
      tprintf("\t");
    }
    int i;
    for (i = 0; (dir == 0 ? i < num_fwd : i < num_bkw) &&
         i < max_num_edges; ++i) {
      if (DeadEdge((*vec)[i])) continue;
      print_edge_rec((*vec)[i]);
      tprintf(" ");
    }
    if (dir == 0 ? i < num_fwd : i < num_bkw) tprintf("...");
    tprintf("\n");
  }
}

}  // namespace tesseract
