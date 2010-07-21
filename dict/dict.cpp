///////////////////////////////////////////////////////////////////////
// File:        dict.cpp
// Description: dict class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
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

#include "dict.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)  // Conversion warnings
#endif

namespace tesseract {

class Image;

Dict::Dict(Image* image_ptr)
    : letter_is_okay_(&tesseract::Dict::def_letter_is_okay),
      image_ptr_(image_ptr) {
  dang_ambigs_table_ = NULL;
  replace_ambigs_table_ = NULL;
  keep_word_choices_ = false;
  reject_offset_ = 0.0;
  best_raw_choice_ = NULL;
  best_choices_ = NIL;
  raw_choices_ = NIL;
  go_deeper_fxn_ = NULL;
  hyphen_word_ = NULL;
  last_word_on_line_ = false;
  hyphen_unichar_id_ = INVALID_UNICHAR_ID;
  document_words_ = NULL;
  pending_words_ = NULL;
  freq_dawg_ = NULL;
}

Dict::~Dict() {
  if (hyphen_word_ != NULL) delete hyphen_word_;
}

// Returns true if in light of the current state the letter at word_index
// in the given word is allowed according to at least one of the dawgs in
// dawgs_.
//
// See more extensive comments in dict.h where this function is declared.
//
int Dict::def_letter_is_okay(void* void_dawg_args, int word_index,
                             const void *void_word, bool word_end) {
  DawgArgs *dawg_args = reinterpret_cast<DawgArgs*>(void_dawg_args);
  const WERD_CHOICE *word = reinterpret_cast<const WERD_CHOICE*>(void_word);

  if (dawg_debug_level >= 3) {
    tprintf("def_letter_is_okay: word_index=%d word_end=%d"
            " word=%s num active dawgs=%d num constraints=%d\n",
            word_index, word_end,
            word->debug_string(getUnicharset()).string(),
            dawg_args->active_dawgs->length(),
            dawg_args->constraints->length());
  }

  // Do not accept words that contain kPatternUnicharID.
  // (otherwise pattern dawgs would not function correctly).
  // Do not accept words containing INVALID_UNICHAR_IDs.
  UNICHAR_ID unichar_id = word->unichar_id(word_index);
  if (unichar_id == Dawg::kPatternUnicharID ||
      unichar_id == INVALID_UNICHAR_ID) {
    dawg_args->permuter = NO_PERM;
    return NO_PERM;
  }

  // Initialization.
  PermuterType current_permuter = NO_PERM;
  dawg_args->updated_active_dawgs->clear();
  const DawgInfoVector &constraints = *(dawg_args->constraints);
  *dawg_args->updated_constraints = constraints;

  // Go over the active_dawgs vector and insert DawgInfo records with the
  // updated ref (an edge with the corresponding unichar id) into
  // dawg_args->updated_active_dawgs.
  for (int a = 0; a < dawg_args->active_dawgs->length(); ++a) {
    const DawgInfo &info = (*dawg_args->active_dawgs)[a];
    const Dawg *dawg = dawgs_[info.dawg_index];
    // Obtain unichar_id at this position (could be changed later, so this
    // needs to be inside the loop over all active dawgs).
     unichar_id = word->unichar_id(word_index);
    // The number dawg generalizes all digits to be kPatternUnicharID,
    // so try to match kPatternUnicharID if the current unichar is a digit.
    if (dawg->type() == DAWG_TYPE_NUMBER &&
        getUnicharset().get_isdigit(unichar_id)) {
      unichar_id = Dawg::kPatternUnicharID;
    }
    // Get the starting node for this letter.
    NODE_REF node;
    if (info.ref == NO_EDGE) {
      node = 0;  // beginning to explore this dawg
    } else {
      node = dawg->next_node(info.ref);
      if (node == 0) node = NO_EDGE;  // end of word
    }
    // Find the edge out of the node for the curent unichar_id.
    EDGE_REF edge = (node != NO_EDGE) ?
      dawg->edge_char_of(node, unichar_id, word_end) : NO_EDGE;

    if (dawg_debug_level >= 3) {
      tprintf("Active dawg: [%d, " REFFORMAT "] edge=" REFFORMAT "\n",
              info.dawg_index, node, edge);
    }

    if (edge != NO_EDGE) {  // the unichar was found in the current dawg
      if (ConstraintsOk(*(dawg_args->updated_constraints),
                        word_end, dawg->type())) {
        UpdatePermuter(dawg->permuter(), &current_permuter);
        dawg_args->updated_active_dawgs->add_unique(
            DawgInfo(info.dawg_index, edge),
            "Append current dawg to updated active dawgs: ");
      }
    } else {                // the unichar was not found in the current dawg
      // Handle leading/trailing punctuation dawgs that denote a word pattern
      // as an edge with kPatternUnicharID. If such an edge is found we add a
      // constraint denoting the state of the dawg before the word pattern.
      // This constraint will be applied later when this dawg is found among
      // successor dawgs as well potentially at the end of the word.
      if (dawg->type() == DAWG_TYPE_PUNCTUATION) {
        edge = dawg->edge_char_of(node, Dawg::kPatternUnicharID, word_end);
        if (edge != NO_EDGE) {
          dawg_args->updated_constraints->add_unique(
              DawgInfo(info.dawg_index, edge), "Recording constraint: ");
        } else {
          // Do not explore successors of this dawg, since this
          // must be invalid leading or trailing punctuation.
          if (dawg_debug_level >= 3) {
            tprintf("Invalid punctuation from dawg %d\n", info.dawg_index);
          }
          continue;
        }
      }

      if (info.ref == NO_EDGE) {
        if (dawg_debug_level >= 3) {
          tprintf("No letters matched in dawg %d\n", info.dawg_index);
        }
        continue;
      }

      // Discard the dawg if the pattern can not end at previous letter.
      if (edge == NO_EDGE &&  // previous part is not leading punctuation
          !dawg->end_of_word(info.ref)) {
        if (dawg_debug_level >= 3) {
          tprintf("No valid pattern end in dawg %d\n", info.dawg_index);
        }
        continue;
      }

      // Look for the unichar in each of this dawg's successors
      // and append those in which it is found to active_dawgs.
      const SuccessorList &slist = *(successors_[info.dawg_index]);
      for (int s = 0; s < slist.length(); ++s) {
        int sdawg_index = slist[s];
        const Dawg *sdawg = dawgs_[sdawg_index];
        NODE_REF snode = 0;
        // Apply constraints to the successor dawg.
        for (int c = 0; c < constraints.length(); ++c) {
          // If the successor dawg is described in the constraints change
          // the start ref from 0 to the one recorded as the constraint.
          const DawgInfo &cinfo = constraints[c];
          if (cinfo.dawg_index == sdawg_index) {
            snode = sdawg->next_node(cinfo.ref);
            // Make sure we do not search the successor dawg if after
            // applying the saved constraint we are at the end of the word.
            if (snode == 0) snode = NO_EDGE;
            if (dawg_debug_level >= 3) {
               tprintf("Applying constraint [%d, " REFFORMAT "]\n",
                       sdawg_index, snode);
            }
          }
        }
        // Look for the letter in this successor dawg.
        EDGE_REF sedge = sdawg->edge_char_of(
            snode, word->unichar_id(word_index), word_end);
        // If we found the letter append sdawg to the active_dawgs list.
        if (sedge != NO_EDGE &&
            ConstraintsOk(*(dawg_args->updated_constraints), word_end,
                          dawgs_[sdawg_index]->type())) {
          UpdatePermuter(sdawg->permuter(), &current_permuter);
          if (sdawg->next_node(sedge) != 0) {  // if not word end
            dawg_args->updated_active_dawgs->add_unique(
              DawgInfo(sdawg_index, sedge),
              "Append successor to updated active dawgs: ");
          }
        }
      }  // end successors loop
    }  // end if/else
  }  // end for
  // Update dawg_args->permuter if it used to be NO_PERM or if we found
  // the current letter in a non-punctuation dawg. This allows preserving
  // information on which dawg the "core" word came from.
  if ((current_permuter == PUNC_PERM &&
       current_permuter > dawg_args->permuter) ||
      current_permuter != PUNC_PERM) {
    dawg_args->permuter = current_permuter;
  }
  return dawg_args->permuter;
}

}  // namespace tesseract
