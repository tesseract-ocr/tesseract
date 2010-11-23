/**********************************************************************
 * File:        tess_lang_mod_edge.h
 * Description: Declaration of the Tesseract Language Model Edge Class
 * Author:    Ahmad Abdulkader
 * Created:   2008
 *
 * (C) Copyright 2008, Google Inc.
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
 **********************************************************************/

// The TessLangModEdge models an edge in the Tesseract language models
// It inherits from the LangModEdge class

#ifndef TESS_LANG_MOD_EDGE_H
#define TESS_LANG_MOD_EDGE_H

#include "dawg.h"
#include "char_set.h"

#include "lang_mod_edge.h"
#include "cube_reco_context.h"
#include "cube_utils.h"

// Macros needed to identify punctuation in the langmodel state
#ifdef _HMSW32_H
#define LEAD_PUNC_EDGE_REF_MASK  (inT64) 0x0000000100000000i64
#define TRAIL_PUNC_EDGE_REF_MASK (inT64) 0x0000000200000000i64
#define TRAIL_PUNC_REPEAT_MASK (inT64)   0xffff000000000000i64
#else
#define LEAD_PUNC_EDGE_REF_MASK  (inT64) 0x0000000100000000ll
#define TRAIL_PUNC_EDGE_REF_MASK (inT64) 0x0000000200000000ll
#define TRAIL_PUNC_REPEAT_MASK (inT64)   0xffff000000000000ll
#endif

// Number state machine macros
#define NUMBER_STATE_SHIFT        0
#define NUMBER_STATE_MASK         0x0000000fl
#define NUMBER_LITERAL_SHIFT      4
#define NUMBER_LITERAL_MASK       0x000000f0l
#define NUMBER_REPEAT_SHIFT       8
#define NUMBER_REPEAT_MASK        0x00000f00l
#define NUM_TRM                   -99
#define TRAIL_PUNC_REPEAT_SHIFT   48

#define IsLeadingPuncEdge(edge_mask) \
  ((edge_mask & LEAD_PUNC_EDGE_REF_MASK) != 0)
#define IsTrailingPuncEdge(edge_mask) \
  ((edge_mask & TRAIL_PUNC_EDGE_REF_MASK) != 0)
#define TrailingPuncCount(edge_mask) \
  ((edge_mask & TRAIL_PUNC_REPEAT_MASK) >> TRAIL_PUNC_REPEAT_SHIFT)
#define TrailingPuncEdgeMask(Cnt) \
  (TRAIL_PUNC_EDGE_REF_MASK | ((Cnt) << TRAIL_PUNC_REPEAT_SHIFT))

// State machine IDs
#define DAWG_OOD      0
#define DAWG_NUMBER   1

namespace tesseract {
class TessLangModEdge : public LangModEdge {
 public:
  // Different ways of constructing a TessLangModEdge
  TessLangModEdge(CubeRecoContext *cntxt, const Dawg *edge_array,
                  EDGE_REF edge, int class_id);
  TessLangModEdge(CubeRecoContext *cntxt, const Dawg *edge_array,
                  EDGE_REF start_edge_idx, EDGE_REF end_edge_idx,
                  int class_id);
  TessLangModEdge(CubeRecoContext *cntxt, int class_id);
  ~TessLangModEdge() {}

  // Accessors
  inline bool IsRoot() const {
    return root_;
  }
  inline void SetRoot(bool flag) { root_ = flag; }

  inline bool IsOOD() const {
    return (dawg_ == (Dawg *)DAWG_OOD);
  }

  inline bool IsNumber() const {
    return (dawg_ == (Dawg *)DAWG_NUMBER);
  }

  inline bool IsEOW() const {
    return (IsTerminal() || (dawg_->end_of_word(end_edge_) != 0));
  }

  inline const Dawg *GetDawg() const { return dawg_; }
  inline EDGE_REF StartEdge() const { return start_edge_; }
  inline EDGE_REF EndEdge() const { return end_edge_; }
  inline EDGE_REF EdgeMask() const { return edge_mask_; }
  inline const char_32 * EdgeString() const { return str_; }
  inline int ClassID () const { return class_id_; }
  inline int PathCost() const { return path_cost_; }
  inline void SetEdgeMask(EDGE_REF edge_mask) { edge_mask_ = edge_mask; }
  inline void SetDawg(Dawg *dawg) { dawg_ = dawg; }
  inline void SetStartEdge(EDGE_REF edge_idx) { start_edge_ = edge_idx; }
  inline void SetEndEdge(EDGE_REF edge_idx) { end_edge_ = edge_idx; }

