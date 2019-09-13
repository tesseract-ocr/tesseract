/* -*-C-*-
 ********************************************************************************
 *
 * File:        trie.h
 * Description: Functions to build a trie data structure.
 * Author:      Mark Seaman, SW Productivity
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
#ifndef TRIE_H
#define TRIE_H

#include "dawg.h"
#include "genericvector.h"

class UNICHARSET;

// Note: if we consider either NODE_REF or EDGE_INDEX to ever exceed
// max int32, we will need to change GenericVector to use int64 for size
// and address indices. This does not seem to be needed immediately,
// since currently the largest number of edges limit used by tesseract
// (kMaxNumEdges in wordlist2dawg.cpp) is far less than max int32.
// There are also int casts below to satisfy the WIN32 compiler that would
// need to be changed.
// It might be cleanest to change the types of most of the Trie/Dawg related
// typedefs to int and restrict the casts to extracting these values from
// the 64 bit EDGE_RECORD.
using EDGE_INDEX = int64_t ;  // index of an edge in a given node
using NODE_MARKER = bool *;
using EDGE_VECTOR = GenericVector<EDGE_RECORD> ;

struct TRIE_NODE_RECORD {
  EDGE_VECTOR forward_edges;
  EDGE_VECTOR backward_edges;
};
using TRIE_NODES = GenericVector<TRIE_NODE_RECORD *> ;

namespace tesseract {

/**
 * Concrete class for Trie data structure that allows to store a list of
 * words (extends Dawg base class) as well as dynamically add new words.
 * This class stores a vector of pointers to TRIE_NODE_RECORDs, each of
 * which has a vector of forward and backward edges.
 */
class Trie : public Dawg {
 public:
  enum RTLReversePolicy {
    RRP_DO_NO_REVERSE,
    RRP_REVERSE_IF_HAS_RTL,
    RRP_FORCE_REVERSE,
  };

  // Minimum number of concrete characters at the beginning of user patterns.
  static const int kSaneNumConcreteChars = 0;
  // Various unicode whitespace characters are used to denote unichar patterns,
  // (character classifier would never produce these whitespace characters as a
  // valid classification).
  static const char kAlphaPatternUnicode[];
  static const char kDigitPatternUnicode[];
  static const char kAlphanumPatternUnicode[];
  static const char kPuncPatternUnicode[];
  static const char kLowerPatternUnicode[];
  static const char kUpperPatternUnicode[];

  static const char *get_reverse_policy_name(
      RTLReversePolicy reverse_policy);

  // max_num_edges argument allows limiting the amount of memory this
  // Trie can consume (if a new word insert would cause the Trie to
  // contain more edges than max_num_edges, all the edges are cleared
  // so that new inserts can proceed).
  Trie(DawgType type, const STRING &lang, PermuterType perm,
       int unicharset_size, int debug_level)
      : Dawg(type, lang, perm, debug_level) {
    init(unicharset_size);
    num_edges_ = 0;
    deref_node_index_mask_ = ~letter_mask_;
    new_dawg_node();  // need to allocate node 0
    initialized_patterns_ = false;
  }
  ~Trie() override { nodes_.delete_data_pointers(); }

  // Reset the Trie to empty.
  void clear();

  /** Returns the edge that corresponds to the letter out of this node. */
  EDGE_REF edge_char_of(NODE_REF node_ref, UNICHAR_ID unichar_id,
                        bool word_end) const override {
    EDGE_RECORD *edge_ptr;
    EDGE_INDEX edge_index;
    if (!edge_char_of(node_ref, NO_EDGE, FORWARD_EDGE, word_end, unichar_id,
                      &edge_ptr, &edge_index)) return NO_EDGE;
    return make_edge_ref(node_ref, edge_index);
  }

