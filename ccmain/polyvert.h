/**********************************************************************
 * File:        polyvert.h  (Formerly polypt.h)
 * Description: Code for the POLYPT class.
 * Author:		Ray Smith
 * Created:		Wed Oct 23 11:02:56 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           POLYVERT_H
#define           POLYVERT_H

#include          "elst2.h"
#include          "rect.h"

class POLYPT:public ELIST2_LINK
{
  public:
    POLYPT() {  //empty
    }
    POLYPT(                         //constructor
           const FCOORD &position,  //coords
           const FCOORD &vector) {  //step to next
      pos = position;
      vec = vector;              //just copy
    }

    void prep_serialise() {  //set ptrs to counts
    }
    void dump(  //write external bits
              FILE *) {
    }
    void de_dump(  //read external bits
                 FILE *) {
    }

    static POLYPT* deep_copy(const POLYPT* src) {
      return new POLYPT(*src);
    }
                                 //really simple
    make_serialise (POLYPT)

    FCOORD pos;
    FCOORD vec;                  //vector to next
};

ELIST2IZEH_S (POLYPT)
#endif
