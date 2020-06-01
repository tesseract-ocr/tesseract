/**********************************************************************
 * File:        drawfx.cpp  (Formerly drawfx.c)
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include          "drawfx.h"
#include          "normalis.h"
#include          "werd.h"

#ifndef GRAPHICS_DISABLED
#define FXDEMOWIN     "FXDemo"
#define FXDEMOXPOS      250
#define FXDEMOYPOS      0
#define FXDEMOXSIZE     600
#define FXDEMOYSIZE     256
#define BLN_MAX       512        //max coord for bln
#define WERDWIDTH       (BLN_MAX*20)
                                 //title of window
#define DEBUG_WIN_NAME    "FXDebug"

STRING_VAR(fx_debugfile, DEBUG_WIN_NAME, "Name of debugfile");

ScrollView* fx_win = nullptr;
FILE* fx_debug = nullptr;

/**********************************************************************
 * create_fx_win
 *
 * Create the fx window used to show the fit.
 **********************************************************************/

void create_fx_win() {  //make features win
  fx_win = new ScrollView (FXDEMOWIN,
    FXDEMOXPOS, FXDEMOYPOS, FXDEMOXSIZE, FXDEMOYSIZE,
    WERDWIDTH*2, BLN_MAX*2, true);
}


/**********************************************************************
 * clear_fx_win
 *
 * Clear the fx window and draw on the base/mean lines.
 **********************************************************************/

void clear_fx_win() {  //make features win
  fx_win->Clear();
  fx_win->Pen(64,64,64);
  fx_win->Line(-WERDWIDTH, kBlnBaselineOffset, WERDWIDTH, kBlnBaselineOffset);
  fx_win->Line(-WERDWIDTH, kBlnXHeight + kBlnBaselineOffset, WERDWIDTH,
               kBlnXHeight + kBlnBaselineOffset);
}

#endif  // GRAPHICS_DISABLED

/**********************************************************************
 * create_fxdebug_win
 *
 * Create the fx window used to show the fit.
 **********************************************************************/

void create_fxdebug_win() {  //make gradients win
}
