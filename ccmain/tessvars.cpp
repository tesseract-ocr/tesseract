/**********************************************************************
 * File:        tessvars.cpp  (Formerly tessvars.c)
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

#include "mfcpch.h"
#include          "tessvars.h"

#define EXTERN

EXTERN INT_VAR (tessedit_adapt_kludge, 0,
"Use acceptable result or dangambigs");
EXTERN BOOL_VAR (interactive_mode, FALSE, "Run interactively?");
EXTERN BOOL_VAR (edit_variables, FALSE, "Variables Editor Window?");
// xiaofan EXTERN STRING_VAR(file_type,".bl","Filename extension");
EXTERN STRING_VAR (file_type, ".tif", "Filename extension");
INT_VAR (testedit_match_debug, 0, "Integer match debug ctrl");
EXTERN INT_VAR (tessedit_dangambigs_chop, FALSE,
"Use UnicharAmbigs to direct chop");
EXTERN INT_VAR (tessedit_dangambigs_assoc, FALSE,
"Use UnicharAmbigs to direct assoc");

EXTERN IMAGE page_image;         //image of page
EXTERN FILE *debug_fp = stderr;           //write debug stuff here
