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

                                 //progress monitor
extern ETEXT_DESC *global_monitor;

int init_tesseract(const char *arg0,
    const char *textbase,
    const char *language,
    const char *configfile,
    int configc,
    const char *const *configv);

int init_tesseract_lm(const char *arg0,
    const char *textbase,
    const char *language,
    const char *configfile,
    int configc,
    const char *const *configv);

void recognize_page(STRING& image_name);
void end_tesseract();

void set_tess_tweak_vars();
#endif
