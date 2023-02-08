///////////////////////////////////////////////////////////////////////
// File:        recodebeam.h
// Description: Beam search to decode from the re-encoded CJK as a sequence of
//              smaller numbers in place of a single large code.
// Author:      Ray Smith
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
#include "genericvector.h"
#include "kdpair.h"
#include "networkio.h"
#include "ratngs.h"
#include "unicharcompress.h"

#include <unordered_set> // for std::unordered_set
#include <vector>        // for std::vector

namespace tesseract {

// Enum describing what can follow the current node.
// Consider the following softmax outputs:
// Timestep    0    1    2    3    4    5    6    7    8
// X-score    0.01 0.55 0.98 0.42 0.01 0.01 0.40 0.95 0.01
// Y-score    0.00 0.01 0.01 0.01 0.01 0.97 0.59 0.04 0.01
// Null-score 0.99 0.44 0.01 0.57 0.98 0.02 0.01 0.01 0.98
// Then the correct CTC decoding (in which adjacent equal classes are folded,
// and then all nulls are dropped) is clearly XYX, but simple decoding (taking
// the max at each timestep) leads to:
// Null@0.99 X@0.55 X@0.98 Null@0.57 Null@0.98 Y@0.97 Y@0.59 X@0.95 Null@0.98,
// which folds to the correct XYX. The conversion to Tesseract rating and
// certainty uses the sum of the log probs (log of the product of probabilities)
// for the Rating and the minimum log prob for the certainty, but that yields a
// minimum certainty of log(0.55), which is poor for such an obvious case.
// CTC says that the probability of the result is the SUM of the products of the
// probabilities over ALL PATHS that decode to the same result, which includes:
// NXXNNYYXN, NNXNNYYN, NXXXNYYXN, NNXXNYXXN, and others including XXXXXYYXX.
// That is intractable, so some compromise between simple and ideal is needed.
// Observing that evenly split timesteps rarely happen next to each other, we
// allow scores at a transition between classes to be added for decoding thus:
// N@0.99 (N+X)@0.99 X@0.98 (N+X)@0.99 N@0.98 Y@0.97 (X+Y+N)@1.00 X@0.95 N@0.98.
// This works because NNX and NXX both decode to X, so in the middle we can use
// N+X. Note that the classes either side of a sum must stand alone, i.e. use a
// single score, to force all paths to pass through them and decode to the same
// result. Also in the special case of a transition from X to Y, with only one
// timestep between, it is possible to add X+Y+N, since XXY, XYY, and XNY all
// decode to XY.
// An important condition is that we cannot combine X and Null between two
// stand-alone Xs, since that can decode as XNX->XX or XXX->X, so the scores for
// X and Null have to go in separate paths. Combining scores in this way
// provides a much better minimum certainty of log(0.95).
// In the implementation of the beam search, we have to place the possibilities
// X, X+N and X+Y+N in the beam under appropriate conditions of the previous
// node, and constrain what can follow, to enforce the rules explained above.
// We therefore have 3 different types of node determined by what can follow:
enum NodeContinuation {
  NC_ANYTHING, // This node used just its own score, so anything can follow.
  NC_ONLY_DUP, // The current node combined another score with the score for
               // itself, without a stand-alone duplicate before, so must be
               // followed by a stand-alone duplicate.
  NC_NO_DUP,   // The current node combined another score with the score for
               // itself, after a stand-alone, so can only be followed by
               // something other than a duplicate of the current node.
  NC_COUNT
};

// Enum describing the top-n status of a code.
enum TopNState {
  TN_TOP2,     // Winner or 2nd.
  TN_TOPN,     // Runner up in top-n, but not 1st or 2nd.
  TN_ALSO_RAN, // Not in the top-n.
  TN_COUNT
};

// Lattice element for Re-encode beam search.
struct RecodeNode {
  RecodeNode()
      : code(-1)
      , unichar_id(INVALID_UNICHAR_ID)
      , permuter(TOP_CHOICE_PERM)
      , start_of_dawg(false)
      , start_of_word(false)
      , end_of_word(false)
      , duplicate(false)
      , certainty(0.0f)
      , score(0.0f)
      , prev(nullptr)
      , dawgs(nullptr)
      , code_hash(0) {}
  RecodeNode(int c, int uni_id, PermuterType perm, bool dawg_start, bool word_start, bool end,
             bool dup, float cert, float s, const RecodeNode *p, DawgPositionVector *d,
             uint64_t hash)
      : code(c)
      , unichar_id(uni_id)
      , permuter(perm)
      , start_of_dawg(dawg_start)
      , start_of_word(word_start)
      , end_of_word(end)
      , duplicate(dup)
      , certainty(cert)
      , score(s)
      , prev(p)
      , dawgs(d)
      , code_hash(hash) {}
  // NOTE: If we could use C++11, then this would be a move constructor.
  // Instead we have copy constructor that does a move!! This is because we
  // don't want to copy the whole DawgPositionVector each time, and true
  // copying isn't necessary for this struct. It does get moved around a lot
  // though inside the heap and during heap push, hence the move semantics.
  RecodeNode(const RecodeNode &src) : dawgs(nullptr) {
    *this = src;
    ASSERT_HOST(src.dawgs == nullptr);
  }
  RecodeNode &operator=(const RecodeNode &src) {
    delete dawgs;
    memcpy(this, &src, sizeof(src));
    ((RecodeNode &)src).dawgs = nullptr;
    return *this;
  }
  ~RecodeNode() {
    delete dawgs;
  }
  // Prints details of the node.
  void Print(int null_char, const UNICHARSET &unicharset, int depth) const;

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
  // candidate end by a continuation, eg 'the' continues to 'these'.
  bool end_of_word;
  // True if this->code is a duplicate of prev->code. Some training modes
  // allow the network to output duplicate characters and crush them with CTC,
  // but that would mess up the dictionary search, so we just smash them
  // together on the fly using the duplicate flag.
  bool duplicate;
  // Certainty (log prob) of (just) this position.
  float certainty;
  // Total certainty of the path to this position.
  float score;
  // The previous node in this chain. Borrowed pointer.
  const RecodeNode *prev;
  // The currently active dawgs at this position. Owned pointer.
  DawgPositionVector *dawgs;
  // A hash of all codes in the prefix and this->code as well. Used for
  // duplicate path removal.
  uint64_t code_hash;
};

using RecodePair = KDPairInc<double, RecodeNode>;
using RecodeHeap = GenericHeap<RecodePair>;

// Class that holds the entire beam search for recognition of a text line.
class TESS_API RecodeBeamSearch {
public:
  // Borrows the pointer, which is expected to survive until *this is deleted.
  RecodeBeamSearch(const UnicharCompress &recoder, int null_char, bool simple_text, Dict *dict);
  ~RecodeBeamSearch();

