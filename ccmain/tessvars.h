/**********************************************************************
 * File:        tessvars.h  (Formerly tessvars.h)
 * Description: Variables and other globals for tessedit.
 * Author:		Ray Smith
 * Created:		Mon Apr 13 13:13:23 BST 1992
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

#ifndef           TESSVARS_H
#define           TESSVARS_H

#include          "varable.h"
#include          "img.h"
#include          "tordmain.h"
#include          "notdll.h"

extern INT_VAR_H (tessedit_adapt_kludge, 0,
"Use acceptable result or dangambigs");
extern BOOL_VAR_H (interactive_mode, FALSE, "Run interactively?");
extern BOOL_VAR_H (edit_variables, FALSE, "Variables Editor Window?");
//xiaofan extern STRING_VAR_H(file_type,".bl","Filename extension");
extern STRING_VAR_H (file_type, ".tif", "Filename extension");
extern INT_VAR_H (tessedit_truncate_wordchoice_log, 10,
"Max words to keep in list");
extern INT_VAR_H (testedit_match_debug, 0, "Integer match debug ctrl");
extern INT_VAR_H (tessedit_truncate_chopper, 1,
"Shorten chopper seam search");
extern INT_VAR_H (tessedit_fix_sideways_chops, 1,
"Fix sideways chop problem");
extern INT_VAR_H (tessedit_dangambigs_chop, FALSE,
"Use UnicharAmbigs to direct chop");
extern INT_VAR_H (tessedit_dangambigs_assoc, FALSE,
"Use UnicharAmbigs to direct assoc");

extern IMAGE page_image;         //image of page
extern FILE *debug_fp;           //write debug stuff here
#endif
