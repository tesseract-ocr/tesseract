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
#include "lang_model_helpers.h"

#if defined(_WIN32)
#include <direct.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include "dawg.h"
#include "fileio.h"
#include "tessdatamanager.h"
#include "trie.h"
#include "unicharcompress.h"

namespace tesseract {

// Helper makes a filename (<output_dir>/<lang>/<lang><suffix>) and writes data
// to the file, using writer if not null, otherwise, a default writer.
// Default writer will overwrite any existing file, but a supplied writer
// can do its own thing. If lang is empty, returns true but does nothing.
// NOTE that suffix should contain any required . for the filename.
bool WriteFile(const std::string& output_dir, const std::string& lang,
               const std::string& suffix, const GenericVector<char>& data,
               FileWriter writer) {
  if (lang.empty()) return true;
  std::string dirname = output_dir + "/" + lang;
  // Attempt to make the directory, but ignore errors, as it may not be a
  // standard filesystem, and the writer will complain if not successful.
#if defined(_WIN32)
  _mkdir(dirname.c_str());
#else
  mkdir(dirname.c_str(), S_IRWXU | S_IRWXG);
#endif
  std::string filename = dirname + "/" + lang + suffix;
  if (writer == nullptr)
    return SaveDataToFile(data, filename.c_str());
  else
    return (*writer)(data, filename.c_str());
}

// Helper reads a file with optional reader and returns a STRING.
// On failure emits a warning message and returns and empty STRING.
STRING ReadFile(const std::string& filename, FileReader reader) {
  if (filename.empty()) return STRING();
  GenericVector<char> data;
  bool read_result;
  if (reader == nullptr)
    read_result = LoadDataFromFile(filename.c_str(), &data);
  else
    read_result = (*reader)(filename.c_str(), &data);
  if (read_result) return STRING(&data[0], data.size());
  tprintf("Failed to read data from: %s\n", filename.c_str());
  return STRING();
}

// Helper writes the unicharset to file and to the traineddata.
bool WriteUnicharset(const UNICHARSET& unicharset, const std::string& output_dir,
                     const std::string& lang, FileWriter writer,
                     TessdataManager* traineddata) {
  GenericVector<char> unicharset_data;
  TFile fp;
  fp.OpenWrite(&unicharset_data);
  if (!unicharset.save_to_file(&fp)) return false;
  traineddata->OverwriteEntry(TESSDATA_LSTM_UNICHARSET, &unicharset_data[0],
                              unicharset_data.size());
  return WriteFile(output_dir, lang, ".unicharset", unicharset_data, writer);
}

// Helper creates the recoder and writes it to the traineddata, and a human-
// readable form to file.
bool WriteRecoder(const UNICHARSET& unicharset, bool pass_through,
                  const std::string& output_dir, const std::string& lang,
                  FileWriter writer, STRING* radical_table_data,
                  TessdataManager* traineddata) {
  UnicharCompress recoder;
  // Where the unicharset is carefully setup already to contain a good
  // compact encoding, use a pass-through recoder that does nothing.
  // For scripts that have a large number of unicodes (Han, Hangul) we want
  // to use the recoder to compress the symbol space by re-encoding each
  // unicode as multiple codes from a smaller 'alphabet' that are related to the
  // shapes in the character. Hangul Jamo is a perfect example of this.
  // See the Hangul Syllables section, sub-section "Equivalence" in:
  // http://www.unicode.org/versions/Unicode10.0.0/ch18.pdf
  if (pass_through) {
    recoder.SetupPassThrough(unicharset);
  } else {
    int null_char =
        unicharset.has_special_codes() ? UNICHAR_BROKEN : unicharset.size();
    tprintf("Null char=%d\n", null_char);
    if (!recoder.ComputeEncoding(unicharset, null_char, radical_table_data)) {
      tprintf("Creation of encoded unicharset failed!!\n");
      return false;
    }
  }
  TFile fp;
  GenericVector<char> recoder_data;
  fp.OpenWrite(&recoder_data);
  if (!recoder.Serialize(&fp)) return false;
  traineddata->OverwriteEntry(TESSDATA_LSTM_RECODER, &recoder_data[0],
                              recoder_data.size());
  STRING encoding = recoder.GetEncodingAsString(unicharset);
  recoder_data.init_to_size(encoding.length(), 0);
  memcpy(&recoder_data[0], &encoding[0], encoding.length());
  STRING suffix;
  suffix.add_str_int(".charset_size=", recoder.code_range());
  suffix += ".txt";
  return WriteFile(output_dir, lang, suffix.c_str(), recoder_data, writer);
}

// Helper builds a dawg from the given words, using the unicharset as coding,
// and reverse_policy for LTR/RTL, and overwrites file_type in the traineddata.
static bool WriteDawg(const GenericVector<STRING>& words,
                      const UNICHARSET& unicharset,
                      Trie::RTLReversePolicy reverse_policy,
                      TessdataType file_type, TessdataManager* traineddata) {
  // The first 3 arguments are not used in this case.
  Trie trie(DAWG_TYPE_WORD, "", SYSTEM_DAWG_PERM, unicharset.size(), 0);
  trie.add_word_list(words, unicharset, reverse_policy);
  tprintf("Reducing Trie to SquishedDawg\n");
  std::unique_ptr<SquishedDawg> dawg(trie.trie_to_dawg());
  if (dawg == nullptr || dawg->NumEdges() == 0) return false;
  TFile fp;
  GenericVector<char> dawg_data;
  fp.OpenWrite(&dawg_data);
  if (!dawg->write_squished_dawg(&fp)) return false;
  traineddata->OverwriteEntry(file_type, &dawg_data[0], dawg_data.size());
  return true;
}

// Builds and writes the dawgs, given a set of words, punctuation
// patterns, number patterns, to the traineddata. Encoding uses the given
// unicharset, and the punc dawgs is reversed if lang_is_rtl.
static bool WriteDawgs(const GenericVector<STRING>& words,
                       const GenericVector<STRING>& puncs,
                       const GenericVector<STRING>& numbers, bool lang_is_rtl,
                       const UNICHARSET& unicharset,
                       TessdataManager* traineddata) {
  if (puncs.empty()) {
    tprintf("Must have non-empty puncs list to use language models!!\n");
    return false;
  }
  // For each of the dawg types, make the dawg, and write to traineddata.
  // Dawgs are reversed as follows:
  // Words: According to the word content.
  // Puncs: According to lang_is_rtl.
  // Numbers: Never.
  // System dawg (main wordlist).
  if (!words.empty() &&
      !WriteDawg(words, unicharset, Trie::RRP_REVERSE_IF_HAS_RTL,
                 TESSDATA_LSTM_SYSTEM_DAWG, traineddata)) {
    return false;
  }
  // punc/punc-dawg.
  Trie::RTLReversePolicy reverse_policy =
      lang_is_rtl ? Trie::RRP_FORCE_REVERSE : Trie::RRP_DO_NO_REVERSE;
  if (!WriteDawg(puncs, unicharset, reverse_policy, TESSDATA_LSTM_PUNC_DAWG,
                 traineddata)) {
    return false;
  }
  // numbers/number-dawg.
  if (!numbers.empty() &&
      !WriteDawg(numbers, unicharset, Trie::RRP_DO_NO_REVERSE,
                 TESSDATA_LSTM_NUMBER_DAWG, traineddata)) {
    return false;
  }
  return true;
}

// The main function for combine_lang_model.cpp.
// Returns EXIT_SUCCESS or EXIT_FAILURE for error.
int CombineLangModel(const UNICHARSET& unicharset, const std::string& script_dir,
                     const std::string& version_str, const std::string& output_dir,
                     const std::string& lang, bool pass_through_recoder,
                     const GenericVector<STRING>& words,
                     const GenericVector<STRING>& puncs,
                     const GenericVector<STRING>& numbers, bool lang_is_rtl,
                     FileReader reader, FileWriter writer) {
  // Build the traineddata file.
  TessdataManager traineddata;
  if (!version_str.empty()) {
    traineddata.SetVersionString(traineddata.VersionString() + ":" +
                                 version_str);
  }
  // Unicharset and recoder.
  if (!WriteUnicharset(unicharset, output_dir, lang, writer, &traineddata)) {
    tprintf("Error writing unicharset!!\n");
    return EXIT_FAILURE;
  } else {
    tprintf("Config file is optional, continuing...\n");
  }
  // If there is a config file, read it and add to traineddata.
  std::string config_filename = script_dir + "/" + lang + "/" + lang + ".config";
  STRING config_file = ReadFile(config_filename, reader);
  if (config_file.length() > 0) {
    traineddata.OverwriteEntry(TESSDATA_LANG_CONFIG, &config_file[0],
                               config_file.length());
  }
  std::string radical_filename = script_dir + "/radical-stroke.txt";
  STRING radical_data = ReadFile(radical_filename, reader);
  if (radical_data.length() == 0) {
    tprintf("Error reading radical code table %s\n", radical_filename.c_str());
    return EXIT_FAILURE;
  }
  if (!WriteRecoder(unicharset, pass_through_recoder, output_dir, lang, writer,
                    &radical_data, &traineddata)) {
    tprintf("Error writing recoder!!\n");
  }
  if (!words.empty() || !puncs.empty() || !numbers.empty()) {
    if (!WriteDawgs(words, puncs, numbers, lang_is_rtl, unicharset,
                    &traineddata)) {
      tprintf("Error during conversion of wordlists to DAWGs!!\n");
      return EXIT_FAILURE;
    }
  }

  // Traineddata file.
  GenericVector<char> traineddata_data;
  traineddata.Serialize(&traineddata_data);
  if (!WriteFile(output_dir, lang, ".traineddata", traineddata_data, writer)) {
    tprintf("Error writing output traineddata file!!\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

}  // namespace tesseract
