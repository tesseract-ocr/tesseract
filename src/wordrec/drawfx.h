/**********************************************************************
 * File:        drawfx.h
 * Description: Draw things to do with feature extraction.
 * Author:      Ray Smith
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

#ifndef DRAWFX_H
#define DRAWFX_H

#include "params.h"
#include "scrollview.h"

namespace tesseract {

#ifndef GRAPHICS_DISABLED
extern ScrollView *fx_win;
#endif // !GRAPHICS_DISABLED
void create_fx_win();      // make features win
void clear_fx_win();       // make features win
void create_fxdebug_win(); // make gradients win

} // namespace tesseract

#endif
