/**********************************************************************
 * File:        mainblk.h  (Formerly main.h)
 * Description: Function to call from main() to setup.
 * Author:					Ray Smith
 * Created:					Tue Oct 22 11:09:40 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef           MAINBLK_H
#define           MAINBLK_H

#include          "varable.h"
#include          "notdll.h"

extern DLLSYM STRING datadir;    //dir for data files
                                 //name of image
extern DLLSYM STRING imagebasename;
extern BOOL_VAR_H(m_print_variables, FALSE,
                  "Print initial values of all variables");
extern STRING_VAR_H(m_data_sub_dir, "data/", "Directory for data files");
extern INT_VAR_H(memgrab_size, 13000000, "Preallocation size for batch use");
// > ccutil.h
//void main_setup(                         /*main demo program */
//                const char *argv0,       //program name
//                const char *basename,    //name of image
//                int argc,                /*argument count */
//                const char *const *argv  /*arguments */
//               );
#endif
