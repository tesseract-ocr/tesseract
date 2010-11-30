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

#include "associate.h"
#include "classify.h"
#include "dict.h"
#include "language_model.h"
#include "ratngs.h"
#include "matrix.h"
#include "matchtab.h"
#include "oldheap.h"
#include "gradechop.h"
#include "seam.h"
#include "findseam.h"
#include "callcpp.h"
#include "associate.h"
#include "pieces.h"
#include "ratngs.h"
#include "tally.h"

struct CHUNKS_RECORD;
struct SEARCH_RECORD;
class WERD_RES;

// A struct for storing child/parent pairs of the BLOB_CHOICE_LISTs
// to be processed by the segmentation search.
struct SEG_SEARCH_PENDING : public ELIST_LINK {
  SEG_SEARCH_PENDING(int child_row_arg,
                     BLOB_CHOICE_LIST *parent_arg,
                     tesseract::LanguageModelFlagsType changed_arg) :
    child_row(child_row_arg), parent(parent_arg), changed(changed_arg) {}

  // Comparator function for add_sorted().
  static int compare(const void *p1, const void *p2) {
    const SEG_SEARCH_PENDING *e1 = *reinterpret_cast<
      const SEG_SEARCH_PENDING * const *>(p1);
    const SEG_SEARCH_PENDING *e2 = *reinterpret_cast<
      const SEG_SEARCH_PENDING * const *>(p2);
    if (e1->child_row == e2->child_row &&
        e1->parent == e2->parent) return 0;
    return (e1->child_row < e2->child_row) ? -1 : 1;
  }

  int child_row;  // row of the child in the ratings matrix
  BLOB_CHOICE_LIST *parent;  // pointer to the parent BLOB_CHOICE_LIST
  // Flags that indicate which language model components are still active
  // on the parent path (i.e. recorded some changes to the language model
  // state) and need to be invoked for this pending entry.
  // This field is used as an argument to LanguageModel::UpdateState()
  // in Wordrec::UpdateSegSearchNodes().
  tesseract::LanguageModelFlagsType changed;
};

ELISTIZEH(SEG_SEARCH_PENDING);


namespace tesseract {

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
};
ELISTIZEH(FRAGMENT)


