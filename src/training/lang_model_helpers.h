// Copyright 2017 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
// Purpose: Collection of convenience functions to simplify creation of the
//          unicharset, recoder, and dawgs for an LSTM model.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef TESSERACT_TRAINING_LANG_MODEL_HELPERS_H_
#define TESSERACT_TRAINING_LANG_MODEL_HELPERS_H_

#include <string>
#include <tesseract/genericvector.h>
#include <tesseract/serialis.h>
#include <tesseract/strngs.h>
#include "tessdatamanager.h"
#include "unicharset.h"

namespace tesseract {

// Helper makes a filename (<output_dir>/<lang>/<lang><suffix>) and writes data
// to the file, using writer if not null, otherwise, a default writer.
// Default writer will overwrite any existing file, but a supplied writer
// can do its own thing. If lang is empty, returns true but does nothing.
// NOTE that suffix should contain any required . for the filename.
bool WriteFile(const std::string& output_dir, const std::string& lang,
               const std::string& suffix, const GenericVector<char>& data,
               FileWriter writer);
// Helper reads a file with optional reader and returns a STRING.
// On failure emits a warning message and returns and empty STRING.
STRING ReadFile(const std::string& filename, FileReader reader);

// Helper writes the unicharset to file and to the traineddata.
bool WriteUnicharset(const UNICHARSET& unicharset, const std::string& output_dir,
                     const std::string& lang, FileWriter writer,
                     TessdataManager* traineddata);
// Helper creates the recoder from the unicharset and writes it to the
// traineddata, with a human-readable form to file at:
// <output_dir>/<lang>/<lang>.charset_size=<num> for some num being the size
// of the re-encoded character set. The charset_size file is written using
// writer if not null, or using a default file writer otherwise, overwriting
// any existing content.
// If pass_through is true, then the recoder will be a no-op, passing the
// unicharset codes through unchanged. Otherwise, the recoder will "compress"
// the unicharset by encoding Hangul in Jamos, decomposing multi-unicode
// symbols into sequences of unicodes, and encoding Han using the data in the
// radical_table_data, which must be the content of the file:
// langdata/radical-stroke.txt.
bool WriteRecoder(const UNICHARSET& unicharset, bool pass_through,
                  const std::string& output_dir, const std::string& lang,
                  FileWriter writer, STRING* radical_table_data,
                  TessdataManager* traineddata);

// The main function for combine_lang_model.cpp.
// Returns EXIT_SUCCESS or EXIT_FAILURE for error.
// unicharset: can be a hand-created file with incomplete fields. Its basic
//             and script properties will be set before it is used.
// script_dir: should point to the langdata (github repo) directory.
// version_str: arbitrary version label.
// Output files will be written to <output_dir>/<lang>/<lang>.*
// If pass_through_recoder is true, the unicharset will be used unchanged as
// labels in the classifier, otherwise, the unicharset will be "compressed" to
// make the recognition task simpler and faster.
// The words/puncs/numbers lists may be all empty. If any are non-empty then
// puncs must be non-empty.
// lang_is_rtl indicates that the language is generally written from right
// to left (eg Arabic/Hebrew).
int CombineLangModel(const UNICHARSET& unicharset, const std::string& script_dir,
                     const std::string& version_str, const std::string& output_dir,
                     const std::string& lang, bool pass_through_recoder,
                     const GenericVector<STRING>& words,
                     const GenericVector<STRING>& puncs,
                     const GenericVector<STRING>& numbers, bool lang_is_rtl,
                     FileReader reader, FileWriter writer);

}  // namespace tesseract

#endif  // TESSERACT_TRAINING_LANG_MODEL_HELPERS_H_
