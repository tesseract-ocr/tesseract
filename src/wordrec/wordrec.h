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

#ifndef TESSERACT_WORDREC_WORDREC_H_
#define TESSERACT_WORDREC_WORDREC_H_

#ifdef DISABLED_LEGACY_ENGINE

#include <cstdint>             // for int16_t, int32_t
#include "classify.h"          // for Classify
#include "params.h"            // for INT_VAR_H, IntParam, BOOL_VAR_H, BoolP...
#include "ratngs.h"            // for WERD_CHOICE

namespace tesseract { class TessdataManager; }

namespace tesseract {

/* ccmain/tstruct.cpp */

class Wordrec : public Classify {
 public:
  // config parameters

  BOOL_VAR_H(wordrec_debug_blamer, false, "Print blamer debug messages");

  BOOL_VAR_H(wordrec_run_blamer, false, "Try to set the blame for errors");

  // methods
  Wordrec();
  virtual ~Wordrec() = default;

  // tface.cpp
  void program_editup(const char *textbase, TessdataManager *init_classifier,
                      TessdataManager *init_dict);
  void program_editdown(int32_t elasped_time);
  int end_recog();
  int dict_word(const WERD_CHOICE &word);

  // Member variables
  WERD_CHOICE *prev_word_best_choice_;
};

}  // namespace tesseract

#else  // DISABLED_LEGACY_ENGINE not defined

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include <cstdint>             // for int16_t, int32_t
#include <memory>
#include "associate.h"
#include "callcpp.h"           // for C_COL
#include "chop.h"              // for PointHeap, MAX_NUM_POINTS
#include "classify.h"          // for Classify
#include "dict.h"
#include "elst.h"              // for ELIST_ITERATOR, ELISTIZEH, ELIST_LINK
#include "findseam.h"          // for SeamQueue, SeamPile
#include "genericvector.h"     // for GenericVector
#include "language_model.h"
#include "matrix.h"
#include "oldlist.h"           // for LIST
#include "params.h"            // for INT_VAR_H, IntParam, BOOL_VAR_H, BoolP...
#include "points.h"            // for ICOORD
#include "ratngs.h"            // for BLOB_CHOICE_LIST (ptr only), BLOB_CHOI...
#include "seam.h"              // for SEAM (ptr only), PRIORITY
#include "stopper.h"           // for DANGERR

class EDGEPT_CLIST;
class MATRIX;
class STRING;
class TBOX;
class UNICHARSET;
class WERD_RES;

namespace tesseract { class LMPainPoints; }
namespace tesseract { class TessdataManager; }
namespace tesseract { struct BestChoiceBundle; }

struct BlamerBundle;
struct EDGEPT;
struct MATRIX_COORD;
struct SPLIT;
struct TBLOB;
struct TESSLINE;
struct TWERD;

namespace tesseract {

// A class for storing which nodes are to be processed by the segmentation
// search. There is a single SegSearchPending for each column in the ratings
// matrix, and it indicates whether the segsearch should combine all
// BLOB_CHOICES in the column, or just the given row with the parents
// corresponding to *this SegSearchPending, and whether only updated parent
// ViterbiStateEntries should be combined, or all, with the BLOB_CHOICEs.
class SegSearchPending {
 public:
  SegSearchPending()
    : classified_row_(-1),
      revisit_whole_column_(false),
      column_classified_(false) {}

  // Marks the whole column as just classified. Used to start a search on
  // a newly initialized ratings matrix.
  void SetColumnClassified() {
    column_classified_ = true;
  }
  // Marks the matrix entry at the given row as just classified.
  // Used after classifying a new matrix cell.
  // Additional to, not overriding a previous RevisitWholeColumn.
  void SetBlobClassified(int row) {
    classified_row_ = row;
  }
  // Marks the whole column as needing work, but not just classified.
  // Used when the parent vse list is updated.
  // Additional to, not overriding a previous SetBlobClassified.
  void RevisitWholeColumn() {
    revisit_whole_column_ = true;
  }

