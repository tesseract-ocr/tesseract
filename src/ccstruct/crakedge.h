/**********************************************************************
 * File:        crakedge.h      (Formerly: crkedge.h)
 * Description: Structures for the Crack following edge detector.
 * Author:      Ray Smith
 * Created:     Fri Mar 22 16:06:38 GMT 1991
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

#ifndef CRAKEDGE_H
#define CRAKEDGE_H

#include "points.h"
#include "mod128.h"

class CRACKEDGE {
 public:
  CRACKEDGE() = default;

  ICOORD pos;                  /*position of crack */
  int8_t stepx;                  //edge step
  int8_t stepy;
  int8_t stepdir;                //chaincode
  CRACKEDGE *prev;             /*previous point */
  CRACKEDGE *next;             /*next point */
};
#endif
