/* -*-C-*-
 ********************************************************************************
 *
 * File:        djmenus.h  (Formerly djmenus.h)
 * Description:  Create and initialize menus
 * Author:       Mark Seaman, OCR Technology
 * Created:      Mon Sep 24 09:34:21 1990
 * Modified:     Thu Apr 18 14:56:19 1991 (Dan Johnson) danj@hpgrlj
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
 ********************************************************************************/
#ifndef DJMENUS_H
#define DJMENUS_H

void dj_cleanup(); 

void dj_statistics(FILE *File); 

void init_dj_debug(); 
#endif
