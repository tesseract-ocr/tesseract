///////////////////////////////////////////////////////////////////////
// File:        recodebeam.cpp
// Description: Beam search to decode from the re-encoded CJK as a sequence of
//              smaller numbers in place of a single large code.
// Author:      Ray Smith
// Created:     Fri Mar 13 09:39:01 PDT 2015
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

namespace tesseract {

// Clipping value for certainty inside Tesseract. Reflects the minimum value
// of certainty that will be returned by ExtractBestPathAsUnicharIds.
// Supposedly on a uniform scale that can be compared across languages and
// engines.
const float RecodeBeamSearch::kMinCertainty = -20.0f;

// The beam width at each code position.
const int RecodeBeamSearch::kBeamWidths[RecodedCharID::kMaxCodeLen + 1] = {
    5, 10, 16, 16, 16, 16, 16, 16, 16, 16,
};

// Borrows the pointer, which is expected to survive until *this is deleted.
RecodeBeamSearch::RecodeBeamSearch(const UnicharCompress& recoder,
                                   int null_char, bool simple_text, Dict* dict)
    : recoder_(recoder),
      beam_size_(0),
      dict_(dict),
      space_delimited_(true),
      is_simple_text_(simple_text),
      null_char_(null_char) {
  if (dict_ != NULL && !dict_->IsSpaceDelimitedLang()) space_delimited_ = false;
}

// Decodes the set of network outputs, storing the lattice internally.
void RecodeBeamSearch::Decode(const NetworkIO& output, double dict_ratio,
                              double cert_offset, double worst_dict_cert,
                              const UNICHARSET* charset) {
  beam_size_ = 0;
  int width = output.Width();
  for (int t = 0; t < width; ++t) {
    ComputeTopN(output.f(t), output.NumFeatures(), kBeamWidths[0]);
    DecodeStep(output.f(t), t, dict_ratio, cert_offset, worst_dict_cert,
               charset);
  }
}
void RecodeBeamSearch::Decode(const GENERIC_2D_ARRAY<float>& output,
                              double dict_ratio, double cert_offset,
                              double worst_dict_cert,
                              const UNICHARSET* charset) {
  beam_size_ = 0;
  int width = output.dim1();
  for (int t = 0; t < width; ++t) {
    ComputeTopN(output[t], output.dim2(), kBeamWidths[0]);
    DecodeStep(output[t], t, dict_ratio, cert_offset, worst_dict_cert, charset);
  }
}

// Returns the best path as labels/scores/xcoords similar to simple CTC.
void RecodeBeamSearch::ExtractBestPathAsLabels(
    GenericVector<int>* labels, GenericVector<int>* xcoords) const {
  labels->truncate(0);
  xcoords->truncate(0);
  GenericVector<const RecodeNode*> best_nodes;
  ExtractBestPaths(&best_nodes, NULL);
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
    bool debug, const UNICHARSET* unicharset, GenericVector<int>* unichar_ids,
    GenericVector<float>* certs, GenericVector<float>* ratings,
    GenericVector<int>* xcoords) const {
  GenericVector<const RecodeNode*> best_nodes;
  ExtractBestPaths(&best_nodes, NULL);
  ExtractPathAsUnicharIds(best_nodes, unichar_ids, certs, ratings, xcoords);
  if (debug) {
    DebugPath(unicharset, best_nodes);
    DebugUnicharPath(unicharset, best_nodes, *unichar_ids, *certs, *ratings,
                     *xcoords);
  }
}

// Returns the best path as a set of WERD_RES.
void RecodeBeamSearch::ExtractBestPathAsWords(const TBOX& line_box,
                                              float scale_factor, bool debug,
                                              const UNICHARSET* unicharset,
                                              PointerVector<WERD_RES>* words) {
  words->truncate(0);
  GenericVector<int> unichar_ids;
  GenericVector<float> certs;
  GenericVector<float> ratings;
  GenericVector<int> xcoords;
  GenericVector<const RecodeNode*> best_nodes;
  GenericVector<const RecodeNode*> second_nodes;
  ExtractBestPaths(&best_nodes, &second_nodes);
  if (debug) {
    DebugPath(unicharset, best_nodes);
    ExtractPathAsUnicharIds(second_nodes, &unichar_ids, &certs, &ratings,
                            &xcoords);
    tprintf("\nSecond choice path:\n");
    DebugUnicharPath(unicharset, second_nodes, unichar_ids, certs, ratings,
                     xcoords);
  }
  ExtractPathAsUnicharIds(best_nodes, &unichar_ids, &certs, &ratings, &xcoords);
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
      if (unichar_ids[word_end] == UNICHAR_SPACE) break;
      int index = xcoords[word_end];
      if (best_nodes[index]->start_of_word) break;
      if (best_nodes[index]->permuter == TOP_CHOICE_PERM &&
          (!unicharset->IsSpaceDelimited(unichar_ids[word_end]) ||
           !unicharset->IsSpaceDelimited(unichar_ids[word_end - 1])))
        break;
    }
    float space_cert = 0.0f;
    if (word_end < num_ids && unichar_ids[word_end] == UNICHAR_SPACE)
      space_cert = certs[word_end];
    bool leading_space =
        word_start > 0 && unichar_ids[word_start - 1] == UNICHAR_SPACE;
    // Create a WERD_RES for the output word.
    WERD_RES* word_res = InitializeWord(
        leading_space, line_box, word_start, word_end,
        MIN(space_cert, prev_space_cert), unicharset, xcoords, scale_factor);
    for (int i = word_start; i < word_end; ++i) {
      BLOB_CHOICE_LIST* choices = new BLOB_CHOICE_LIST;
      BLOB_CHOICE_IT bc_it(choices);
      BLOB_CHOICE* choice = new BLOB_CHOICE(
          unichar_ids[i], ratings[i], certs[i], -1, 1.0f,
          static_cast<float>(MAX_INT16), 0.0f, BCC_STATIC_CLASSIFIER);
      int col = i - word_start;
      choice->set_matrix_cell(col, col);
      bc_it.add_after_then_move(choice);
      word_res->ratings->put(col, col, choices);
    }
    int index = xcoords[word_end - 1];
    word_res->FakeWordFromRatings(best_nodes[index]->permuter);
    words->push_back(word_res);
    prev_space_cert = space_cert;
    if (word_end < num_ids && unichar_ids[word_end] == UNICHAR_SPACE)
      ++word_end;
  }
}