  // is this a terminal node:
  // we can terminate at any OOD char, trailing punc or
  // when the dawg terminates
  inline bool IsTerminal() const {
    return (IsOOD() || IsNumber() || IsTrailingPuncEdge(start_edge_) ||
            dawg_->next_node(end_edge_) == 0);
  }

  // How many signals does the LM provide for tuning. These are flags like:
  // OOD or not, Number of not that are used by the training to compute
  // extra costs for each word.
  inline int SignalCnt() const {
    return 2;
  }

  // returns the weight assigned to a specified signal
  inline double SignalWgt(int signal) const {
    CubeTuningParams *params =
        reinterpret_cast<CubeTuningParams *>(cntxt_->Params());
    if (params != NULL) {
      switch (signal) {
        case 0:
          return params->OODWgt();
          break;

        case 1:
          return params->NumWgt();
          break;
      }
    }

    return 0.0;
  }

  // sets the weight assigned to a specified signal: Used in training
  void SetSignalWgt(int signal, double wgt) {
    CubeTuningParams *params =
        reinterpret_cast<CubeTuningParams *>(cntxt_->Params());
    if (params != NULL) {
      switch (signal) {
        case 0:
          params->SetOODWgt(wgt);
          break;

        case 1:
          params->SetNumWgt(wgt);
          break;
      }
    }
  }

  // returns the actual value of a specified signal
  int Signal(int signal) {
    switch (signal) {
      case 0:
        return IsOOD() ? MIN_PROB_COST : 0;
        break;

      case 1:
        return IsNumber() ? MIN_PROB_COST : 0;
        break;

      default:
        return 0;
    }
  }

  // returns the Hash value of the edge. Used by the SearchNode hash table
  // to quickly lookup exisiting edges to converge during search
  inline unsigned int Hash() const {
    return static_cast<unsigned int>(((start_edge_ | end_edge_) ^
      ((reinterpret_cast<unsigned long int>(dawg_)))) ^
      ((unsigned int)edge_mask_) ^
      class_id_);
  }

  // A verbal description of the edge: Used by visualizers
  char *Description() const;

  // Is this edge identical to the specified edge
  inline bool IsIdentical(LangModEdge *lang_mod_edge) const {
    return (class_id_ ==
        reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->class_id_ &&
        str_ == reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->str_ &&
        dawg_ == reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->dawg_ &&
        start_edge_ ==
        reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->start_edge_ &&
        end_edge_ ==
        reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->end_edge_ &&
        edge_mask_ ==
        reinterpret_cast<TessLangModEdge *>(lang_mod_edge)->edge_mask_);
  }

  // Creates a set of fan-out edges for the specified edge
  static int CreateChildren(CubeRecoContext *cntxt,
                            const Dawg *edges,
                            NODE_REF edge_reg,
                            LangModEdge **lm_edges);

 private:
  bool root_;
  CubeRecoContext *cntxt_;
  const Dawg *dawg_;
  EDGE_REF  start_edge_;
  EDGE_REF  end_edge_;
  EDGE_REF  edge_mask_;
  int path_cost_;
  int class_id_;
  const char_32 * str_;
  // returns the cost of the lang_mod_edge
  inline int Cost() const {
    if (cntxt_ != NULL) {
      CubeTuningParams *params =
          reinterpret_cast<CubeTuningParams *>(cntxt_->Params());
      if (dawg_ == (Dawg *)DAWG_OOD) {
        return static_cast<int>(params->OODWgt() * MIN_PROB_COST);
      } else if (dawg_ == (Dawg *)DAWG_NUMBER) {
        return static_cast<int>(params->NumWgt() * MIN_PROB_COST);
      }
    }
    return 0;
  }
};
}  // namespace tesseract

#endif  // TESS_LANG_MOD_EDGE_H
