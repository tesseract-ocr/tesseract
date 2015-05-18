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

#include          "bits16.h"

/**********************************************************************
 * Constructor. Something to get it past the compiler as almost all inlined.
 *
 **********************************************************************/
BITS16::BITS16(                // constructor
               uinT16 init) {  // initial val
  val = init;
}
