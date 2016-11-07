///////////////////////////////////////////////////////////////////////
// File:        recodebeam.h
// Description: Beam search to decode from the re-encoded CJK as a sequence of
//              smaller numbers in place of a single large code.
// Author:      Ray Smith
// Created:     Fri Mar 13 09:12:01 PDT 2015
//
// (C) Copyright 2015, Google Inc.
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

#ifndef THIRD_PARTY_TESSERACT_LSTM_RECODEBEAM_H_
#define THIRD_PARTY_TESSERACT_LSTM_RECODEBEAM_H_

#include "dawg.h"
#include "dict.h"
#include "genericheap.h"
#include "kdpair.h"
#include "networkio.h"
#include "ratngs.h"
#include "unicharcompress.h"

namespace tesseract {

// Lattice element for Re-encode beam search.
struct RecodeNode {
  RecodeNode()
      : code(-1),
        unichar_id(INVALID_UNICHAR_ID),
        permuter(TOP_CHOICE_PERM),
        start_of_dawg(false),
        start_of_word(false),
        end_of_word(false),
        duplicate(false),
        certainty(0.0f),
        score(0.0f),
        prev(NULL),
        dawgs(NULL) {}
  RecodeNode(int c, int uni_id, PermuterType perm, bool dawg_start,
             bool word_start, bool end, bool dup, float cert, float s,
             const RecodeNode* p, DawgPositionVector* d)
      : code(c),
        unichar_id(uni_id),
        permuter(perm),
        start_of_dawg(dawg_start),
        start_of_word(word_start),
        end_of_word(end),
        duplicate(dup),
        certainty(cert),
        score(s),
        prev(p),
        dawgs(d) {}
  // NOTE: If we could use C++11, then this would be a move constructor.
  // Instead we have copy constructor that does a move!! This is because we
  // don't want to copy the whole DawgPositionVector each time, and true
  // copying isn't necessary for this struct. It does get moved around a lot
  // though inside the heap and during heap push, hence the move semantics.
  RecodeNode(RecodeNode& src) : dawgs(NULL) {
    *this = src;
    ASSERT_HOST(src.dawgs == NULL);
  }
  RecodeNode& operator=(RecodeNode& src) {
    delete dawgs;
    memcpy(this, &src, sizeof(src));
    src.dawgs = NULL;
    return *this;
  }
  ~RecodeNode() { delete dawgs; }

  // The re-encoded code here = index to network output.
  int code;
  // The decoded unichar_id is only valid for the final code of a sequence.
  int unichar_id;
  // The type of permuter active at this point. Intervals between start_of_word
  // and end_of_word make valid words of type given by permuter where
  // end_of_word is true. These aren't necessarily delimited by spaces.
  PermuterType permuter;
  // True if this is the initial dawg state. May be attached to a space or,
  // in a non-space-delimited lang, the end of the previous word.
  bool start_of_dawg;
  // True if this is the first node in a dictionary word.
  bool start_of_word;
  // True if this represents a valid candidate end of word position. Does not
  // necessarily mark the end of a word, since a word can be extended beyond a
  // candidiate end by a continuation, eg 'the' continues to 'these'.
  bool end_of_word;
  // True if this is a duplicate of prev in all respects. Some training modes
  // allow the network to output duplicate characters and crush them with CTC,
  // but that would mess up the decoding, so we just smash them together on the
  // fly using the duplicate flag.
  bool duplicate;
  // Certainty (log prob) of (just) this position.
  float certainty;
  // Total certainty of the path to this position.
  float score;
  // The previous node in this chain. Borrowed pointer.
  const RecodeNode* prev;
  // The currently active dawgs at this position. Owned pointer.
  DawgPositionVector* dawgs;
};

typedef KDPairInc<double, RecodeNode> RecodePair;
typedef GenericHeap<RecodePair> RecodeHeap;

// Class that holds the entire beam search for recognition of a text line.
class RecodeBeamSearch {
 public:
  // Borrows the pointer, which is expected to survive until *this is deleted.
  RecodeBeamSearch(const UnicharCompress& recoder, int null_char,
                   bool simple_text, Dict* dict);

  // Decodes the set of network outputs, storing the lattice internally.
  // If charset is not null, it enables detailed debugging of the beam search.
  void Decode(const NetworkIO& output, double dict_ratio, double cert_offset,
              double worst_dict_cert, const UNICHARSET* charset);
  void Decode(const GENERIC_2D_ARRAY<float>& output, double dict_ratio,
              double cert_offset, double worst_dict_cert,
              const UNICHARSET* charset);

