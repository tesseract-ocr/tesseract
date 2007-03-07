/**********************************************************************
 * File:        getopt.c
 * Description: Re-implementation of the unix code.
 * Author:					Ray Smith
 * Created:					Tue Nov 28 05:52:50 MST 1995
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

#include          "mfcpch.h"     //precompiled headers
#include          <string.h>
#include          <stdio.h>
#include          "getopt.h"
#include          "notdll.h"     //must be last include

int optind;
char *optarg;

/**********************************************************************
 * getopt
 *
 * parse command line args.
 **********************************************************************/

int
getopt (                         //parse args
INT32 argc,                      //arg count
char *argv[],                    //args
const char *arglist                    //string of arg chars
) {
  char *arg;                     //arg char

  if (optind == 0)
    optind = 1;
  if (optind < argc && argv[optind][0] == '-') {
    arg = strchr (arglist, argv[optind][1]);
    if (arg == NULL || *arg == ':')
      return '?';                //dud option
    optind++;
    optarg = argv[optind];
    if (arg[1] == ':') {
      if (argv[optind - 1][2] != '\0')
                                 //immediately after
        optarg = argv[optind - 1] + 2;
      else
        optind++;
    }
    return *arg;
  }
  else
    return EOF;
}
