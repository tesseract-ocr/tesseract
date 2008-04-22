/**********************************************************************
 * File:        points.c  (Formerly coords.c)
 * Description: Member functions for coordinate classes.
 * Author:					Ray Smith
 * Created:					Fri Mar 15 08:58:17 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers
#include          "serialis.h"
#include          "points.h"

ELISTIZE_S (ICOORDELT)           //turn to list
bool FCOORD::normalise() {  //Convert to unit vec
  float len = length ();

  if (len < 0.0000000001) {
    return false;
  }
  xcoord /= len;
  ycoord /= len;
  return true;
}


void ICOORD::serialise_asc(         //convert to ascii
                           FILE *f  //file to write
                          ) {
  serialise_INT32(f, xcoord);
  serialise_INT32(f, ycoord);
}


void ICOORD::de_serialise_asc(         //convert from ascii
                              FILE *f  //file to write
                             ) {
  xcoord = (inT16) de_serialise_INT32 (f);
  ycoord = (inT16) de_serialise_INT32 (f);
}


void ICOORDELT::serialise_asc(         //convert to ascii
                              FILE *f  //file to write
                             ) {
  ((ICOORD *) this)->serialise_asc (f);
}


void ICOORDELT::de_serialise_asc(         //convert from ascii
                                 FILE *f  //file to write
                                ) {
  ((ICOORD *) this)->de_serialise_asc (f);
}