  // Returns the best path as labels/scores/xcoords similar to simple CTC.
  void ExtractBestPathAsLabels(GenericVector<int>* labels,
                               GenericVector<int>* xcoords) const;
  // Returns the best path as unichar-ids/certs/ratings/xcoords skipping
  // duplicates, nulls and intermediate parts.
  void ExtractBestPathAsUnicharIds(bool debug, const UNICHARSET* unicharset,
                                   GenericVector<int>* unichar_ids,
                                   GenericVector<float>* certs,
                                   GenericVector<float>* ratings,
                                   GenericVector<int>* xcoords) const;

  // Returns the best path as a set of WERD_RES.
  void ExtractBestPathAsWords(const TBOX& line_box, float scale_factor,
                              bool debug, const UNICHARSET* unicharset,
                              PointerVector<WERD_RES>* words);

  // Generates debug output of the content of the beams after a Decode.
  void DebugBeams(const UNICHARSET& unicharset) const;

  // Clipping value for certainty inside Tesseract. Reflects the minimum value
  // of certainty that will be returned by ExtractBestPathAsUnicharIds.
  // Supposedly on a uniform scale that can be compared across languages and
  // engines.
  static const float kMinCertainty;

 private:
  // Struct for the Re-encode beam search. This struct holds the data for
  // a single time-step position of the output. Use a PointerVector<RecodeBeam>
  // to hold all the timesteps and prevent reallocation of the individual heaps.
  struct RecodeBeam {
    // Resets to the initial state without deleting all the memory.
    void Clear() {
      for (int i = 0; i <= RecodedCharID::kMaxCodeLen; ++i) {
        beams_[i].clear();
        dawg_beams_[i].clear();
      }
      RecodeNode empty;
      best_initial_dawg_ = empty;
    }
    // A separate beam for each code position. Since there aren't that many
    // code positions, this allows the beam to be quite narrow, and yet still
    // have a low chance of losing the best path.
    // Each heap is stored with the WORST result at the top, so we can quickly
    // get the top-n values.
    RecodeHeap beams_[RecodedCharID::kMaxCodeLen + 1];
    // Although, we can only use complete codes in the dawg, we have to separate
    // partial code paths that lead back to a mid-dawg word from paths that are
    // not part of a dawg word, as they have a different score. Since a dawg
    // word can dead-end at any point, we need to keep the non dawg path going
    // so the dawg beams_ are totally separate set with a heap for each length
    // just like the non-dawg beams.
    RecodeHeap dawg_beams_[RecodedCharID::kMaxCodeLen + 1];
    // While the language model is only a single word dictionary, we can use
    // word starts as a choke point in the beam, and keep only a single dict
    // start node at each step, so we find the best one here and push it on
    // the heap, if it qualifies, after processing all of the step.
    RecodeNode best_initial_dawg_;
  };
  typedef KDPairInc<float, int> TopPair;

  // Generates debug output of the content of a single beam position.
  void DebugBeamPos(const UNICHARSET& unicharset, const RecodeHeap& heap) const;

  // Returns the given best_nodes as unichar-ids/certs/ratings/xcoords skipping
  // duplicates, nulls and intermediate parts.
  static void ExtractPathAsUnicharIds(
      const GenericVector<const RecodeNode*>& best_nodes,
      GenericVector<int>* unichar_ids, GenericVector<float>* certs,
      GenericVector<float>* ratings, GenericVector<int>* xcoords);

  // Sets up a word with the ratings matrix and fake blobs with boxes in the
  // right places.
  WERD_RES* InitializeWord(bool leading_space, const TBOX& line_box,
                           int word_start, int word_end, float space_certainty,
                           const UNICHARSET* unicharset,
                           const GenericVector<int>& xcoords,
                           float scale_factor);

  // Fills top_n_flags_ with bools that are true iff the corresponding output
  // is one of the top_n.
  void ComputeTopN(const float* outputs, int num_outputs, int top_n);

  // Adds the computation for the current time-step to the beam. Call at each
  // time-step in sequence from left to right. outputs is the activation vector
  // for the current timestep.
  void DecodeStep(const float* outputs, int t, double dict_ratio,
                  double cert_offset, double worst_dict_cert,
                  const UNICHARSET* charset);

