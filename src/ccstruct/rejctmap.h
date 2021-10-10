/**********************************************************************
 * File:        rejctmap.h  (Formerly rejmap.h)
 * Description: REJ and REJMAP class functions.
 * Author:    Phil Cheatle
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
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

This module may look unnecessarily verbose, but here's the philosophy...

ALL processing of the reject map is done in this module. There are lots of
separate calls to set reject/accept flags. These have DELIBERATELY been kept
distinct so that this module can decide what to do.

Basically, there is a flag for each sort of rejection or acceptance. This
provides a history of what has happened to EACH character.

Determining whether a character is CURRENTLY rejected depends on implicit
understanding of the SEQUENCE of possible calls. The flags are defined and
grouped in the REJ_FLAGS enum. These groupings are used in determining a
characters CURRENT rejection status. Basically, a character is ACCEPTED if

    none of the permanent rej flags are set
  AND (    the character has never been rejected
      OR an accept flag is set which is LATER than the latest reject flag )

IT IS FUNDAMENTAL THAT ANYONE HACKING THIS CODE UNDERSTANDS THE SIGNIFICANCE
OF THIS IMPLIED TEMPORAL ORDERING OF THE FLAGS!!!!
**********************************************************************/

#ifndef REJCTMAP_H
#define REJCTMAP_H

#include "errcode.h"
#include "params.h"

#include <bitset>
#include <memory>

namespace tesseract {

enum REJ_FLAGS {
  /* Reject modes which are NEVER overridden */
  R_TESS_FAILURE,   // PERM Tess didn't classify
  R_SMALL_XHT,      // PERM Xht too small
  R_EDGE_CHAR,      // PERM Too close to edge of image
  R_1IL_CONFLICT,   // PERM 1Il confusion
  R_POSTNN_1IL,     // PERM 1Il unrejected by NN
  R_REJ_CBLOB,      // PERM Odd blob
  R_MM_REJECT,      // PERM Matrix match rejection (m's)
  R_BAD_REPETITION, // TEMP Repeated char which doesn't match trend

  /* Initial reject modes (pre NN_ACCEPT) */
  R_POOR_MATCH,        // TEMP Ray's original heuristic (Not used)
  R_NOT_TESS_ACCEPTED, // TEMP Tess didn't accept WERD
  R_CONTAINS_BLANKS,   // TEMP Tess failed on other chs in WERD
  R_BAD_PERMUTER,      // POTENTIAL Bad permuter for WERD

  /* Reject modes generated after NN_ACCEPT but before MM_ACCEPT */
  R_HYPHEN,       // TEMP Post NN dodgy hyphen or full stop
  R_DUBIOUS,      // TEMP Post NN dodgy chars
  R_NO_ALPHANUMS, // TEMP No alphanumerics in word after NN
  R_MOSTLY_REJ,   // TEMP Most of word rejected so rej the rest
  R_XHT_FIXUP,    // TEMP Xht tests unsure

  /* Reject modes generated after MM_ACCEPT but before QUALITY_ACCEPT */
  R_BAD_QUALITY, // TEMP Quality metrics bad for WERD

  /* Reject modes generated after QUALITY_ACCEPT but before MINIMAL_REJ accep*/
  R_DOC_REJ,   // TEMP Document rejection
  R_BLOCK_REJ, // TEMP Block rejection
  R_ROW_REJ,   // TEMP Row rejection
  R_UNLV_REJ,  // TEMP ~ turned to - or ^ turned to space

  /* Accept modes which occur between the above rejection groups */
  R_NN_ACCEPT,         // NN acceptance
  R_HYPHEN_ACCEPT,     // Hyphen acceptance
  R_MM_ACCEPT,         // Matrix match acceptance
  R_QUALITY_ACCEPT,    // Accept word in good quality doc
  R_MINIMAL_REJ_ACCEPT // Accept EVERYTHING except tess failures
};

/* REJECT MAP VALUES */

#define MAP_ACCEPT '1'
#define MAP_REJECT_PERM '0'
#define MAP_REJECT_TEMP '2'
#define MAP_REJECT_POTENTIAL '3'

class REJ {
  std::bitset<32> flags;

