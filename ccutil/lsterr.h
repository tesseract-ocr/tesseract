/**********************************************************************
 * File:        lsterr.h  (Formerly listerr.h)
 * Description: Errors shared by list modules
 * Author:      Phil Cheatle
 * Created:     Wed Jan 23 09:10:35 GMT 1991
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

#include          "errcode.h"    //must be last include

#ifndef LSTERR_H
#define LSTERR_H

const ERRCODE DONT_CONSTRUCT_LIST_BY_COPY =
"Can't create a list by assignment";
const ERRCODE DONT_ASSIGN_LISTS = "Can't assign to lists";
const ERRCODE SERIALISE_LINKS = "Attempted to (de)serialise a link element";

#ifndef NDEBUG

const ERRCODE NO_LIST = "Iterator not set to a list";
const ERRCODE NULL_OBJECT = "List found this = NULL!";
const ERRCODE NULL_DATA = "List would have returned a NULL data pointer";
const ERRCODE NULL_CURRENT = "List current position is NULL";
const ERRCODE NULL_NEXT = "Next element on the list is NULL";
const ERRCODE NULL_PREV = "Previous element on the list is NULL";
const ERRCODE EMPTY_LIST = "List is empty";
const ERRCODE BAD_PARAMETER = "List parameter error";
const ERRCODE STILL_LINKED =
    "Attempting to add an element with non NULL links, to a list";
#endif
#endif
