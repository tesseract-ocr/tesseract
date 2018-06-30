/**********************************************************************
 * File:        underlin.h  (Formerly undrline.h)
 * Description: Code to chop blobs apart from underlines.
 * Author:		Ray Smith
 * Created:		Mon Aug  8 11:14:00 BST 1994
 *
 * (C) Copyright 1994, Hewlett-Packard Ltd.
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

#ifndef           UNDERLIN_H
#define           UNDERLIN_H

#include          "fpchop.h"

extern double_VAR_H (textord_underline_offset, 0.1,
"Fraction of x to ignore");
extern BOOL_VAR_H (textord_restore_underlines, FALSE,
"Chop underlines & put back");
void restore_underlined_blobs(                 //get chop points
                              TO_BLOCK *block  //block to do
                             );
TO_ROW *most_overlapping_row(                    //find best row
                             TO_ROW_LIST *rows,  //list of rows
                             BLOBNBOX *blob      //blob to place
                            );
void find_underlined_blobs(                            //get chop points
                           BLOBNBOX *u_line,           //underlined unit
                           QSPLINE *baseline,          //actual baseline
                           float xheight,              //height of line
                           float baseline_offset,      //amount to shrinke it
                           ICOORDELT_LIST *chop_cells  //places to chop
                          );
void vertical_cunderline_projection(                        //project outlines
                                    C_OUTLINE *outline,     //outline to project
                                    QSPLINE *baseline,      //actual baseline
                                    float xheight,          //height of line
                                    float baseline_offset,  //amount to shrinke it
                                    STATS *lower_proj,      //below baseline
                                    STATS *middle_proj,     //centre region
                                    STATS *upper_proj       //top region
                                   );
#endif