  /**
   * Fills the given NodeChildVector with all the unichar ids (and the
   * corresponding EDGE_REFs) for which there is an edge out of this node.
   */
  void unichar_ids_of(NODE_REF node, NodeChildVector *vec,
                      bool word_end) const override {
    const EDGE_VECTOR &forward_edges =
      nodes_[static_cast<int>(node)]->forward_edges;
    for (int i = 0; i < forward_edges.size(); ++i) {
      if (!word_end || end_of_word_from_edge_rec(forward_edges[i])) {
        vec->push_back(NodeChild(unichar_id_from_edge_rec(forward_edges[i]),
                                 make_edge_ref(node, i)));
      }
    }
  }

  /**
   * Returns the next node visited by following the edge
   * indicated by the given EDGE_REF.
   */
  NODE_REF next_node(EDGE_REF edge_ref) const override {
    if (edge_ref == NO_EDGE || num_edges_ == 0) return NO_EDGE;
    return next_node_from_edge_rec(*deref_edge_ref(edge_ref));
  }

  /**
   * Returns true if the edge indicated by the given EDGE_REF
   * marks the end of a word.
   */
  bool end_of_word(EDGE_REF edge_ref) const override {
    if (edge_ref == NO_EDGE || num_edges_ == 0) return false;
    return end_of_word_from_edge_rec(*deref_edge_ref(edge_ref));
  }

  /** Returns UNICHAR_ID stored in the edge indicated by the given EDGE_REF. */
  UNICHAR_ID edge_letter(EDGE_REF edge_ref) const override {
    if (edge_ref == NO_EDGE || num_edges_ == 0) return INVALID_UNICHAR_ID;
    return unichar_id_from_edge_rec(*deref_edge_ref(edge_ref));
  }
  // Sets the UNICHAR_ID in the given edge_rec to unicharset_size_, marking
  // the edge dead.
  void KillEdge(EDGE_RECORD* edge_rec) const {
    *edge_rec &= ~letter_mask_;
    *edge_rec |= (unicharset_size_ << LETTER_START_BIT);
  }
  bool DeadEdge(const EDGE_RECORD& edge_rec) const {
    return unichar_id_from_edge_rec(edge_rec) == unicharset_size_;
  }

  // Prints the contents of the node indicated by the given NODE_REF.
  // At most max_num_edges will be printed.
  void print_node(NODE_REF node, int max_num_edges) const override;

  // Writes edges from nodes_ to an EDGE_ARRAY and creates a SquishedDawg.
  // Eliminates redundant edges and returns the pointer to the SquishedDawg.
  // Note: the caller is responsible for deallocating memory associated
  // with the returned SquishedDawg pointer.
  SquishedDawg *trie_to_dawg();

  // Reads a list of words from the given file and adds into the Trie.
  // Calls WERD_CHOICE::reverse_unichar_ids_if_rtl() according to the reverse
  // policy and information in the unicharset.
  // Returns false on error.
  bool read_and_add_word_list(const char *filename,
                              const UNICHARSET &unicharset,
                              Trie::RTLReversePolicy reverse);

  // Reads a list of words from the given file.
  // Returns false on error.
  bool read_word_list(const char *filename,
                      GenericVector<STRING>* words);
  // Adds a list of words previously read using read_word_list to the trie
  // using the given unicharset and reverse_policy to convert to unichar-ids.
  // Returns false on error.
  bool add_word_list(const GenericVector<STRING> &words,
                     const UNICHARSET &unicharset,
                     Trie::RTLReversePolicy reverse_policy);