  // Clears *this to indicate no work to do.
  void Clear() {
    classified_row_ = -1;
    revisit_whole_column_ = false;
    column_classified_ = false;
  }

  // Returns true if there are updates to do in the column that *this
  // represents.
  bool WorkToDo() const {
    return revisit_whole_column_ || column_classified_ || classified_row_ >= 0;
  }
  // Returns true if the given row was just classified.
  bool IsRowJustClassified(int row) const {
    return row == classified_row_ || column_classified_;
  }
  // Returns the single row to process if there is only one, otherwise -1.
  int SingleRow() const {
    return revisit_whole_column_ || column_classified_ ? -1 : classified_row_;
  }

 private:
  // If non-negative, indicates the single row in the ratings matrix that has
  // just been classified, and so should be combined with all the parents in the
  // column that this SegSearchPending represents.
  // Operates independently of revisit_whole_column.
  int classified_row_;
  // If revisit_whole_column is true, then all BLOB_CHOICEs in this column will
  // be processed, but classified_row can indicate a row that is newly
  // classified. Overridden if column_classified is true.
  bool revisit_whole_column_;
  // If column_classified is true, parent vses are processed with all rows
  // regardless of whether they are just updated, overriding
  // revisit_whole_column and classified_row.
  bool column_classified_;
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
};
ELISTIZEH(FRAGMENT)


class Wordrec : public Classify {
 public:
  // config parameters *******************************************************
  BOOL_VAR_H(merge_fragments_in_matrix, true,
             "Merge the fragments in the ratings matrix and delete them "
             "after merging");
  BOOL_VAR_H(wordrec_enable_assoc, true, "Associator Enable");
  BOOL_VAR_H(force_word_assoc, false,
             "force associator to run regardless of what enable_assoc is."
             "This is used for CJK where component grouping is necessary.");
  INT_VAR_H(repair_unchopped_blobs, 1, "Fix blobs that aren't chopped");
  double_VAR_H(tessedit_certainty_threshold, -2.25, "Good blob limit");
  INT_VAR_H(chop_debug, 0, "Chop debug");
  BOOL_VAR_H(chop_enable, 1, "Chop enable");
  BOOL_VAR_H(chop_vertical_creep, 0, "Vertical creep");
  INT_VAR_H(chop_split_length, 10000, "Split Length");
  INT_VAR_H(chop_same_distance, 2, "Same distance");
  INT_VAR_H(chop_min_outline_points, 6, "Min Number of Points on Outline");
  INT_VAR_H(chop_seam_pile_size, 150, "Max number of seams in seam_pile");
  BOOL_VAR_H(chop_new_seam_pile, 1, "Use new seam_pile");
  INT_VAR_H(chop_inside_angle, -50, "Min Inside Angle Bend");
  INT_VAR_H(chop_min_outline_area, 2000, "Min Outline Area");
  double_VAR_H(chop_split_dist_knob, 0.5, "Split length adjustment");
  double_VAR_H(chop_overlap_knob, 0.9, "Split overlap adjustment");
  double_VAR_H(chop_center_knob, 0.15, "Split center adjustment");
  INT_VAR_H(chop_centered_maxwidth, 90, "Width of (smaller) chopped blobs "
            "above which we don't care that a chop is not near the center.");
  double_VAR_H(chop_sharpness_knob, 0.06, "Split sharpness adjustment");
  double_VAR_H(chop_width_change_knob, 5.0, "Width change adjustment");
  double_VAR_H(chop_ok_split, 100.0, "OK split limit");
  double_VAR_H(chop_good_split, 50.0, "Good split limit");
  INT_VAR_H(chop_x_y_weight, 3, "X / Y  length weight");
  BOOL_VAR_H(assume_fixed_pitch_char_segment, false,
             "include fixed-pitch heuristics in char segmentation");
  INT_VAR_H(wordrec_debug_level, 0, "Debug level for wordrec");
  INT_VAR_H(wordrec_max_join_chunks, 4,
            "Max number of broken pieces to associate");
  BOOL_VAR_H(wordrec_skip_no_truth_words, false,
             "Only run OCR for words that had truth recorded in BlamerBundle");
  BOOL_VAR_H(wordrec_debug_blamer, false, "Print blamer debug messages");
  BOOL_VAR_H(wordrec_run_blamer, false, "Try to set the blame for errors");
  INT_VAR_H(segsearch_debug_level, 0, "SegSearch debug level");
  INT_VAR_H(segsearch_max_pain_points, 2000,
            "Maximum number of pain points stored in the queue");
  INT_VAR_H(segsearch_max_futile_classifications, 10,
            "Maximum number of pain point classifications per word.");
  double_VAR_H(segsearch_max_char_wh_ratio, 2.0,
               "Maximum character width-to-height ratio");
  BOOL_VAR_H(save_alt_choices, true,
             "Save alternative paths found during chopping "
             "and segmentation search");