// Generates debug output of the content of the beams after a Decode.
void RecodeBeamSearch::DebugBeams(const UNICHARSET& unicharset) const {
  for (int p = 0; p < beam_size_; ++p) {
    // Print all the best scoring nodes for each unichar found.
    tprintf("Position %d: Nondict beam\n", p);
    DebugBeamPos(unicharset, beam_[p]->beams_[0]);
    tprintf("Position %d: Dict beam\n", p);
    DebugBeamPos(unicharset, beam_[p]->dawg_beams_[0]);
  }
}

// Generates debug output of the content of a single beam position.
void RecodeBeamSearch::DebugBeamPos(const UNICHARSET& unicharset,
                                    const RecodeHeap& heap) const {
  GenericVector<const RecodeNode*> unichar_bests;
  unichar_bests.init_to_size(unicharset.size(), NULL);
  const RecodeNode* null_best = NULL;
  int heap_size = heap.size();
  for (int i = 0; i < heap_size; ++i) {
    const RecodeNode* node = &heap.get(i).data;
    if (node->unichar_id == INVALID_UNICHAR_ID) {
      if (null_best == NULL || null_best->score < node->score) null_best = node;
    } else {
      if (unichar_bests[node->unichar_id] == NULL ||
          unichar_bests[node->unichar_id]->score < node->score) {
        unichar_bests[node->unichar_id] = node;
      }
    }
  }
  for (int u = 0; u < unichar_bests.size(); ++u) {
    if (unichar_bests[u] != NULL) {
      const RecodeNode& node = *unichar_bests[u];
      tprintf("label=%d, uid=%d=%s score=%g, c=%g, s=%d, e=%d, perm=%d\n",
              node.code, node.unichar_id,
              unicharset.debug_str(node.unichar_id).string(), node.score,
              node.certainty, node.start_of_word, node.end_of_word,
              node.permuter);
    }
  }
  if (null_best != NULL) {
    tprintf("null_char score=%g, c=%g, s=%d, e=%d, perm=%d\n", null_best->score,
            null_best->certainty, null_best->start_of_word,
            null_best->end_of_word, null_best->permuter);
  }
}

