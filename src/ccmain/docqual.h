/******************************************************************
 * File:        docqual.h  (Formerly docqual.h)
 * Description: Document Quality Metrics
 * Author:      Phil Cheatle
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

#ifndef DOCQUAL_H
#define DOCQUAL_H

#include <cstdint> // for int16_t

namespace tesseract {

class PAGE_RES_IT;
class ROW;
class WERD_RES;

enum GARBAGE_LEVEL { G_NEVER_CRUNCH, G_OK, G_DODGY, G_TERRIBLE };

int16_t word_blob_quality(WERD_RES *word);
void reject_whole_page(PAGE_RES_IT &page_res_it);

} // namespace tesseract

#endif
