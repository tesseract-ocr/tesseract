/**********************************************************************
 * File:        tessopt.h
 * Description: Re-implementation of the unix code.
 * Author:      Ray Smith
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

#ifndef TESSERACT_TRAINING_TESSOPT_H_
#define TESSERACT_TRAINING_TESSOPT_H_

#include          "host.h"

extern int tessoptind;
extern char *tessoptarg;

int tessopt (                     //parse args
int32_t argc,                      //arg count
char *argv[],                    //args
const char *arglist                    //string of arg chars
);

#endif  // TESSERACT_TRAINING_TESSOPT_H_
