/**********************************************************************
 * File:        fileerr.h  (Formerly filerr.h)
 * Description: Errors for file utilities.
 * Author:      Ray Smith
 * Created:     Tue Aug 14 15:45:16 BST 1990
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

#ifndef           FILEERR_H
#define           FILEERR_H

#include          "errcode.h"

const ERRCODE CANTOPENFILE = "Can't open file";
const ERRCODE CANTCREATEFILE = "Can't create file";
const ERRCODE CANTMAKEPIPE = "Can't create pipe";
const ERRCODE CANTCONNECTPIPE = "Can't reconnect pipes to stdin/stdout";
const ERRCODE READFAILED = "Read of file failed";
const ERRCODE WRITEFAILED = "Write of file failed";
const ERRCODE SELECTFAILED = "Select failed";

const ERRCODE EXECFAILED = "Could not exec new process";
#endif
