///////////////////////////////////////////////////////////////////////
// File:        wordrec.h
// Description: wordrec class.
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

#ifndef TESSERACT_WORDREC_WORDREC_H__
#define TESSERACT_WORDREC_WORDREC_H__

#include "classify.h"
#include "ratngs.h"
#include "matrix.h"
#include "seam.h"
#include "callback.h"
#include "associate.h"
#include "badwords.h"

struct CHUNKS_RECORD;
struct SEARCH_RECORD;

namespace tesseract {
class Wordrec : public Classify {
 public:
  Wordrec();
  ~Wordrec();
  void save_summary(inT32 elapsed_time);
  /* tface.cpp ***************************************************************/
  void program_editup(const char *textbase, bool init_permute);
  BLOB_CHOICE_LIST_VECTOR *cc_recog(TWERD *tessword,
                                    WERD_CHOICE *best_choice,
                                    WERD_CHOICE *best_raw_choice,
                                    BOOL8 tester,
                                    BOOL8 trainer,
                                    bool last_word_on_line);
  void program_editdown(inT32 elasped_time);
  void set_pass1();
  void set_pass2();
  int end_recog();
  int start_recog(const char *textbase);
  BLOB_CHOICE_LIST *call_matcher(                  //call a matcher
                    TBLOB *ptblob,    //previous
                    TBLOB *tessblob,  //blob to match
                    TBLOB *ntblob,    //next
                    void *,           //unused parameter
                    TEXTROW *         //always null anyway
                   );
  /* tessinit.cpp ************************************************************/
  void program_init();
  /* wordclass.cpp ***********************************************************/
  BLOB_CHOICE_LIST *classify_blob(TBLOB *pblob,
                                  TBLOB *blob,
                                  TBLOB *nblob,
                                  TEXTROW *row,
                                  const char *string,
                                  C_COL color);
  void update_blob_classifications(TWERD *word,
                                   const BLOB_CHOICE_LIST_VECTOR &choices);
  /* bestfirst.cpp ***********************************************************/
  BLOB_CHOICE_LIST_VECTOR *evaluate_chunks(CHUNKS_RECORD *chunks_record,
                                           SEARCH_STATE search_state);
  void update_ratings(const BLOB_CHOICE_LIST_VECTOR &new_choices,
                      const CHUNKS_RECORD *chunks_record,
                      const SEARCH_STATE search_state);
  inT16 evaluate_state(CHUNKS_RECORD *chunks_record,
                       SEARCH_RECORD *the_search,
                       DANGERR *fixpt);
  void best_first_search(CHUNKS_RECORD *chunks_record,
                         WERD_CHOICE *best_choice,
                         WERD_CHOICE *raw_choice,
                         STATE *state,
                         DANGERR *fixpt,
                         STATE *best_state);
  void expand_node(FLOAT32 worst_priority,
                   CHUNKS_RECORD *chunks_record,
                   SEARCH_RECORD *the_search);
  BLOB_CHOICE_LIST_VECTOR *rebuild_current_state(
      TBLOB *blobs,
      SEAMS seam_list,
      STATE *state,
      BLOB_CHOICE_LIST_VECTOR *char_choices,
      int fx,
      bool force_rebuild,
      const WERD_CHOICE &best_choice,
      const MATRIX *ratings);
  BLOB_CHOICE_LIST *join_blobs_and_classify(
      TBLOB *blobs, SEAMS seam_list,
      int x, int y, int fx, const MATRIX *ratings,
      BLOB_CHOICE_LIST_VECTOR *old_choices);

