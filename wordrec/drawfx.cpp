/**********************************************************************
 * File:        drawfx.cpp  (Formerly drawfx.c)
 * Description: Draw things to do with feature extraction.
 * Author:		Ray Smith
 * Created:		Mon Jan 27 11:02:16 GMT 1992
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

#include "mfcpch.h"
#include          "debugwin.h"
#include          "werd.h"
#include          "drawfx.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#ifndef GRAPHICS_DISABLED
#define FXDEMOWIN     "FXDemo"
#define FXDEMOXPOS      250
#define FXDEMOYPOS      0
#define FXDEMOXSIZE     600
#define FXDEMOYSIZE     256
#define BLN_MAX       512        //max coord for bln
#define WERDWIDTH       (BLN_MAX*20)
#define DECENT_WERD_WIDTH (5*bln_x_height)
                                 //title of window
#define DEBUG_WIN_NAME    "FXDebug"
#define DEBUG_XPOS      0
#define DEBUG_YPOS      120
#define DEBUG_XSIZE     80
#define DEBUG_YSIZE     32
#define YMAX        3508
#define XMAX        2550
#define MAXEDGELENGTH   1024     //max steps inoutline

#define EXTERN

EXTERN STRING_VAR (fx_debugfile, DEBUG_WIN_NAME, "Name of debugfile");

EXTERN ScrollView* fx_win = NULL;
EXTERN FILE *fx_debug = NULL;

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
  fx_win->Line(-WERDWIDTH, bln_baseline_offset, WERDWIDTH, bln_baseline_offset);
  fx_win->Line(-WERDWIDTH, bln_x_height + bln_baseline_offset, WERDWIDTH, bln_x_height + bln_baseline_offset);
}

#endif  // GRAPHICS_DISABLED

/**********************************************************************
 * create_fxdebug_win
 *
 * Create the fx window used to show the fit.
 **********************************************************************/

void create_fxdebug_win() {  //make gradients win
  //      if (strcmp(fx_debugfile.string(),DEBUG_WIN_NAME)==0)
  //              fx_debug=create_debug_window(fx_debugfile.string(),
  //                      DEBUG_XPOS,DEBUG_YPOS,
  //                      DEBUG_XSIZE,DEBUG_YSIZE);
  //      else
  //              fx_debug=fopen(fx_debugfile.string(),"w");
}