  void set_flag(REJ_FLAGS rej_flag) {
    flags.set(rej_flag);
  }

public:
  REJ() = default;

  REJ( // classwise copy
      const REJ &source) {
    flags = source.flags;
  }

  REJ &operator=( // assign REJ
      const REJ &source) = default;

  bool flag(REJ_FLAGS rej_flag) const {
    return flags[rej_flag];
  }

  char display_char() const {
    if (perm_rejected()) {
      return MAP_REJECT_PERM;
    } else if (accept_if_good_quality()) {
      return MAP_REJECT_POTENTIAL;
    } else if (rejected()) {
      return MAP_REJECT_TEMP;
    } else {
      return MAP_ACCEPT;
    }
  }

  bool perm_rejected() const { // Is char perm reject?
    return (flag(R_TESS_FAILURE) || flag(R_SMALL_XHT) || flag(R_EDGE_CHAR) ||
            flag(R_1IL_CONFLICT) || flag(R_POSTNN_1IL) || flag(R_REJ_CBLOB) ||
            flag(R_BAD_REPETITION) || flag(R_MM_REJECT));
  }

private:
  bool rej_before_nn_accept() const {
    return flag(R_POOR_MATCH) || flag(R_NOT_TESS_ACCEPTED) ||
           flag(R_CONTAINS_BLANKS) || flag(R_BAD_PERMUTER);
  }

  bool rej_between_nn_and_mm() const {
    return flag(R_HYPHEN) || flag(R_DUBIOUS) || flag(R_NO_ALPHANUMS) ||
           flag(R_MOSTLY_REJ) || flag(R_XHT_FIXUP);
  }

  bool rej_between_mm_and_quality_accept() const {
    return flag(R_BAD_QUALITY);
  }

  bool rej_between_quality_and_minimal_rej_accept() const {
    return flag(R_DOC_REJ) || flag(R_BLOCK_REJ) || flag(R_ROW_REJ) ||
           flag(R_UNLV_REJ);
  }

  bool rej_before_mm_accept() const {
    return rej_between_nn_and_mm() ||
           (rej_before_nn_accept() && !flag(R_NN_ACCEPT) &&
            !flag(R_HYPHEN_ACCEPT));
  }

  bool rej_before_quality_accept() const {
    return rej_between_mm_and_quality_accept() ||
           (!flag(R_MM_ACCEPT) && rej_before_mm_accept());
  }

public:
  bool rejected() const { // Is char rejected?
    if (flag(R_MINIMAL_REJ_ACCEPT)) {
      return false;
    } else {
      return (perm_rejected() || rej_between_quality_and_minimal_rej_accept() ||
              (!flag(R_QUALITY_ACCEPT) && rej_before_quality_accept()));
    }
  }

  bool accept_if_good_quality() const { // potential rej?
    return (rejected() && !perm_rejected() && flag(R_BAD_PERMUTER) &&
            !flag(R_POOR_MATCH) && !flag(R_NOT_TESS_ACCEPTED) &&
            !flag(R_CONTAINS_BLANKS) &&
            (!rej_between_nn_and_mm() && !rej_between_mm_and_quality_accept() &&
             !rej_between_quality_and_minimal_rej_accept()));
  }

  void setrej_tess_failure() { // Tess generated blank
    set_flag(R_TESS_FAILURE);
  }

  void setrej_small_xht() { // Small xht char/wd
    set_flag(R_SMALL_XHT);
  }

  void setrej_edge_char() { // Close to image edge
    set_flag(R_EDGE_CHAR);
  }

  void setrej_1Il_conflict() { // Initial reject map
    set_flag(R_1IL_CONFLICT);
  }

  void setrej_postNN_1Il() { // 1Il after NN
    set_flag(R_POSTNN_1IL);
  }

  void setrej_rej_cblob() { // Insert duff blob
    set_flag(R_REJ_CBLOB);
  }

  void setrej_mm_reject() { // Matrix matcher
    set_flag(R_MM_REJECT);
  }

  void setrej_bad_repetition() { // Odd repeated char
    set_flag(R_BAD_REPETITION);
  }

  void setrej_poor_match() { // Failed Rays heuristic
    set_flag(R_POOR_MATCH);
  }