// Returns the given best_nodes as unichar-ids/certs/ratings/xcoords skipping
// duplicates, nulls and intermediate parts.
/* static */
void RecodeBeamSearch::ExtractPathAsUnicharIds(
    const GenericVector<const RecodeNode*>& best_nodes,
    GenericVector<int>* unichar_ids, GenericVector<float>* certs,
    GenericVector<float>* ratings, GenericVector<int>* xcoords) {
  unichar_ids->truncate(0);
  certs->truncate(0);
  ratings->truncate(0);
  xcoords->truncate(0);
  // Backtrack extracting only valid, non-duplicate unichar-ids.
  int t = 0;
  int width = best_nodes.size();
  while (t < width) {
    double certainty = 0.0;
    double rating = 0.0;
    while (t < width && best_nodes[t]->unichar_id == INVALID_UNICHAR_ID) {
      double cert = best_nodes[t++]->certainty;
      if (cert < certainty) certainty = cert;
      rating -= cert;
    }
    if (t < width) {
      int unichar_id = best_nodes[t]->unichar_id;
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
      certs->push_back(certainty);
      ratings->push_back(rating);
    } else if (!certs->empty()) {
      if (certainty < certs->back()) certs->back() = certainty;
      ratings->back() += rating;
    }
  }
  xcoords->push_back(width);
}

// Sets up a word with the ratings matrix and fake blobs with boxes in the
// right places.
WERD_RES* RecodeBeamSearch::InitializeWord(bool leading_space,
                                           const TBOX& line_box, int word_start,
                                           int word_end, float space_certainty,
                                           const UNICHARSET* unicharset,
                                           const GenericVector<int>& xcoords,
                                           float scale_factor) {
  // Make a fake blob for each non-zero label.
  C_BLOB_LIST blobs;
  C_BLOB_IT b_it(&blobs);
  for (int i = word_start; i < word_end; ++i) {
    int min_half_width = xcoords[i + 1] - xcoords[i];
    if (i > 0 && xcoords[i] - xcoords[i - 1] < min_half_width)
      min_half_width = xcoords[i] - xcoords[i - 1];
    if (min_half_width < 1) min_half_width = 1;
    // Make a fake blob.
    TBOX box(xcoords[i] - min_half_width, 0, xcoords[i] + min_half_width,
             line_box.height());
    box.scale(scale_factor);
    box.move(ICOORD(line_box.left(), line_box.bottom()));
    box.set_top(line_box.top());
    b_it.add_after_then_move(C_BLOB::FakeBlob(box));
  }
  // Make a fake word from the blobs.
  WERD* word = new WERD(&blobs, leading_space, NULL);
  // Make a WERD_RES from the word.
  WERD_RES* word_res = new WERD_RES(word);
  word_res->uch_set = unicharset;
  word_res->combination = true;  // Give it ownership of the word.
  word_res->space_certainty = space_certainty;
  word_res->ratings = new MATRIX(word_end - word_start, 1);
  return word_res;
}

