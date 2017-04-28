// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef           GAP_MAP_H
#define           GAP_MAP_H

#include          "blobbox.h"

class GAPMAP
{
  public:
    GAPMAP(  //constructor
           TO_BLOCK *block);

    ~GAPMAP () {                 //destructor
      if (map != NULL)
        free_mem(map);
    }

    BOOL8 table_gap(               //Is gap a table?
                    inT16 left,    //From here
                    inT16 right);  //To here

  private:
    inT16 total_rows;            //in block
    inT16 min_left;              //Left extreme
    inT16 max_right;             //Right extreme
    inT16 bucket_size;           // half an x ht
    inT16 *map;                  //empty counts
    inT16 map_max;               //map[0..max_map]       defind
    BOOL8 any_tabs;
};

/*-----------------------------*/

extern BOOL_VAR_H (gapmap_debug, FALSE, "Say which blocks have tables");
extern BOOL_VAR_H (gapmap_use_ends, FALSE,
"Use large space at start and end of rows");
extern BOOL_VAR_H (gapmap_no_isolated_quanta, FALSE,
"Ensure gaps not less than 2quanta wide");
extern double_VAR_H (gapmap_big_gaps, 1.75, "xht multiplier");
#endif
