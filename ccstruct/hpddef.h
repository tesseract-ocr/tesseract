/**********************************************************************
 * File:        hpddef.h
 * Description: Defines for dll symbols for handpd.dll.
 * Author:					Ray Smith
 * Created:					Tue Apr 30 17:15:01 MDT 1996
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
//#ifndef                                                       HPDDEF_H
//#define                                                       HPDDEF_H

#undef DLLSYM
#ifndef __IPEDLL
#             define DLLSYM
#else
#             ifdef __BUILDING_HANDPD__
#             define DLLSYM DLLEXPORT
#             else
#             define DLLSYM DLLIMPORT
#             endif
#endif
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#             pragma import on
#endif

//#endif
