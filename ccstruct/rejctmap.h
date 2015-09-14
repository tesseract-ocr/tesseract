/**********************************************************************
 * File:        rejctmap.h  (Formerly rejmap.h)
 * Description: REJ and REJMAP class functions.
 * Author:		Phil Cheatle
 * Created:		Thu Jun  9 13:46:38 BST 1994
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

#ifndef           REJCTMAP_H
#define           REJCTMAP_H

#ifdef __UNIX__
#include          <assert.h>
#endif
#include          "memry.h"
#include          "bits16.h"
#include                   "params.h"

enum REJ_FLAGS
{
  /* Reject modes which are NEVER overridden */
  R_TESS_FAILURE,                // PERM Tess didn't classify
  R_SMALL_XHT,                   // PERM Xht too small
  R_EDGE_CHAR,                   // PERM Too close to edge of image
  R_1IL_CONFLICT,                // PERM 1Il confusion
  R_POSTNN_1IL,                  // PERM 1Il unrejected by NN
  R_REJ_CBLOB,                   // PERM Odd blob
  R_MM_REJECT,                   // PERM Matrix match rejection (m's)
  R_BAD_REPETITION,              // TEMP Repeated char which doesn't match trend

  /* Initial reject modes (pre NN_ACCEPT) */
  R_POOR_MATCH,                  // TEMP Ray's original heuristic (Not used)
  R_NOT_TESS_ACCEPTED,           // TEMP Tess didn't accept WERD
  R_CONTAINS_BLANKS,             // TEMP Tess failed on other chs in WERD
  R_BAD_PERMUTER,                // POTENTIAL Bad permuter for WERD

  /* Reject modes generated after NN_ACCEPT but before MM_ACCEPT */
  R_HYPHEN,                      // TEMP Post NN dodgy hyphen or full stop
  R_DUBIOUS,                     // TEMP Post NN dodgy chars
  R_NO_ALPHANUMS,                // TEMP No alphanumerics in word after NN
  R_MOSTLY_REJ,                  // TEMP Most of word rejected so rej the rest
  R_XHT_FIXUP,                   // TEMP Xht tests unsure

  /* Reject modes generated after MM_ACCEPT but before QUALITY_ACCEPT */
  R_BAD_QUALITY,                 // TEMP Quality metrics bad for WERD

  /* Reject modes generated after QUALITY_ACCEPT but before MINIMAL_REJ accep*/
  R_DOC_REJ,                     // TEMP Document rejection
  R_BLOCK_REJ,                   // TEMP Block rejection
  R_ROW_REJ,                     // TEMP Row rejection
  R_UNLV_REJ,                    // TEMP ~ turned to - or ^ turned to space

  /* Accept modes which occur between the above rejection groups */
  R_NN_ACCEPT,                   //NN acceptance
  R_HYPHEN_ACCEPT,               //Hyphen acceptance
  R_MM_ACCEPT,                   //Matrix match acceptance
  R_QUALITY_ACCEPT,              //Accept word in good quality doc
  R_MINIMAL_REJ_ACCEPT           //Accept EVERYTHING except tess failures
};

/* REJECT MAP VALUES */

#define           MAP_ACCEPT '1'
#define           MAP_REJECT_PERM '0'
#define           MAP_REJECT_TEMP '2'
#define           MAP_REJECT_POTENTIAL '3'

class REJ
{
  BITS16 flags1;
  BITS16 flags2;

  void set_flag(REJ_FLAGS rej_flag) {
    if (rej_flag < 16)
      flags1.turn_on_bit (rej_flag);
    else
      flags2.turn_on_bit (rej_flag - 16);
  }

  BOOL8 rej_before_nn_accept();
  BOOL8 rej_between_nn_and_mm();
  BOOL8 rej_between_mm_and_quality_accept();
  BOOL8 rej_between_quality_and_minimal_rej_accept();
  BOOL8 rej_before_mm_accept();
  BOOL8 rej_before_quality_accept();

  public:
    REJ() {  //constructor
    }

    REJ(  //classwise copy
        const REJ &source) {
      flags1 = source.flags1;
      flags2 = source.flags2;
    }

    REJ & operator= (            //assign REJ
    const REJ & source) {        //from this
      flags1 = source.flags1;
      flags2 = source.flags2;
      return *this;
    }