  /* chopper.cpp *************************************************************/
  bool improve_one_blob(TWERD *word,
                        BLOB_CHOICE_LIST_VECTOR *char_choices,
                        int fx,
                        inT32 *blob_number,
                        SEAMS *seam_list,
                        DANGERR *fixpt,
                        bool split_next_to_fragment);
  void modify_blob_choice(BLOB_CHOICE_LIST *answer,
                          int chop_index);
  bool chop_one_blob(TWERD *word,
                     BLOB_CHOICE_LIST_VECTOR *char_choices,
                     inT32 *blob_number,
                     SEAMS *seam_list,
                     int *right_chop_index);
  BLOB_CHOICE_LIST_VECTOR *chop_word_main(register TWERD *word,
                                          int fx,
                                          WERD_CHOICE *best_choice,
                                          WERD_CHOICE *raw_choice,
                                          BOOL8 tester,
                                          BOOL8 trainer);
  void improve_by_chopping(register TWERD *word,
                           BLOB_CHOICE_LIST_VECTOR *char_choices,
                           int fx,
                           STATE *best_state,
                           WERD_CHOICE *best_choice,
                           WERD_CHOICE *raw_choice,
                           SEAMS *seam_list,
                           DANGERR *fixpt,
                           STATE *chop_states,
                           inT32 *state_count);
  MATRIX *word_associator(TBLOB *blobs,
                          SEAMS seams,
                          STATE *state,
                          int fxid,
                          WERD_CHOICE *best_choice,
                          WERD_CHOICE *raw_choice,
                          char *correct,
                          DANGERR *fixpt,
                          STATE *best_state);
  inT16 select_blob_to_split(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                             float rating_ceiling,
                             bool split_next_to_fragment);
  /* mfvars.cpp **************************************************************/
  void mfeature_init();
  /* pieces.cpp **************************************************************/
  BLOB_CHOICE_LIST *classify_piece(TBLOB *pieces,
                                   SEAMS seams,
                                   inT16 start,
                                   inT16 end);
  BLOB_CHOICE_LIST *get_piece_rating(MATRIX *ratings,
                                     TBLOB *blobs,
                                     SEAMS seams,
                                     inT16 start,
                                     inT16 end);
  /* djmenus.cpp **************************************************************/
  // Prints out statistics gathered.
  void dj_statistics(FILE *File) {
    PrintAdaptiveStatistics(File);
    PrintBadWords(File);
  }
  // Does clean up (should be called at the end of the program).
  void dj_cleanup() { EndAdaptiveClassifier(); }


  /* heuristic.cpp ************************************************************/
  FLOAT32 prioritize_state(CHUNKS_RECORD *chunks_record,
                           SEARCH_RECORD *the_search);
  FLOAT32 width_priority(CHUNKS_RECORD *chunks_record,
                         STATE *state,
                         int num_joints);
  FLOAT32 seamcut_priority(SEAMS seams,
                           STATE *state,
                           int num_joints);
  FLOAT32 rating_priority(CHUNKS_RECORD *chunks_record,
                          STATE *state,
                          int num_joints);

  /* member variables *********************************************************/
  /* tface.cpp ****************************************************************/
  POLY_MATCHER tess_matcher;//current matcher
  POLY_TESTER tess_tester;  //current tester
  POLY_TESTER tess_trainer; //current trainer
  DENORM *tess_denorm;      //current denorm
  WERD *tess_word;          //current word
  int dict_word(const WERD_CHOICE &word);
};



/* ccmain/tstruct.cpp *********************************************************/
class FRAGMENT:public ELIST_LINK
{
  public:
    FRAGMENT() {  //constructor
    }
    FRAGMENT(EDGEPT *head_pt,   //start
             EDGEPT *tail_pt);  //end

    ICOORD head;                 //coords of start
    ICOORD tail;                 //coords of end
    EDGEPT *headpt;              //start point
    EDGEPT *tailpt;              //end point

    NEWDELETE2 (FRAGMENT)
};

ELISTIZEH (FRAGMENT)
PBLOB *make_ed_blob(                 //construct blob
                    TBLOB *tessblob  //blob to convert
                   );
OUTLINE *make_ed_outline(                     //constructoutline
                         FRAGMENT_LIST *list  //list of fragments
                        );
void register_outline(                     //add fragments
                      TESSLINE *outline,   //tess format
                      FRAGMENT_LIST *list  //list to add to
                     );

}  // namespace tesseract

#endif  // TESSERACT_WORDREC_WORDREC_H__