  // Decodes the set of network outputs, storing the lattice internally.
  // If charset is not null, it enables detailed debugging of the beam search.
  void Decode(const NetworkIO &output, double dict_ratio, double cert_offset,
              double worst_dict_cert, const UNICHARSET *charset, int lstm_choice_mode = 0);
  void Decode(const GENERIC_2D_ARRAY<float> &output, double dict_ratio, double cert_offset,
              double worst_dict_cert, const UNICHARSET *charset);

  void DecodeSecondaryBeams(const NetworkIO &output, double dict_ratio, double cert_offset,
                            double worst_dict_cert, const UNICHARSET *charset,
                            int lstm_choice_mode = 0);

  // Returns the best path as labels/scores/xcoords similar to simple CTC.
  void ExtractBestPathAsLabels(std::vector<int> *labels, std::vector<int> *xcoords) const;
  // Returns the best path as unichar-ids/certs/ratings/xcoords skipping
  // duplicates, nulls and intermediate parts.
  void ExtractBestPathAsUnicharIds(bool debug, const UNICHARSET *unicharset,
                                   std::vector<int> *unichar_ids, std::vector<float> *certs,
                                   std::vector<float> *ratings, std::vector<int> *xcoords) const;

  // Returns the best path as a set of WERD_RES.
  void ExtractBestPathAsWords(const TBOX &line_box, float scale_factor, bool debug,
                              const UNICHARSET *unicharset, PointerVector<WERD_RES> *words,
                              int lstm_choice_mode = 0);