  // Inserts the list of patterns from the given file into the Trie.
  // The pattern list file should contain one pattern per line in UTF-8 format.
  //
  // Each pattern can contain any non-whitespace characters, however only the
  // patterns that contain characters from the unicharset of the corresponding
  // language will be useful.
  // The only meta character is '\'. To be used in a pattern as an ordinary
  // string it should be escaped with '\' (e.g. string "C:\Documents" should
  // be written in the patterns file as "C:\\Documents").
  // This function supports a very limited regular expression syntax. One can
  // express a character, a certain character class and a number of times the
  // entity should be repeated in the pattern.
  //
  // To denote a character class use one of:
  // \c - unichar for which UNICHARSET::get_isalpha() is true (character)
  // \d - unichar for which UNICHARSET::get_isdigit() is true
  // \n - unichar for which UNICHARSET::get_isdigit() and
  //      UNICHARSET::isalpha() are true
  // \p - unichar for which UNICHARSET::get_ispunct() is true
  // \a - unichar for which UNICHARSET::get_islower() is true
  // \A - unichar for which UNICHARSET::get_isupper() is true
  //
  // \* could be specified after each character or pattern to indicate that
  // the character/pattern can be repeated any number of times before the next
  // character/pattern occurs.
  //
  // Examples:
  // 1-8\d\d-GOOG-411 will be expanded to strings:
  // 1-800-GOOG-411, 1-801-GOOG-411, ... 1-899-GOOG-411.
  //
  // http://www.\n\*.com will be expanded to strings like:
  // http://www.a.com http://www.a123.com ... http://www.ABCDefgHIJKLMNop.com
  //
  // Note: In choosing which patterns to include please be aware of the fact
  // providing very generic patterns will make tesseract run slower.
  // For example \n\* at the beginning of the pattern will make Tesseract
  // consider all the combinations of proposed character choices for each
  // of the segmentations, which will be unacceptably slow.
  // Because of potential problems with speed that could be difficult to
  // identify, each user pattern has to have at least kSaneNumConcreteChars
  // concrete characters from the unicharset at the beginning.
  bool read_pattern_list(const char *filename, const UNICHARSET &unicharset);

  // Initializes the values of *_pattern_ unichar ids.
  // This function should be called before calling read_pattern_list().
  void initialize_patterns(UNICHARSET *unicharset);

  // Fills in the given unichar id vector with the unichar ids that represent
  // the patterns of the character classes of the given unichar_id.
  void unichar_id_to_patterns(UNICHAR_ID unichar_id,
                              const UNICHARSET &unicharset,
                              GenericVector<UNICHAR_ID> *vec) const override;

  // Returns the given EDGE_REF if the EDGE_RECORD that it points to has
  // a self loop and the given unichar_id matches the unichar_id stored in the
  // EDGE_RECORD, returns NO_EDGE otherwise.
  EDGE_REF pattern_loop_edge(EDGE_REF edge_ref,
                                     UNICHAR_ID unichar_id,
                                     bool word_end) const override {
    if (edge_ref == NO_EDGE) return NO_EDGE;
    EDGE_RECORD *edge_rec = deref_edge_ref(edge_ref);
    return (marker_flag_from_edge_rec(*edge_rec) &&
            unichar_id == unichar_id_from_edge_rec(*edge_rec) &&
            word_end == end_of_word_from_edge_rec(*edge_rec)) ?
            edge_ref : NO_EDGE;
  }

  // Adds a word to the Trie (creates the necessary nodes and edges).
  //
  // If repetitions vector is not nullptr, each entry in the vector indicates
  // whether the unichar id with the corresponding index in the word is allowed
  // to repeat an unlimited number of times. For each entry that is true, MARKER
  // flag of the corresponding edge created for this unichar id is set to true).
  //
  // Return true if add succeeded, false otherwise (e.g. when a word contained
  // an invalid unichar id or the trie was getting too large and was cleared).
  bool add_word_to_dawg(const WERD_CHOICE &word,
                        const GenericVector<bool> *repetitions);
  bool add_word_to_dawg(const WERD_CHOICE &word) {
    return add_word_to_dawg(word, nullptr);
  }

 protected:
  // The structure of an EDGE_REF for Trie edges is as follows:
  // [LETTER_START_BIT, flag_start_bit_):
  //                             edge index in *_edges in a TRIE_NODE_RECORD
  // [flag_start_bit, 30th bit]: node index in nodes (TRIE_NODES vector)
  //
  // With this arrangement there are enough bits to represent edge indices
  // (each node can have at most unicharset_size_ forward edges and
  // the position of flag_start_bit is set to be log2(unicharset_size_)).
  // It is also possible to accommodate a maximum number of nodes that is at
  // least as large as that of the SquishedDawg representation (in SquishedDawg
  // each EDGE_RECORD has 32-(flag_start_bit+NUM_FLAG_BITS) bits to represent
  // the next node index).
  //