class Wordrec : public Classify {
 public:
  // config parameters *******************************************************
  BOOL_VAR_H(wordrec_no_block, FALSE, "Don't output block information");
  BOOL_VAR_H(wordrec_enable_assoc, TRUE, "Associator Enable");
  BOOL_VAR_H(force_word_assoc, FALSE,
             "force associator to run regardless of what enable_assoc is."
             "This is used for CJK where component grouping is necessary.");
  INT_VAR_H(wordrec_num_seg_states, 30, "Segmentation states");
  double_VAR_H(wordrec_worst_state, 1, "Worst segmentation state");
  BOOL_VAR_H(fragments_guide_chopper, FALSE,
             "Use information from fragments to guide chopping process");
  INT_VAR_H(repair_unchopped_blobs, 1, "Fix blobs that aren't chopped");
  double_VAR_H(tessedit_certainty_threshold, -2.25, "Good blob limit");
  INT_VAR_H(chop_debug, 0, "Chop debug");
  BOOL_VAR_H(chop_enable, 1, "Chop enable");
  BOOL_VAR_H(chop_vertical_creep, 0, "Vertical creep");
  INT_VAR_H(chop_split_length, 10000, "Split Length");
  INT_VAR_H(chop_same_distance, 2, "Same distance");
  INT_VAR_H(chop_min_outline_points, 6, "Min Number of Points on Outline");
  INT_VAR_H(chop_inside_angle, -50, "Min Inside Angle Bend");
  INT_VAR_H(chop_min_outline_area, 2000, "Min Outline Area");
  double_VAR_H(chop_split_dist_knob, 0.5, "Split length adjustment");
  double_VAR_H(chop_overlap_knob, 0.9, "Split overlap adjustment");
  double_VAR_H(chop_center_knob, 0.15, "Split center adjustment");
  double_VAR_H(chop_sharpness_knob, 0.06, "Split sharpness adjustment");
  double_VAR_H(chop_width_change_knob, 5.0, "Width change adjustment");
  double_VAR_H(chop_ok_split, 100.0, "OK split limit");
  double_VAR_H(chop_good_split, 50.0, "Good split limit");
  INT_VAR_H(chop_x_y_weight, 3, "X / Y  length weight");
  INT_VAR_H(segment_adjust_debug, 0, "Segmentation adjustment debug");
  BOOL_VAR_H(assume_fixed_pitch_char_segment, FALSE,
             "include fixed-pitch heuristics in char segmentation");
  BOOL_VAR_H(use_new_state_cost, FALSE,
             "use new state cost heuristics for segmentation state evaluation");
  double_VAR_H(heuristic_segcost_rating_base, 1.25,
               "base factor for adding segmentation cost into word rating."
               "It's a multiplying factor, the larger the value above 1, "
               "the bigger the effect of segmentation cost.");
  double_VAR_H(heuristic_weight_rating, 1,
               "weight associated with char rating in combined cost of state");
  double_VAR_H(heuristic_weight_width, 0,
               "weight associated with width evidence in combined cost of state");
  double_VAR_H(heuristic_weight_seamcut, 0,
               "weight associated with seam cut in combined cost of state");
  double_VAR_H(heuristic_max_char_wh_ratio, 2.0,
               "max char width-to-height ratio allowed in segmentation");
  INT_VAR_H(wordrec_debug_level, 0, "Debug level for wordrec");
  BOOL_VAR_H(enable_new_segsearch, false,
             "Enable new segmentation search path.");
  INT_VAR_H(segsearch_debug_level, 0, "SegSearch debug level");
  INT_VAR_H(segsearch_max_pain_points, 2000,
            "Maximum number of pain points stored in the queue");
  INT_VAR_H(segsearch_max_futile_classifications, 10,
            "Maximum number of pain point classifications per word.");
  double_VAR_H(segsearch_max_char_wh_ratio, 2.0,
               "Maximum character width-to-height ratio");
  double_VAR_H(segsearch_max_fixed_pitch_char_wh_ratio, 2.0,
               "Maximum character width-to-height ratio for"
               "fixed pitch fonts");

  // methods from wordrec/*.cpp ***********************************************
  Wordrec();
  virtual ~Wordrec();

  void CopyCharChoices(const BLOB_CHOICE_LIST_VECTOR &from,
                       BLOB_CHOICE_LIST_VECTOR *to);

  // tface.cpp
  void program_editup(const char *textbase,
                      bool init_classifier,
                      bool init_permute);
  BLOB_CHOICE_LIST_VECTOR *cc_recog(WERD_RES *word);
  void program_editdown(inT32 elasped_time);
  void set_pass1();
  void set_pass2();
  int end_recog();
  BLOB_CHOICE_LIST *call_matcher(TBLOB* blob);
  int dict_word(const WERD_CHOICE &word);
  // wordclass.cpp
  BLOB_CHOICE_LIST *classify_blob(TBLOB *blob,
                                  const char *string,
                                  C_COL color);
  BLOB_CHOICE_LIST *fake_classify_blob(UNICHAR_ID class_id,
                                       float rating, float certainty);
  void update_blob_classifications(TWERD *word,
                                   const BLOB_CHOICE_LIST_VECTOR &choices);

