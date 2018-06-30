/**********************************************************************
 * File:        genblob.h  (Formerly gblob.h)
 * Description: Generic Blob processing routines
 * Author:      Phil Cheatle
 * Created:     Mon Nov 25 10:53:26 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           GENBLOB_H
#define           GENBLOB_H

// Sort function to sort blobs by ascending left edge.
int c_blob_comparator(const void *blob1p,  // ptr to ptr to blob1
                      const void *blob2p);

#endif
