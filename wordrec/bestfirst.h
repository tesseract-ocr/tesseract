/* -*-C-*-
 ********************************************************************************
 *
 * File:        bestfirst.h  (Formerly bestfirst.h)
 * Description:  Best first search functions
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon May 14 11:23:29 1990
 * Modified:     Mon Apr 29 14:21:57 1991 (Mark Seaman) marks@hpgrlt
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
 *******************************************************************************/

#ifndef BESTFIRST_H
#define BESTFIRST_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/

#include "associate.h"
#include "blobs.h"
#include "closed.h"
#include "oldheap.h"
#include "ratngs.h"
#include "seam.h"
#include "states.h"
#include "stopper.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
struct SEARCH_RECORD
{
  HEAP *open_states;
  HASH_TABLE closed_states;
  STATE *this_state;
  STATE *first_state;
  STATE *best_state;
  int num_joints;
  long num_states;
  long before_best;
  float segcost_bias;
  WERD_CHOICE *best_choice;
  WERD_CHOICE *raw_choice;
  BLOB_CHOICE_LIST_VECTOR *best_char_choices;
};


/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int chunks_width(WIDTH_RECORD *width_record, int start_chunk, int last_chunk);
int chunks_gap(WIDTH_RECORD *width_record, int last_chunk);
STATE *pop_queue(HEAP *queue);
void replace_char_widths(CHUNKS_RECORD *chunks_record, SEARCH_STATE state);

#endif