  // Generates debug output of the content of the beams after a Decode.
  void DebugBeams(const UNICHARSET &unicharset) const;

  // Extract the best characters from the current decode iteration and block
  // those symbols for the next iteration. In contrast to Tesseract's standard
  // method to chose the best overall node chain, this methods looks at a short
  // node chain segmented by the character boundaries and chooses the best
  // option independent of the remaining node chain.
  void extractSymbolChoices(const UNICHARSET *unicharset);

  // Generates debug output of the content of the beams after a Decode.
  void PrintBeam2(bool uids, int num_outputs, const UNICHARSET *charset, bool secondary) const;
  // Segments the timestep bundle by the character_boundaries.
  void segmentTimestepsByCharacters();
  std::vector<std::vector<std::pair<const char *, float>>>
  // Unions the segmented timestep character bundles to one big bundle.
  combineSegmentedTimesteps(
      std::vector<std::vector<std::vector<std::pair<const char *, float>>>> *segmentedTimesteps);
  // Stores the alternative characters of every timestep together with their
  // probability.
  std::vector<std::vector<std::pair<const char *, float>>> timesteps;
  std::vector<std::vector<std::vector<std::pair<const char *, float>>>> segmentedTimesteps;
  // Stores the character choices found in the ctc algorithm
  std::vector<std::vector<std::pair<const char *, float>>> ctc_choices;
  // Stores all unicharids which are excluded for future iterations
  std::vector<std::unordered_set<int>> excludedUnichars;
  // Stores the character boundaries regarding timesteps.
  std::vector<int> character_boundaries_;
  // Clipping value for certainty inside Tesseract. Reflects the minimum value
  // of certainty that will be returned by ExtractBestPathAsUnicharIds.
  // Supposedly on a uniform scale that can be compared across languages and
  // engines.
  static constexpr float kMinCertainty = -20.0f;
  // Number of different code lengths for which we have a separate beam.
  static const int kNumLengths = RecodedCharID::kMaxCodeLen + 1;
  // Total number of beams: dawg/nodawg * number of NodeContinuation * number
  // of different lengths.
  static const int kNumBeams = 2 * NC_COUNT * kNumLengths;
  // Returns the relevant factor in the beams_ index.
  static int LengthFromBeamsIndex(int index) {
    return index % kNumLengths;
  }
  static NodeContinuation ContinuationFromBeamsIndex(int index) {
    return static_cast<NodeContinuation>((index / kNumLengths) % NC_COUNT);
  }
  static bool IsDawgFromBeamsIndex(int index) {
    return index / (kNumLengths * NC_COUNT) > 0;
  }
  // Computes a beams_ index from the given factors.
  static int BeamIndex(bool is_dawg, NodeContinuation cont, int length) {
    return (is_dawg * NC_COUNT + cont) * kNumLengths + length;
  }

private:
  // Struct for the Re-encode beam search. This struct holds the data for
  // a single time-step position of the output. Use a vector<RecodeBeam>
  // to hold all the timesteps and prevent reallocation of the individual heaps.
  struct RecodeBeam {
    // Resets to the initial state without deleting all the memory.
    void Clear() {
      for (auto &beam : beams_) {
        beam.clear();
      }
      RecodeNode empty;
      for (auto &best_initial_dawg : best_initial_dawgs_) {
        best_initial_dawg = empty;
      }
    }

