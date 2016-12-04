/**********************************************************************
 * File:        bits16.h  (Formerly bits8.h)
 * Description: Code for 8 bit field class.
 * Author:					Phil Cheatle
 * Created:					Thu Oct 17 10:10:05 BST 1991
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

#ifndef           BITS16_H
#define           BITS16_H

#include "host.h"

class DLLSYM BITS16
{
  public:
    uinT16 val;

    BITS16() {
      val = 0;
    }                            // constructor

    BITS16(               // constructor
           uinT16 init);  // initial val

    void turn_on_bit(                  // flip specified bit
                     uinT8 bit_num) {  // bit to flip 0..7
      val = val | 01 << bit_num;
    }

    void turn_off_bit(                  // flip specified bit
                      uinT8 bit_num) {  // bit to flip 0..7
      val = val & ~(01 << bit_num);
    }

    void set_bit(                // flip specified bit
                 uinT8 bit_num,  // bit to flip 0..7
                 BOOL8 value) {  // value to flip to
      if (value)
        val = val | 01 << bit_num;
      else
        val = val & ~(01 << bit_num);
    }

    BOOL8 bit(                        // access bit
              uinT8 bit_num) const {  // bit to access
      return (val >> bit_num) & 01;
    }
};
#endif
