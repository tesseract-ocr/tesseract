/**********************************************************************
 * File:        tessedit.h  (Formerly tessedit.h)
 * Description: Main program for merge of tess and editor.
 * Author:		Ray Smith
 * Created:		Tue Jan 07 15:21:46 GMT 1992
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

#ifndef           TESSEDIT_H
#define           TESSEDIT_H

#include          "tessclas.h"
#include          "ocrclass.h"
#include                    "pgedit.h"
#include          "notdll.h"

// Includes libtiff if HAVE_LIBTIFF is defined
#ifdef HAVE_LIBTIFF
#ifdef GOOGLE3
#include "third_party/tiff/tiffio.h"
#else
#include "tiffio.h"
#endif
#endif

                                 //progress monitor
extern ETEXT_DESC *global_monitor;

int init_tesseract(const char *arg0,
                   const char *textbase,
                   const char *configfile,
                   int configc,
                   const char *const *configv);
void recognize_page(STRING& image_name);
void end_tesseract();

#ifdef _TIFFIO_
void read_tiff_image(TIFF* tif, IMAGE* image);
#endif

//handle for "MODES"
void extend_menu(RADIO_MENU *modes_menu,
                 INT16 modes_id_base,         //mode cmd ids offset
                 NON_RADIO_MENU *other_menu,  //handle for "OTHER"
                 INT16 other_id_base          //mode cmd ids offset
                );
                                 //current mode
void extend_moded_commands(INT32 mode,
                           BOX selection_box  //area selected
                          );
                                 //current mode
void extend_unmoded_commands(INT32 cmd_event,
                             char *new_value  //changed value if any
                            );
void set_tess_tweak_vars();
#endif
