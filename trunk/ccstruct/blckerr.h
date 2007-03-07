/**********************************************************************
 * File:        blckerr.h  (Formerly blockerr.h)
 * Description: Error codes for the page block classes.
 * Author:					Ray Smith
 * Created:					Tue Mar 19 17:43:30 GMT 1991
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

#ifndef           BLCKERR_H
#define           BLCKERR_H

#include          "errcode.h"

const ERRCODE BADBLOCKLINE = "Y coordinate in block out of bounds";
const ERRCODE LOSTBLOCKLINE = "Can't find rectangle for line";
const ERRCODE ILLEGAL_GRADIENT = "Gradient wrong side of edge step!";
const ERRCODE WRONG_WORD = "Word doesn't have blobs of that type";
#endif