// Fills top_n_flags_ with bools that are true iff the corresponding output
// is one of the top_n.
void RecodeBeamSearch::ComputeTopN(const float* outputs, int num_outputs,
                                   int top_n) {
  top_n_flags_.init_to_size(num_outputs, false);
  top_heap_.clear();
  for (int i = 0; i < num_outputs; ++i) {
    if (top_heap_.size() < top_n || outputs[i] > top_heap_.PeekTop().key) {
      TopPair entry(outputs[i], i);
      top_heap_.Push(&entry);
      if (top_heap_.size() > top_n) top_heap_.Pop(&entry);
    }
  }
  while (!top_heap_.empty()) {
    TopPair entry;
    top_heap_.Pop(&entry);
    top_n_flags_[entry.data] = true;
  }
}

// Adds the computation for the current time-step to the beam. Call at each
// time-step in sequence from left to right. outputs is the activation vector
// for the current timestep.
void RecodeBeamSearch::DecodeStep(const float* outputs, int t,
                                  double dict_ratio, double cert_offset,
                                  double worst_dict_cert,
                                  const UNICHARSET* charset) {
  if (t == beam_.size()) beam_.push_back(new RecodeBeam);
  RecodeBeam* step = beam_[t];
  beam_size_ = t + 1;
  step->Clear();
  if (t == 0) {
    // The first step can only use singles and initials.
    ContinueContext(NULL, 0, outputs, false, true, dict_ratio, cert_offset,
                    worst_dict_cert, step);
    if (dict_ != NULL)
      ContinueContext(NULL, 0, outputs, true, true, dict_ratio, cert_offset,
                      worst_dict_cert, step);
  } else {
    RecodeBeam* prev = beam_[t - 1];
    if (charset != NULL) {
      for (int i = prev->dawg_beams_[0].size() - 1; i >= 0; --i) {
        GenericVector<const RecodeNode*> path;
        ExtractPath(&prev->dawg_beams_[0].get(i).data, &path);
        tprintf("Step %d: Dawg beam %d:\n", t, i);
        DebugPath(charset, path);
      }
    }
    int total_beam = 0;
    // Try true and then false only if the beam is empty. This enables extending
    // the context using only the top-n results first, which may have an empty
    // intersection with the valid codes, so we fall back to the rest if the
    // beam is empty.
    for (int flag = 1; flag >= 0 && total_beam == 0; --flag) {
      for (int length = 0; length <= RecodedCharID::kMaxCodeLen; ++length) {
        // Working backwards through the heaps doesn't guarantee that we see the
        // best first, but it comes before a lot of the worst, so it is slightly
        // more efficient than going forwards.
        for (int i = prev->dawg_beams_[length].size() - 1; i >= 0; --i) {
          ContinueContext(&prev->dawg_beams_[length].get(i).data, length,
                          outputs, true, flag, dict_ratio, cert_offset,
                          worst_dict_cert, step);
        }
        for (int i = prev->beams_[length].size() - 1; i >= 0; --i) {
          ContinueContext(&prev->beams_[length].get(i).data, length, outputs,
                          false, flag, dict_ratio, cert_offset, worst_dict_cert,
                          step);
        }
      }
      for (int length = 0; length <= RecodedCharID::kMaxCodeLen; ++length) {
        total_beam += step->beams_[length].size();
        total_beam += step->dawg_beams_[length].size();
      }
    }
    // Special case for the best initial dawg. Push it on the heap if good
    // enough, but there is only one, so it doesn't blow up the beam.
    RecodeHeap* dawg_heap = &step->dawg_beams_[0];
    if (step->best_initial_dawg_.code >= 0 &&
        (dawg_heap->size() < kBeamWidths[0] ||
         step->best_initial_dawg_.score > dawg_heap->PeekTop().data.score)) {
      RecodePair entry(step->best_initial_dawg_.score,
                       step->best_initial_dawg_);
      dawg_heap->Push(&entry);
      if (dawg_heap->size() > kBeamWidths[0]) dawg_heap->Pop(&entry);
    }
  }
}

