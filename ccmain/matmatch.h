/**********************************************************************
 * File:        matmatch.h  (Formerly matrix_match.h)
 * Description: matrix matching routines for Tessedit
 * Author:      Chris Newton
 * Created:     Wed Nov 24 15:57:41 GMT 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef           MATMATCH_H
#define           MATMATCH_H

#include          "img.h"
#include          "hosthplb.h"
#include          "notdll.h"

#define BINIM_BLACK 0
#define BINIM_WHITE 1
#define BAD_MATCH 9999.0

extern BOOL_VAR_H (tessedit_display_mm, FALSE, "Display matrix matches");
extern BOOL_VAR_H (tessedit_mm_debug, FALSE,
"Print debug information for matrix matcher");
extern INT_VAR_H (tessedit_mm_prototype_min_size, 3,
"Smallest number of samples in a cluster for a prototype to be used");
float matrix_match(  // returns match score
                   IMAGE *image1,
                   IMAGE *image2);
float match1(  /* returns match score */
             IMAGE *image_w,
             IMAGE *image_n);
void display_images(IMAGE *image_w, IMAGE *image_n, IMAGE *match_image);
ScrollView* display_image(IMAGE *image,
                     const char *title,
                     inT32 x,
                     inT32 y,
                     BOOL8 wait);
#endif
