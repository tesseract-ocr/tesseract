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

#include "callcpp.h"
#include "ratngs.h"
#include "tprintf.h"
#include "unicharset.h"

// Print the best guesses out of the match rating matrix.
void MATRIX::print(const UNICHARSET &unicharset) const {
  tprintf("Ratings Matrix (top choices)\n");
  int row, col;
  for (col = 0; col < this->dimension(); ++col) tprintf("\t%d", col);
  tprintf("\n");
  for (row = 0; row < this->dimension(); ++row) {
    for (col = 0; col <= row; ++col) {
      if (col == 0) tprintf("%d\t", row);
      BLOB_CHOICE_LIST *rating = this->get(col, row);
      if (rating != NOT_CLASSIFIED) {
        BLOB_CHOICE_IT b_it(rating);
        int counter = 0;
        for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
          tprintf("%s ", unicharset.id_to_unichar(b_it.data()->unichar_id()));
          ++counter;
          if (counter == 3) break;
        }
        tprintf("\t");
      } else {
        tprintf(" \t");
      }
    }
    tprintf("\n");
  }
}