// Adds to the appropriate beams the legal (according to recoder)
// continuations of context prev, which is of the given length, using the
// given network outputs to provide scores to the choices. Uses only those
// choices for which top_n_flags[index] == top_n_flag.
void RecodeBeamSearch::ContinueContext(const RecodeNode* prev, int length,
                                       const float* outputs, bool use_dawgs,
                                       bool top_n_flag, double dict_ratio,
                                       double cert_offset,
                                       double worst_dict_cert,
                                       RecodeBeam* step) {
  RecodedCharID prefix;
  RecodedCharID full_code;
  const RecodeNode* previous = prev;
  for (int p = length - 1; p >= 0; --p, previous = previous->prev) {
    while (previous != NULL &&
           (previous->duplicate || previous->code == null_char_)) {
      previous = previous->prev;
    }
    prefix.Set(p, previous->code);
    full_code.Set(p, previous->code);
  }
  if (prev != NULL && !is_simple_text_) {
    float cert = NetworkIO::ProbToCertainty(outputs[prev->code]) + cert_offset;
    if ((cert >= kMinCertainty || prev->code == null_char_) &&
        top_n_flags_[prev->code] == top_n_flag) {
      if (use_dawgs) {
        if (cert > worst_dict_cert) {
          PushDupIfBetter(kBeamWidths[length], cert, prev,
                          &step->dawg_beams_[length]);
        }
      } else {
        PushDupIfBetter(kBeamWidths[length], cert * dict_ratio, prev,
                        &step->beams_[length]);
      }
    }
    if (prev->code != null_char_ && length > 0 &&
        top_n_flags_[null_char_] == top_n_flag) {
      // Allow nulls within multi code sequences, as the nulls within are not
      // explicitly included in the code sequence.
      cert = NetworkIO::ProbToCertainty(outputs[null_char_]) + cert_offset;
      if (cert >= kMinCertainty && (!use_dawgs || cert > worst_dict_cert)) {
        if (use_dawgs) {
          PushNoDawgIfBetter(kBeamWidths[length], null_char_,
                             INVALID_UNICHAR_ID, NO_PERM, cert, prev,
                             &step->dawg_beams_[length]);
        } else {
          PushNoDawgIfBetter(kBeamWidths[length], null_char_,
                             INVALID_UNICHAR_ID, TOP_CHOICE_PERM,
                             cert * dict_ratio, prev, &step->beams_[length]);
        }
      }
    }
  }
  const GenericVector<int>* final_codes = recoder_.GetFinalCodes(prefix);
  if (final_codes != NULL) {
    for (int i = 0; i < final_codes->size(); ++i) {
      int code = (*final_codes)[i];
      if (top_n_flags_[code] != top_n_flag) continue;
      if (prev != NULL && prev->code == code && !is_simple_text_) continue;
      float cert = NetworkIO::ProbToCertainty(outputs[code]) + cert_offset;
      if (cert < kMinCertainty && code != null_char_) continue;
      full_code.Set(length, code);
      int unichar_id = recoder_.DecodeUnichar(full_code);
      // Map the null char to INVALID.
      if (length == 0 && code == null_char_) unichar_id = INVALID_UNICHAR_ID;
      if (use_dawgs) {
        if (cert > worst_dict_cert) {
          ContinueDawg(kBeamWidths[0], code, unichar_id, cert, prev,
                       &step->dawg_beams_[0], step);
        }
      } else {
        PushNoDawgIfBetter(kBeamWidths[0], code, unichar_id, TOP_CHOICE_PERM,
                           cert * dict_ratio, prev, &step->beams_[0]);
        if (dict_ != NULL &&
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
          if (unichar_id == UNICHAR_SPACE)
            permuter = NO_PERM;
          else
            dawg_cert *= dict_ratio;
          PushInitialDawgIfBetter(code, unichar_id, permuter, false, false,
                                  dawg_cert, prev, &step->best_initial_dawg_);
        }
      }
    }
  }
  const GenericVector<int>* next_codes = recoder_.GetNextCodes(prefix);
  if (next_codes != NULL) {
    for (int i = 0; i < next_codes->size(); ++i) {
      int code = (*next_codes)[i];
      if (top_n_flags_[code] != top_n_flag) continue;
      if (prev != NULL && prev->code == code && !is_simple_text_) continue;
      float cert = NetworkIO::ProbToCertainty(outputs[code]) + cert_offset;
      if (cert < kMinCertainty && code != null_char_) continue;
      if (use_dawgs) {
        if (cert > worst_dict_cert) {
          ContinueDawg(kBeamWidths[length + 1], code, INVALID_UNICHAR_ID, cert,
                       prev, &step->dawg_beams_[length + 1], step);
        }
      } else {
        PushNoDawgIfBetter(kBeamWidths[length + 1], code, INVALID_UNICHAR_ID,
                           TOP_CHOICE_PERM, cert * dict_ratio, prev,
                           &step->beams_[length + 1]);
      }
    }
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, cert, prev,
// appropriate-dawg-args, cert) to the given heap (dawg_beam_) if unichar_id
// is a valid continuation of whatever is in prev.
void RecodeBeamSearch::ContinueDawg(int max_size, int code, int unichar_id,
                                    float cert, const RecodeNode* prev,
                                    RecodeHeap* heap, RecodeBeam* step) {
  if (unichar_id == INVALID_UNICHAR_ID) {
    PushNoDawgIfBetter(max_size, code, unichar_id, NO_PERM, cert, prev, heap);
    return;
  }
  // Avoid dictionary probe if score a total loss.
  float score = cert;
  if (prev != NULL) score += prev->score;
  if (heap->size() >= max_size && score <= heap->PeekTop().data.score) return;
  const RecodeNode* uni_prev = prev;
  // Prev may be a partial code, null_char, or duplicate, so scan back to the
  // last valid unichar_id.
  while (uni_prev != NULL &&
         (uni_prev->unichar_id == INVALID_UNICHAR_ID || uni_prev->duplicate))
    uni_prev = uni_prev->prev;
  if (unichar_id == UNICHAR_SPACE) {
    if (uni_prev != NULL && uni_prev->end_of_word) {
      // Space is good. Push initial state, to the dawg beam and a regular
      // space to the top choice beam.
      PushInitialDawgIfBetter(code, unichar_id, uni_prev->permuter, false,
                              false, cert, prev, &step->best_initial_dawg_);
      PushNoDawgIfBetter(max_size, code, unichar_id, uni_prev->permuter, cert,
                         prev, &step->beams_[0]);
    }
    return;
  } else if (uni_prev != NULL && uni_prev->start_of_dawg &&
             uni_prev->unichar_id != UNICHAR_SPACE &&
             dict_->getUnicharset().IsSpaceDelimited(uni_prev->unichar_id) &&
             dict_->getUnicharset().IsSpaceDelimited(unichar_id)) {
    return;  // Can't break words between space delimited chars.
  }
  DawgPositionVector initial_dawgs;
  DawgPositionVector* updated_dawgs = new DawgPositionVector;
  DawgArgs dawg_args(&initial_dawgs, updated_dawgs, NO_PERM);
  bool word_start = false;
  if (uni_prev == NULL) {
    // Starting from beginning of line.
    dict_->default_dawgs(&initial_dawgs, false);
    word_start = true;
  } else if (uni_prev->dawgs != NULL) {
    // Continuing a previous dict word.
    dawg_args.active_dawgs = uni_prev->dawgs;
    word_start = uni_prev->start_of_dawg;
  } else {
    return;  // Can't continue if not a dict word.
  }
  PermuterType permuter = static_cast<PermuterType>(
      dict_->def_letter_is_okay(&dawg_args, unichar_id, false));
  if (permuter != NO_PERM) {
    PushHeapIfBetter(max_size, code, unichar_id, permuter, false, word_start,
                     dawg_args.valid_end, false, cert, prev,
                     dawg_args.updated_dawgs, heap);
    if (dawg_args.valid_end && !space_delimited_) {
      // We can start another word right away, so push initial state as well,
      // to the dawg beam, and the regular character to the top choice beam,
      // since non-dict words can start here too.
      PushInitialDawgIfBetter(code, unichar_id, permuter, word_start, true,
                              cert, prev, &step->best_initial_dawg_);
      PushHeapIfBetter(max_size, code, unichar_id, permuter, false, word_start,
                       true, false, cert, prev, NULL, &step->beams_[0]);
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
                                               const RecodeNode* prev,
                                               RecodeNode* best_initial_dawg) {
  float score = cert;
  if (prev != NULL) score += prev->score;
  if (best_initial_dawg->code < 0 || score > best_initial_dawg->score) {
    DawgPositionVector* initial_dawgs = new DawgPositionVector;
    dict_->default_dawgs(initial_dawgs, false);
    RecodeNode node(code, unichar_id, permuter, true, start, end, false, cert,
                    score, prev, initial_dawgs);
    *best_initial_dawg = node;
  }
}

// Adds a copy of the given prev as a duplicate of and successor to prev, if
// there is room or if better than the current worst element if already full.
/* static */
void RecodeBeamSearch::PushDupIfBetter(int max_size, float cert,
                                       const RecodeNode* prev,
                                       RecodeHeap* heap) {
  PushHeapIfBetter(max_size, prev->code, prev->unichar_id, prev->permuter,
                   false, false, false, true, cert, prev, NULL, heap);
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
// false, false, false, false, cert, prev, NULL) to heap if there is room
// or if better than the current worst element if already full.
/* static */
void RecodeBeamSearch::PushNoDawgIfBetter(int max_size, int code,
                                          int unichar_id, PermuterType permuter,
                                          float cert, const RecodeNode* prev,
                                          RecodeHeap* heap) {
  float score = cert;
  if (prev != NULL) score += prev->score;
  if (heap->size() < max_size || score > heap->PeekTop().data.score) {
    RecodeNode node(code, unichar_id, permuter, false, false, false, false,
                    cert, score, prev, NULL);
    RecodePair entry(score, node);
    heap->Push(&entry);
    if (heap->size() > max_size) heap->Pop(&entry);
  }
}

// Adds a RecodeNode composed of the tuple (code, unichar_id, permuter,
// dawg_start, word_start, end, dup, cert, prev, d) to heap if there is room
// or if better than the current worst element if already full.
/* static */
void RecodeBeamSearch::PushHeapIfBetter(int max_size, int code, int unichar_id,
                                        PermuterType permuter, bool dawg_start,
                                        bool word_start, bool end, bool dup,
                                        float cert, const RecodeNode* prev,
                                        DawgPositionVector* d,
                                        RecodeHeap* heap) {
  float score = cert;
  if (prev != NULL) score += prev->score;
  if (heap->size() < max_size || score > heap->PeekTop().data.score) {
    RecodeNode node(code, unichar_id, permuter, dawg_start, word_start, end,
                    dup, cert, score, prev, d);
    RecodePair entry(score, node);
    heap->Push(&entry);
    ASSERT_HOST(entry.data.dawgs == NULL);
    if (heap->size() > max_size) heap->Pop(&entry);
  } else {
    delete d;
  }
}

// Backtracks to extract the best path through the lattice that was built
// during Decode. On return the best_nodes vector essentially contains the set
// of code, score pairs that make the optimal path with the constraint that
// the recoder can decode the code sequence back to a sequence of unichar-ids.
void RecodeBeamSearch::ExtractBestPaths(
    GenericVector<const RecodeNode*>* best_nodes,
    GenericVector<const RecodeNode*>* second_nodes) const {
  // Scan both beams to extract the best and second best paths.
  const RecodeNode* best_node = NULL;
  const RecodeNode* second_best_node = NULL;
  const RecodeBeam* last_beam = beam_[beam_size_ - 1];
  int heap_size = last_beam->beams_[0].size();
  for (int i = 0; i < heap_size; ++i) {
    const RecodeNode* node = &last_beam->beams_[0].get(i).data;
    if (best_node == NULL || node->score > best_node->score) {
      second_best_node = best_node;
      best_node = node;
    } else if (second_best_node == NULL ||
               node->score > second_best_node->score) {
      second_best_node = node;
    }
  }
  // Scan the entire dawg heap for the best *valid* nodes, if any.
  int dawg_size = last_beam->dawg_beams_[0].size();
  for (int i = 0; i < dawg_size; ++i) {
    const RecodeNode* dawg_node = &last_beam->dawg_beams_[0].get(i).data;
    // dawg_node may be a null_char, or duplicate, so scan back to the last
    // valid unichar_id.
    const RecodeNode* back_dawg_node = dawg_node;
    while (back_dawg_node != NULL &&
           (back_dawg_node->unichar_id == INVALID_UNICHAR_ID ||
            back_dawg_node->duplicate))
      back_dawg_node = back_dawg_node->prev;
    if (back_dawg_node != NULL &&
        (back_dawg_node->end_of_word ||
         back_dawg_node->unichar_id == UNICHAR_SPACE)) {
      // Dawg node is valid. Use it in preference to back_dawg_node, as the
      // score comparison is fair that way.
      if (best_node == NULL || dawg_node->score > best_node->score) {
        second_best_node = best_node;
        best_node = dawg_node;
      } else if (second_best_node == NULL ||
                 dawg_node->score > second_best_node->score) {
        second_best_node = dawg_node;
      }
    }
  }
  if (second_nodes != NULL) ExtractPath(second_best_node, second_nodes);
  ExtractPath(best_node, best_nodes);
}

// Helper backtracks through the lattice from the given node, storing the
// path and reversing it.
void RecodeBeamSearch::ExtractPath(
    const RecodeNode* node, GenericVector<const RecodeNode*>* path) const {
  path->truncate(0);
  while (node != NULL) {
    path->push_back(node);
    node = node->prev;
  }
  path->reverse();
}

// Helper prints debug information on the given lattice path.
void RecodeBeamSearch::DebugPath(
    const UNICHARSET* unicharset,
    const GenericVector<const RecodeNode*>& path) const {
  for (int c = 0; c < path.size(); ++c) {
    const RecodeNode& node = *path[c];
    tprintf("%d %d=%s score=%g, c=%g, s=%d, e=%d, perm=%d\n", c,
            node.unichar_id, unicharset->debug_str(node.unichar_id).string(),
            node.score, node.certainty, node.start_of_word, node.end_of_word,
            node.permuter);
  }
}

// Helper prints debug information on the given unichar path.
void RecodeBeamSearch::DebugUnicharPath(
    const UNICHARSET* unicharset, const GenericVector<const RecodeNode*>& path,
    const GenericVector<int>& unichar_ids, const GenericVector<float>& certs,
    const GenericVector<float>& ratings,
    const GenericVector<int>& xcoords) const {
  int num_ids = unichar_ids.size();
  double total_rating = 0.0;
  for (int c = 0; c < num_ids; ++c) {
    int coord = xcoords[c];
    tprintf("%d %d=%s r=%g, c=%g, s=%d, e=%d, perm=%d\n", coord, unichar_ids[c],
            unicharset->debug_str(unichar_ids[c]).string(), ratings[c],
            certs[c], path[coord]->start_of_word, path[coord]->end_of_word,
            path[coord]->permuter);
    total_rating += ratings[c];
  }
  tprintf("Path total rating = %g\n", total_rating);
}

}  // namespace tesseract.