    // A separate beam for each combination of code length,
    // NodeContinuation, and dictionary flag. Separating out all these types
    // allows the beam to be quite narrow, and yet still have a low chance of
    // losing the best path.
    // We have to keep all these beams separate, since the highest scoring paths
    // come from the paths that are most likely to dead-end at any time, like
    // dawg paths, NC_ONLY_DUP etc.
    // Each heap is stored with the WORST result at the top, so we can quickly
    // get the top-n values.
    RecodeHeap beams_[kNumBeams];
    // While the language model is only a single word dictionary, we can use
    // word starts as a choke point in the beam, and keep only a single dict
    // start node at each step (for each NodeContinuation type), so we find the
    // best one here and push it on the heap, if it qualifies, after processing
    // all of the step.
    RecodeNode best_initial_dawgs_[NC_COUNT];
  };
  using TopPair = KDPairInc<float, int>;

  // Generates debug output of the content of a single beam position.
  void DebugBeamPos(const UNICHARSET &unicharset, const RecodeHeap &heap) const;

  // Returns the given best_nodes as unichar-ids/certs/ratings/xcoords skipping
  // duplicates, nulls and intermediate parts.
  static void ExtractPathAsUnicharIds(const std::vector<const RecodeNode *> &best_nodes,
                                      std::vector<int> *unichar_ids, std::vector<float> *certs,
                                      std::vector<float> *ratings, std::vector<int> *xcoords,
                                      std::vector<int> *character_boundaries = nullptr);

  // Sets up a word with the ratings matrix and fake blobs with boxes in the
  // right places.
  WERD_RES *InitializeWord(bool leading_space, const TBOX &line_box, int word_start, int word_end,
                           float space_certainty, const UNICHARSET *unicharset,
                           const std::vector<int> &xcoords, float scale_factor);

  // Fills top_n_flags_ with bools that are true iff the corresponding output
  // is one of the top_n.
  void ComputeTopN(const float *outputs, int num_outputs, int top_n);

  void ComputeSecTopN(std::unordered_set<int> *exList, const float *outputs, int num_outputs,
                      int top_n);

  // Adds the computation for the current time-step to the beam. Call at each
  // time-step in sequence from left to right. outputs is the activation vector
  // for the current timestep.
  void DecodeStep(const float *outputs, int t, double dict_ratio, double cert_offset,
                  double worst_dict_cert, const UNICHARSET *charset, bool debug = false);

  void DecodeSecondaryStep(const float *outputs, int t, double dict_ratio, double cert_offset,
                           double worst_dict_cert, const UNICHARSET *charset, bool debug = false);

  // Saves the most certain choices for the current time-step.
  void SaveMostCertainChoices(const float *outputs, int num_outputs, const UNICHARSET *charset,
                              int xCoord);

  // Calculates more accurate character boundaries which can be used to
  // provide more accurate alternative symbol choices.
  static void calculateCharBoundaries(std::vector<int> *starts, std::vector<int> *ends,
                                      std::vector<int> *character_boundaries_, int maxWidth);

