/**********************************************************************
 * File:        tessbox.h  (Formerly tessbox.h)
 * Description: Black boxed Tess for developing a resaljet.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 11:03:36 BST 1992
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

#ifndef           TESSBOX_H
#define           TESSBOX_H

#include          "ratngs.h"
#include          "notdll.h"
#include "tesseractclass.h"

void tess_training_tester(
                          const STRING& filename,
                          PBLOB *blob,
                          DENORM *denorm,
                          BOOL8 correct,
                          char *text,
                          inT32 count,
                          BLOB_CHOICE_LIST *ratings
                         );
#endif