  void setrej_not_tess_accepted() {
    // TEMP reject_word
    set_flag(R_NOT_TESS_ACCEPTED);
  }

  void setrej_contains_blanks() {
    // TEMP reject_word
    set_flag(R_CONTAINS_BLANKS);
  }

  void setrej_bad_permuter() { // POTENTIAL reject_word
    set_flag(R_BAD_PERMUTER);
  }

  void setrej_hyphen() { // PostNN dubious hyphen or .
    set_flag(R_HYPHEN);
  }

  void setrej_dubious() { // PostNN dubious limit
    set_flag(R_DUBIOUS);
  }

  void setrej_no_alphanums() { // TEMP reject_word
    set_flag(R_NO_ALPHANUMS);
  }

  void setrej_mostly_rej() { // TEMP reject_word
    set_flag(R_MOSTLY_REJ);
  }

  void setrej_xht_fixup() { // xht fixup
    set_flag(R_XHT_FIXUP);
  }

  void setrej_bad_quality() { // TEMP reject_word
    set_flag(R_BAD_QUALITY);
  }

  void setrej_doc_rej() { // TEMP reject_word
    set_flag(R_DOC_REJ);
  }

  void setrej_block_rej() { // TEMP reject_word
    set_flag(R_BLOCK_REJ);
  }

  void setrej_row_rej() { // TEMP reject_word
    set_flag(R_ROW_REJ);
  }

  void setrej_unlv_rej() { // TEMP reject_word
    set_flag(R_UNLV_REJ);
  }

  void setrej_hyphen_accept() { // NN Flipped a char
    set_flag(R_HYPHEN_ACCEPT);
  }

  void setrej_nn_accept() { // NN Flipped a char
    set_flag(R_NN_ACCEPT);
  }

  void setrej_mm_accept() { // Matrix matcher
    set_flag(R_MM_ACCEPT);
  }

  void setrej_quality_accept() { // Quality flip a char
    set_flag(R_QUALITY_ACCEPT);
  }

  void setrej_minimal_rej_accept() {
    // Accept all except blank
    set_flag(R_MINIMAL_REJ_ACCEPT);
  }

  bool accepted() const { // Is char accepted?
    return !rejected();
  }

  bool recoverable() const {
    return (rejected() && !perm_rejected());
  }

  void full_print(FILE *fp) const;
};

class REJMAP {
  std::unique_ptr<REJ[]> ptr; // ptr to the chars
  uint16_t len = 0;           // Number of chars

public:
  REJMAP() = default;

  REJMAP(const REJMAP &rejmap) {
    *this = rejmap;
  }

  REJMAP &operator=(const REJMAP &source);

  // Sets up the ptr array to length, whatever it was before.
  void initialise(uint16_t length);

  REJ &operator[](         // access function
      uint16_t index) const // map index
  {
    ASSERT_HOST(index < len);
    return ptr[index]; // no bounds checks
  }

  uint16_t length() const { // map length
    return len;
  }

  int16_t accept_count() const; // How many accepted?

  int16_t reject_count() const { // How many rejects?
    return len - accept_count();
  }

  // Cut out an element.
  void remove_pos(uint16_t pos);

  void print(FILE *fp) const;

  void full_print(FILE *fp) const;

  bool recoverable_rejects() const; // Any non perm rejs?

  bool quality_recoverable_rejects() const;
  // Any potential rejs?

  void rej_word_small_xht(); // Reject whole word
                             // Reject whole word
  void rej_word_tess_failure();
  void rej_word_not_tess_accepted();
  // Reject whole word
  // Reject whole word
  void rej_word_contains_blanks();
  // Reject whole word
  void rej_word_bad_permuter();
  void rej_word_xht_fixup(); // Reject whole word
                             // Reject whole word
  void rej_word_no_alphanums();
  void rej_word_mostly_rej();  // Reject whole word
  void rej_word_bad_quality(); // Reject whole word
  void rej_word_doc_rej();     // Reject whole word
  void rej_word_block_rej();   // Reject whole word
  void rej_word_row_rej();     // Reject whole word
};

} // namespace tesseract

#endif
