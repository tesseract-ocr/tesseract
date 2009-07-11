/* -*-C-*-
 ********************************************************************************
 *
 * File:        matrix.c  (Formerly matrix.c)
 * Description:  Ratings matrix code. (Used by associator)
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 13:18:47 1990
 * Modified:     Wed Mar 20 09:44:47 1991 (Mark Seaman) marks@hpgrlt
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
#include "matrix.h"

#include "ratngs.h"
#include "unicharset.h"
#include "callcpp.h"

// Print the best guesses out of the match rating matrix.
void MATRIX::print(const UNICHARSET &current_unicharset) {
  cprintf("Ratings Matrix (top choices)\n");

  /* Do each diagonal */
  for (int spread = 0; spread < this->dimension(); spread++) {
    /* For each spot */
    for (int x = 0; x < this->dimension() - spread; x++) {
      /* Process one square */
      BLOB_CHOICE_LIST *rating = this->get(x, x + spread);
      if (rating != NOT_CLASSIFIED) {
        cprintf("\t[%d,%d] : ", x, x + spread);
        // Print first 3 BLOB_CHOICES from ratings.
        BLOB_CHOICE_IT rating_it;
        rating_it.set_to_list(rating);
        int count = 0;
        for (rating_it.mark_cycle_pt();
             count < 3 && !rating_it.cycled_list();
             ++count, rating_it.forward()) {
          UNICHAR_ID unichar_id = rating_it.data()->unichar_id();
          cprintf("%-10s%4.0f%s", current_unicharset.id_to_unichar(unichar_id),
                  rating_it.data()->rating(),
                  (!rating_it.at_last() && count+1 < 3) ? "\t|\t" : "\n");
        }
      }
    }
  }
}
