/* -*-C-*-
 ********************************************************************************
 *
 * File:        dawg.c  (Formerly dawg.c)
 * Description:  Use a Directed Accyclic Word Graph
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

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#pragma warning(disable:4800)  // int/bool warnings
#endif
#include "dawg.h"

#include "cutil.h"
#include "dict.h"
#include "emalloc.h"
#include "freelist.h"
#include "helpers.h"
#include "strngs.h"
#include "tesscallback.h"
#include "tprintf.h"

/*----------------------------------------------------------------------
              F u n c t i o n s   f o r   D a w g
----------------------------------------------------------------------*/
namespace tesseract {

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
  if (filename == NULL) return 0;

  FILE       *word_file;
  char       string [CHARS_PER_LINE];
  int misses = 0;
  UNICHAR_ID wildcard = unicharset.unichar_to_id(kWildcard);

  word_file = open_file (filename, "r");

  while (fgets (string, CHARS_PER_LINE, word_file) != NULL) {
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

void CallWithUTF8(TessCallback1<const char *> *cb, const WERD_CHOICE *wc) {
  STRING s;
  wc->string_and_lengths(&s, NULL);
  cb->Run(s.string());
}

void Dawg::iterate_words(const UNICHARSET &unicharset,
                         TessCallback1<const char *> *cb) const {
  TessCallback1<const WERD_CHOICE *> *shim =
      NewPermanentTessCallback(CallWithUTF8, cb);
  WERD_CHOICE word(&unicharset);
  iterate_words_rec(word, 0, shim);
  delete shim;
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

bool Dawg::match_words(WERD_CHOICE *word, inT32 index,
                       NODE_REF node, UNICHAR_ID wildcard) const {
  EDGE_REF edge;
  inT32 word_end;

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

void Dawg::init(DawgType type, const STRING &lang,
                PermuterType perm, int unicharset_size, int debug_level) {
  type_ = type;
  lang_ = lang;
  perm_ = perm;
  ASSERT_HOST(unicharset_size > 0);
  unicharset_size_ = unicharset_size;
  // Set bit masks. We will use the value unicharset_size_ as a null char, so
  // the actual number of unichars is unicharset_size_ + 1.
  flag_start_bit_ = ceil(log(unicharset_size_ + 1.0) / log(2.0));
  next_node_start_bit_ = flag_start_bit_ + NUM_FLAG_BITS;
  letter_mask_ = ~(~0ull << flag_start_bit_);
  next_node_mask_ = ~0ull << (flag_start_bit_ + NUM_FLAG_BITS);
  flags_mask_ = ~(letter_mask_ | next_node_mask_);

  debug_level_ = debug_level;
}


/*----------------------------------------------------------------------
         F u n c t i o n s   f o r   S q u i s h e d    D a w g
----------------------------------------------------------------------*/

SquishedDawg::~SquishedDawg() { memfree(edges_); }

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

inT32 SquishedDawg::num_forward_edges(NODE_REF node) const {
  EDGE_REF   edge = node;
  inT32        num  = 0;

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

void SquishedDawg::read_squished_dawg(FILE *file,
                                      DawgType type,
                                      const STRING &lang,
                                      PermuterType perm,
                                      int debug_level) {
  if (debug_level) tprintf("Reading squished dawg\n");

  // Read the magic number and if it does not match kDawgMagicNumber
  // set swap to true to indicate that we need to switch endianness.
  inT16 magic;
  fread(&magic, sizeof(inT16), 1, file);
  bool swap = (magic != kDawgMagicNumber);

  int unicharset_size;
  fread(&unicharset_size, sizeof(inT32), 1, file);
  fread(&num_edges_, sizeof(inT32), 1, file);

  if (swap) {
    ReverseN(&unicharset_size, sizeof(unicharset_size));
    ReverseN(&num_edges_, sizeof(num_edges_));
  }
  ASSERT_HOST(num_edges_ > 0);  // DAWG should not be empty
  Dawg::init(type, lang, perm, unicharset_size, debug_level);

  edges_ = (EDGE_ARRAY) memalloc(sizeof(EDGE_RECORD) * num_edges_);
  fread(&edges_[0], sizeof(EDGE_RECORD), num_edges_, file);
  EDGE_REF edge;
  if (swap) {
    for (edge = 0; edge < num_edges_; ++edge) {
      ReverseN(&edges_[edge], sizeof(edges_[edge]));
    }
  }
  if (debug_level > 2) {
    tprintf("type: %d lang: %s perm: %d unicharset_size: %d num_edges: %d\n",
            type_, lang_.string(), perm_, unicharset_size_, num_edges_);
    for (edge = 0; edge < num_edges_; ++edge)
      print_edge(edge);
  }
}

NODE_MAP SquishedDawg::build_node_map(inT32 *num_nodes) const {
  EDGE_REF   edge;
  NODE_MAP   node_map;
  inT32       node_counter;
  inT32       num_edges;

  node_map = (NODE_MAP) malloc(sizeof(EDGE_REF) * num_edges_);

  for (edge = 0; edge < num_edges_; edge++)       // init all slots
    node_map [edge] = -1;

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
  return (node_map);
}

void SquishedDawg::write_squished_dawg(FILE *file) {
  EDGE_REF    edge;
  inT32       num_edges;
  inT32       node_count = 0;
  NODE_MAP    node_map;
  EDGE_REF    old_index;
  EDGE_RECORD temp_record;

  if (debug_level_) tprintf("write_squished_dawg\n");

  node_map = build_node_map(&node_count);

  // Write the magic number to help detecting a change in endianness.
  inT16 magic = kDawgMagicNumber;
  fwrite(&magic, sizeof(inT16), 1, file);
  fwrite(&unicharset_size_, sizeof(inT32), 1, file);

  // Count the number of edges in this Dawg.
  num_edges = 0;
  for (edge=0; edge < num_edges_; edge++)
    if (forward_edge(edge))
      num_edges++;

  fwrite(&num_edges, sizeof(inT32), 1, file);  // write edge count to file

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
        fwrite(&(temp_record), sizeof(EDGE_RECORD), 1, file);
        set_next_node(edge, old_index);
      } while (!last_edge(edge++));

      if (edge >= num_edges_) break;
      if (backward_edge(edge))  // skip back links
        while (!last_edge(edge++));

      edge--;
    }
  }
  free(node_map);
}

}  // namespace tesseract
