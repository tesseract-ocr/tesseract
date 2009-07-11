/* -*-C-*-
 ********************************************************************************
 *
 * File:         wordclass.h
 * Description:  Word level classifier
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon Feb  5 11:42:51 1990
 * Modified:     Thu Apr 18 14:33:24 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
 *********************************************************************************/
#ifndef WERDCLASSH
#define WERDCLASSH

#include "callcpp.h"
#include "ratngs.h"
#include "states.h"
#include "tessclas.h"

/*----------------------------------------------------------------------
            Variables
----------------------------------------------------------------------*/
extern inT16 first_pass;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void write_text_files(TWERD *word,
                      char *raw_choice,
                      int same_row,
                      int good_word,
                      int firstpass);
#endif
