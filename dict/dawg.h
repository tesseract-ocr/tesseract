/* -*-C-*-
 ********************************************************************************
 *
 * File:         dawg.h  (Formerly dawg.h)
 * Description:  Definition of a class that represents Directed Accyclic Word
 *               Graph (DAWG), functions to build and manipulate the DAWG.
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jun 19 16:50:24 1991 (Mark Seaman) marks@hpgrlt
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

#ifndef DAWG_H
#define DAWG_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/

#include "elst.h"
#include "general.h"
#include "ratngs.h"
#include "varable.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/

extern INT_VAR_H(dawg_debug_level, 0, "Set to 1 for general debug info, to"
                 " 2 for more details, to 3 to see all the debug messages");

#ifdef __MSW32__
#define NO_EDGE                (inT64) 0xffffffffffffffffi64
#else
#define NO_EDGE                (inT64) 0xffffffffffffffffll
#endif

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
class UNICHARSET;

typedef uinT64 EDGE_RECORD;
typedef EDGE_RECORD *EDGE_ARRAY;
typedef inT64 EDGE_REF;
typedef inT64 NODE_REF;
typedef EDGE_REF *NODE_MAP;

namespace tesseract {

struct NodeChild {
  UNICHAR_ID unichar_id;
  EDGE_REF edge_ref;
  NodeChild(UNICHAR_ID id, EDGE_REF ref): unichar_id(id), edge_ref(ref) {}
  NodeChild(): unichar_id(INVALID_UNICHAR_ID), edge_ref(NO_EDGE) {}
};

typedef GenericVector<NodeChild> NodeChildVector;
typedef GenericVector<int> SuccessorList;
typedef GenericVector<SuccessorList *> SuccessorListsVector;

enum DawgType {
  DAWG_TYPE_PUNCTUATION,
  DAWG_TYPE_PREFIX,
  DAWG_TYPE_ROOT,
  DAWG_TYPE_WORD,
  DAWG_TYPE_SUFFIX,
  DAWG_TYPE_NUMBER,

  DAWG_TYPE_COUNT  // number of enum entries
};

/*----------------------------------------------------------------------
              C o n s t a n t s
----------------------------------------------------------------------*/
#define FORWARD_EDGE           (inT32) 0
#define BACKWARD_EDGE          (inT32) 1
#define MAX_NODE_EDGES_DISPLAY (inT64) 100
#define LAST_FLAG              (inT64) 1
#define DIRECTION_FLAG         (inT64) 2
#define WERD_END_FLAG          (inT64) 4
#define LETTER_START_BIT       0
#define NUM_FLAG_BITS          3
#define REFFORMAT "%lld"

// Set kBeginningDawgsType[i] to true if a Dawg of
// DawgType i can contain the beginning of a word.
static const bool kBeginningDawgsType[] = {1, 1, 0, 1, 0, 1 };

static const bool kDawgSuccessors[DAWG_TYPE_COUNT][DAWG_TYPE_COUNT] = {
  { 0, 1, 0, 1, 0, 0 },  // for DAWG_TYPE_PUNCTUATION
  { 0, 0, 1, 1, 0, 0 },  // for DAWG_TYPE_PREFIX
  { 0, 0, 0, 0, 1, 0 },  // for DAWG_TYPE_ROOT
  { 1, 0, 0, 0, 0, 0 },  // for DAWG_TYPE_WORD
  { 1, 0, 0, 0, 0, 0 },  // for DAWG_TYPE_SUFFIX
  { 0, 0, 0, 0, 0, 0 }   // for DAWG_TYPE_NUMBER
};

static const char kWildcard[] = "*";


/*----------------------------------------------------------------------
              C l a s s e s   a n d   S t r u c t s
----------------------------------------------------------------------*/
//
/// Abstract class (an interface) that declares methods needed by the
/// various tesseract classes to operate on SquishedDawg and Trie objects.
///
/// This class initializes all the edge masks (since their usage by
/// SquishedDawg and Trie is identical) and implements simple accessors
/// for each of the fields encoded in an EDGE_RECORD.
/// This class also implements word_in_dawg() and check_for_words()
/// (since they use only the public methods of SquishedDawg and Trie
/// classes that are inherited from the Dawg base class).
//
class Dawg {
 public:
  /// Magic number to determine endianness when reading the Dawg from file.
  static const inT16 kDawgMagicNumber = 42;
  /// A special unichar id that indicates that any appropriate pattern
  /// (e.g.dicitonary word, 0-9 digit, etc) can be inserted instead
  /// Used for expressing patterns in punctuation and number Dawgs.
  static const UNICHAR_ID kPatternUnicharID = 0;

  inline DawgType type() const { return type_; }
  inline const STRING &lang() const { return lang_; }
  inline PermuterType permuter() const { return perm_; }

