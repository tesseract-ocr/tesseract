/**********************************************************************
 * File:        tfacepp.h  (Formerly tface++.h)
 * Description: C++ side of the C/C++ Tess/Editor interface.
 * Author:					Ray Smith
 * Created:					Thu Apr 23 15:39:23 BST 1992
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

#ifndef           TFACEPP_H
#define           TFACEPP_H

#include          "varable.h"
#include          "tstruct.h"
#include          "ratngs.h"
#include          "tessclas.h"
#include          "notdll.h"
#include          "tesseractclass.h"

extern BOOL_VAR_H (tessedit_override_permuter, TRUE,
"According to dict_word");
void call_tester(                     //call a tester
                 TBLOB *tessblob,     //blob to test
                 BOOL8 correct_blob,  //true if good
                 char *text,          //source text
                 inT32 count,         //chars in text
                 LIST result          //output of matcher
                );
void call_train_tester(                     //call a tester
                       TBLOB *tessblob,     //blob to test
                       BOOL8 correct_blob,  //true if good
                       char *text,          //source text
                       inT32 count,         //chars in text
                       LIST result          //output of matcher
                      );
#endif
