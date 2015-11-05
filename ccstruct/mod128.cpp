/**********************************************************************
 * File:        mod128.c  (Formerly dir128.c)
 * Description: Code to convert a DIR128 to an ICOORD.
 * Author:					Ray Smith
 * Created:					Tue Oct 22 11:56:09 BST 1991
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

#include          "mod128.h"

const inT16 idirtab[] = {
  1000, 0, 998, 49, 995, 98, 989, 146,
  980, 195, 970, 242, 956, 290, 941, 336,
  923, 382, 903, 427, 881, 471, 857, 514,
  831, 555, 803, 595, 773, 634, 740, 671,
  707, 707, 671, 740, 634, 773, 595, 803,
  555, 831, 514, 857, 471, 881, 427, 903,
  382, 923, 336, 941, 290, 956, 242, 970,
  195, 980, 146, 989, 98, 995, 49, 998,
  0, 1000, -49, 998, -98, 995, -146, 989,
  -195, 980, -242, 970, -290, 956, -336, 941,
  -382, 923, -427, 903, -471, 881, -514, 857,
  -555, 831, -595, 803, -634, 773, -671, 740,
  -707, 707, -740, 671, -773, 634, -803, 595,
  -831, 555, -857, 514, -881, 471, -903, 427,
  -923, 382, -941, 336, -956, 290, -970, 242,
  -980, 195, -989, 146, -995, 98, -998, 49,
  -1000, 0, -998, -49, -995, -98, -989, -146,
  -980, -195, -970, -242, -956, -290, -941, -336,
  -923, -382, -903, -427, -881, -471, -857, -514,
  -831, -555, -803, -595, -773, -634, -740, -671,
  -707, -707, -671, -740, -634, -773, -595, -803,
  -555, -831, -514, -857, -471, -881, -427, -903,
  -382, -923, -336, -941, -290, -956, -242, -970,
  -195, -980, -146, -989, -98, -995, -49, -998,
  0, -1000, 49, -998, 98, -995, 146, -989,
  195, -980, 242, -970, 290, -956, 336, -941,
  382, -923, 427, -903, 471, -881, 514, -857,
  555, -831, 595, -803, 634, -773, 671, -740,
  707, -707, 740, -671, 773, -634, 803, -595,
  831, -555, 857, -514, 881, -471, 903, -427,
  923, -382, 941, -336, 956, -290, 970, -242,
  980, -195, 989, -146, 995, -98, 998, -49
};

const ICOORD *dirtab = (ICOORD *) idirtab;

/**********************************************************************
 * DIR128::DIR128
 *
 * Quantize the direction of an FCOORD to make a DIR128.
 **********************************************************************/

DIR128::DIR128(                 //from fcoord
               const FCOORD fc  //vector to quantize
              ) {
  int high, low, current;        //binary search

  low = 0;
  if (fc.y () == 0) {
    if (fc.x () >= 0)
      dir = 0;
    else
      dir = MODULUS / 2;
    return;
  }
  high = MODULUS;
  do {
    current = (high + low) / 2;
    if (dirtab[current] * fc >= 0)
      low = current;
    else
      high = current;
  }
  while (high - low > 1);
  dir = low;
}


/**********************************************************************
 * dir_to_gradient
 *
 * Convert a direction to a vector.
 **********************************************************************/

#if 0 // code is buggy for negative dir and unused
ICOORD DIR128::vector() const {  //convert to vector
  return dirtab[dir];            //easy really
}
#endif