    BOOL8 flag(REJ_FLAGS rej_flag) {
      if (rej_flag < 16)
        return flags1.bit (rej_flag);
      else
        return flags2.bit (rej_flag - 16);
    }

    char display_char() {
      if (perm_rejected ())
        return MAP_REJECT_PERM;
      else if (accept_if_good_quality ())
        return MAP_REJECT_POTENTIAL;
      else if (rejected ())
        return MAP_REJECT_TEMP;
      else
        return MAP_ACCEPT;
    }

    BOOL8 perm_rejected();  //Is char perm reject?

    BOOL8 rejected();  //Is char rejected?

    BOOL8 accepted() {  //Is char accepted?
      return !rejected ();
    }

                                 //potential rej?
    BOOL8 accept_if_good_quality();

    BOOL8 recoverable() {
      return (rejected () && !perm_rejected ());
    }

    void setrej_tess_failure();  //Tess generated blank
    void setrej_small_xht();  //Small xht char/wd
    void setrej_edge_char();  //Close to image edge
    void setrej_1Il_conflict();  //Initial reject map
    void setrej_postNN_1Il();  //1Il after NN
    void setrej_rej_cblob();  //Insert duff blob
    void setrej_mm_reject();  //Matrix matcher
                                 //Odd repeated char
    void setrej_bad_repetition();
    void setrej_poor_match();  //Failed Rays heuristic
                                 //TEMP reject_word
    void setrej_not_tess_accepted();
                                 //TEMP reject_word
    void setrej_contains_blanks();
    void setrej_bad_permuter();  //POTENTIAL reject_word
    void setrej_hyphen();  //PostNN dubious hyph or .
    void setrej_dubious();  //PostNN dubious limit
    void setrej_no_alphanums();  //TEMP reject_word
    void setrej_mostly_rej();  //TEMP reject_word
    void setrej_xht_fixup();  //xht fixup
    void setrej_bad_quality();  //TEMP reject_word
    void setrej_doc_rej();  //TEMP reject_word
    void setrej_block_rej();  //TEMP reject_word
    void setrej_row_rej();  //TEMP reject_word
    void setrej_unlv_rej();  //TEMP reject_word
    void setrej_nn_accept();  //NN Flipped a char
    void setrej_hyphen_accept();  //Good aspect ratio
    void setrej_mm_accept();  //Matrix matcher
                                 //Quality flip a char
    void setrej_quality_accept();
                                 //Accept all except blank
    void setrej_minimal_rej_accept();

    void full_print(FILE *fp);
};

class REJMAP
{
  REJ *ptr;                      //ptr to the chars
  inT16 len;                     //Number of chars

  public:
    REJMAP() {  //constructor
      ptr = NULL;
      len = 0;
    }

    REJMAP(  //classwise copy
           const REJMAP &rejmap);

    REJMAP & operator= (         //assign REJMAP
      const REJMAP & source);    //from this

    ~REJMAP () {                 //destructor
      if (ptr != NULL)
        free_struct (ptr, len * sizeof (REJ), "REJ");
    }

    void initialise(  //Redefine map
                    inT16 length);

    REJ & operator[](            //access function
      inT16 index) const         //map index
    {
      ASSERT_HOST (index < len);
      return ptr[index];         //no bounds checks
    }

    inT32 length() const {  //map length
      return len;
    }

    inT16 accept_count();  //How many accepted?

    inT16 reject_count() {  //How many rejects?
      return len - accept_count ();
    }

    void remove_pos(             //Cut out an element
                    inT16 pos);  //element to remove

    void print(FILE *fp);

    void full_print(FILE *fp);

    BOOL8 recoverable_rejects();  //Any non perm rejs?

    BOOL8 quality_recoverable_rejects();
    //Any potential rejs?

    void rej_word_small_xht();  //Reject whole word
                                 //Reject whole word
    void rej_word_tess_failure();
    void rej_word_not_tess_accepted();
    //Reject whole word
                                 //Reject whole word
    void rej_word_contains_blanks();
                                 //Reject whole word
    void rej_word_bad_permuter();
    void rej_word_xht_fixup();  //Reject whole word
                                 //Reject whole word
    void rej_word_no_alphanums();
    void rej_word_mostly_rej();  //Reject whole word
    void rej_word_bad_quality();  //Reject whole word
    void rej_word_doc_rej();  //Reject whole word
    void rej_word_block_rej();  //Reject whole word
    void rej_word_row_rej();  //Reject whole word
};
#endif