  virtual ~Dawg() {};

  /// Returns true if the given word is in the Dawg.
  bool word_in_dawg(const WERD_CHOICE &word) const;

  /// Checks the Dawg for the words that are listed in the requested file.
  /// Returns the number of words in the given file missing from the Dawg.
  int check_for_words(const char *filename,
                      const UNICHARSET &unicharset,
                      bool enable_wildcard) const;

  // Pure virtual function that should be implemented by the derived classes.

  /// Returns the edge that corresponds to the letter out of this node.
  virtual EDGE_REF edge_char_of(NODE_REF node, UNICHAR_ID unichar_id,
                                bool word_end) const = 0;

  /// Fills the given NodeChildVector with all the unichar ids (and the
  /// corresponding EDGE_REFs) for which there is an edge out of this node.
  virtual void unichar_ids_of(NODE_REF node, NodeChildVector *vec) const = 0;

  /// Returns the next node visited by following the edge
  /// indicated by the given EDGE_REF.
  virtual NODE_REF next_node(EDGE_REF edge_ref) const = 0;

  /// Returns true if the edge indicated by the given EDGE_REF
  /// marks the end of a word.
  virtual bool end_of_word(EDGE_REF edge_ref) const = 0;

  /// Returns UNICHAR_ID stored in the edge indicated by the given EDGE_REF.
  virtual UNICHAR_ID edge_letter(EDGE_REF edge_ref) const = 0;

  /// Prints the contents of the node indicated by the given NODE_REF.
  /// At most max_num_edges will be printed.
  virtual void print_node(NODE_REF node, int max_num_edges) const = 0;

 protected:
  Dawg() {}

  /// Returns the next node visited by following this edge.
  inline NODE_REF next_node_from_edge_rec(const EDGE_RECORD &edge_rec) const {
    return ((edge_rec & next_node_mask_) >> next_node_start_bit_);
  }
  /// Returns the direction flag of this edge.
  inline int direction_from_edge_rec(const EDGE_RECORD &edge_rec) const {
    return ((edge_rec & (DIRECTION_FLAG << flag_start_bit_))) ?
      BACKWARD_EDGE : FORWARD_EDGE;
  }
  /// Returns true if this edge marks the end of a word.
  inline bool end_of_word_from_edge_rec(const EDGE_RECORD &edge_rec) const {
    return (edge_rec & (WERD_END_FLAG << flag_start_bit_)) != 0;
  }
  /// Returns UNICHAR_ID recorded in this edge.
  inline UNICHAR_ID unichar_id_from_edge_rec(
      const EDGE_RECORD &edge_rec) const {
    return ((edge_rec & letter_mask_) >> LETTER_START_BIT);
  }
  /// Sets the next node link for this edge in the Dawg.
  inline void set_next_node_in_edge_rec(
      EDGE_RECORD *edge_rec, EDGE_REF value) {
    *edge_rec &= (~next_node_mask_);
    *edge_rec |= ((value << next_node_start_bit_) & next_node_mask_);
  }
  /// Sets this edge record to be the last one in a sequence of edges.
  inline void set_last_flag_in_edge_rec(EDGE_RECORD *edge_rec) {
    *edge_rec |= (LAST_FLAG << flag_start_bit_);
  }
  /// Sequentially compares the given values of unichar ID, next node
  /// and word end marker with the values in the given EDGE_RECORD.
  /// Returns: 1 if at any step the given input value exceeds
  ///            that of edge_rec (and all the values already
  ///            checked are the same)
  ///          0 if edge_rec_match() returns true
  ///         -1 otherwise
  inline int given_greater_than_edge_rec(NODE_REF next_node,
                                         bool word_end,
                                         UNICHAR_ID unichar_id,
                                         const EDGE_RECORD &edge_rec) const {
    UNICHAR_ID curr_unichar_id = unichar_id_from_edge_rec(edge_rec);
    NODE_REF curr_next_node = next_node_from_edge_rec(edge_rec);
    bool curr_word_end = end_of_word_from_edge_rec(edge_rec);
    if (edge_rec_match(next_node, word_end, unichar_id, curr_next_node,
                       curr_word_end, curr_unichar_id)) return 0;
    if (unichar_id > curr_unichar_id) return 1;
    if (unichar_id == curr_unichar_id) {
      if (next_node > curr_next_node) return 1;
      if (next_node == curr_next_node) {
        if (word_end > curr_word_end) return 1;
      }
    }
    return -1;
  }
  /// Returns true if all the values are equal (any value matches
  /// next_node if next_node == NO_EDGE, any value matches word_end
  /// if word_end is false).
  inline bool edge_rec_match(NODE_REF next_node,
                             bool word_end,
                             UNICHAR_ID unichar_id,
                             NODE_REF other_next_node,
                             bool other_word_end,
                             UNICHAR_ID other_unichar_id) const {
    return ((unichar_id == other_unichar_id) &&
            (next_node == NO_EDGE || next_node == other_next_node) &&
            (!word_end || (word_end == other_word_end)));
  }

