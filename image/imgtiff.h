/**********************************************************************
 * File:        imgtiff.h  (Formerly tiff.h)
 * Description: Header file for tiff format image reader/writer.
 * Author:      Ray Smith
 * Created:     Mon Jun 11 15:19:41 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef           IMGTIFF_H
#define           IMGTIFF_H

#include          "host.h"

// CountTiffPages
// Returns the number of pages in the file if it is a tiff file, otherwise 0.
int CountTiffPages(FILE* fp);

#endif