  // Adds to the appropriate beams the legal (according to recoder)
  // continuations of context prev, which is from the given index to beams_,
  // using the given network outputs to provide scores to the choices. Uses only
  // those choices for which top_n_flags[code] == top_n_flag.
  void ContinueContext(const RecodeNode *prev, int index, const float *outputs,
                       TopNState top_n_flag, const UNICHARSET *unicharset, double dict_ratio,
                       double cert_offset, double worst_dict_cert, RecodeBeam *step);
  // Continues for a new unichar, using dawg or non-dawg as per flag.
  void ContinueUnichar(int code, int unichar_id, float cert, float worst_dict_cert,
                       float dict_ratio, bool use_dawgs, NodeContinuation cont,
                       const RecodeNode *prev, RecodeBeam *step);
  // Adds a RecodeNode composed of the args to the correct heap in step if
  // unichar_id is a valid dictionary continuation of whatever is in prev.
  void ContinueDawg(int code, int unichar_id, float cert, NodeContinuation cont,
                    const RecodeNode *prev, RecodeBeam *step);
  // Sets the correct best_initial_dawgs_ with a RecodeNode composed of the args
  // if better than what is already there.
  void PushInitialDawgIfBetter(int code, int unichar_id, PermuterType permuter, bool start,
                               bool end, float cert, NodeContinuation cont, const RecodeNode *prev,
                               RecodeBeam *step);
  // Adds a RecodeNode composed of the args to the correct heap in step for
  // partial unichar or duplicate if there is room or if better than the
  // current worst element if already full.
  void PushDupOrNoDawgIfBetter(int length, bool dup, int code, int unichar_id, float cert,
                               float worst_dict_cert, float dict_ratio, bool use_dawgs,
                               NodeContinuation cont, const RecodeNode *prev, RecodeBeam *step);
  // Adds a RecodeNode composed of the args to the correct heap in step if there
  // is room or if better than the current worst element if already full.
  void PushHeapIfBetter(int max_size, int code, int unichar_id, PermuterType permuter,
                        bool dawg_start, bool word_start, bool end, bool dup, float cert,
                        const RecodeNode *prev, DawgPositionVector *d, RecodeHeap *heap);
  // Adds a RecodeNode to heap if there is room
  // or if better than the current worst element if already full.
  void PushHeapIfBetter(int max_size, RecodeNode *node, RecodeHeap *heap);
  // Searches the heap for an entry matching new_node, and updates the entry
  // with reshuffle if needed. Returns true if there was a match.
  bool UpdateHeapIfMatched(RecodeNode *new_node, RecodeHeap *heap);
  // Computes and returns the code-hash for the given code and prev.
  uint64_t ComputeCodeHash(int code, bool dup, const RecodeNode *prev) const;
  // Backtracks to extract the best path through the lattice that was built
  // during Decode. On return the best_nodes vector essentially contains the set
  // of code, score pairs that make the optimal path with the constraint that
  // the recoder can decode the code sequence back to a sequence of unichar-ids.
  void ExtractBestPaths(std::vector<const RecodeNode *> *best_nodes,
                        std::vector<const RecodeNode *> *second_nodes) const;
  // Helper backtracks through the lattice from the given node, storing the
  // path and reversing it.
  void ExtractPath(const RecodeNode *node, std::vector<const RecodeNode *> *path) const;
  void ExtractPath(const RecodeNode *node, std::vector<const RecodeNode *> *path,
                   int limiter) const;
  // Helper prints debug information on the given lattice path.
  void DebugPath(const UNICHARSET *unicharset, const std::vector<const RecodeNode *> &path) const;
  // Helper prints debug information on the given unichar path.
  void DebugUnicharPath(const UNICHARSET *unicharset, const std::vector<const RecodeNode *> &path,
                        const std::vector<int> &unichar_ids, const std::vector<float> &certs,
                        const std::vector<float> &ratings, const std::vector<int> &xcoords) const;

  static const int kBeamWidths[RecodedCharID::kMaxCodeLen + 1];

  // The encoder/decoder that we will be using.
  const UnicharCompress &recoder_;
  // The beam for each timestep in the output.
  std::vector<RecodeBeam *> beam_;
  // Secondary Beam for Results with less Probability
  std::vector<RecodeBeam *> secondary_beam_;
  // The number of timesteps valid in beam_;
  int beam_size_;
  // A flag to indicate which outputs are the top-n choices. Current timestep
  // only.
  std::vector<TopNState> top_n_flags_;
  // A record of the highest and second scoring codes.
  int top_code_;
  int second_code_;
  // Heap used to compute the top_n_flags_.
  GenericHeap<TopPair> top_heap_;
  // Borrowed pointer to the dictionary to use in the search.
  Dict *dict_;
  // True if the language is space-delimited, which is true for most languages
  // except chi*, jpn, tha.
  bool space_delimited_;
  // True if the input is simple text, ie adjacent equal chars are not to be
  // eliminated.
  bool is_simple_text_;
  // The encoded (class label) of the null/reject character.
  int null_char_;
};

} // namespace tesseract.

#endif // THIRD_PARTY_TESSERACT_LSTM_RECODEBEAM_H_
