/**********************************************************************
 * File:        commandlineflags.h
 * Description: Header file for commandline flag parsing.
 * Author:      Ranjith Unnikrishnan
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

#include "export.h"
#include "params.h"

#include <cstdlib>

#define INT_PARAM_FLAG(name, val, comment) INT_VAR(FLAGS_##name, val, comment)
#define DECLARE_INT_PARAM_FLAG(name) extern INT_VAR_H(FLAGS_##name)
#define DOUBLE_PARAM_FLAG(name, val, comment) double_VAR(FLAGS_##name, val, comment)
#define DECLARE_DOUBLE_PARAM_FLAG(name) extern double_VAR_H(FLAGS_##name)
#define BOOL_PARAM_FLAG(name, val, comment) BOOL_VAR(FLAGS_##name, val, comment)
#define DECLARE_BOOL_PARAM_FLAG(name) extern BOOL_VAR_H(FLAGS_##name)
#define STRING_PARAM_FLAG(name, val, comment) STRING_VAR(FLAGS_##name, val, comment)
#define DECLARE_STRING_PARAM_FLAG(name) extern STRING_VAR_H(FLAGS_##name)

namespace tesseract {

// Flags from commontraining.cpp
// Command line arguments for font_properties, xheights and unicharset.
TESS_COMMON_TRAINING_API
DECLARE_INT_PARAM_FLAG(debug_level);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(D);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(F);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(O);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(U);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(X);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(fonts_dir);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(fontconfig_tmpdir);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(output_trainer);
TESS_COMMON_TRAINING_API
DECLARE_STRING_PARAM_FLAG(test_ch);

// Parse commandline flags and values. Prints the usage string and exits on
// input of --help or --version.
//
// If remove_flags is true, the argv pointer is advanced so that (*argv)[1]
// points to the first non-flag argument, (*argv)[0] points to the same string
// as before, and argc is decremented to reflect the new shorter length of argv.
// eg. If the input *argv is
// { "program", "--foo=4", "--bar=true", "file1", "file2" } with *argc = 5, the
// output *argv is { "program", "file1", "file2" } with *argc = 3
TESS_COMMON_TRAINING_API
void ParseCommandLineFlags(const char *usage, int *argc, char ***argv, const bool remove_flags);

} // namespace tesseract

#endif // TESSERACT_TRAINING_COMMANDLINEFLAGS_H_
