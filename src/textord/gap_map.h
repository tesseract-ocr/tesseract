// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GAP_MAP_H
#define GAP_MAP_H

#include "blobbox.h"

class GAPMAP
{
  public:
    GAPMAP(  //constructor
           TO_BLOCK *block);

    ~GAPMAP () {                 //destructor
      delete[] map;
    }

    bool table_gap(               //Is gap a table?
            int16_t left,    //From here
            int16_t right);  //To here

  private:
    int16_t total_rows;            //in block
    int16_t min_left;              //Left extreme
    int16_t max_right;             //Right extreme
    int16_t bucket_size;           // half an x ht
    int16_t *map;                  //empty counts
    int16_t map_max;               //map[0..max_map] defined
    bool any_tabs;
};

/*-----------------------------*/

extern BOOL_VAR_H (gapmap_debug, FALSE, "Say which blocks have tables");
extern BOOL_VAR_H (gapmap_use_ends, FALSE,
"Use large space at start and end of rows");
extern BOOL_VAR_H (gapmap_no_isolated_quanta, FALSE,
"Ensure gaps not less than 2quanta wide");
extern double_VAR_H (gapmap_big_gaps, 1.75, "xht multiplier");

#endif