  // methods from wordrec/*.cpp ***********************************************
  Wordrec();
  ~Wordrec() override = default;

  // Fills word->alt_choices with alternative paths found during
  // chopping/segmentation search that are kept in best_choices.
  void SaveAltChoices(const LIST &best_choices, WERD_RES *word);

  // Fills character choice lattice in the given BlamerBundle
  // using the given ratings matrix and best choice list.
  void FillLattice(const MATRIX &ratings, const WERD_CHOICE_LIST &best_choices,
                   const UNICHARSET &unicharset, BlamerBundle *blamer_bundle);

  // Calls fill_lattice_ member function
  // (assumes that fill_lattice_ is not nullptr).
  void CallFillLattice(const MATRIX &ratings,
                       const WERD_CHOICE_LIST &best_choices,
                       const UNICHARSET &unicharset,
                       BlamerBundle *blamer_bundle) {
    (this->*fill_lattice_)(ratings, best_choices, unicharset, blamer_bundle);
  }

  // tface.cpp
  void program_editup(const char *textbase, TessdataManager *init_classifier,
                      TessdataManager *init_dict);
  void cc_recog(WERD_RES *word);
  void program_editdown(int32_t elasped_time);
  void set_pass1();
  void set_pass2();
  int end_recog();
  BLOB_CHOICE_LIST *call_matcher(TBLOB* blob);
  int dict_word(const WERD_CHOICE &word);
  // wordclass.cpp
  BLOB_CHOICE_LIST *classify_blob(TBLOB *blob,
                                  const char *string,
                                  C_COL color,
                                  BlamerBundle *blamer_bundle);

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
  //
  // If associate_blobs is set to false no new classifications will be done
  // to combine blobs. Segmentation search will run only one "iteration"
  // on the classifications already recorded in chunks_record.ratings.
  //
  // Note: this function assumes that word_res, best_choice_bundle arguments
  // are not nullptr.
  void SegSearch(WERD_RES* word_res,
                 BestChoiceBundle* best_choice_bundle,
                 BlamerBundle* blamer_bundle);

  // Setup and run just the initial segsearch on an established matrix,
  // without doing any additional chopping or joining.
  // (Internal factored version that can be used as part of the main SegSearch.)
  void InitialSegSearch(WERD_RES* word_res, LMPainPoints* pain_points,
                        GenericVector<SegSearchPending>* pending,
                        BestChoiceBundle* best_choice_bundle,
                        BlamerBundle* blamer_bundle);

  // Runs SegSearch() function (above) without needing a best_choice_bundle
  // or blamer_bundle. Used for testing.
  void DoSegSearch(WERD_RES* word_res);

  // chop.cpp
  PRIORITY point_priority(EDGEPT *point);
  void add_point_to_list(PointHeap* point_heap, EDGEPT *point);
  // Returns true if the edgept supplied as input is an inside angle.  This
  // is determined by the angular change of the vectors from point to point.
  bool is_inside_angle(EDGEPT *pt);
  int angle_change(EDGEPT *point1, EDGEPT *point2, EDGEPT *point3);
  EDGEPT *pick_close_point(EDGEPT *critical_point,
                           EDGEPT *vertical_point,
                           int *best_dist);
  void prioritize_points(TESSLINE *outline, PointHeap* points);
  void new_min_point(EDGEPT *local_min, PointHeap* points);
  void new_max_point(EDGEPT *local_max, PointHeap* points);
  void vertical_projection_point(EDGEPT *split_point, EDGEPT *target_point,
                                 EDGEPT** best_point,
                                 EDGEPT_CLIST *new_points);