  /// Sets type_, lang_, perm_, unicharset_size_.
  /// Initializes the values of various masks from unicharset_size_.
  void init(DawgType type, const STRING &lang,
            PermuterType perm, int unicharset_size);

  /// Matches all of the words that are represented by this string.
  /// If wilcard is set to something other than INVALID_UNICHAR_ID,
  /// the *'s in this string are interpreted as wildcards.
  /// WERD_CHOICE param is not passed by const so that wildcard searches
  /// can modify it and work without having to copy WERD_CHOICEs.
  bool match_words(WERD_CHOICE *word, inT32 index,
                   NODE_REF node, UNICHAR_ID wildcard) const;

  // Member Variables.
  DawgType type_;
  STRING lang_;
  /// Permuter code that should be used if the word is found in this Dawg.
  PermuterType perm_;
  // Variables to construct various edge masks. Formerly:
  // #define NEXT_EDGE_MASK (inT64) 0xfffffff800000000i64
  // #define FLAGS_MASK     (inT64) 0x0000000700000000i64
  // #define LETTER_MASK    (inT64) 0x00000000ffffffffi64
  int unicharset_size_;
  int flag_start_bit_;
  int next_node_start_bit_;
  uinT64 next_node_mask_;
  uinT64 flags_mask_;
  uinT64 letter_mask_;
};

//
/// DawgInfo struct and DawgInfoVector class are used for
/// storing information about the current Dawg search state.
//
struct DawgInfo {
  DawgInfo() : dawg_index(-1), ref(NO_EDGE) {}
  DawgInfo(int i, EDGE_REF r) : dawg_index(i), ref(r) {}
  bool operator==(const DawgInfo &other) {
    return (this->dawg_index == other.dawg_index &&
            this->ref == other.ref);
  }
  int dawg_index;
  EDGE_REF ref;
};
class DawgInfoVector : public GenericVector<DawgInfo> {
 public:
  /// Overload destructor, since clear() does not delete data_[] any more.
  ~DawgInfoVector() {
    if (size_reserved_ > 0) {
      delete[] data_;
      size_used_ = 0;
      size_reserved_ = 0;
    }
  }
  /// Overload clear() in order to avoid allocating/deallocating memory
  /// when clearing the vector and re-inserting entries into it later.
  void clear() { size_used_ = 0; }
  /// Adds an entry for the given dawg_index with the given node to the vec.
  /// Returns false if the same entry already exists in the vector,
  /// true otherwise.
  inline bool add_unique(const DawgInfo &new_info, const char *debug_msg) {
    for (int i = 0; i < size_used_; ++i) {
      if (data_[i] == new_info) return false;
    }
    push_back(new_info);
    if (dawg_debug_level) {
      tprintf("%s[%d, " REFFORMAT "]\n", debug_msg,
              new_info.dawg_index, new_info.ref);
    }
    return true;
  }
  /// Removes an entry that equals to the given DawgInfo.
  /// This function assumes that the entries in the vector are unique.
  /// Returns true if an entry was found and removed.
  inline bool remove(const DawgInfo &info) {
    for (int i = 0; i < size_used_; ++i) {
      if (data_[i] == info) {
        for (int j = i + 1; j < size_used_; ++j) {
          data_[j-1] = data_[j];
        }
        size_used_--;
        return true;
      }
    }
    return false;
  }
};

