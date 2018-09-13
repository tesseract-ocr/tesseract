/* -*-C-*-
 ********************************************************************************
 *
 * File:         dawg.cpp  (Formerly dawg.c)
 * Description:  Use a Directed Acyclic Word Graph
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 24 16:59:16 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
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

#include "dawg.h"

#include "dict.h"
#include "emalloc.h"
#include "helpers.h"
#include "strngs.h"
#include "tesscallback.h"
#include "tprintf.h"

#include <memory>

/*----------------------------------------------------------------------
              F u n c t i o n s   f o r   D a w g
----------------------------------------------------------------------*/
namespace tesseract {

// Destructor.
// It is defined here, so the compiler can create a single vtable
// instead of weak vtables in every compilation unit.
Dawg::~Dawg() = default;

bool Dawg::prefix_in_dawg(const WERD_CHOICE &word,
                          bool requires_complete) const {
  if (word.length() == 0) return !requires_complete;
  NODE_REF node = 0;
  int end_index = word.length() - 1;
  for (int i = 0; i < end_index; i++) {
    EDGE_REF edge = edge_char_of(node, word.unichar_id(i), false);
    if (edge == NO_EDGE) {
      return false;
    }
    if ((node = next_node(edge)) == 0) {
      // This only happens if all words following this edge terminate --
      // there are no larger words.  See Trie::add_word_to_dawg()
      return false;
    }
  }
  // Now check the last character.
  return edge_char_of(node, word.unichar_id(end_index), requires_complete) !=
      NO_EDGE;
}

bool Dawg::word_in_dawg(const WERD_CHOICE &word) const {
  return prefix_in_dawg(word, true);
}

int Dawg::check_for_words(const char *filename,
                          const UNICHARSET &unicharset,
                          bool enable_wildcard) const {
  if (filename == nullptr) return 0;

  FILE       *word_file;
  char       string [CHARS_PER_LINE];
  int misses = 0;
  UNICHAR_ID wildcard = unicharset.unichar_to_id(kWildcard);

  word_file = fopen(filename, "r");
  if (word_file == nullptr) {
    tprintf("Error: Could not open file %s\n", filename);
    ASSERT_HOST(word_file);
  }

  while (fgets (string, CHARS_PER_LINE, word_file) != nullptr) {
    chomp_string(string);  // remove newline
    WERD_CHOICE word(string, unicharset);
    if (word.length() > 0 &&
        !word.contains_unichar_id(INVALID_UNICHAR_ID)) {
      if (!match_words(&word, 0, 0,
                       enable_wildcard ? wildcard : INVALID_UNICHAR_ID)) {
        tprintf("Missing word: %s\n", string);
        ++misses;
      }
    } else {
      tprintf("Failed to create a valid word from %s\n", string);
    }
  }
  fclose (word_file);
  // Make sure the user sees this with fprintf instead of tprintf.
  if (debug_level_) tprintf("Number of lost words=%d\n", misses);
  return misses;
}

void Dawg::iterate_words(const UNICHARSET &unicharset,
                         TessCallback1<const WERD_CHOICE *> *cb) const {
  WERD_CHOICE word(&unicharset);
  iterate_words_rec(word, 0, cb);
}

static void CallWithUTF8(TessCallback1<const char *> *cb,
                         const WERD_CHOICE *wc) {
  STRING s;
  wc->string_and_lengths(&s, nullptr);
  cb->Run(s.string());
}

void Dawg::iterate_words(const UNICHARSET &unicharset,
                         TessCallback1<const char *> *cb) const {
  std::unique_ptr<TessCallback1<const WERD_CHOICE *>> shim(
      NewPermanentTessCallback(CallWithUTF8, cb));
  WERD_CHOICE word(&unicharset);
  iterate_words_rec(word, 0, shim.get());
}

void Dawg::iterate_words_rec(const WERD_CHOICE &word_so_far,
                             NODE_REF to_explore,
                             TessCallback1<const WERD_CHOICE *> *cb) const {
  NodeChildVector children;
  this->unichar_ids_of(to_explore, &children, false);
  for (int i = 0; i < children.size(); i++) {
    WERD_CHOICE next_word(word_so_far);
    next_word.append_unichar_id(children[i].unichar_id, 1, 0.0, 0.0);
    if (this->end_of_word(children[i].edge_ref)) {
      cb->Run(&next_word);
    }
    NODE_REF next = next_node(children[i].edge_ref);
    if (next != 0) {
      iterate_words_rec(next_word, next, cb);
    }
  }
}

bool Dawg::match_words(WERD_CHOICE *word, int32_t index,
                       NODE_REF node, UNICHAR_ID wildcard) const {
  EDGE_REF edge;
  int32_t word_end;

  if (wildcard != INVALID_UNICHAR_ID && word->unichar_id(index) == wildcard) {
    bool any_matched = false;
    NodeChildVector vec;
    this->unichar_ids_of(node, &vec, false);
    for (int i = 0; i < vec.size(); ++i) {
      word->set_unichar_id(vec[i].unichar_id, index);
      if (match_words(word, index, node, wildcard))
        any_matched = true;
    }
    word->set_unichar_id(wildcard, index);
    return any_matched;
  } else {
    word_end = index == word->length() - 1;
    edge = edge_char_of(node, word->unichar_id(index), word_end);
    if (edge != NO_EDGE) {  // normal edge in DAWG
      node = next_node(edge);
      if (word_end) {
        if (debug_level_ > 1) word->print("match_words() found: ");
        return true;
      } else if (node != 0) {
        return match_words(word, index+1, node, wildcard);
      }
    }
  }
  return false;
}

void Dawg::init(int unicharset_size) {
  ASSERT_HOST(unicharset_size > 0);
  unicharset_size_ = unicharset_size;
  // Set bit masks. We will use the value unicharset_size_ as a null char, so
  // the actual number of unichars is unicharset_size_ + 1.
  flag_start_bit_ = ceil(log(unicharset_size_ + 1.0) / log(2.0));
  next_node_start_bit_ = flag_start_bit_ + NUM_FLAG_BITS;
  letter_mask_ = ~(~0ull << flag_start_bit_);
  next_node_mask_ = ~0ull << (flag_start_bit_ + NUM_FLAG_BITS);
  flags_mask_ = ~(letter_mask_ | next_node_mask_);
}


/*----------------------------------------------------------------------
         F u n c t i o n s   f o r   S q u i s h e d    D a w g
----------------------------------------------------------------------*/

SquishedDawg::~SquishedDawg() { delete[] edges_; }

EDGE_REF SquishedDawg::edge_char_of(NODE_REF node,
                                    UNICHAR_ID unichar_id,
                                    bool word_end) const {
  EDGE_REF edge = node;
  if (node == 0) {  // binary search
    EDGE_REF start = 0;
    EDGE_REF end = num_forward_edges_in_node0 - 1;
    int compare;
    while (start <= end) {
      edge = (start + end) >> 1;  // (start + end) / 2
      compare = given_greater_than_edge_rec(NO_EDGE, word_end,
                                            unichar_id, edges_[edge]);
      if (compare == 0) {  // given == vec[k]
        return edge;
      } else if (compare == 1) {  // given > vec[k]
        start = edge + 1;
      } else {  // given < vec[k]
        end = edge - 1;
      }
    }
  } else {  // linear search
    if (edge != NO_EDGE && edge_occupied(edge)) {
      do {
        if ((unichar_id_from_edge_rec(edges_[edge]) == unichar_id) &&
            (!word_end || end_of_word_from_edge_rec(edges_[edge])))
          return (edge);
      } while (!last_edge(edge++));
    }
  }
  return (NO_EDGE);  // not found
}

int32_t SquishedDawg::num_forward_edges(NODE_REF node) const {
  EDGE_REF   edge = node;
  int32_t        num  = 0;

  if (forward_edge (edge)) {
    do {
      num++;
    } while (!last_edge(edge++));
  }

  return (num);
}

void SquishedDawg::print_node(NODE_REF node, int max_num_edges) const {
  if (node == NO_EDGE) return;  // nothing to print

  EDGE_REF   edge = node;
  const char       *forward_string  = "FORWARD";
  const char       *backward_string = "       ";

  const char       *last_string     = "LAST";
  const char       *not_last_string = "    ";

  const char       *eow_string      = "EOW";
  const char       *not_eow_string  = "   ";

  const char       *direction;
  const char       *is_last;
  const char       *eow;

  UNICHAR_ID unichar_id;

  if (edge_occupied(edge)) {
    do {
      direction =
        forward_edge(edge) ? forward_string : backward_string;
      is_last = last_edge(edge) ? last_string : not_last_string;
      eow = end_of_word(edge) ? eow_string : not_eow_string;

      unichar_id = edge_letter(edge);
      tprintf(REFFORMAT " : next = " REFFORMAT ", unichar_id = %d, %s %s %s\n",
              edge, next_node(edge), unichar_id,
              direction, is_last, eow);

      if (edge - node > max_num_edges) return;
    } while (!last_edge(edge++));

    if (edge < num_edges_ &&
        edge_occupied(edge) && backward_edge(edge)) {
      do {
        direction =
          forward_edge(edge) ? forward_string : backward_string;
        is_last = last_edge(edge) ? last_string : not_last_string;
        eow = end_of_word(edge) ? eow_string : not_eow_string;

        unichar_id = edge_letter(edge);
        tprintf(REFFORMAT " : next = " REFFORMAT
                ", unichar_id = %d, %s %s %s\n",
                edge, next_node(edge), unichar_id,
                direction, is_last, eow);

        if (edge - node > MAX_NODE_EDGES_DISPLAY) return;
      } while (!last_edge(edge++));
    }
  }
  else {
    tprintf(REFFORMAT " : no edges in this node\n", node);
  }
  tprintf("\n");
}

void SquishedDawg::print_edge(EDGE_REF edge) const {
  if (edge == NO_EDGE) {
    tprintf("NO_EDGE\n");
  } else {
    tprintf(REFFORMAT " : next = " REFFORMAT
            ", unichar_id = '%d', %s %s %s\n", edge,
            next_node(edge), edge_letter(edge),
            (forward_edge(edge) ? "FORWARD" : "       "),
            (last_edge(edge) ? "LAST"    : "    "),
            (end_of_word(edge) ? "EOW"     : ""));
  }
}

bool SquishedDawg::read_squished_dawg(TFile *file) {
  if (debug_level_) tprintf("Reading squished dawg\n");

  // Read the magic number and check that it matches kDawgMagicNumber, as
  // auto-endian fixing should make sure it is always correct.
  int16_t magic;
  if (!file->DeSerialize(&magic)) return false;
  if (magic != kDawgMagicNumber) {
    tprintf("Bad magic number on dawg: %d vs %d\n", magic, kDawgMagicNumber);
    return false;
  }

  int32_t unicharset_size;
  if (!file->DeSerialize(&unicharset_size)) return false;
  if (!file->DeSerialize(&num_edges_)) return false;
  ASSERT_HOST(num_edges_ > 0);  // DAWG should not be empty
  Dawg::init(unicharset_size);

  edges_ = new EDGE_RECORD[num_edges_];
  if (!file->DeSerialize(&edges_[0], num_edges_)) return false;
  if (debug_level_ > 2) {
    tprintf("type: %d lang: %s perm: %d unicharset_size: %d num_edges: %d\n",
            type_, lang_.string(), perm_, unicharset_size_, num_edges_);
    for (EDGE_REF edge = 0; edge < num_edges_; ++edge) print_edge(edge);
  }
  return true;
}

std::unique_ptr<EDGE_REF[]> SquishedDawg::build_node_map(
    int32_t *num_nodes) const {
  EDGE_REF   edge;
  std::unique_ptr<EDGE_REF[]> node_map(new EDGE_REF[num_edges_]);
  int32_t    node_counter;
  int32_t    num_edges;

  for (edge = 0; edge < num_edges_; edge++)       // init all slots
    node_map[edge] = -1;

  node_counter = num_forward_edges(0);

  *num_nodes   = 0;
  for (edge = 0; edge < num_edges_; edge++) {     // search all slots

    if (forward_edge(edge)) {
      (*num_nodes)++;                          // count nodes links
      node_map[edge] = (edge ? node_counter : 0);
      num_edges = num_forward_edges(edge);
      if (edge != 0) node_counter += num_edges;
      edge += num_edges;
      if (edge >= num_edges_) break;
      if (backward_edge(edge)) while (!last_edge(edge++));
      edge--;
    }
  }
  return node_map;
}

bool SquishedDawg::write_squished_dawg(TFile *file) {
  EDGE_REF    edge;
  int32_t     num_edges;
  int32_t     node_count = 0;
  EDGE_REF    old_index;
  EDGE_RECORD temp_record;

  if (debug_level_) tprintf("write_squished_dawg\n");

  std::unique_ptr<EDGE_REF[]> node_map(build_node_map(&node_count));

  // Write the magic number to help detecting a change in endianness.
  int16_t magic = kDawgMagicNumber;
  if (!file->Serialize(&magic)) return false;
  if (!file->Serialize(&unicharset_size_)) return false;

  // Count the number of edges in this Dawg.
  num_edges = 0;
  for (edge=0; edge < num_edges_; edge++)
    if (forward_edge(edge))
      num_edges++;

  // Write edge count to file.
  if (!file->Serialize(&num_edges)) return false;

  if (debug_level_) {
    tprintf("%d nodes in DAWG\n", node_count);
    tprintf("%d edges in DAWG\n", num_edges);
  }

  for (edge = 0; edge < num_edges_; edge++) {
    if (forward_edge(edge)) {  // write forward edges
      do {
        old_index = next_node_from_edge_rec(edges_[edge]);
        set_next_node(edge, node_map[old_index]);
        temp_record = edges_[edge];
        if (!file->Serialize(&temp_record)) return false;
        set_next_node(edge, old_index);
      } while (!last_edge(edge++));

      if (edge >= num_edges_) break;
      if (backward_edge(edge))  // skip back links
        while (!last_edge(edge++));

      edge--;
    }
  }
  return true;
}

}  // namespace tesseract
