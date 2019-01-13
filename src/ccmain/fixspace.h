/******************************************************************
 * File:        fixspace.h  (Formerly fixspace.h)
 * Description: Implements a pass over the page res, exploring the alternative
 *              spacing possibilities, trying to use context to improve the
 *              word spacing
 * Author:      Phil Cheatle
 * Created:     Thu Oct 21 11:38:43 BST 1993
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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

#ifndef FIXSPACE_H
#define FIXSPACE_H

class WERD_RES;
class WERD_RES_LIST;

void initialise_search(WERD_RES_LIST &src_list, WERD_RES_LIST &new_list);
void transform_to_next_perm(WERD_RES_LIST &words);
void fixspace_dbg(WERD_RES *word);

#endif