  // bestfirst.cpp
  BLOB_CHOICE_LIST_VECTOR *evaluate_chunks(CHUNKS_RECORD *chunks_record,
                                           SEARCH_STATE search_state);
  void update_ratings(const BLOB_CHOICE_LIST_VECTOR &new_choices,
                      const CHUNKS_RECORD *chunks_record,
                      const SEARCH_STATE search_state);
  inT16 evaluate_state(CHUNKS_RECORD *chunks_record,
                       SEARCH_RECORD *the_search,
                       DANGERR *fixpt);
  SEARCH_RECORD *new_search(CHUNKS_RECORD *chunks_record,
                            int num_joints,
                            BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                            WERD_CHOICE *best_choice,
                            WERD_CHOICE *raw_choice,
                            STATE *state);
  void best_first_search(CHUNKS_RECORD *chunks_record,
                         BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                         WERD_RES *word,
                         STATE *state,
                         DANGERR *fixpt,
                         STATE *best_state);
  void delete_search(SEARCH_RECORD *the_search);
  void expand_node(FLOAT32 worst_priority,
                   CHUNKS_RECORD *chunks_record,
                   SEARCH_RECORD *the_search);
  void replace_char_widths(CHUNKS_RECORD *chunks_record,
                           SEARCH_STATE state);
  // Transfers the given state to the word's output fields: rebuild_word,
  // best_state, box_word, and returns the corresponding blob choices.
  BLOB_CHOICE_LIST_VECTOR *rebuild_current_state(
      WERD_RES *word,
      STATE *state,
      BLOB_CHOICE_LIST_VECTOR *char_choices,
      MATRIX *ratings);
  // Creates a fake blob choice from the combination of the given fragments.
  // unichar is the class to be made from the combination,
  // expanded_fragment_lengths[choice_index] is the number of fragments to use.
  // old_choices[choice_index] has the classifier output for each fragment.
  // choice index initially indexes the last fragment and should be decremented
  // expanded_fragment_lengths[choice_index] times to get the earlier fragments.
  // Guarantees to return something non-null, or abort!
  BLOB_CHOICE* rebuild_fragments(
      const char* unichar,
      const char* expanded_fragment_lengths,
      int choice_index,
      BLOB_CHOICE_LIST_VECTOR *old_choices);
  // Creates a joined copy of the blobs between x and y (inclusive) and
  // insert into the rebuild_word in word.
  // Returns a deep copy of the classifier results for the blob.
  BLOB_CHOICE_LIST *join_blobs_and_classify(
      WERD_RES* word, int x, int y, int choice_index, MATRIX *ratings,
      BLOB_CHOICE_LIST_VECTOR *old_choices);
  STATE *pop_queue(HEAP *queue);
  void push_queue(HEAP *queue, STATE *state, FLOAT32 worst_priority,
                  FLOAT32 priority, bool debug);

  // segsearch.cpp
  // SegSearch works on the lower diagonal matrix of BLOB_CHOICE_LISTs.
  // Each entry in the matrix represents the classification choice
  // for a chunk, i.e. an entry in row 2, column 1 represents the list
  // of ratings for the chunks 1 and 2 classified as a single blob.
  // The entries on the diagonal of the matrix are classifier choice lists
  // for a single chunk from the maximal segmentation.
  //
  // The ratings matrix given to SegSearch represents the segmentation
  // graph / trellis for the current word. The nodes in the graph are the
  // individual BLOB_CHOICEs in each of the BLOB_CHOICE_LISTs in the ratings
  // matrix. The children of each node (nodes connected by outgoing links)
  // are the entries in the column that is equal to node's row+1. The parents
  // (nodes connected by the incoming links) are the entries in the row that
  // is equal to the node's column-1. Here is an example ratings matrix:
  //
  //    0    1    2   3   4
  //  -------------------------
  // 0| c,(                   |
  // 1| d    l,1              |
  // 2|           o           |
  // 3|              c,(      |
  // 4|              g,y  l,1 |
  //  -------------------------
  //
  // In the example above node "o" has children (outgoing connection to nodes)
  // "c","(","g","y" and parents (incoming connections from nodes) "l","1","d".
  //
  // The objective of the search is to find the least cost path, where the cost
  // is determined by the language model components and the properties of the
  // cut between the blobs on the path. SegSearch starts by populating the
  // matrix with the all the entries that were classified by the chopper and
  // finding the initial best path. Based on the classifier ratings, language
  // model scores and the properties of each cut, a list of "pain points" is
  // constructed - those are the points on the path where the choices do not
  // look consistent with the neighboring choices, the cuts look particularly
  // problematic, or the certainties of the blobs are low. The most troublesome
  // "pain point" is picked from the list and the new entry in the ratings
  // matrix corresponding to this "pain point" is filled in. Then the language
  // model state is updated to reflect the new classification and the new
  // "pain points" are added to the list and the next most troublesome
  // "pain point" is determined. This continues until either the word choice
  // composed from the best paths in the segmentation graph is "good enough"
  // (e.g. above a certain certainty threshold, is an unambiguous dictionary
  // word, etc) or there are no more "pain points" to explore.
  void SegSearch(CHUNKS_RECORD *chunks_record,
                 WERD_CHOICE *best_choice,
                 BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                 WERD_CHOICE *raw_choice,
                 STATE *output_best_state);

