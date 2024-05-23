///////////////////////////////////////////////////////////////////////
// File:        recodebeam.cpp
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

#include "recodebeam.h"

#include "networkio.h"
#include "pageres.h"
#include "unicharcompress.h"

#include <algorithm> // for std::reverse

namespace tesseract {

// The beam width at each code position.
const int RecodeBeamSearch::kBeamWidths[RecodedCharID::kMaxCodeLen + 1] = {
    5, 10, 16, 16, 16, 16, 16, 16, 16, 16,
};

static const char *kNodeContNames[] = {"Anything", "OnlyDup", "NoDup"};

// Prints debug details of the node.
void RecodeNode::Print(int null_char, const UNICHARSET &unicharset,
                       int depth) const {
  if (code == null_char) {
    tprintf("null_char");
  } else {
    tprintf("label=%d, uid=%d=%s", code, unichar_id,
            unicharset.debug_str(unichar_id).c_str());
  }
  tprintf(" score=%g, c=%g,%s%s%s perm=%d, hash=%" PRIx64, score, certainty,
          start_of_dawg ? " DawgStart" : "", start_of_word ? " Start" : "",
          end_of_word ? " End" : "", permuter, code_hash);
  if (depth > 0 && prev != nullptr) {
    tprintf(" prev:");
    prev->Print(null_char, unicharset, depth - 1);
  } else {
    tprintf("\n");
  }
}

// Borrows the pointer, which is expected to survive until *this is deleted.
RecodeBeamSearch::RecodeBeamSearch(const UnicharCompress &recoder,
                                   int null_char, bool simple_text, Dict *dict)
    : recoder_(recoder),
      beam_size_(0),
      top_code_(-1),
      second_code_(-1),
      dict_(dict),
      space_delimited_(true),
      is_simple_text_(simple_text),
      null_char_(null_char) {
  if (dict_ != nullptr && !dict_->IsSpaceDelimitedLang()) {
    space_delimited_ = false;
  }
}

RecodeBeamSearch::~RecodeBeamSearch() {
  for (auto data : beam_) {
    delete data;
  }
  for (auto data : secondary_beam_) {
    delete data;
  }
}

// Decodes the set of network outputs, storing the lattice internally.
void RecodeBeamSearch::Decode(const NetworkIO &output, double dict_ratio,
                              double cert_offset, double worst_dict_cert,
                              const UNICHARSET *charset, int lstm_choice_mode) {
  beam_size_ = 0;
  int width = output.Width();
  if (lstm_choice_mode) {
    timesteps.clear();
  }
  for (int t = 0; t < width; ++t) {
    ComputeTopN(output.f(t), output.NumFeatures(), kBeamWidths[0]);
    DecodeStep(output.f(t), t, dict_ratio, cert_offset, worst_dict_cert,
               charset);
    if (lstm_choice_mode) {
      SaveMostCertainChoices(output.f(t), output.NumFeatures(), charset, t);
    }
  }
}
void RecodeBeamSearch::Decode(const GENERIC_2D_ARRAY<float> &output,
                              double dict_ratio, double cert_offset,
                              double worst_dict_cert,
                              const UNICHARSET *charset) {
  beam_size_ = 0;
  int width = output.dim1();
  for (int t = 0; t < width; ++t) {
    ComputeTopN(output[t], output.dim2(), kBeamWidths[0]);
    DecodeStep(output[t], t, dict_ratio, cert_offset, worst_dict_cert, charset);
  }
}

void RecodeBeamSearch::DecodeSecondaryBeams(
    const NetworkIO &output, double dict_ratio, double cert_offset,
    double worst_dict_cert, const UNICHARSET *charset, int lstm_choice_mode) {
  for (auto data : secondary_beam_) {
    delete data;
  }
  secondary_beam_.clear();
  if (character_boundaries_.size() < 2) {
    return;
  }
  int width = output.Width();
  unsigned bucketNumber = 0;
  for (int t = 0; t < width; ++t) {
    while ((bucketNumber + 1) < character_boundaries_.size() &&
           t >= character_boundaries_[bucketNumber + 1]) {
      ++bucketNumber;
    }
    ComputeSecTopN(&(excludedUnichars)[bucketNumber], output.f(t),
                   output.NumFeatures(), kBeamWidths[0]);
    DecodeSecondaryStep(output.f(t), t, dict_ratio, cert_offset,
                        worst_dict_cert, charset);
  }
}

void RecodeBeamSearch::SaveMostCertainChoices(const float *outputs,
                                              int num_outputs,
                                              const UNICHARSET *charset,
                                              int xCoord) {
  std::vector<std::pair<const char *, float>> choices;
  for (int i = 0; i < num_outputs; ++i) {
    if (outputs[i] >= 0.01f) {
      const char *character;
      if (i + 2 >= num_outputs) {
        character = "";
      } else if (i > 0) {
        character = charset->id_to_unichar_ext(i + 2);
      } else {
        character = charset->id_to_unichar_ext(i);
      }
      size_t pos = 0;
      // order the possible choices within one timestep
      // beginning with the most likely
      while (choices.size() > pos && choices[pos].second > outputs[i]) {
        pos++;
      }
      choices.insert(choices.begin() + pos,
                     std::pair<const char *, float>(character, outputs[i]));
    }
  }
  timesteps.push_back(choices);
}

void RecodeBeamSearch::segmentTimestepsByCharacters() {
  for (unsigned i = 1; i < character_boundaries_.size(); ++i) {
    std::vector<std::vector<std::pair<const char *, float>>> segment;
    for (int j = character_boundaries_[i - 1]; j < character_boundaries_[i];
         ++j) {
      segment.push_back(timesteps[j]);
    }
    segmentedTimesteps.push_back(segment);
  }
}
std::vector<std::vector<std::pair<const char *, float>>>
RecodeBeamSearch::combineSegmentedTimesteps(
    std::vector<std::vector<std::vector<std::pair<const char *, float>>>>
        *segmentedTimesteps) {
  std::vector<std::vector<std::pair<const char *, float>>> combined_timesteps;
  for (auto &segmentedTimestep : *segmentedTimesteps) {
    for (auto &j : segmentedTimestep) {
      combined_timesteps.push_back(j);
    }
  }
  return combined_timesteps;
}

void RecodeBeamSearch::calculateCharBoundaries(std::vector<int> *starts,
                                               std::vector<int> *ends,
                                               std::vector<int> *char_bounds_,
                                               int maxWidth) {
  char_bounds_->push_back(0);
  for (unsigned i = 0; i < ends->size(); ++i) {
    int middle = ((*starts)[i + 1] - (*ends)[i]) / 2;
    char_bounds_->push_back((*ends)[i] + middle);
  }
  char_bounds_->pop_back();
  char_bounds_->push_back(maxWidth);
}

// Returns the best path as labels/scores/xcoords similar to simple CTC.
void RecodeBeamSearch::ExtractBestPathAsLabels(
    std::vector<int> *labels, std::vector<int> *xcoords) const {
  labels->clear();
  xcoords->clear();
  std::vector<const RecodeNode *> best_nodes;
  ExtractBestPaths(&best_nodes, nullptr);
  // Now just run CTC on the best nodes.
  int t = 0;
  int width = best_nodes.size();
  while (t < width) {
    int label = best_nodes[t]->code;
    if (label != null_char_) {
      labels->push_back(label);
      xcoords->push_back(t);
    }
    while (++t < width && !is_simple_text_ && best_nodes[t]->code == label) {
    }
  }
  xcoords->push_back(width);
}

// Returns the best path as unichar-ids/certs/ratings/xcoords skipping
// duplicates, nulls and intermediate parts.
void RecodeBeamSearch::ExtractBestPathAsUnicharIds(
    bool debug, const UNICHARSET *unicharset, std::vector<int> *unichar_ids,
    std::vector<float> *certs, std::vector<float> *ratings,
    std::vector<int> *xcoords) const {
  std::vector<const RecodeNode *> best_nodes;
  ExtractBestPaths(&best_nodes, nullptr);
  ExtractPathAsUnicharIds(best_nodes, unichar_ids, certs, ratings, xcoords);
  if (debug) {
    DebugPath(unicharset, best_nodes);
    DebugUnicharPath(unicharset, best_nodes, *unichar_ids, *certs, *ratings,
                     *xcoords);
  }
}

// Returns the best path as a set of WERD_RES.
void RecodeBeamSearch::ExtractBestPathAsWords(const TBOX &line_box,
                                              float scale_factor, bool debug,
                                              const UNICHARSET *unicharset,
                                              PointerVector<WERD_RES> *words,
                                              int lstm_choice_mode) {
  words->truncate(0);
  std::vector<int> unichar_ids;
  std::vector<float> certs;
  std::vector<float> ratings;
  std::vector<int> xcoords;
  std::vector<const RecodeNode *> best_nodes;
  std::vector<const RecodeNode *> second_nodes;
  character_boundaries_.clear();
  ExtractBestPaths(&best_nodes, &second_nodes);
  if (debug) {
    DebugPath(unicharset, best_nodes);
    ExtractPathAsUnicharIds(second_nodes, &unichar_ids, &certs, &ratings,
                            &xcoords);
    tprintf("\nSecond choice path:\n");
    DebugUnicharPath(unicharset, second_nodes, unichar_ids, certs, ratings,
                     xcoords);
  }
  // If lstm choice mode is required in granularity level 2, it stores the x
  // Coordinates of every chosen character, to match the alternative choices to
  // it.
  ExtractPathAsUnicharIds(best_nodes, &unichar_ids, &certs, &ratings, &xcoords,
                          &character_boundaries_);
  int num_ids = unichar_ids.size();
  if (debug) {
    DebugUnicharPath(unicharset, best_nodes, unichar_ids, certs, ratings,
                     xcoords);
  }
  // Convert labels to unichar-ids.
  int word_end = 0;
  float prev_space_cert = 0.0f;
  for (int word_start = 0; word_start < num_ids; word_start = word_end) {
    for (word_end = word_start + 1; word_end < num_ids; ++word_end) {
      // A word is terminated when a space character or start_of_word flag is
      // hit. We also want to force a separate word for every non
      // space-delimited character when not in a dictionary context.
      if (unichar_ids[word_end] == UNICHAR_SPACE) {
        break;
      }
      int index = xcoords[word_end];
      if (best_nodes[index]->start_of_word) {
        break;
      }
      if (best_nodes[index]->permuter == TOP_CHOICE_PERM &&
          (!unicharset->IsSpaceDelimited(unichar_ids[word_end]) ||
           !unicharset->IsSpaceDelimited(unichar_ids[word_end - 1]))) {
        break;
      }
    }
    float space_cert = 0.0f;
    if (word_end < num_ids && unichar_ids[word_end] == UNICHAR_SPACE) {
      space_cert = certs[word_end];
    }
    bool leading_space =
        word_start > 0 && unichar_ids[word_start - 1] == UNICHAR_SPACE;
    // Create a WERD_RES for the output word.
    WERD_RES *word_res =
        InitializeWord(leading_space, line_box, word_start, word_end,
                       std::min(space_cert, prev_space_cert), unicharset,
                       xcoords, scale_factor);
    for (int i = word_start; i < word_end; ++i) {
      auto *choices = new BLOB_CHOICE_LIST;
      BLOB_CHOICE_IT bc_it(choices);
      auto *choice = new BLOB_CHOICE(unichar_ids[i], ratings[i], certs[i], -1,
                                     1.0f, static_cast<float>(INT16_MAX), 0.0f,
                                     BCC_STATIC_CLASSIFIER);
      int col = i - word_start;
      choice->set_matrix_cell(col, col);
      bc_it.add_after_then_move(choice);
      word_res->ratings->put(col, col, choices);
    }
    int index = xcoords[word_end - 1];
    word_res->FakeWordFromRatings(best_nodes[index]->permuter);
    words->push_back(word_res);
    prev_space_cert = space_cert;
    if (word_end < num_ids && unichar_ids[word_end] == UNICHAR_SPACE) {
      ++word_end;
    }
  }
}

struct greater_than {
  inline bool operator()(const RecodeNode *&node1, const RecodeNode *&node2) const {
    return (node1->score > node2->score);
  }
};

void RecodeBeamSearch::PrintBeam2(bool uids, int num_outputs,
                                  const UNICHARSET *charset,
                                  bool secondary) const {
  std::vector<std::vector<const RecodeNode *>> topology;
  std::unordered_set<const RecodeNode *> visited;
  const std::vector<RecodeBeam *> &beam = !secondary ? beam_ : secondary_beam_;
  // create the topology
  for (int step = beam.size() - 1; step >= 0; --step) {
    std::vector<const RecodeNode *> layer;
    topology.push_back(layer);
  }
  // fill the topology with depths first
  for (int step = beam.size() - 1; step >= 0; --step) {
    std::vector<tesseract::RecodePair> &heaps = beam.at(step)->beams_->heap();
    for (auto &&node : heaps) {
      int backtracker = 0;
      const RecodeNode *curr = &node.data();
      while (curr != nullptr && !visited.count(curr)) {
        visited.insert(curr);
        topology[step - backtracker].push_back(curr);
        curr = curr->prev;
        ++backtracker;
      }
    }
  }
  int ct = 0;
  unsigned cb = 1;
  for (const std::vector<const RecodeNode *> &layer : topology) {
    if (cb >= character_boundaries_.size()) {
      break;
    }
    if (ct == character_boundaries_[cb]) {
      tprintf("***\n");
      ++cb;
    }
    for (const RecodeNode *node : layer) {
      const char *code;
      int intCode;
      if (node->unichar_id != INVALID_UNICHAR_ID) {
        code = charset->id_to_unichar(node->unichar_id);
        intCode = node->unichar_id;
      } else if (node->code == null_char_) {
        intCode = 0;
        code = " ";
      } else {
        intCode = 666;
        code = "*";
      }
      int intPrevCode = 0;
      const char *prevCode;
      float prevScore = 0;
      if (node->prev != nullptr) {
        prevScore = node->prev->score;
        if (node->prev->unichar_id != INVALID_UNICHAR_ID) {
          prevCode = charset->id_to_unichar(node->prev->unichar_id);
          intPrevCode = node->prev->unichar_id;
        } else if (node->code == null_char_) {
          intPrevCode = 0;
          prevCode = " ";
        } else {
          prevCode = "*";
          intPrevCode = 666;
        }
      } else {
        prevCode = " ";
      }
      if (uids) {
        tprintf("%x(|)%f(>)%x(|)%f\n", intPrevCode, prevScore, intCode,
                node->score);
      } else {
        tprintf("%s(|)%f(>)%s(|)%f\n", prevCode, prevScore, code, node->score);
      }
    }
    tprintf("-\n");
    ++ct;
  }
  tprintf("***\n");
}

void RecodeBeamSearch::extractSymbolChoices(const UNICHARSET *unicharset) {
  if (character_boundaries_.size() < 2) {
    return;
  }
  // For the first iteration the original beam is analyzed. After that a
  // new beam is calculated based on the results from the original beam.
  std::vector<RecodeBeam *> &currentBeam =
      secondary_beam_.empty() ? beam_ : secondary_beam_;
  character_boundaries_[0] = 0;
  for (unsigned j = 1; j < character_boundaries_.size(); ++j) {
    std::vector<int> unichar_ids;
    std::vector<float> certs;
    std::vector<float> ratings;
    std::vector<int> xcoords;
    int backpath = character_boundaries_[j] - character_boundaries_[j - 1];
    std::vector<tesseract::RecodePair> &heaps =
        currentBeam.at(character_boundaries_[j] - 1)->beams_->heap();
    std::vector<const RecodeNode *> best_nodes;
    std::vector<const RecodeNode *> best;
    // Scan the segmented node chain for valid unichar ids.
    for (auto &&entry : heaps) {
      bool validChar = false;
      int backcounter = 0;
      const RecodeNode *node = &entry.data();
      while (node != nullptr && backcounter < backpath) {
        if (node->code != null_char_ &&
            node->unichar_id != INVALID_UNICHAR_ID) {
          validChar = true;
          break;
        }
        node = node->prev;
        ++backcounter;
      }
      if (validChar) {
        best.push_back(&entry.data());
      }
    }
    // find the best rated segmented node chain and extract the unichar id.
    if (!best.empty()) {
      std::sort(best.begin(), best.end(), greater_than());
      ExtractPath(best[0], &best_nodes, backpath);
      ExtractPathAsUnicharIds(best_nodes, &unichar_ids, &certs, &ratings,
                              &xcoords);
    }
    if (!unichar_ids.empty()) {
      int bestPos = 0;
      for (unsigned i = 1; i < unichar_ids.size(); ++i) {
        if (ratings[i] < ratings[bestPos]) {
          bestPos = i;
        }
      }
#if 0 // TODO: bestCode is currently unused (see commit 2dd5d0d60).
      int bestCode = -10;
      for (auto &node : best_nodes) {
        if (node->unichar_id == unichar_ids[bestPos]) {
          bestCode = node->code;
        }
      }
#endif
      // Exclude the best choice for the followup decoding.
      std::unordered_set<int> excludeCodeList;
      for (auto &best_node : best_nodes) {
        if (best_node->code != null_char_) {
          excludeCodeList.insert(best_node->code);
        }
      }
      if (j - 1 < excludedUnichars.size()) {
        for (auto elem : excludeCodeList) {
          excludedUnichars[j - 1].insert(elem);
        }
      } else {
        excludedUnichars.push_back(excludeCodeList);
      }
      // Save the best choice for the choice iterator.
      if (j - 1 < ctc_choices.size()) {
        int id = unichar_ids[bestPos];
        const char *result = unicharset->id_to_unichar_ext(id);
        float rating = ratings[bestPos];
        ctc_choices[j - 1].push_back(
            std::pair<const char *, float>(result, rating));
      } else {
        std::vector<std::pair<const char *, float>> choice;
        int id = unichar_ids[bestPos];
        const char *result = unicharset->id_to_unichar_ext(id);
        float rating = ratings[bestPos];
        choice.emplace_back(result, rating);
        ctc_choices.push_back(choice);
      }
      // fill the blank spot with an empty array
    } else {
      if (j - 1 >= excludedUnichars.size()) {
        std::unordered_set<int> excludeCodeList;
        excludedUnichars.push_back(excludeCodeList);
      }
      if (j - 1 >= ctc_choices.size()) {
        std::vector<std::pair<const char *, float>> choice;
        ctc_choices.push_back(choice);
      }
    }
  }
  for (auto data : secondary_beam_) {
    delete data;
  }
  secondary_beam_.clear();
}

// Generates debug output of the content of the beams after a Decode.
void RecodeBeamSearch::DebugBeams(const UNICHARSET &unicharset) const {
  for (int p = 0; p < beam_size_; ++p) {
    for (int d = 0; d < 2; ++d) {
      for (int c = 0; c < NC_COUNT; ++c) {
        auto cont = static_cast<NodeContinuation>(c);
        int index = BeamIndex(d, cont, 0);
        if (beam_[p]->beams_[index].empty()) {
          continue;
        }
        // Print all the best scoring nodes for each unichar found.
        tprintf("Position %d: %s+%s beam\n", p, d ? "Dict" : "Non-Dict",
                kNodeContNames[c]);
        DebugBeamPos(unicharset, beam_[p]->beams_[index]);
      }
    }
  }
}

// Generates debug output of the content of a single beam position.
void RecodeBeamSearch::DebugBeamPos(const UNICHARSET &unicharset,
                                    const RecodeHeap &heap) const {
  std::vector<const RecodeNode *> unichar_bests(unicharset.size());
  const RecodeNode *null_best = nullptr;
  int heap_size = heap.size();
  for (int i = 0; i < heap_size; ++i) {
    const RecodeNode *node = &heap.get(i).data();
    if (node->unichar_id == INVALID_UNICHAR_ID) {
      if (null_best == nullptr || null_best->score < node->score) {
        null_best = node;
      }
    } else {
      if (unichar_bests[node->unichar_id] == nullptr ||
          unichar_bests[node->unichar_id]->score < node->score) {
        unichar_bests[node->unichar_id] = node;
      }
    }
  }
  for (auto &unichar_best : unichar_bests) {
    if (unichar_best != nullptr) {
      const RecodeNode &node = *unichar_best;
      node.Print(null_char_, unicharset, 1);
    }
  }
  if (null_best != nullptr) {
    null_best->Print(null_char_, unicharset, 1);
  }
}

// Returns the given best_nodes as unichar-ids/certs/ratings/xcoords skipping
// duplicates, nulls and intermediate parts.
/* static */
void RecodeBeamSearch::ExtractPathAsUnicharIds(
    const std::vector<const RecodeNode *> &best_nodes,
    std::vector<int> *unichar_ids, std::vector<float> *certs,
    std::vector<float> *ratings, std::vector<int> *xcoords,
    std::vector<int> *character_boundaries) {
  unichar_ids->clear();
  certs->clear();
  ratings->clear();
  xcoords->clear();
  std::vector<int> starts;
  std::vector<int> ends;
  // Backtrack extracting only valid, non-duplicate unichar-ids.
  int t = 0;
  int width = best_nodes.size();
  while (t < width) {
    double certainty = 0.0;
    double rating = 0.0;
    while (t < width && best_nodes[t]->unichar_id == INVALID_UNICHAR_ID) {
      double cert = best_nodes[t++]->certainty;
      if (cert < certainty) {
        certainty = cert;
      }
      rating -= cert;
    }
    starts.push_back(t);
    if (t < width) {
      int unichar_id = best_nodes[t]->unichar_id;
      if (unichar_id == UNICHAR_SPACE && !certs->empty() &&
          best_nodes[t]->permuter != NO_PERM) {
        // All the rating and certainty go on the previous character except
        // for the space itself.
        if (certainty < certs->back()) {
          certs->back() = certainty;
        }
        ratings->back() += rating;
        certainty = 0.0;
        rating = 0.0;
      }
      unichar_ids->push_back(unichar_id);
      xcoords->push_back(t);
      do {
        double cert = best_nodes[t++]->certainty;
        // Special-case NO-PERM space to forget the certainty of the previous
        // nulls. See long comment in ContinueContext.
        if (cert < certainty || (unichar_id == UNICHAR_SPACE &&
                                 best_nodes[t - 1]->permuter == NO_PERM)) {
          certainty = cert;
        }
        rating -= cert;
      } while (t < width && best_nodes[t]->duplicate);
      ends.push_back(t);
      certs->push_back(certainty);
      ratings->push_back(rating);
    } else if (!certs->empty()) {
      if (certainty < certs->back()) {
        certs->back() = certainty;
      }
      ratings->back() += rating;
    }
  }
  starts.push_back(width);
  if (character_boundaries != nullptr) {
    calculateCharBoundaries(&starts, &ends, character_boundaries, width);
  }
  xcoords->push_back(width);
}

// Sets up a word with the ratings matrix and fake blobs with boxes in the
// right places.
WERD_RES *RecodeBeamSearch::InitializeWord(bool leading_space,
                                           const TBOX &line_box, int word_start,
                                           int word_end, float space_certainty,
                                           const UNICHARSET *unicharset,
                                           const std::vector<int> &xcoords,
                                           float scale_factor) {
  // Make a fake blob for each non-zero label.
  C_BLOB_LIST blobs;
  C_BLOB_IT b_it(&blobs);
  for (int i = word_start; i < word_end; ++i) {
    if (static_cast<unsigned>(i + 1) < character_boundaries_.size()) {
      TBOX box(static_cast<int16_t>(
                   std::floor(character_boundaries_[i] * scale_factor)) +
                   line_box.left(),
               line_box.bottom(),
               static_cast<int16_t>(
                   std::ceil(character_boundaries_[i + 1] * scale_factor)) +
                   line_box.left(),
               line_box.top());
      b_it.add_after_then_move(C_BLOB::FakeBlob(box));
    }
  }
  // Make a fake word from the blobs.
  WERD *word = new WERD(&blobs, leading_space, nullptr);
  // Make a WERD_RES from the word.
  auto *word_res = new WERD_RES(word);
  word_res->end = word_end - word_start + leading_space;
  word_res->uch_set = unicharset;
  word_res->combination = true; // Give it ownership of the word.
  word_res->space_certainty = space_certainty;
  word_res->ratings = new MATRIX(word_end - word_start, 1);
  return word_res;
}

// Fills top_n_flags_ with bools that are true iff the corresponding output
// is one of the top_n.
void RecodeBeamSearch::ComputeTopN(const float *outputs, int num_outputs,
                                   int top_n) {
  top_n_flags_.clear();
  top_n_flags_.resize(num_outputs, TN_ALSO_RAN);
  top_code_ = -1;
  second_code_ = -1;
  top_heap_.clear();
  for (int i = 0; i < num_outputs; ++i) {
    if (top_heap_.size() < top_n || outputs[i] > top_heap_.PeekTop().key()) {
      TopPair entry(outputs[i], i);
      top_heap_.Push(&entry);
      if (top_heap_.size() > top_n) {
        top_heap_.Pop(&entry);
      }
    }
  }
  while (!top_heap_.empty()) {
    TopPair entry;
    top_heap_.Pop(&entry);
    if (top_heap_.size() > 1) {
      top_n_flags_[entry.data()] = TN_TOPN;
    } else {
      top_n_flags_[entry.data()] = TN_TOP2;
      if (top_heap_.empty()) {
        top_code_ = entry.data();
      } else {
        second_code_ = entry.data();
      }
    }
  }
  top_n_flags_[null_char_] = TN_TOP2;
}

void RecodeBeamSearch::ComputeSecTopN(std::unordered_set<int> *exList,
                                      const float *outputs, int num_outputs,
                                      int top_n) {
  top_n_flags_.clear();
  top_n_flags_.resize(num_outputs, TN_ALSO_RAN);
  top_code_ = -1;
  second_code_ = -1;
  top_heap_.clear();
  for (int i = 0; i < num_outputs; ++i) {
    if ((top_heap_.size() < top_n || outputs[i] > top_heap_.PeekTop().key()) &&
        !exList->count(i)) {
      TopPair entry(outputs[i], i);
      top_heap_.Push(&entry);
      if (top_heap_.size() > top_n) {
        top_heap_.Pop(&entry);
      }
    }
  }
  while (!top_heap_.empty()) {
    TopPair entry;
    top_heap_.Pop(&entry);
    if (top_heap_.size() > 1) {
      top_n_flags_[entry.data()] = TN_TOPN;
    } else {
      top_n_flags_[entry.data()] = TN_TOP2;
      if (top_heap_.empty()) {
        top_code_ = entry.data();
      } else {
        second_code_ = entry.data();
      }
    }
  }
  top_n_flags_[null_char_] = TN_TOP2;
}

// Adds the computation for the current time-step to the beam. Call at each
// time-step in sequence from left to right. outputs is the activation vector
// for the current timestep.
void RecodeBeamSearch::DecodeStep(const float *outputs, int t,
                                  double dict_ratio, double cert_offset,
                                  double worst_dict_cert,
                                  const UNICHARSET *charset, bool debug) {
  if (t == static_cast<int>(beam_.size())) {
    beam_.push_back(new RecodeBeam);
  }
  RecodeBeam *step = beam_[t];
  beam_size_ = t + 1;
  step->Clear();
  if (t == 0) {
    // The first step can only use singles and initials.
    ContinueContext(nullptr, BeamIndex(false, NC_ANYTHING, 0), outputs, TN_TOP2,
                    charset, dict_ratio, cert_offset, worst_dict_cert, step);
    if (dict_ != nullptr) {
      ContinueContext(nullptr, BeamIndex(true, NC_ANYTHING, 0), outputs,
                      TN_TOP2, charset, dict_ratio, cert_offset,
                      worst_dict_cert, step);
    }
  } else {
    RecodeBeam *prev = beam_[t - 1];
    if (debug) {
      int beam_index = BeamIndex(true, NC_ANYTHING, 0);
      for (int i = prev->beams_[beam_index].size() - 1; i >= 0; --i) {
        std::vector<const RecodeNode *> path;
        ExtractPath(&prev->beams_[beam_index].get(i).data(), &path);
        tprintf("Step %d: Dawg beam %d:\n", t, i);
        DebugPath(charset, path);
      }
      beam_index = BeamIndex(false, NC_ANYTHING, 0);
      for (int i = prev->beams_[beam_index].size() - 1; i >= 0; --i) {
        std::vector<const RecodeNode *> path;
        ExtractPath(&prev->beams_[beam_index].get(i).data(), &path);
        tprintf("Step %d: Non-Dawg beam %d:\n", t, i);
        DebugPath(charset, path);
      }
    }
    int total_beam = 0;
    // Work through the scores by group (top-2, top-n, the rest) while the beam
    // is empty. This enables extending the context using only the top-n results
    // first, which may have an empty intersection with the valid codes, so we
    // fall back to the rest if the beam is empty.
    for (int tn = 0; tn < TN_COUNT && total_beam == 0; ++tn) {
      auto top_n = static_cast<TopNState>(tn);
      for (int index = 0; index < kNumBeams; ++index) {
        // Working backwards through the heaps doesn't guarantee that we see the
        // best first, but it comes before a lot of the worst, so it is slightly
        // more efficient than going forwards.
        for (int i = prev->beams_[index].size() - 1; i >= 0; --i) {
          ContinueContext(&prev->beams_[index].get(i).data(), index, outputs,
                          top_n, charset, dict_ratio, cert_offset,
                          worst_dict_cert, step);
        }
      }
      for (int index = 0; index < kNumBeams; ++index) {
        if (ContinuationFromBeamsIndex(index) == NC_ANYTHING) {
          total_beam += step->beams_[index].size();
        }
      }
    }
    // Special case for the best initial dawg. Push it on the heap if good
    // enough, but there is only one, so it doesn't blow up the beam.
    for (int c = 0; c < NC_COUNT; ++c) {
      if (step->best_initial_dawgs_[c].code >= 0) {
        int index = BeamIndex(true, static_cast<NodeContinuation>(c), 0);
        RecodeHeap *dawg_heap = &step->beams_[index];
        PushHeapIfBetter(kBeamWidths[0], &step->best_initial_dawgs_[c],
                         dawg_heap);
      }
    }
  }
}

void RecodeBeamSearch::DecodeSecondaryStep(
    const float *outputs, int t, double dict_ratio, double cert_offset,
    double worst_dict_cert, const UNICHARSET *charset, bool debug) {
  if (t == static_cast<int>(secondary_beam_.size())) {
    secondary_beam_.push_back(new RecodeBeam);
  }
  RecodeBeam *step = secondary_beam_[t];
  step->Clear();
  if (t == 0) {
    // The first step can only use singles and initials.
    ContinueContext(nullptr, BeamIndex(false, NC_ANYTHING, 0), outputs, TN_TOP2,
                    charset, dict_ratio, cert_offset, worst_dict_cert, step);
    if (dict_ != nullptr) {
      ContinueContext(nullptr, BeamIndex(true, NC_ANYTHING, 0), outputs,
                      TN_TOP2, charset, dict_ratio, cert_offset,
                      worst_dict_cert, step);
    }
  } else {
    RecodeBeam *prev = secondary_beam_[t - 1];
    if (debug) {
      int beam_index = BeamIndex(true, NC_ANYTHING, 0);
      for (int i = prev->beams_[beam_index].size() - 1; i >= 0; --i) {
        std::vector<const RecodeNode *> path;
        ExtractPath(&prev->beams_[beam_index].get(i).data(), &path);
        tprintf("Step %d: Dawg beam %d:\n", t, i);
        DebugPath(charset, path);
      }
      beam_index = BeamIndex(false, NC_ANYTHING, 0);
      for (int i = prev->beams_[beam_index].size() - 1; i >= 0; --i) {
        std::vector<const RecodeNode *> path;
        ExtractPath(&prev->beams_[beam_index].get(i).data(), &path);
        tprintf("Step %d: Non-Dawg beam %d:\n", t, i);
        DebugPath(charset, path);
      }
    }
    int total_beam = 0;
    // Work through the scores by group (top-2, top-n, the rest) while the beam
    // is empty. This enables extending the context using only the top-n results
    // first, which may have an empty intersection with the valid codes, so we
    // fall back to the rest if the beam is empty.
    for (int tn = 0; tn < TN_COUNT && total_beam == 0; ++tn) {
      auto top_n = static_cast<TopNState>(tn);
      for (int index = 0; index < kNumBeams; ++index) {
        // Working backwards through the heaps doesn't guarantee that we see the
        // best first, but it comes before a lot of the worst, so it is slightly
        // more efficient than going forwards.
        for (int i = prev->beams_[index].size() - 1; i >= 0; --i) {
          ContinueContext(&prev->beams_[index].get(i).data(), index, outputs,
                          top_n, charset, dict_ratio, cert_offset,
                          worst_dict_cert, step);
        }
      }
      for (int index = 0; index < kNumBeams; ++index) {
        if (ContinuationFromBeamsIndex(index) == NC_ANYTHING) {
          total_beam += step->beams_[index].size();
        }
      }
    }
    // Special case for the best initial dawg. Push it on the heap if good
    // enough, but there is only one, so it doesn't blow up the beam.
    for (int c = 0; c < NC_COUNT; ++c) {
      if (step->best_initial_dawgs_[c].code >= 0) {
        int index = BeamIndex(true, static_cast<NodeContinuation>(c), 0);
        RecodeHeap *dawg_heap = &step->beams_[index];
        PushHeapIfBetter(kBeamWidths[0], &step->best_initial_dawgs_[c],
                         dawg_heap);
      }
    }
  }
}

// Adds to the appropriate beams the legal (according to recoder)
// continuations of context prev, which is of the given length, using the
// given network outputs to provide scores to the choices. Uses only those
// choices for which top_n_flags[index] == top_n_flag.
void RecodeBeamSearch::ContinueContext(
    const RecodeNode *prev, int index, const float *outputs,
    TopNState top_n_flag, const UNICHARSET *charset, double dict_ratio,
    double cert_offset, double worst_dict_cert, RecodeBeam *step) {
  RecodedCharID prefix;
  RecodedCharID full_code;
  const RecodeNode *previous = prev;
  int length = LengthFromBeamsIndex(index);
  bool use_dawgs = IsDawgFromBeamsIndex(index);
  NodeContinuation prev_cont = ContinuationFromBeamsIndex(index);
  for (int p = length - 1; p >= 0 && previous != nullptr; --p) {
    while (previous->duplicate || previous->code == null_char_) {
      previous = previous->prev;
    }
    prefix.Set(p, previous->code);
    full_code.Set(p, previous->code);
    previous = previous->prev;
  }
  if (prev != nullptr && !is_simple_text_) {
    if (top_n_flags_[prev->code] == top_n_flag) {
      if (prev_cont != NC_NO_DUP) {
        float cert =
            NetworkIO::ProbToCertainty(outputs[prev->code]) + cert_offset;
        PushDupOrNoDawgIfBetter(length, true, prev->code, prev->unichar_id,
                                cert, worst_dict_cert, dict_ratio, use_dawgs,
                                NC_ANYTHING, prev, step);
      }
      if (prev_cont == NC_ANYTHING && top_n_flag == TN_TOP2 &&
          prev->code != null_char_) {
        float cert = NetworkIO::ProbToCertainty(outputs[prev->code] +
                                                outputs[null_char_]) +
                     cert_offset;
        PushDupOrNoDawgIfBetter(length, true, prev->code, prev->unichar_id,
                                cert, worst_dict_cert, dict_ratio, use_dawgs,
                                NC_NO_DUP, prev, step);
      }
    }
    if (prev_cont == NC_ONLY_DUP) {
      return;
    }
    if (prev->code != null_char_ && length > 0 &&
        top_n_flags_[null_char_] == top_n_flag) {
      // Allow nulls within multi code sequences, as the nulls within are not
      // explicitly included in the code sequence.
      float cert =
          NetworkIO::ProbToCertainty(outputs[null_char_]) + cert_offset;
      PushDupOrNoDawgIfBetter(length, false, null_char_, INVALID_UNICHAR_ID,
                              cert, worst_dict_cert, dict_ratio, use_dawgs,
                              NC_ANYTHING, prev, step);
    }
  }
  const std::vector<int> *final_codes = recoder_.GetFinalCodes(prefix);
  if (final_codes != nullptr) {
    for (int code : *final_codes) {
      if (top_n_flags_[code] != top_n_flag) {
        continue;
      }
      if (prev != nullptr && prev->code == code && !is_simple_text_) {
        continue;
      }
      float cert = NetworkIO::ProbToCertainty(outputs[code]) + cert_offset;
      if (cert < kMinCertainty && code != null_char_) {
        continue;
      }
      full_code.Set(length, code);
      int unichar_id = recoder_.DecodeUnichar(full_code);
      // Map the null char to INVALID.
      if (length == 0 && code == null_char_) {
        unichar_id = INVALID_UNICHAR_ID;
      }
      if (unichar_id != INVALID_UNICHAR_ID && charset != nullptr &&
          !charset->get_enabled(unichar_id)) {
        continue; // disabled by whitelist/blacklist
      }
      ContinueUnichar(code, unichar_id, cert, worst_dict_cert, dict_ratio,
                      use_dawgs, NC_ANYTHING, prev, step);
      if (top_n_flag == TN_TOP2 && code != null_char_) {
        float prob = outputs[code] + outputs[null_char_];
        if (prev != nullptr && prev_cont == NC_ANYTHING &&
            prev->code != null_char_ &&
            ((prev->code == top_code_ && code == second_code_) ||
             (code == top_code_ && prev->code == second_code_))) {
          prob += outputs[prev->code];
        }
        cert = NetworkIO::ProbToCertainty(prob) + cert_offset;
        ContinueUnichar(code, unichar_id, cert, worst_dict_cert, dict_ratio,
                        use_dawgs, NC_ONLY_DUP, prev, step);
      }
    }
  }
  const std::vector<int> *next_codes = recoder_.GetNextCodes(prefix);
  if (next_codes != nullptr) {
    for (int code : *next_codes) {
      if (top_n_flags_[code] != top_n_flag) {
        continue;
      }
      if (prev != nullptr && prev->code == code && !is_simple_text_) {
        continue;
      }
      float cert = NetworkIO::ProbToCertainty(outputs[code]) + cert_offset;
      PushDupOrNoDawgIfBetter(length + 1, false, code, INVALID_UNICHAR_ID, cert,
                              worst_dict_cert, dict_ratio, use_dawgs,
                              NC_ANYTHING, prev, step);
      if (top_n_flag == TN_TOP2 && code != null_char_) {
        float prob = outputs[code] + outputs[null_char_];
        if (prev != nullptr && prev_cont == NC_ANYTHING &&
            prev->code != null_char_ &&
            ((prev->code == top_code_ && code == second_code_) ||
             (code == top_code_ && prev->code == second_code_))) {
          prob += outputs[prev->code];
        }
        cert = NetworkIO::ProbToCertainty(prob) + cert_offset;
        PushDupOrNoDawgIfBetter(length + 1, false, code, INVALID_UNICHAR_ID,
                                cert, worst_dict_cert, dict_ratio, use_dawgs,
                                NC_ONLY_DUP, prev, step);
      }
    }
  }
}

// Continues for a new unichar, using dawg or non-dawg as per flag.
void RecodeBeamSearch::ContinueUnichar(int code, int unichar_id, float cert,
                                       float worst_dict_cert, float dict_ratio,
                                       bool use_dawgs, NodeContinuation cont,
                                       const RecodeNode *prev,
                                       RecodeBeam *step) {
  if (use_dawgs) {
    if (cert > worst_dict_cert) {
      ContinueDawg(code, unichar_id, cert, cont, prev, step);
    }
  } else {
    RecodeHeap *nodawg_heap = &step->beams_[BeamIndex(false, cont, 0)];
    PushHeapIfBetter(kBeamWidths[0], code, unichar_id, TOP_CHOICE_PERM, false,
                     false, false, false, cert * dict_ratio, prev, nullptr,
                     nodawg_heap);
    if (dict_ != nullptr &&
        ((unichar_id == UNICHAR_SPACE && cert > worst_dict_cert) ||
         !dict_->getUnicharset().IsSpaceDelimited(unichar_id))) {
      // Any top choice position that can start a new word, ie a space or
      // any non-space-delimited character, should also be considered
      // by the dawg search, so push initial dawg to the dawg heap.
      float dawg_cert = cert;
      PermuterType permuter = TOP_CHOICE_PERM;
      // Since we use the space either side of a dictionary word in the
      // certainty of the word, (to properly handle weak spaces) and the
      // space is coming from a non-dict word, we need special conditions
      // to avoid degrading the certainty of the dict word that follows.
      // With a space we don't multiply the certainty by dict_ratio, and we
      // flag the space with NO_PERM to indicate that we should not use the
      // predecessor nulls to generate the confidence for the space, as they
      // have already been multiplied by dict_ratio, and we can't go back to
      // insert more entries in any previous heaps.
      if (unichar_id == UNICHAR_SPACE) {
        permuter = NO_PERM;
      } else {
        dawg_cert *= dict_ratio;
      }
      PushInitialDawgIfBetter(code, unichar_id, permuter, false, false,
                              dawg_cert, cont, prev, step);
    }
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, cert, prev,
// appropriate-dawg-args, cert) to the given heap (dawg_beam_) if unichar_id
// is a valid continuation of whatever is in prev.
void RecodeBeamSearch::ContinueDawg(int code, int unichar_id, float cert,
                                    NodeContinuation cont,
                                    const RecodeNode *prev, RecodeBeam *step) {
  RecodeHeap *dawg_heap = &step->beams_[BeamIndex(true, cont, 0)];
  RecodeHeap *nodawg_heap = &step->beams_[BeamIndex(false, cont, 0)];
  if (unichar_id == INVALID_UNICHAR_ID) {
    PushHeapIfBetter(kBeamWidths[0], code, unichar_id, NO_PERM, false, false,
                     false, false, cert, prev, nullptr, dawg_heap);
    return;
  }
  // Avoid dictionary probe if score a total loss.
  float score = cert;
  if (prev != nullptr) {
    score += prev->score;
  }
  if (dawg_heap->size() >= kBeamWidths[0] &&
      score <= dawg_heap->PeekTop().data().score &&
      nodawg_heap->size() >= kBeamWidths[0] &&
      score <= nodawg_heap->PeekTop().data().score) {
    return;
  }
  const RecodeNode *uni_prev = prev;
  // Prev may be a partial code, null_char, or duplicate, so scan back to the
  // last valid unichar_id.
  while (uni_prev != nullptr &&
         (uni_prev->unichar_id == INVALID_UNICHAR_ID || uni_prev->duplicate)) {
    uni_prev = uni_prev->prev;
  }
  if (unichar_id == UNICHAR_SPACE) {
    if (uni_prev != nullptr && uni_prev->end_of_word) {
      // Space is good. Push initial state, to the dawg beam and a regular
      // space to the top choice beam.
      PushInitialDawgIfBetter(code, unichar_id, uni_prev->permuter, false,
                              false, cert, cont, prev, step);
      PushHeapIfBetter(kBeamWidths[0], code, unichar_id, uni_prev->permuter,
                       false, false, false, false, cert, prev, nullptr,
                       nodawg_heap);
    }
    return;
  } else if (uni_prev != nullptr && uni_prev->start_of_dawg &&
             uni_prev->unichar_id != UNICHAR_SPACE &&
             dict_->getUnicharset().IsSpaceDelimited(uni_prev->unichar_id) &&
             dict_->getUnicharset().IsSpaceDelimited(unichar_id)) {
    return; // Can't break words between space delimited chars.
  }
  DawgPositionVector initial_dawgs;
  auto *updated_dawgs = new DawgPositionVector;
  DawgArgs dawg_args(&initial_dawgs, updated_dawgs, NO_PERM);
  bool word_start = false;
  if (uni_prev == nullptr) {
    // Starting from beginning of line.
    dict_->default_dawgs(&initial_dawgs, false);
    word_start = true;
  } else if (uni_prev->dawgs != nullptr) {
    // Continuing a previous dict word.
    dawg_args.active_dawgs = uni_prev->dawgs;
    word_start = uni_prev->start_of_dawg;
  } else {
    return; // Can't continue if not a dict word.
  }
  auto permuter = static_cast<PermuterType>(dict_->def_letter_is_okay(
      &dawg_args, dict_->getUnicharset(), unichar_id, false));
  if (permuter != NO_PERM) {
    PushHeapIfBetter(kBeamWidths[0], code, unichar_id, permuter, false,
                     word_start, dawg_args.valid_end, false, cert, prev,
                     dawg_args.updated_dawgs, dawg_heap);
    if (dawg_args.valid_end && !space_delimited_) {
      // We can start another word right away, so push initial state as well,
      // to the dawg beam, and the regular character to the top choice beam,
      // since non-dict words can start here too.
      PushInitialDawgIfBetter(code, unichar_id, permuter, word_start, true,
                              cert, cont, prev, step);
      PushHeapIfBetter(kBeamWidths[0], code, unichar_id, permuter, false,
                       word_start, true, false, cert, prev, nullptr,
                       nodawg_heap);
    }
  } else {
    delete updated_dawgs;
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id,
// initial-dawg-state, prev, cert) to the given heap if/ there is room or if
// better than the current worst element if already full.
void RecodeBeamSearch::PushInitialDawgIfBetter(int code, int unichar_id,
                                               PermuterType permuter,
                                               bool start, bool end, float cert,
                                               NodeContinuation cont,
                                               const RecodeNode *prev,
                                               RecodeBeam *step) {
  RecodeNode *best_initial_dawg = &step->best_initial_dawgs_[cont];
  float score = cert;
  if (prev != nullptr) {
    score += prev->score;
  }
  if (best_initial_dawg->code < 0 || score > best_initial_dawg->score) {
    auto *initial_dawgs = new DawgPositionVector;
    dict_->default_dawgs(initial_dawgs, false);
    RecodeNode node(code, unichar_id, permuter, true, start, end, false, cert,
                    score, prev, initial_dawgs,
                    ComputeCodeHash(code, false, prev));
    *best_initial_dawg = node;
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
// false, false, false, false, cert, prev, nullptr) to heap if there is room
// or if better than the current worst element if already full.
/* static */
void RecodeBeamSearch::PushDupOrNoDawgIfBetter(
    int length, bool dup, int code, int unichar_id, float cert,
    float worst_dict_cert, float dict_ratio, bool use_dawgs,
    NodeContinuation cont, const RecodeNode *prev, RecodeBeam *step) {
  int index = BeamIndex(use_dawgs, cont, length);
  if (use_dawgs) {
    if (cert > worst_dict_cert) {
      PushHeapIfBetter(kBeamWidths[length], code, unichar_id,
                       prev ? prev->permuter : NO_PERM, false, false, false,
                       dup, cert, prev, nullptr, &step->beams_[index]);
    }
  } else {
    cert *= dict_ratio;
    if (cert >= kMinCertainty || code == null_char_) {
      PushHeapIfBetter(kBeamWidths[length], code, unichar_id,
                       prev ? prev->permuter : TOP_CHOICE_PERM, false, false,
                       false, dup, cert, prev, nullptr, &step->beams_[index]);
    }
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
// dawg_start, word_start, end, dup, cert, prev, d) to heap if there is room
// or if better than the current worst element if already full.
void RecodeBeamSearch::PushHeapIfBetter(int max_size, int code, int unichar_id,
                                        PermuterType permuter, bool dawg_start,
                                        bool word_start, bool end, bool dup,
                                        float cert, const RecodeNode *prev,
                                        DawgPositionVector *d,
                                        RecodeHeap *heap) {
  float score = cert;
  if (prev != nullptr) {
    score += prev->score;
  }
  if (heap->size() < max_size || score > heap->PeekTop().data().score) {
    uint64_t hash = ComputeCodeHash(code, dup, prev);
    RecodeNode node(code, unichar_id, permuter, dawg_start, word_start, end,
                    dup, cert, score, prev, d, hash);
    if (UpdateHeapIfMatched(&node, heap)) {
      return;
    }
    RecodePair entry(score, node);
    heap->Push(&entry);
    ASSERT_HOST(entry.data().dawgs == nullptr);
    if (heap->size() > max_size) {
      heap->Pop(&entry);
    }
  } else {
    delete d;
  }
}

// Adds a RecodeNode to heap if there is room
// or if better than the current worst element if already full.
void RecodeBeamSearch::PushHeapIfBetter(int max_size, RecodeNode *node,
                                        RecodeHeap *heap) {
  if (heap->size() < max_size || node->score > heap->PeekTop().data().score) {
    if (UpdateHeapIfMatched(node, heap)) {
      return;
    }
    RecodePair entry(node->score, *node);
    heap->Push(&entry);
    ASSERT_HOST(entry.data().dawgs == nullptr);
    if (heap->size() > max_size) {
      heap->Pop(&entry);
    }
  }
}

// Searches the heap for a matching entry, and updates the score with
// reshuffle if needed. Returns true if there was a match.
bool RecodeBeamSearch::UpdateHeapIfMatched(RecodeNode *new_node,
                                           RecodeHeap *heap) {
  // TODO(rays) consider hash map instead of linear search.
  // It might not be faster because the hash map would have to be updated
  // every time a heap reshuffle happens, and that would be a lot of overhead.
  std::vector<RecodePair> &nodes = heap->heap();
  for (auto &i : nodes) {
    RecodeNode &node = i.data();
    if (node.code == new_node->code && node.code_hash == new_node->code_hash &&
        node.permuter == new_node->permuter &&
        node.start_of_dawg == new_node->start_of_dawg) {
      if (new_node->score > node.score) {
        // The new one is better. Update the entire node in the heap and
        // reshuffle.
        node = *new_node;
        i.key() = node.score;
        heap->Reshuffle(&i);
      }
      return true;
    }
  }
  return false;
}

// Computes and returns the code-hash for the given code and prev.
uint64_t RecodeBeamSearch::ComputeCodeHash(int code, bool dup,
                                           const RecodeNode *prev) const {
  uint64_t hash = prev == nullptr ? 0 : prev->code_hash;
  if (!dup && code != null_char_) {
    int num_classes = recoder_.code_range();
    uint64_t carry = (((hash >> 32) * num_classes) >> 32);
    hash *= num_classes;
    hash += carry;
    hash += code;
  }
  return hash;
}

// Backtracks to extract the best path through the lattice that was built
// during Decode. On return the best_nodes vector essentially contains the set
// of code, score pairs that make the optimal path with the constraint that
// the recoder can decode the code sequence back to a sequence of unichar-ids.
void RecodeBeamSearch::ExtractBestPaths(
    std::vector<const RecodeNode *> *best_nodes,
    std::vector<const RecodeNode *> *second_nodes) const {
  // Scan both beams to extract the best and second best paths.
  const RecodeNode *best_node = nullptr;
  const RecodeNode *second_best_node = nullptr;
  const RecodeBeam *last_beam = beam_[beam_size_ - 1];
  for (int c = 0; c < NC_COUNT; ++c) {
    if (c == NC_ONLY_DUP) {
      continue;
    }
    auto cont = static_cast<NodeContinuation>(c);
    for (int is_dawg = 0; is_dawg < 2; ++is_dawg) {
      int beam_index = BeamIndex(is_dawg, cont, 0);
      int heap_size = last_beam->beams_[beam_index].size();
      for (int h = 0; h < heap_size; ++h) {
        const RecodeNode *node = &last_beam->beams_[beam_index].get(h).data();
        if (is_dawg) {
          // dawg_node may be a null_char, or duplicate, so scan back to the
          // last valid unichar_id.
          const RecodeNode *dawg_node = node;
          while (dawg_node != nullptr &&
                 (dawg_node->unichar_id == INVALID_UNICHAR_ID ||
                  dawg_node->duplicate)) {
            dawg_node = dawg_node->prev;
          }
          if (dawg_node == nullptr ||
              (!dawg_node->end_of_word &&
               dawg_node->unichar_id != UNICHAR_SPACE)) {
            // Dawg node is not valid.
            continue;
          }
        }
        if (best_node == nullptr || node->score > best_node->score) {
          second_best_node = best_node;
          best_node = node;
        } else if (second_best_node == nullptr ||
                   node->score > second_best_node->score) {
          second_best_node = node;
        }
      }
    }
  }
  if (second_nodes != nullptr) {
    ExtractPath(second_best_node, second_nodes);
  }
  ExtractPath(best_node, best_nodes);
}

// Helper backtracks through the lattice from the given node, storing the
// path and reversing it.
void RecodeBeamSearch::ExtractPath(
    const RecodeNode *node, std::vector<const RecodeNode *> *path) const {
  path->clear();
  while (node != nullptr) {
    path->push_back(node);
    node = node->prev;
  }
  std::reverse(path->begin(), path->end());
}

void RecodeBeamSearch::ExtractPath(const RecodeNode *node,
                                   std::vector<const RecodeNode *> *path,
                                   int limiter) const {
  int pathcounter = 0;
  path->clear();
  while (node != nullptr && pathcounter < limiter) {
    path->push_back(node);
    node = node->prev;
    ++pathcounter;
  }
  std::reverse(path->begin(), path->end());
}

// Helper prints debug information on the given lattice path.
void RecodeBeamSearch::DebugPath(
    const UNICHARSET *unicharset,
    const std::vector<const RecodeNode *> &path) const {
  for (unsigned c = 0; c < path.size(); ++c) {
    const RecodeNode &node = *path[c];
    tprintf("%u ", c);
    node.Print(null_char_, *unicharset, 1);
  }
}

// Helper prints debug information on the given unichar path.
void RecodeBeamSearch::DebugUnicharPath(
    const UNICHARSET *unicharset, const std::vector<const RecodeNode *> &path,
    const std::vector<int> &unichar_ids, const std::vector<float> &certs,
    const std::vector<float> &ratings, const std::vector<int> &xcoords) const {
  auto num_ids = unichar_ids.size();
  double total_rating = 0.0;
  for (unsigned c = 0; c < num_ids; ++c) {
    int coord = xcoords[c];
    tprintf("%d %d=%s r=%g, c=%g, s=%d, e=%d, perm=%d\n", coord, unichar_ids[c],
            unicharset->debug_str(unichar_ids[c]).c_str(), ratings[c], certs[c],
            path[coord]->start_of_word, path[coord]->end_of_word,
            path[coord]->permuter);
    total_rating += ratings[c];
  }
  tprintf("Path total rating = %g\n", total_rating);
}

} // namespace tesseract.
