///////////////////////////////////////////////////////////////////////
// File:        strtok_r.h
// Description: Header file for strtok_r.cpp
// source: https://github.com/heimdal/heimdal/blob/master/lib/roken/
//              strtok_r.c
// Author:      zdenop
// Created:     Fri Aug 12 23:55:06 CET 2011
//
// (C) Copyright 2011, Google Inc.
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

#ifndef VS2010_PORT_STRTOK_R_H_
#define VS2010_PORT_STRTOK_R_H_

char *strtok_r(char *s1, const char *s2, char **lasts);

#endif  // VS2010_PORT_STRTOK_R_H_