  // chop.cpp
  PRIORITY point_priority(EDGEPT *point);
  void add_point_to_list(POINT_GROUP point_list, EDGEPT *point);
  int angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3);
  int is_little_chunk(EDGEPT *point1, EDGEPT *point2);
  int is_small_area(EDGEPT *point1, EDGEPT *point2);
  EDGEPT *pick_close_point(EDGEPT *critical_point,
                           EDGEPT *vertical_point,
                           int *best_dist);
  void prioritize_points(TESSLINE *outline, POINT_GROUP points);
  void new_min_point(EDGEPT *local_min, POINT_GROUP points);
  void new_max_point(EDGEPT *local_max, POINT_GROUP points);
  void vertical_projection_point(EDGEPT *split_point, EDGEPT *target_point,
                                 EDGEPT** best_point);

  // chopper.cpp
  SEAM *attempt_blob_chop(TWERD *word, inT32 blob_number, bool italic_blob,
                          SEAMS seam_list);
  bool improve_one_blob(TWERD *word,
                        BLOB_CHOICE_LIST_VECTOR *char_choices,
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
  BLOB_CHOICE_LIST_VECTOR *chop_word_main(WERD_RES *word);
  void improve_by_chopping(WERD_RES *word,
                           BLOB_CHOICE_LIST_VECTOR *char_choices,
                           STATE *best_state,
                           BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                           DANGERR *fixpt,
                           bool *updated_best_choice);
  MATRIX *word_associator(WERD_RES *word,
                          STATE *state,
                          BLOB_CHOICE_LIST_VECTOR *best_char_choices,
                          DANGERR *fixpt,
                          STATE *best_state);
  inT16 select_blob_to_split(const BLOB_CHOICE_LIST_VECTOR &char_choices,
                             float rating_ceiling,
                             bool split_next_to_fragment);

  // findseam.cpp
  void junk_worst_seam(SEAM_QUEUE seams, SEAM *new_seam, float new_priority);
  void choose_best_seam(SEAM_QUEUE seam_queue,
                        SEAM_PILE *seam_pile,
                        SPLIT *split,
                        PRIORITY priority,
                        SEAM **seam_result,
                        TBLOB *blob);
  void combine_seam(SEAM_QUEUE seam_queue, SEAM_PILE seam_pile, SEAM *seam);
  inT16 constrained_split(SPLIT *split, TBLOB *blob);
  void delete_seam_pile(SEAM_PILE seam_pile);
  SEAM *pick_good_seam(TBLOB *blob);
  PRIORITY seam_priority(SEAM *seam, inT16 xmin, inT16 xmax);
  void try_point_pairs (EDGEPT * points[MAX_NUM_POINTS],
                        inT16 num_points,
                        SEAM_QUEUE seam_queue,
                        SEAM_PILE * seam_pile, SEAM ** seam, TBLOB * blob);
  void try_vertical_splits(EDGEPT * points[MAX_NUM_POINTS],
                           inT16 num_points,
                           SEAM_QUEUE seam_queue,
                           SEAM_PILE * seam_pile, SEAM ** seam, TBLOB * blob);

  // gradechop.cpp
  PRIORITY full_split_priority(SPLIT *split, inT16 xmin, inT16 xmax);
  PRIORITY grade_center_of_blob(register BOUNDS_RECT rect);
  PRIORITY grade_overlap(register BOUNDS_RECT rect);
  PRIORITY grade_split_length(register SPLIT *split);
  PRIORITY grade_sharpness(register SPLIT *split);
  PRIORITY grade_width_change(register BOUNDS_RECT rect);
  void set_outline_bounds(register EDGEPT *point1,
                          register EDGEPT *point2,
                          BOUNDS_RECT rect);

  // outlines.cpp
  int crosses_outline(EDGEPT *p0, EDGEPT *p1, EDGEPT *outline);
  int is_crossed(TPOINT a0, TPOINT a1, TPOINT b0, TPOINT b1);
  int is_same_edgept(EDGEPT *p1, EDGEPT *p2);
  EDGEPT *near_point(EDGEPT *point, EDGEPT *line_pt_0, EDGEPT *line_pt_1);
  void reverse_outline(EDGEPT *outline);

  // pieces.cpp
  virtual BLOB_CHOICE_LIST *classify_piece(TBLOB *pieces,
                                           SEAMS seams,
                                           inT16 start,
                                           inT16 end);
  BLOB_CHOICE_LIST *get_piece_rating(MATRIX *ratings,
                                     TBLOB *blobs,
                                     SEAMS seams,
                                     inT16 start,
                                     inT16 end);
  BOUNDS_LIST record_blob_bounds(TBLOB *blobs);
  MATRIX *record_piece_ratings(TBLOB *blobs);

  // heuristic.cpp
  WIDTH_RECORD* state_char_widths(WIDTH_RECORD *chunk_widths,
                                  STATE *state,
                                  int num_joints);
  FLOAT32 get_width_variance(WIDTH_RECORD *wrec, float norm_height);
  FLOAT32 get_gap_variance(WIDTH_RECORD *wrec, float norm_height);
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

  // Member variables.

  LanguageModel *language_model_;
  PRIORITY pass2_ok_split;
  int pass2_seg_states;
  int num_joints;
  int num_pushed;
  int num_popped;
  TALLY states_before_best;
  TALLY best_certainties[2];
  TALLY character_widths;          /* Width histogram */
  BlobMatchTable blob_match_table;
  EVALUATION_ARRAY last_segmentation;
  // Stores the best choice for the previous word in the paragraph.
  // This variable is modified by PAGE_RES_IT when iterating over
  // words to OCR on the page.
  WERD_CHOICE *prev_word_best_choice_;

 protected:
  // Updates the language model state recorded for the child entries specified
  // in pending[starting_col]. Enqueues the children of the updated entries
  // into pending and proceedes to update (and remove from pending) all the
  // remaining entries in pending[col] (col >= starting_col). Upon termination
  // of this function all the pending[col] lists will be empty.
  //
  // The arguments:
  //
  // starting_col: index of the column in chunks_record->ratings from
  // which the update should be started
  //
  // pending: list of entries listing chunks_record->ratings entries
  // that should be updated
  //
  // pain_points: priority heap listing the pain points generated by
  // the language model
  //
  // temp_pain_points: temporary storage for tentative pain points generated
  // by the language model after a single call to LanguageModel::UpdateState()
  // (the agrument is passed in rather than created before each
  // LanguageModel::UpdateState() call to avoid dynamic memory re-allocation)
  //
  // best_choice_bundle: a collection of variables that should be updated
  // if a new best choice is found
  //
  void UpdateSegSearchNodes(int starting_col,
                            SEG_SEARCH_PENDING_LIST *pending[],
                            BestPathByColumn *best_path_by_column[],
                            CHUNKS_RECORD *chunks_record,
                            HEAP *pain_points,
                            BestChoiceBundle *best_choice_bundle);
};


}  // namespace tesseract

#endif  // TESSERACT_WORDREC_WORDREC_H__
