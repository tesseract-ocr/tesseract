/**********************************************************************
 * File:        tessembedded.h 
 * Description: Access to initialization functions in embedded environment
 * Author:    Marius Renn
 * Created:   Sun Oct 21
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

#ifndef TESSEMBEDDED_H
#define TESSEMBEDDED_H

#include          "ocrblock.h"
#include          "varable.h"
#include          "notdll.h"

int init_tessembedded(const char *arg0,
                      const char *textbase,
                      const char *configfile,
                      int configc,
                      const char *const *configv);
                      
void tessembedded_read_file(STRING &name,
                            BLOCK_LIST *blocks);

void end_tessembedded();

#endif