  // Adds to the appropriate beams the legal (according to recoder)
  // continuations of context prev, which is of the given length, using the
  // given network outputs to provide scores to the choices. Uses only those
  // choices for which top_n_flags[index] == top_n_flag.
  void ContinueContext(const RecodeNode* prev, int length, const float* outputs,
                       bool use_dawgs, bool top_n_flag, double dict_ratio,
                       double cert_offset, double worst_dict_cert,
                       RecodeBeam* step);
  // Adds a RecodeNode composed of the tuple (code, unichar_id, cert, prev,
  // appropriate-dawg-args, cert) to the given heap (dawg_beam_) if unichar_id
  // is a valid continuation of whatever is in prev.
  void ContinueDawg(int max_size, int code, int unichar_id, float cert,
                    const RecodeNode* prev, RecodeHeap* heap, RecodeBeam* step);
  // Adds a RecodeNode composed of the tuple (code, unichar_id,
  // initial-dawg-state, prev, cert) to the given heap if/ there is room or if
  // better than the current worst element if already full.
  void PushInitialDawgIfBetter(int code, int unichar_id, PermuterType permuter,
                               bool start, bool end, float cert,
                               const RecodeNode* prev,
                               RecodeNode* best_initial_dawg);
  // Adds a copy of the given prev as a duplicate of and successor to prev, if
  // there is room or if better than the current worst element if already full.
  static void PushDupIfBetter(int max_size, float cert, const RecodeNode* prev,
                              RecodeHeap* heap);
  // Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
  // false, false, false, false, cert, prev, NULL) to heap if there is room
  // or if better than the current worst element if already full.
  static void PushNoDawgIfBetter(int max_size, int code, int unichar_id,
                                 PermuterType permuter, float cert,
                                 const RecodeNode* prev, RecodeHeap* heap);
  // Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
  // dawg_start, word_start, end, dup, cert, prev, d) to heap if there is room
  // or if better than the current worst element if already full.
  static void PushHeapIfBetter(int max_size, int code, int unichar_id,
                               PermuterType permuter, bool dawg_start,
                               bool word_start, bool end, bool dup, float cert,
                               const RecodeNode* prev, DawgPositionVector* d,
                               RecodeHeap* heap);
  // Backtracks to extract the best path through the lattice that was built
  // during Decode. On return the best_nodes vector essentially contains the set
  // of code, score pairs that make the optimal path with the constraint that
  // the recoder can decode the code sequence back to a sequence of unichar-ids.
  void ExtractBestPaths(GenericVector<const RecodeNode*>* best_nodes,
                        GenericVector<const RecodeNode*>* second_nodes) const;
  // Helper backtracks through the lattice from the given node, storing the
  // path and reversing it.
  void ExtractPath(const RecodeNode* node,
                   GenericVector<const RecodeNode*>* path) const;
  // Helper prints debug information on the given lattice path.
  void DebugPath(const UNICHARSET* unicharset,
                 const GenericVector<const RecodeNode*>& path) const;
  // Helper prints debug information on the given unichar path.
  void DebugUnicharPath(const UNICHARSET* unicharset,
                        const GenericVector<const RecodeNode*>& path,
                        const GenericVector<int>& unichar_ids,
                        const GenericVector<float>& certs,
                        const GenericVector<float>& ratings,
                        const GenericVector<int>& xcoords) const;

  static const int kBeamWidths[RecodedCharID::kMaxCodeLen + 1];

  // The encoder/decoder that we will be using.
  const UnicharCompress& recoder_;
  // The beam for each timestep in the output.
  PointerVector<RecodeBeam> beam_;
  // The number of timesteps valid in beam_;
  int beam_size_;
  // A flag to indicate which outputs are the top-n choices. Current timestep
  // only.
  GenericVector<bool> top_n_flags_;
  // Heap used to compute the top_n_flags_.
  GenericHeap<TopPair> top_heap_;
  // Borrowed pointer to the dictionary to use in the search.
  Dict* dict_;
  // True if the language is space-delimited, which is true for most languages
  // except chi*, jpn, tha.
  bool space_delimited_;
  // True if the input is simple text, ie adjacent equal chars are not to be
  // eliminated.
  bool is_simple_text_;
  // The encoded (class label) of the null/reject character.
  int null_char_;
};

}  // namespace tesseract.

#endif  // THIRD_PARTY_TESSERACT_LSTM_RECODEBEAM_H_
