/**********************************************************************
 * File:        commandlineflags.h
 * Description: Header file for commandline flag parsing.
 * Author:      Ranjith Unnikrishnan
 * Created:     July 2013
 *
 * (C) Copyright 2013, Google Inc.
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
#ifndef TESSERACT_TRAINING_COMMANDLINEFLAGS_H_
#define TESSERACT_TRAINING_COMMANDLINEFLAGS_H_

#ifdef USE_STD_NAMESPACE

#include <stdlib.h>
#include "tprintf.h"
#include "params.h"

#define INT_PARAM_FLAG(name, val, comment)      \
  INT_VAR(FLAGS_##name, val, comment)
#define DECLARE_INT_PARAM_FLAG(name)            \
  extern INT_VAR_H(FLAGS_##name, 0, "")
#define DOUBLE_PARAM_FLAG(name, val, comment)   \
  double_VAR(FLAGS_##name, val, comment)
#define DECLARE_DOUBLE_PARAM_FLAG(name)         \
  extern double_VAR_H(FLAGS_##name, "", "")
#define BOOL_PARAM_FLAG(name, val, comment)     \
  BOOL_VAR(FLAGS_##name, val, comment)
#define DECLARE_BOOL_PARAM_FLAG(name)           \
  extern BOOL_VAR_H(FLAGS_##name, 0, "")
#define STRING_PARAM_FLAG(name, val, comment)   \
  STRING_VAR(FLAGS_##name, val, comment)
#define DECLARE_STRING_PARAM_FLAG(name)         \
  extern STRING_VAR_H(FLAGS_##name, "", "")

#else

#include "base/commandlineflags.h"
#define INT_PARAM_FLAG(name, val, comment) \
  DEFINE_int32(name, val, comment)
#define DECLARE_INT_PARAM_FLAG(name) \
  DECLARE_int32(name)
#define DOUBLE_PARAM_FLAG(name, val, comment) \
  DEFINE_double(name, val, comment)
#define DECLARE_DOUBLE_PARAM_FLAG(name) \
  DECLARE_double(name)
#define BOOL_PARAM_FLAG(name, val, comment) \
  DEFINE_bool(name, val, comment)
#define DECLARE_BOOL_PARAM_FLAG(name) \
  DECLARE_bool(name)
#define STRING_PARAM_FLAG(name, val, comment) \
  DEFINE_string(name, val, comment)
#define DECLARE_STRING_PARAM_FLAG(name) \
  DECLARE_string(name)

#endif

namespace tesseract {

// Parse commandline flags and values. Prints the usage string and exits on
// input of --help or --helpshort.
//
// If remove_flags is true, the argv pointer is advanced so that (*argv)[1]
// points to the first non-flag argument, (*argv)[0] points to the same string
// as before, and argc is decremented to reflect the new shorter length of argv.
// eg. If the input *argv is
// { "program", "--foo=4", "--bar=true", "file1", "file2" } with *argc = 5, the
// output *argv is { "program", "file1", "file2" } with *argc = 3
void ParseCommandLineFlags(const char* usage, int* argc,
                           char*** argv, const bool remove_flags);

}

#endif  // TESSERACT_TRAINING_COMMANDLINEFLAGS_H_