  // Returns the pointer to EDGE_RECORD after decoding the location
  // of the edge from the information in the given EDGE_REF.
  // This function assumes that EDGE_REF holds valid node/edge indices.
  inline EDGE_RECORD *deref_edge_ref(EDGE_REF edge_ref) const {
    int edge_index = static_cast<int>(
      (edge_ref & letter_mask_) >> LETTER_START_BIT);
    int node_index = static_cast<int>(
      (edge_ref & deref_node_index_mask_) >> flag_start_bit_);
    TRIE_NODE_RECORD *node_rec = nodes_[node_index];
    return &(node_rec->forward_edges[edge_index]);
  }
  /** Constructs EDGE_REF from the given node_index and edge_index. */
  inline EDGE_REF make_edge_ref(NODE_REF node_index,
                                EDGE_INDEX edge_index) const {
    return ((node_index << flag_start_bit_) |
            (edge_index << LETTER_START_BIT));
  }
  /** Sets up this edge record to the requested values. */
  inline void link_edge(EDGE_RECORD *edge, NODE_REF nxt, bool repeats,
                        int direction, bool word_end, UNICHAR_ID unichar_id) {
    EDGE_RECORD flags = 0;
    if (repeats) flags |= MARKER_FLAG;
    if (word_end) flags |= WERD_END_FLAG;
    if (direction == BACKWARD_EDGE) flags |= DIRECTION_FLAG;
    *edge = ((nxt << next_node_start_bit_) |
             (static_cast<EDGE_RECORD>(flags) << flag_start_bit_) |
             (static_cast<EDGE_RECORD>(unichar_id) << LETTER_START_BIT));
  }
  /** Prints the given EDGE_RECORD. */
  inline void print_edge_rec(const EDGE_RECORD &edge_rec) const {
    tprintf("|" REFFORMAT "|%s%s%s|%d|", next_node_from_edge_rec(edge_rec),
            marker_flag_from_edge_rec(edge_rec) ? "R," : "",
            (direction_from_edge_rec(edge_rec) == FORWARD_EDGE) ? "F" : "B",
            end_of_word_from_edge_rec(edge_rec) ? ",E" : "",
            unichar_id_from_edge_rec(edge_rec));
  }
  // Returns true if the next node in recorded the given EDGE_RECORD
  // has exactly one forward edge.
  inline bool can_be_eliminated(const EDGE_RECORD &edge_rec) {
    NODE_REF node_ref = next_node_from_edge_rec(edge_rec);
    return (node_ref != NO_EDGE &&
            nodes_[static_cast<int>(node_ref)]->forward_edges.size() == 1);
  }

  // Prints the contents of the Trie.
  // At most max_num_edges will be printed for each node.
  void print_all(const char* msg, int max_num_edges) {
    tprintf("\n__________________________\n%s\n", msg);
    for (int i = 0; i < nodes_.size(); ++i) print_node(i, max_num_edges);
    tprintf("__________________________\n");
  }

  // Finds the edge with the given direction, word_end and unichar_id
  // in the node indicated by node_ref. Fills in the pointer to the
  // EDGE_RECORD and the index of the edge with the the values
  // corresponding to the edge found. Returns true if an edge was found.
  bool edge_char_of(NODE_REF node_ref, NODE_REF next_node,
                    int direction, bool word_end, UNICHAR_ID unichar_id,
                    EDGE_RECORD **edge_ptr, EDGE_INDEX *edge_index) const;

  // Adds an single edge linkage between node1 and node2 in the direction
  // indicated by direction argument.
  bool add_edge_linkage(NODE_REF node1, NODE_REF node2, bool repeats,
                        int direction, bool word_end,
                        UNICHAR_ID unichar_id);

