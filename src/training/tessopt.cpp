/**********************************************************************
 * File:        tessopt.cpp
 * Description: Re-implementation of the unix code.
 * Author:      Ray Smith
 * Created:     Tue Nov 28 05:52:50 MST 1995
 *
 * (C) Copyright 1995, Hewlett-Packard Co.
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

#include <cstring>
#include <cstdio>
#include "tessopt.h"

int tessoptind;
char *tessoptarg;

/**********************************************************************
 * tessopt
 *
 * parse command line args.
 **********************************************************************/

int tessopt (                         //parse args
int32_t argc,                      //arg count
char *argv[],                    //args
const char *arglist                    //string of arg chars
) {
  const char *arg;                     //arg char

  if (tessoptind == 0)
    tessoptind = 1;
  if (tessoptind < argc && argv[tessoptind][0] == '-') {
    arg = strchr (arglist, argv[tessoptind][1]);
    if (arg == nullptr || *arg == ':')
      return '?';                //dud option
    tessoptind++;
    tessoptarg = argv[tessoptind];
    if (arg[1] == ':') {
      if (argv[tessoptind - 1][2] != '\0')
                                 //immediately after
        tessoptarg = argv[tessoptind - 1] + 2;
      else
        tessoptind++;
    }
    return *arg;
  }
  else
    return EOF;
}
