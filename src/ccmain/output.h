/******************************************************************
 * File:        output.h  (Formerly output.h)
 * Description: Output pass
 * Author:		Phil Cheatle
 * Created:		Thu Aug  4 10:56:08 BST 1994
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

#ifndef           OUTPUT_H
#define           OUTPUT_H

#include          "params.h"
//#include                                      "epapconv.h"
#include          "pageres.h"

/** test line ends */
char determine_newline_type(WERD *word,        ///< word to do
                            BLOCK *block,      ///< current block
                            WERD *next_word,   ///< next word
                            BLOCK *next_block  ///< block of next word
                           );
#endif
