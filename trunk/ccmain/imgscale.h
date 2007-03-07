/**********************************************************************
 * File:        imgscale.h  (Formerly dyn_prog.h)
 * Description: Dynamic programming for smart scaling of images.
 * Author:      Phil Cheatle
 * Created:     Wed Nov 18 16:12:03 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef           IMGSCALE_H
#define           IMGSCALE_H

void dyn_prog(  //The clever bit
              int n,
              int *x,
              int *y,
              int ymax,
              int *oldx,
              int *oldy,
              int oldn,
              float factor);
#endif
