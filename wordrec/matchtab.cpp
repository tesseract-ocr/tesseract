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
#include "helpers.h"
#include "ratngs.h"

#define NUM_MATCH_ENTRIES 500    /* Entries in match_table */

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
  if (been_initialized_) {
    /* Reclaim old choices */
    for (int x = 0; x < NUM_MATCH_ENTRIES; x++) {
      if (!IsEmpty(x)) {
        match_table_[x].rating->clear();
        delete match_table_[x].rating;
        // Reinitialize the entry.
        match_table_[x].box = TBOX();
        match_table_[x].rating = NULL;
      }
    }
  } else {
    /* Allocate memory once */
    match_table_ = new MATCH[NUM_MATCH_ENTRIES];
    been_initialized_ = true;
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
  if (!blob) return;
  /* Hash into table */
  TBOX bbox(blob->bounding_box());
  int start = Hash(bbox);

  /* Look for empty */
  int x = start;
  do {
    if (IsEmpty(x)) {
      /* Add this entry */
      match_table_[x].box = bbox;
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
  return get_match_by_box(blob->bounding_box());
}

/**********************************************************************
 * Hash
 *
 * The hash function we use to translate a bounding box to a starting
 * hash position in our array.
 **********************************************************************/
int BlobMatchTable::Hash(const TBOX &box) const {
  int topleft = (box.top() << 16) + box.left();
  int botright = (box.bottom() << 16) + box.right();
  return Modulo(topleft + botright, NUM_MATCH_ENTRIES);
}

/**********************************************************************
 * IsEmpty
 *
 * Returns whether the idx entry in the array is still empty.
 **********************************************************************/
bool BlobMatchTable::IsEmpty(int idx) const {
  return TBOX() == match_table_[idx].box &&
      NULL == match_table_[idx].rating;
}

/**********************************************************************
 * get_match_by_box
 *
 * Look up this blob in the match table to see if it needs to be
 * matched.  If it is not present then NULL is returned.
 **********************************************************************/
BLOB_CHOICE_LIST *BlobMatchTable::get_match_by_box(const TBOX &box) {
  int start = Hash(box);
  int x = start;
  /* Search for match */
  do {
    /* Not found when blank */
    if (IsEmpty(x))
      break;
    /* Is this the match ? */
    if (match_table_[x].box == box) {
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
  TBOX bbox = blob->bounding_box();
  int start = Hash(bbox);
  int x = start;
  do {
    if (IsEmpty(x)) {
      fprintf(stderr, "Can not update uninitialized entry in match_table\n");
      ASSERT_HOST(!IsEmpty(x));
    }
    if (match_table_[x].box == bbox) {
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