  // Adds forward edge linkage from node1 to node2 and the corresponding
  // backward edge linkage in the other direction.
  bool add_new_edge(NODE_REF node1, NODE_REF node2,
                    bool repeats, bool word_end, UNICHAR_ID unichar_id) {
    return (add_edge_linkage(node1, node2, repeats, FORWARD_EDGE,
                             word_end, unichar_id) &&
            add_edge_linkage(node2, node1, repeats, BACKWARD_EDGE,
                             word_end, unichar_id));
  }

  // Sets the word ending flags in an already existing edge pair.
  // Returns true on success.
  void add_word_ending(EDGE_RECORD *edge,
                       NODE_REF the_next_node,
                       bool repeats,
                       UNICHAR_ID unichar_id);

  // Allocates space for a new node in the Trie.
  NODE_REF new_dawg_node();

  // Removes a single edge linkage to between node1 and node2 in the
  // direction indicated by direction argument.
  void remove_edge_linkage(NODE_REF node1, NODE_REF node2, int direction,
                           bool word_end, UNICHAR_ID unichar_id);

  // Removes forward edge linkage from node1 to node2 and the corresponding
  // backward edge linkage in the other direction.
  void remove_edge(NODE_REF node1, NODE_REF node2,
                   bool word_end, UNICHAR_ID unichar_id) {
    remove_edge_linkage(node1, node2, FORWARD_EDGE, word_end, unichar_id);
    remove_edge_linkage(node2, node1, BACKWARD_EDGE, word_end, unichar_id);
  }

  // Compares edge1 and edge2 in the given node to see if they point to two
  // next nodes that could be collapsed. If they do, performs the reduction
  // and returns true.
  bool eliminate_redundant_edges(NODE_REF node, const EDGE_RECORD &edge1,
                                 const EDGE_RECORD &edge2);

  // Assuming that edge_index indicates the first edge in a group of edges
  // in this node with a particular letter value, looks through these edges
  // to see if any of them can be collapsed. If so does it. Returns to the
  // caller when all edges with this letter have been reduced.
  // Returns true if further reduction is possible with this same letter.
  bool reduce_lettered_edges(EDGE_INDEX edge_index,
                             UNICHAR_ID unichar_id,
                             NODE_REF node,
                             EDGE_VECTOR* backward_edges,
                             NODE_MARKER reduced_nodes);

  /**
   * Order num_edges of consecutive EDGE_RECORDS in the given EDGE_VECTOR in
   * increasing order of unichar ids. This function is normally called
   * for all edges in a single node, and since number of edges in each node
   * is usually quite small, selection sort is used.
   */
  void sort_edges(EDGE_VECTOR *edges);

  /** Eliminates any redundant edges from this node in the Trie. */
  void reduce_node_input(NODE_REF node, NODE_MARKER reduced_nodes);

  // Returns the pattern unichar id for the given character class code.
  UNICHAR_ID character_class_to_pattern(char ch);

  // Member variables
  TRIE_NODES nodes_;              // vector of nodes in the Trie
  // Freelist of edges in the root backwards node that were previously zeroed.
  GenericVector<EDGE_INDEX> root_back_freelist_;
  uint64_t num_edges_;              // sum of all edges (forward and backward)
  uint64_t deref_direction_mask_;   // mask for EDGE_REF to extract direction
  uint64_t deref_node_index_mask_;  // mask for EDGE_REF to extract node index
  // Variables for translating character class codes denoted in user patterns
  // file to the unichar ids used to represent them in a Trie.
  UNICHAR_ID alpha_pattern_;
  UNICHAR_ID digit_pattern_;
  UNICHAR_ID alphanum_pattern_;
  UNICHAR_ID punc_pattern_;
  UNICHAR_ID lower_pattern_;
  UNICHAR_ID upper_pattern_;
  bool initialized_patterns_;
};
}  // namespace tesseract

#endif
