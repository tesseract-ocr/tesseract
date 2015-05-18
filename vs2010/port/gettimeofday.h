///////////////////////////////////////////////////////////////////////
// File:        gettimeofday.h
// Description: Header file for gettimeofday.cpp
// Author:      tomp2010, zdenop
// Created:     Tue Feb 21 21:38:00 CET 2012
//
// (C) Copyright 2012, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef VS2008_PORT_GETTIMEOFDAY_H_
#define VS2008_PORT_GETTIMEOFDAY_H_

#ifdef _WIN32
#include          <winsock.h>    // timeval is defined in here.
#endif

typedef struct  timezone tz; 

int gettimeofday(struct timeval * tp, struct timezone * tzp);

#endif  // VS2008_PORT_GETTIMEOFDAY_H_
