/**********************************************************************
 * File:        tstruct.h  (Formerly tstruct.h)
 * Description: Code to manipulate the structures of the C++/C interface.
 * Author:		Ray Smith
 * Created:		Thu Apr 23 15:49:29 BST 1992
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

#ifndef           TSTRUCT_H
#define           TSTRUCT_H

#include          "werd.h"
#include          "blobs.h"
#include          "ratngs.h"
#include          "notdll.h"

TBLOB *make_tess_blob(PBLOB *blob);
TESSLINE *make_tess_outlines(OUTLINE_LIST *outlinelist,  // List to convert
                             bool is_holes);  // These are hole outlines.
EDGEPT *make_tess_edgepts(                          //make tess edgepts
                          POLYPT_LIST *edgeptlist,  //list to convert
                          TPOINT &tl,               //bounding box
                          TPOINT &br);
#endif
