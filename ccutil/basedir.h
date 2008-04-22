/**********************************************************************
 * File:        basedir.h  (Formerly getpath.h)
 * Description: Header file for getpath.c. Provides relocatability of data.
 * Author:      Ray Smith
 * Created:     Mon Jul 09 09:13:03 BST 1990
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

#ifndef           BASEDIR_H
#define           BASEDIR_H

#include          "host.h"
#include          "strngs.h"

#include          "notdll.h"     //must be last include

DLLSYM inT8 getpath(                   //get dir name of code
                    const char *code,  //executable to locate
                    STRING &path       //output path name
                   );
#endif
