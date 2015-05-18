/**********************************************************************
 * File:        sortflts.cpp  (Formerly sfloats.c)
 * Description: Code to maintain a sorted list of floats.
 * Author:		Ray Smith
 * Created:		Mon Oct  4 16:15:40 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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
 **********************************************************************/

#include          "sortflts.h"

ELISTIZE (SORTED_FLOAT)
/**
 * @name SORTED_FLOATS::add
 *
 * Add a new entry to the sorted lsit of floats.
 */
void SORTED_FLOATS::add(  //add new entry
                        float value,
                        inT32 key) {
  SORTED_FLOAT *new_float = new SORTED_FLOAT (value, key);

  if (list.empty ())
    it.add_after_stay_put (new_float);
  else {
    it.move_to_first ();
    while (!it.at_last () && it.data ()->entry < value)
      it.forward ();
    if (it.data ()->entry < value)
      it.add_after_stay_put (new_float);
    else
      it.add_before_stay_put (new_float);
  }
}


/**
 * @name SORTED_FLOATS::remove
 *
 * Remove an entry from the sorted lsit of floats.
 */

void SORTED_FLOATS::remove(  //remove the entry
                           inT32 key) {
  if (!list.empty ()) {
    for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
      if (it.data ()->address == key) {
        delete it.extract ();
        return;
      }
    }
  }
}


/**
 * @name SORTED_FLOATS::operator[]
 *
 * Return the floating point value of the given index into the list.
 */

float
SORTED_FLOATS::operator[] (      //get an entry
inT32 index                      //to list
) {
  it.move_to_first ();
  return it.data_relative (index)->entry;
}