//
/// Concrete class that can operate on a compacted (squished) Dawg (read,
/// search and write to file). This class is read-only in the sense that
/// new words can not be added to an instance of SquishedDawg.
/// The underlying representation of the nodes and edges in SquishedDawg
/// is stored as a contiguous EDGE_ARRAY (read from file or given as an
/// argument to the constructor).
//
class SquishedDawg : public Dawg {
 public:
  SquishedDawg(FILE *file, DawgType type,
               const STRING &lang, PermuterType perm) {
    read_squished_dawg(file, type, lang, perm);
    num_forward_edges_in_node0 = num_forward_edges(0);
  }
  SquishedDawg(const char* filename, DawgType type,
               const STRING &lang, PermuterType perm) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
      tprintf("Failed to open dawg file %s\n", filename);
      exit(1);
    }
    read_squished_dawg(file, type, lang, perm);
    num_forward_edges_in_node0 = num_forward_edges(0);
    fclose(file);
  }
  SquishedDawg(EDGE_ARRAY edges, int num_edges, DawgType type,
               const STRING &lang, PermuterType perm, int unicharset_size) :
    edges_(edges), num_edges_(num_edges) {
    init(type, lang, perm, unicharset_size);
    num_forward_edges_in_node0 = num_forward_edges(0);
    if (dawg_debug_level > 3) print_all("SquishedDawg:");
  }
  ~SquishedDawg();

  /// Returns the edge that corresponds to the letter out of this node.
  EDGE_REF edge_char_of(NODE_REF node, UNICHAR_ID unichar_id,
                        bool word_end) const;

  /// Fills the given NodeChildVector with all the unichar ids (and the
  /// corresponding EDGE_REFs) for which there is an edge out of this node.
  void unichar_ids_of(NODE_REF node, NodeChildVector *vec) const {
    EDGE_REF edge = node;
    if (!edge_occupied(edge) || edge == NO_EDGE) return;
    assert(forward_edge(edge));  // we don't expect any backward edges to
    do {                         // be present when this funciton is called
      vec->push_back(NodeChild(unichar_id_from_edge_rec(edges_[edge]), edge));
    } while (!last_edge(edge++));
  }

  /// Returns the next node visited by following the edge
  /// indicated by the given EDGE_REF.
  NODE_REF next_node(EDGE_REF edge) const {
    return next_node_from_edge_rec((edges_[edge]));
  }

  /// Returns true if the edge indicated by the given EDGE_REF
  /// marks the end of a word.
  bool end_of_word(EDGE_REF edge_ref) const {
    return end_of_word_from_edge_rec((edges_[edge_ref]));
  }

  /// Returns UNICHAR_ID stored in the edge indicated by the given EDGE_REF.
  UNICHAR_ID edge_letter(EDGE_REF edge_ref) const {
    return unichar_id_from_edge_rec((edges_[edge_ref]));
  }

  /// Prints the contents of the node indicated by the given NODE_REF.
  /// At most max_num_edges will be printed.
  void print_node(NODE_REF node, int max_num_edges) const;

  /// Writes the squished/reduced Dawg to a file.
  void write_squished_dawg(const char *filename);

 private:
  /// Sets the next node link for this edge.
  inline void set_next_node(EDGE_REF edge_ref, EDGE_REF value) {
    set_next_node_in_edge_rec(&(edges_[edge_ref]), value);
  }
  /// Sets the edge to be empty.
  inline void set_empty_edge(EDGE_REF edge_ref) {
    (edges_[edge_ref] = next_node_mask_);
  }
  /// Goes through all the edges and clears each one out.
  inline void clear_all_edges() {
    for (int edge = 0; edge < num_edges_; edge++) set_empty_edge(edge);
  }
  /// Clears the last flag of this edge.
  inline void clear_last_flag(EDGE_REF edge_ref) {
     (edges_[edge_ref] &= ~(LAST_FLAG << flag_start_bit_));
  }
  /// Returns true if this edge is in the forward direction.
  inline bool forward_edge(EDGE_REF edge_ref) const {
    return (edge_occupied(edge_ref) &&
            (FORWARD_EDGE == direction_from_edge_rec(edges_[edge_ref])));
  }
  /// Returns true if this edge is in the backward direction.
  inline bool backward_edge(EDGE_REF edge_ref) const {
    return (edge_occupied(edge_ref) &&
            (BACKWARD_EDGE == direction_from_edge_rec(edges_[edge_ref])));
  }
  /// Returns true if the edge spot in this location is occupied.
  inline bool edge_occupied(EDGE_REF edge_ref) const {
    return (edges_[edge_ref] != next_node_mask_);
  }
  /// Returns true if this edge is the last edge in a sequence.
  inline bool last_edge(EDGE_REF edge_ref) const {
    return (edges_[edge_ref] & (LAST_FLAG << flag_start_bit_)) != 0;
  }

  /// Counts and returns the number of forward edges in this node.
  inT32 num_forward_edges(NODE_REF node) const;

  /// Reads SquishedDawg from a file.
  void read_squished_dawg(FILE *file, DawgType type,
                          const STRING &lang, PermuterType perm);

  /// Prints the contents of an edge indicated by the given EDGE_REF.
  void print_edge(EDGE_REF edge) const;

  /// Prints the contents of the SquishedDawg.
  void print_all(const char* msg) {
    tprintf("\n__________________________\n%s\n", msg);
    for (int i = 0; i < num_edges_; ++i) print_edge(i);
    tprintf("__________________________\n");
  }
  /// Constructs a mapping from the memory node indices to disk node indices.
  NODE_MAP build_node_map(inT32 *num_nodes) const;


  // Member variables.
  EDGE_ARRAY edges_;
  int num_edges_;
  int num_forward_edges_in_node0;
};
}  // namespace tesseract

#endif
