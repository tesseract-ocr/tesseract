/**********************************************************************
 * File:        notdll.h
 * Description: Defines for dll symbols for any program using a dll.
 * Author:					Ray Smith
 * Created:					Tue Apr 30 16:15:01 MDT 1996
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
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

//This file does NOT use the usual single inclusion code as it
//is necessary to allow it to be executed every time it is included.
//#ifndef                                                       NOTDLL_H
//#define                                                       NOTDLL_H

#undef DLLSYM
#define DLLSYM

//#endif
