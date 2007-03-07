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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "matchtab.h"
#include "freelist.h"
#include "callcpp.h"
#include "blobs.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef struct _MATCH_
{
  int topleft;
  int botright;
  LIST rating;
} MATCH;

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
MATCH *match_table;
//?int   missed_count = 0;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define NUM_MATCH_ENTRIES 500    /* Entries in match_table */

/**********************************************************************
 * blank_entry
 *
 * Test an element in the blob match table to see if it is blank.
 * Return a non-zero value if it is blank.
 **********************************************************************/

#define blank_entry(match_table,x)  \
(! (match_table[x].topleft | match_table[x].botright))

/*----------------------------------------------------------------------
          Public Function Code
----------------------------------------------------------------------*/
/**********************************************************************
 * init_match_table
 *
 * Create and clear a match table to be used to speed up the splitter.
 **********************************************************************/
static int been_initialized = 0;
void init_match_table() {
  int x;

  if (been_initialized) {
    /* Reclaim old choices */
    for (x = 0; x < NUM_MATCH_ENTRIES; x++) {
      if ((!blank_entry (match_table, x)) && match_table[x].rating)
        destroy_nodes (match_table[x].rating, free_choice);
    }
  }
  else {
    /* Allocate memory once */
    been_initialized = 1;
    match_table = (MATCH *) memalloc (sizeof (MATCH) * NUM_MATCH_ENTRIES);
  }
  /* Initialize the table */
  for (x = 0; x < NUM_MATCH_ENTRIES; x++) {
    match_table[x].topleft = 0;
    match_table[x].botright = 0;
    match_table[x].rating = NULL;
  }
}

void end_match_table() {
  if (been_initialized) {
    init_match_table();
    memfree(match_table);
    match_table = NULL;
    been_initialized = 0;
  }
}


/**********************************************************************
 * put_match
 *
 * Put a new blob and its corresponding match ratings into the match
 * table.
 **********************************************************************/
void put_match(TBLOB *blob, CHOICES ratings) {
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
    if (blank_entry (match_table, x)) {
      /* Add this entry */
      match_table[x].topleft = topleft;
      match_table[x].botright = botright;
      match_table[x].rating = copy_choices (ratings);
      return;
    }
    if (++x >= NUM_MATCH_ENTRIES)
      x = 0;
  }
  while (x != start);

  cprintf ("error: Match table is full\n");
}


/**********************************************************************
 * get_match
 *
 * Look up this blob in the match table to see if it needs to be
 * matched.  If it is not present then NULL is returned.
 **********************************************************************/
CHOICES get_match(TBLOB *blob) {
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
CHOICES get_match_by_bounds(unsigned int topleft, unsigned int botright) {
  unsigned int start;
  int x;
  /* Do starting hash */
  start = (topleft * botright) % NUM_MATCH_ENTRIES;
  /* Search for match */
  x = start;
  do {
    /* Not found when blank */
    if (blank_entry (match_table, x))
      break;
    /* Is this the match ? */
    if (match_table[x].topleft == topleft &&
    match_table[x].botright == botright) {
      return (copy_choices (match_table[x].rating));
    }
    if (++x >= NUM_MATCH_ENTRIES)
      x = 0;
  }
  while (x != start);

  return (NIL);
}