  // chopper.cpp
  SEAM *attempt_blob_chop(TWERD *word, TBLOB *blob, int32_t blob_number,
                          bool italic_blob, const GenericVector<SEAM*>& seams);
  SEAM *chop_numbered_blob(TWERD *word, int32_t blob_number,
                           bool italic_blob, const GenericVector<SEAM*>& seams);
  SEAM *chop_overlapping_blob(const GenericVector<TBOX>& boxes,
                              bool italic_blob,
                              WERD_RES *word_res, int *blob_number);
  SEAM *improve_one_blob(const GenericVector<BLOB_CHOICE*> &blob_choices,
                         DANGERR *fixpt,
                         bool split_next_to_fragment,
                         bool italic_blob,
                         WERD_RES *word,
                         int *blob_number);
  SEAM *chop_one_blob(const GenericVector<TBOX> &boxes,
                      const GenericVector<BLOB_CHOICE*> &blob_choices,
                      WERD_RES *word_res,
                      int *blob_number);
  void chop_word_main(WERD_RES *word);
  void improve_by_chopping(float rating_cert_scale,
                           WERD_RES *word,
                           BestChoiceBundle *best_choice_bundle,
                           BlamerBundle *blamer_bundle,
                           LMPainPoints *pain_points,
                           GenericVector<SegSearchPending>* pending);
  int select_blob_to_split(const GenericVector<BLOB_CHOICE*> &blob_choices,
                           float rating_ceiling,
                           bool split_next_to_fragment);
  int select_blob_to_split_from_fixpt(DANGERR *fixpt);

  // findseam.cpp
  void add_seam_to_queue(float new_priority, SEAM *new_seam, SeamQueue* seams);
  void choose_best_seam(SeamQueue *seam_queue, const SPLIT *split,
                        PRIORITY priority, SEAM **seam_result, TBLOB *blob,
                        SeamPile *seam_pile);
  void combine_seam(const SeamPile& seam_pile,
                    const SEAM* seam, SeamQueue* seam_queue);
  SEAM *pick_good_seam(TBLOB *blob);
  void try_point_pairs (EDGEPT * points[MAX_NUM_POINTS],
                        int16_t num_points,
                        SeamQueue* seam_queue,
                        SeamPile* seam_pile,
                        SEAM ** seam, TBLOB * blob);
  void try_vertical_splits(EDGEPT * points[MAX_NUM_POINTS],
                           int16_t num_points,
                           EDGEPT_CLIST *new_points,
                           SeamQueue* seam_queue,
                           SeamPile* seam_pile,
                           SEAM ** seam, TBLOB * blob);

  // gradechop.cpp
  PRIORITY grade_split_length(SPLIT *split);
  PRIORITY grade_sharpness(SPLIT *split);

  // outlines.cpp
  bool near_point(EDGEPT *point, EDGEPT *line_pt_0, EDGEPT *line_pt_1,
                  EDGEPT **near_pt);

