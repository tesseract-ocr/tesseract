/* -*-C-*-
 ********************************************************************************
 *
 * File:        matchtab.c  (Formerly matchtab.c)
 * Description:  Match table to retain blobs that were matched.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon Jan 29 09:00:56 1990
 * Modified:     Tue Mar 19 15:09:06 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 *********************************************************************************/
#include "matchtab.h"

#include "blobs.h"
#include "callcpp.h"
#include "elst.h"
#include "freelist.h"
#include "ratngs.h"

#define NUM_MATCH_ENTRIES 500    /* Entries in match_table */

/**********************************************************************
 * blank_entry
 *
 * Test an element in the blob match table to see if it is blank.
 * Return a non-zero value if it is blank.
 **********************************************************************/
#define blank_entry(match_table,x)  \
  (! (match_table[x].topleft | match_table[x].botright))


namespace tesseract {

BlobMatchTable::BlobMatchTable()
  : been_initialized_(false), match_table_(NULL) {
  init_match_table();
}

BlobMatchTable::~BlobMatchTable() {
  end_match_table();
}

/**********************************************************************
 * init_match_table
 *
 * Create and clear a match table to be used to speed up the splitter.
 **********************************************************************/
void BlobMatchTable::init_match_table() {
  int x;

  if (been_initialized_) {
    /* Reclaim old choices */
    for (x = 0; x < NUM_MATCH_ENTRIES; x++) {
      if ((!blank_entry(match_table_, x)) && match_table_[x].rating)
        match_table_[x].rating->clear();
        delete match_table_[x].rating;
    }
  } else {
    /* Allocate memory once */
    been_initialized_ = true;
    match_table_ = new MATCH[NUM_MATCH_ENTRIES];
  }
  /* Initialize the table */
  for (x = 0; x < NUM_MATCH_ENTRIES; x++) {
    match_table_[x].topleft = 0;
    match_table_[x].botright = 0;
    match_table_[x].rating = NULL;
  }
}

void BlobMatchTable::end_match_table() {
  if (been_initialized_) {
    init_match_table();
    delete[] match_table_;
    match_table_ = NULL;
    been_initialized_ = false;
  }
}


/**********************************************************************
 * put_match
 *
 * Put a new blob and its corresponding match ratings into the match
 * table.
 **********************************************************************/
void BlobMatchTable::put_match(TBLOB *blob, BLOB_CHOICE_LIST *ratings) {
  unsigned int topleft;
  unsigned int botright;
  unsigned int start;
  TPOINT tp_topleft;
  TPOINT tp_botright;
  int x;
  /* Hash into table */
  blob_bounding_box(blob, &tp_topleft, &tp_botright);
  topleft = *(unsigned int *) &tp_topleft;
  botright = *(unsigned int *) &tp_botright;
  start = (topleft * botright) % NUM_MATCH_ENTRIES;

  /* Look for empty */
  x = start;
  do {
    if (blank_entry (match_table_, x)) {
      /* Add this entry */
      match_table_[x].topleft = topleft;
      match_table_[x].botright = botright;
      // Copy ratings to match_table_[x].rating
      match_table_[x].rating = new BLOB_CHOICE_LIST();
      match_table_[x].rating->deep_copy(ratings, &BLOB_CHOICE::deep_copy);
      return;
    }
    if (++x >= NUM_MATCH_ENTRIES)
      x = 0;
  } while (x != start);

  cprintf ("error: Match table is full\n");
}


/**********************************************************************
 * get_match
 *
 * Look up this blob in the match table to see if it needs to be
 * matched.  If it is not present then NULL is returned.
 **********************************************************************/
BLOB_CHOICE_LIST *BlobMatchTable::get_match(TBLOB *blob) {
  unsigned int topleft;
  unsigned int botright;
  TPOINT tp_topleft;
  TPOINT tp_botright;
  /* Do starting hash */
  blob_bounding_box(blob, &tp_topleft, &tp_botright);
  topleft = *(unsigned int *) &tp_topleft;
  botright = *(unsigned int *) &tp_botright;
  return (get_match_by_bounds (topleft, botright));
}


/**********************************************************************
 * get_match_by_bounds
 *
 * Look up this blob in the match table to see if it needs to be
 * matched.  If it is not present then NULL is returned.
 **********************************************************************/
BLOB_CHOICE_LIST *BlobMatchTable::get_match_by_bounds(unsigned int topleft,
                                                      unsigned int botright) {
  unsigned int start;
  int x;
  /* Do starting hash */
  start = (topleft * botright) % NUM_MATCH_ENTRIES;
  /* Search for match */
  x = start;
  do {
    /* Not found when blank */
    if (blank_entry (match_table_, x))
      break;
    /* Is this the match ? */
    if (match_table_[x].topleft == topleft &&
        match_table_[x].botright == botright) {
      BLOB_CHOICE_LIST *blist = new BLOB_CHOICE_LIST();
      blist->deep_copy(match_table_[x].rating, &BLOB_CHOICE::deep_copy);
      return blist;
    }
    if (++x >= NUM_MATCH_ENTRIES)
      x = 0;
  } while (x != start);
  return NULL;
}

/**********************************************************************
 * add_to_match
 *
 * Update ratings list in the match_table corresponding to the given
 * blob. The function assumes that:
 * -- the match table contains the initial non-NULL list with choices
 *    for the given blob
 * -- the new ratings list is a superset of the corresponding list in
 *    the match_table and the unichar ids of the blob choices in the
 *    list are unique.
 * The entries that appear in the new ratings list and not in the
 * old one are added to the old ratings list in the match_table.
 **********************************************************************/
void BlobMatchTable::add_to_match(TBLOB *blob, BLOB_CHOICE_LIST *ratings) {
  unsigned int topleft;
  unsigned int botright;
  TPOINT tp_topleft;
  TPOINT tp_botright;
  blob_bounding_box(blob, &tp_topleft, &tp_botright);
  topleft = *(unsigned int *) &tp_topleft;
  botright = *(unsigned int *) &tp_botright;
  unsigned int start;
  int x;
  /* Do starting hash */
  start = (topleft * botright) % NUM_MATCH_ENTRIES;
  /* Search for match */
  x = start;
  do {
    if (blank_entry(match_table_, x)) {
      fprintf(stderr, "Can not update uninitialized entry in match_table\n");
      ASSERT_HOST(!blank_entry(match_table_, x));
    }
    if (match_table_[x].topleft == topleft &&
        match_table_[x].botright == botright) {
      // Copy new ratings to match_table_[x].rating.
      BLOB_CHOICE_IT it;
      it.set_to_list(match_table_[x].rating);
      BLOB_CHOICE_IT new_it;
      new_it.set_to_list(ratings);
      assert(it.length() <= new_it.length());
      for (it.mark_cycle_pt(), new_it.mark_cycle_pt();
           !it.cycled_list() && !new_it.cycled_list(); new_it.forward()) {
        if (it.data()->unichar_id() == new_it.data()->unichar_id()) {
          it.forward();
        } else {
          it.add_before_stay_put(new BLOB_CHOICE(*(new_it.data())));
        }
      }
      return;
    }
    if (++x >= NUM_MATCH_ENTRIES)
      x = 0;
  } while (x != start);
}

}  // namespace tesseract
