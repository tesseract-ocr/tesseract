/* -*-C-*-
 ********************************************************************************
 *
 * File:        trie.c  (Formerly trie.c)
 * Description:  Functions to build a trie data structure.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Jul 26 12:18:10 1991 (Mark Seaman) marks@hpgrlt
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
#include "trie.h"

#include "callcpp.h"
#include "dawg.h"
#include "dict.h"
#include "freelist.h"
#include "helpers.h"

namespace tesseract {

bool Trie::edge_char_of(NODE_REF node_ref, NODE_REF next_node,
                        int direction, bool word_end, UNICHAR_ID unichar_id,
                        EDGE_RECORD **edge_ptr, EDGE_INDEX *edge_index) const {
  if (dawg_debug_level == 3) {
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
  if (node_ref == 0) { // binary search
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

bool Trie::add_edge_linkage(NODE_REF node1, NODE_REF node2, int direction,
                            bool word_end, UNICHAR_ID unichar_id) {
  if (num_edges_ == max_num_edges_) return false;
  EDGE_VECTOR *vec = (direction == FORWARD_EDGE) ?
    &(nodes_[node1]->forward_edges) : &(nodes_[node1]->backward_edges);
  int search_index;
  if (node1 == 0) {
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
  link_edge(&edge_rec, node2, direction, word_end, unichar_id);
  if (search_index < vec->size()) {
    vec->insert(edge_rec, search_index);
  } else {
    vec->push_back(edge_rec);
  }
  if (dawg_debug_level > 1) {
    tprintf("new edge in nodes_[" REFFORMAT "]: ", node1);
    print_edge_rec(edge_rec);
    tprintf("\n");
  }
  num_edges_++;
  return true;
}

void Trie::add_word_ending(EDGE_RECORD *edge_ptr,
                           NODE_REF the_next_node,
                           UNICHAR_ID unichar_id) {
  EDGE_RECORD *back_edge_ptr;
  EDGE_INDEX back_edge_index;
  ASSERT_HOST(edge_char_of(the_next_node, NO_EDGE, BACKWARD_EDGE, false,
                           unichar_id, &back_edge_ptr, &back_edge_index));
  // Mark both directions as end of word.
  *back_edge_ptr |= (WERD_END_FLAG << flag_start_bit_);
  *edge_ptr |= (WERD_END_FLAG << flag_start_bit_);
}

void Trie::add_word_to_dawg(const WERD_CHOICE &word) {
  if (word.length() <= 0) return;  // can't add empty words

  EDGE_RECORD *edge_ptr;
  NODE_REF last_node = 0;
  NODE_REF the_next_node;
  EDGE_INDEX edge_index;
  int i;
  inT32 still_finding_chars = true;
  inT32 word_end = false;
  bool  add_failed = false;
  bool found;

  if (dawg_debug_level > 1) word.print("\nAdding word: ");

  UNICHAR_ID unichar_id;
  for (i = 0; i < word.length() - 1; ++i) {
    unichar_id = word.unichar_id(i);
    if (dawg_debug_level > 1) tprintf("Adding letter %d\n", unichar_id);
    if (still_finding_chars) {
      found = edge_char_of(last_node, NO_EDGE, FORWARD_EDGE, word_end,
                           unichar_id, &edge_ptr, &edge_index);
      if (found && dawg_debug_level > 1) {
        tprintf("exploring edge " REFFORMAT " in node " REFFORMAT "\n",
                edge_index, last_node);
      }
      if (!found) {
        still_finding_chars = false;
      } else if (next_node_from_edge_rec(*edge_ptr) == 0) {
        word_end = true;
        still_finding_chars = false;
        remove_edge(last_node, 0, word_end, unichar_id);
      } else {
        last_node = next_node_from_edge_rec(*edge_ptr);
      }
    }
    if (!still_finding_chars) {
      the_next_node = new_dawg_node();
      if (dawg_debug_level > 1)
        tprintf("adding node " REFFORMAT "\n", the_next_node);
      if (the_next_node == 0) {
        add_failed = true;
        break;
      }
      if (!add_new_edge(last_node, the_next_node, word_end, unichar_id)) {
        add_failed = true;
        break;
      }
      word_end = false;
      last_node = the_next_node;
    }
  }
  the_next_node = 0;
  unichar_id = word.unichar_id(i);
  if (dawg_debug_level > 1) tprintf("Adding letter %d\n", unichar_id);
  if (still_finding_chars &&
      edge_char_of(last_node, NO_EDGE, FORWARD_EDGE, false,
                   unichar_id, &edge_ptr, &edge_index)) {
    // An extension of this word already exists in the trie, so we
    // only have to add the ending flags in both directions.
    add_word_ending(edge_ptr, next_node_from_edge_rec(*edge_ptr), unichar_id);
  } else {
    if (!add_failed &&
        !add_new_edge(last_node, the_next_node, true, unichar_id))
      add_failed = true;
  }
  if (add_failed) {
    tprintf("Re-initializing document dictionary...\n");
    nodes_.delete_data_pointers();
    num_edges_ = 0;
    new_dawg_node();  // need to allocate node 0
  }
}

NODE_REF Trie::new_dawg_node() {
  TRIE_NODE_RECORD *node = new TRIE_NODE_RECORD();
  if (node == NULL) return 0;  // failed to create new node
  nodes_.push_back(node);
  return nodes_.length() - 1;
}

bool Trie::read_word_list(const char *filename,
                          const UNICHARSET &unicharset) {
  FILE *word_file;
  char string [CHARS_PER_LINE];
  int  word_count = 0;

  word_file = open_file (filename, "r");

  while (fgets(string, CHARS_PER_LINE, word_file) != NULL) {
    chomp_string(string);  // remove newline
    WERD_CHOICE word(string, unicharset);
    ++word_count;
    if (dawg_debug_level && word_count % 10000 == 0)
      tprintf("Read %d words so far\n", word_count);
    if (word.length() != 0 && !word.contains_unichar_id(INVALID_UNICHAR_ID)) {
      if (!this->word_in_dawg(word)) {
        this->add_word_to_dawg(word);
        if (!this->word_in_dawg(word)) {
          tprintf("Error: word '%s' not in DAWG after adding it\n", string);
          return false;
        }
      }
    } else if (dawg_debug_level) {
      tprintf("Skipping invalid word %s\n", string);
      if (dawg_debug_level >= 3) word.print();
    }
  }
  if (dawg_debug_level)
    tprintf("Read %d words total.\n", word_count);
  fclose(word_file);
  return true;
}

void Trie::remove_edge_linkage(NODE_REF node1, NODE_REF node2, int direction,
                               bool word_end, UNICHAR_ID unichar_id) {
  EDGE_RECORD *edge_ptr;
  EDGE_INDEX edge_index;
  ASSERT_HOST(edge_char_of(node1, node2, direction, word_end,
                           unichar_id, &edge_ptr, &edge_index));
  if (dawg_debug_level > 1) {
    tprintf("removed edge in nodes_[" REFFORMAT "]: ", node1);
    print_edge_rec(*edge_ptr);
    tprintf("\n");
  }
  if (direction == FORWARD_EDGE) {
    nodes_[node1]->forward_edges.remove(edge_index);
  } else {
    nodes_[node1]->backward_edges.remove(edge_index);
  }
  --num_edges_;
}

SquishedDawg *Trie::trie_to_dawg() {
  if (dawg_debug_level > 2) {
    print_all("Before reduction:", MAX_NODE_EDGES_DISPLAY);
  }
  NODE_MARKER reduced_nodes = new bool[nodes_.size()];
  for (int i = 0; i < nodes_.size(); i++) reduced_nodes[i] = 0;
  this->reduce_node_input(0, reduced_nodes);
  delete[] reduced_nodes;

  if (dawg_debug_level > 2) {
    print_all("After reduction:", MAX_NODE_EDGES_DISPLAY);
  }
  // Build a translation map from node indices in nodes_ vector to
  // their target indices in EDGE_ARRAY.
  NODE_REF *node_ref_map = new NODE_REF[nodes_.size() + 1];
  int i, j;
  node_ref_map[0] = 0;
  for (i = 0; i < nodes_.size(); ++i) {
    node_ref_map[i+1] = node_ref_map[i] + nodes_[i]->forward_edges.size();
  }
  int num_forward_edges = node_ref_map[i];

  // Convert nodes_ vector into EDGE_ARRAY translating the next node references
  // in edges using node_ref_map. Empty nodes and backward edges are dropped.
  EDGE_ARRAY edge_array =
    (EDGE_ARRAY)memalloc(num_forward_edges * sizeof(EDGE_RECORD));
  EDGE_ARRAY edge_array_ptr = edge_array;
  for (i = 0; i < nodes_.size(); ++i) {
    TRIE_NODE_RECORD *node_ptr = nodes_[i];
    int end = node_ptr->forward_edges.size();
    for (j = 0; j < end; ++j) {
      EDGE_RECORD &edge_rec = node_ptr->forward_edges[j];
      NODE_REF node_ref = next_node_from_edge_rec(edge_rec);
      ASSERT_HOST(node_ref < nodes_.size());
      UNICHAR_ID unichar_id = unichar_id_from_edge_rec(edge_rec);
      link_edge(edge_array_ptr, node_ref_map[node_ref], FORWARD_EDGE,
                end_of_word_from_edge_rec(edge_rec), unichar_id);
      if (j == end - 1) set_last_flag_in_edge_rec(edge_array_ptr);
      ++edge_array_ptr;
    }
  }
  delete[] node_ref_map;

  return new SquishedDawg(edge_array, num_forward_edges,
                          type_, lang_, perm_, unicharset_size_);
}

bool Trie::eliminate_redundant_edges(NODE_REF node,
                                     const EDGE_RECORD &edge1,
                                     const EDGE_RECORD &edge2) {
  if (dawg_debug_level > 1) {
    tprintf("\nCollapsing node %d:\n", node);
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
  EDGE_RECORD *edge_ptr;
  EDGE_INDEX edge_index;
  int i;
  // Remove the backward link in node to next_node2.
  const EDGE_RECORD &fwd_edge = next_node2_ptr->forward_edges[0];
  remove_edge_linkage(node, next_node2, BACKWARD_EDGE,
                      end_of_word_from_edge_rec(fwd_edge),
                      unichar_id_from_edge_rec(fwd_edge));
  // Copy all the backward links in next_node2 to node next_node1
  for (i = 0; i < next_node2_ptr->backward_edges.size(); ++i) {
    const EDGE_RECORD &bkw_edge = next_node2_ptr->backward_edges[i];
    NODE_REF curr_next_node = next_node_from_edge_rec(bkw_edge);
    UNICHAR_ID curr_unichar_id = unichar_id_from_edge_rec(bkw_edge);
    int curr_word_end = end_of_word_from_edge_rec(bkw_edge);
    add_edge_linkage(next_node1, curr_next_node, BACKWARD_EDGE,
                     curr_word_end, curr_unichar_id);
    // Relocate the corresponding forward edge in curr_next_node
    ASSERT_HOST(edge_char_of(curr_next_node, next_node2, FORWARD_EDGE,
                             curr_word_end, curr_unichar_id,
                             &edge_ptr, &edge_index));
    set_next_node_in_edge_rec(edge_ptr, next_node1);
  }
  int next_node2_num_edges = (next_node2_ptr->forward_edges.size() +
                              next_node2_ptr->backward_edges.size());
  if (dawg_debug_level > 1) {
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
                                 const EDGE_VECTOR &backward_edges,
                                 NODE_MARKER reduced_nodes) {
  if (dawg_debug_level > 1)
    tprintf("reduce_lettered_edges(edge=" REFFORMAT ")\n", edge_index);
  // Compare each of the edge pairs with the given unichar_id.
  bool did_something = false;
  for (int i = edge_index; i < backward_edges.size() - 1; ++i) {
    // Find the first edge that can be eliminated.
    UNICHAR_ID curr_unichar_id = INVALID_UNICHAR_ID;
    while (i < backward_edges.size() &&
           ((curr_unichar_id = unichar_id_from_edge_rec(backward_edges[i])) ==
            unichar_id) &&
           !can_be_eliminated(backward_edges[i])) ++i;
    if (i == backward_edges.size() || curr_unichar_id != unichar_id) break;
    const EDGE_RECORD &edge_rec = backward_edges[i];
    // Compare it to the rest of the edges with the given unichar_id.
    for (int j = i + 1; j < backward_edges.size(); ++j) {
      const EDGE_RECORD &next_edge_rec = backward_edges[j];
      if (unichar_id_from_edge_rec(next_edge_rec) != unichar_id) break;
      if (end_of_word_from_edge_rec(next_edge_rec) ==
          end_of_word_from_edge_rec(edge_rec) &&
          can_be_eliminated(next_edge_rec) &&
          eliminate_redundant_edges(node, edge_rec, next_edge_rec)) {
        reduced_nodes[next_node_from_edge_rec(edge_rec)] = 0;
        did_something = true;
        --j;  // do not increment j if next_edge_rec was removed
      }
    }
  }
  return did_something;
}

void Trie::sort_edges(EDGE_VECTOR *edges) {
  int num_edges = edges->size();
  if (num_edges <= 1) return;
  for (int i = 0; i < num_edges - 1; ++i) {
    int min = i;
    for (int j = (i + 1); j < num_edges; ++j) {
      if (unichar_id_from_edge_rec((*edges)[j]) <
          unichar_id_from_edge_rec((*edges)[min])) min = j;
    }
    if (i != min) {
      EDGE_RECORD temp = (*edges)[i];
      (*edges)[i] = (*edges)[min];
      (*edges)[min] = temp;
    }
  }
}

void Trie::reduce_node_input(NODE_REF node,
                             NODE_MARKER reduced_nodes) {
  if (dawg_debug_level > 1) {
    tprintf("reduce_node_input(node=" REFFORMAT ")\n", node);
    print_node(node, MAX_NODE_EDGES_DISPLAY);
  }

  EDGE_VECTOR &backward_edges = nodes_[node]->backward_edges;
  if (node != 0) sort_edges(&backward_edges);
  EDGE_INDEX edge_index = 0;
  while (edge_index < backward_edges.size()) {
    UNICHAR_ID unichar_id =
      unichar_id_from_edge_rec(backward_edges[edge_index]);
    while (reduce_lettered_edges(edge_index, unichar_id, node,
                                 backward_edges, reduced_nodes));
    while (++edge_index < backward_edges.size() &&
           unichar_id_from_edge_rec(backward_edges[edge_index]) == unichar_id);
  }
  reduced_nodes[node] = true;  // mark as reduced

  if (dawg_debug_level > 1) {
    tprintf("Node " REFFORMAT " after reduction:\n", node);
    print_node(node, MAX_NODE_EDGES_DISPLAY);
  }

  for (int i = 0; i < backward_edges.size(); ++i) {
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
      print_edge_rec((*vec)[i]);
      tprintf(" ");
    }
    if (dir == 0 ? i < num_fwd : i < num_bkw) tprintf("...");
    tprintf("\n");
  }
}
}  // namespace tesseract