  // pieces.cpp
  virtual BLOB_CHOICE_LIST *classify_piece(const GenericVector<SEAM*>& seams,
                                           int16_t start,
                                           int16_t end,
                                           const char* description,
                                           TWERD *word,
                                           BlamerBundle *blamer_bundle);
  // Try to merge fragments in the ratings matrix and put the result in
  // the corresponding row and column
  void merge_fragments(MATRIX *ratings,
                       int16_t num_blobs);
  // Recursively go through the ratings matrix to find lists of fragments
  // to be merged in the function merge_and_put_fragment_lists.
  // current_frag is the position of the piece we are looking for.
  // current_row is the row in the rating matrix we are currently at.
  // start is the row we started initially, so that we can know where
  // to append the results to the matrix. num_frag_parts is the total
  // number of pieces we are looking for and num_blobs is the size of the
  // ratings matrix.
  void get_fragment_lists(int16_t current_frag,
                          int16_t current_row,
                          int16_t start,
                          int16_t num_frag_parts,
                          int16_t num_blobs,
                          MATRIX *ratings,
                          BLOB_CHOICE_LIST *choice_lists);
  // Merge the fragment lists in choice_lists and append it to the
  // ratings matrix
  void merge_and_put_fragment_lists(int16_t row,
                                    int16_t column,
                                    int16_t num_frag_parts,
                                    BLOB_CHOICE_LIST *choice_lists,
                                    MATRIX *ratings);
  // Filter the fragment list so that the filtered_choices only contain
  // fragments that are in the correct position. choices is the list
  // that we are going to filter. fragment_pos is the position in the
  // fragment that we are looking for and num_frag_parts is the the
  // total number of pieces. The result will be appended to
  // filtered_choices.
  void fill_filtered_fragment_list(BLOB_CHOICE_LIST *choices,
                                   int fragment_pos,
                                   int num_frag_parts,
                                   BLOB_CHOICE_LIST *filtered_choices);

  // Member variables.

  std::unique_ptr<LanguageModel> language_model_;
  PRIORITY pass2_ok_split;
  // Stores the best choice for the previous word in the paragraph.
  // This variable is modified by PAGE_RES_IT when iterating over
  // words to OCR on the page.
  WERD_CHOICE *prev_word_best_choice_;
  // Sums of blame reasons computed by the blamer.
  GenericVector<int> blame_reasons_;
  // Function used to fill char choice lattices.
  void (Wordrec::*fill_lattice_)(const MATRIX &ratings,
                                 const WERD_CHOICE_LIST &best_choices,
                                 const UNICHARSET &unicharset,
                                 BlamerBundle *blamer_bundle);

 protected:
  inline bool SegSearchDone(int num_futile_classifications) {
    return (language_model_->AcceptableChoiceFound() ||
            num_futile_classifications >=
            segsearch_max_futile_classifications);
  }

  // Updates the language model state recorded for the child entries specified
  // in pending[starting_col]. Enqueues the children of the updated entries
  // into pending and proceeds to update (and remove from pending) all the
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
  // (the argument is passed in rather than created before each
  // LanguageModel::UpdateState() call to avoid dynamic memory re-allocation)
  //
  // best_choice_bundle: a collection of variables that should be updated
  // if a new best choice is found
  //
  void UpdateSegSearchNodes(
      float rating_cert_scale,
      int starting_col,
      GenericVector<SegSearchPending>* pending,
      WERD_RES *word_res,
      LMPainPoints *pain_points,
      BestChoiceBundle *best_choice_bundle,
      BlamerBundle *blamer_bundle);

  // Process the given pain point: classify the corresponding blob, enqueue
  // new pain points to join the newly classified blob with its neighbors.
  void ProcessSegSearchPainPoint(float pain_point_priority,
                                 const MATRIX_COORD &pain_point,
                                 const char* pain_point_type,
                                 GenericVector<SegSearchPending>* pending,
                                 WERD_RES *word_res,
                                 LMPainPoints *pain_points,
                                 BlamerBundle *blamer_bundle);
  // Resets enough of the results so that the Viterbi search is re-run.
  // Needed when the n-gram model is enabled, as the multi-length comparison
  // implementation will re-value existing paths to worse values.
  void ResetNGramSearch(WERD_RES* word_res,
                        BestChoiceBundle* best_choice_bundle,
                        GenericVector<SegSearchPending>* pending);

  // Add pain points for classifying blobs on the correct segmentation path
  // (so that we can evaluate correct segmentation path and discover the reason
  // for incorrect result).
  void InitBlamerForSegSearch(WERD_RES *word_res,
                              LMPainPoints *pain_points,
                              BlamerBundle *blamer_bundle,
                              STRING *blamer_debug);
};

}  // namespace tesseract

#endif  // DISABLED_LEGACY_ENGINE

#endif  // TESSERACT_WORDREC_WORDREC_H_
