/**********************************************************************
 * File:        errcode.h  (Formerly error.h)
 * Description: Header file for generic error handler class
 * Author:      Ray Smith
 * Created:     Tue May  1 16:23:36 BST 1990
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

#ifndef           GLOBALOC_H
#define           GLOBALOC_H

#include          "host.h"

// Saves a clone of the given pix, and notes its resolution in thread-specific
// data, so that the image can be written prior to a crash.
struct Pix;
void SavePixForCrash(int resolution, Pix* pix);

void signal_exit(int signal_code);

void TESS_API err_exit(); 

void set_global_loc_code(int loc_code); 

void set_global_subloc_code(int loc_code); 

void set_global_subsubloc_code(int loc_code); 
#endif
