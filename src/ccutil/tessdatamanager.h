///////////////////////////////////////////////////////////////////////
// File:        tessdatamanager.h
// Description: Functions to handle loading/combining tesseract data files.
// Author:      Daria Antonova
//
// (C) Copyright 2009, Google Inc.
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

#ifndef TESSERACT_CCUTIL_TESSDATAMANAGER_H_
#define TESSERACT_CCUTIL_TESSDATAMANAGER_H_

#include <string>
#include <tesseract/genericvector.h>
#include <tesseract/strngs.h>             // for STRING

static const char kTrainedDataSuffix[] = "traineddata";

// When adding new tessdata types and file suffixes, please make sure to
// update TessdataType enum, kTessdataFileSuffixes and kTessdataFileIsText.
static const char kLangConfigFileSuffix[] = "config";
static const char kUnicharsetFileSuffix[] = "unicharset";
static const char kAmbigsFileSuffix[] = "unicharambigs";
static const char kBuiltInTemplatesFileSuffix[] = "inttemp";
static const char kBuiltInCutoffsFileSuffix[] = "pffmtable";
static const char kNormProtoFileSuffix[] = "normproto";
static const char kPuncDawgFileSuffix[] = "punc-dawg";
static const char kSystemDawgFileSuffix[] = "word-dawg";
static const char kNumberDawgFileSuffix[] = "number-dawg";
static const char kFreqDawgFileSuffix[] = "freq-dawg";
static const char kFixedLengthDawgsFileSuffix[] = "fixed-length-dawgs";
static const char kCubeUnicharsetFileSuffix[] = "cube-unicharset";
static const char kCubeSystemDawgFileSuffix[] = "cube-word-dawg";
static const char kShapeTableFileSuffix[] = "shapetable";
static const char kBigramDawgFileSuffix[] = "bigram-dawg";
static const char kUnambigDawgFileSuffix[] = "unambig-dawg";
static const char kParamsModelFileSuffix[] = "params-model";
static const char kLSTMModelFileSuffix[] = "lstm";
static const char kLSTMPuncDawgFileSuffix[] = "lstm-punc-dawg";
static const char kLSTMSystemDawgFileSuffix[] = "lstm-word-dawg";
static const char kLSTMNumberDawgFileSuffix[] = "lstm-number-dawg";
static const char kLSTMUnicharsetFileSuffix[] = "lstm-unicharset";
static const char kLSTMRecoderFileSuffix[] = "lstm-recoder";
static const char kVersionFileSuffix[] = "version";

namespace tesseract {

enum TessdataType {
  TESSDATA_LANG_CONFIG,         // 0
  TESSDATA_UNICHARSET,          // 1
  TESSDATA_AMBIGS,              // 2
  TESSDATA_INTTEMP,             // 3
  TESSDATA_PFFMTABLE,           // 4
  TESSDATA_NORMPROTO,           // 5
  TESSDATA_PUNC_DAWG,           // 6
  TESSDATA_SYSTEM_DAWG,         // 7
  TESSDATA_NUMBER_DAWG,         // 8
  TESSDATA_FREQ_DAWG,           // 9
  TESSDATA_FIXED_LENGTH_DAWGS,  // 10  // deprecated
  TESSDATA_CUBE_UNICHARSET,     // 11  // deprecated
  TESSDATA_CUBE_SYSTEM_DAWG,    // 12  // deprecated
  TESSDATA_SHAPE_TABLE,         // 13
  TESSDATA_BIGRAM_DAWG,         // 14
  TESSDATA_UNAMBIG_DAWG,        // 15
  TESSDATA_PARAMS_MODEL,        // 16
  TESSDATA_LSTM,                // 17
  TESSDATA_LSTM_PUNC_DAWG,      // 18
  TESSDATA_LSTM_SYSTEM_DAWG,    // 19
  TESSDATA_LSTM_NUMBER_DAWG,    // 20
  TESSDATA_LSTM_UNICHARSET,     // 21
  TESSDATA_LSTM_RECODER,        // 22
  TESSDATA_VERSION,             // 23

  TESSDATA_NUM_ENTRIES
};

/**
 * kTessdataFileSuffixes[i] indicates the file suffix for
 * tessdata of type i (from TessdataType enum).
 */
static const char *const kTessdataFileSuffixes[] = {
    kLangConfigFileSuffix,        // 0
    kUnicharsetFileSuffix,        // 1
    kAmbigsFileSuffix,            // 2
    kBuiltInTemplatesFileSuffix,  // 3
    kBuiltInCutoffsFileSuffix,    // 4
    kNormProtoFileSuffix,         // 5
    kPuncDawgFileSuffix,          // 6
    kSystemDawgFileSuffix,        // 7
    kNumberDawgFileSuffix,        // 8
    kFreqDawgFileSuffix,          // 9
    kFixedLengthDawgsFileSuffix,  // 10  // deprecated
    kCubeUnicharsetFileSuffix,    // 11  // deprecated
    kCubeSystemDawgFileSuffix,    // 12  // deprecated
    kShapeTableFileSuffix,        // 13
    kBigramDawgFileSuffix,        // 14
    kUnambigDawgFileSuffix,       // 15
    kParamsModelFileSuffix,       // 16
    kLSTMModelFileSuffix,         // 17
    kLSTMPuncDawgFileSuffix,      // 18
    kLSTMSystemDawgFileSuffix,    // 19
    kLSTMNumberDawgFileSuffix,    // 20
    kLSTMUnicharsetFileSuffix,    // 21
    kLSTMRecoderFileSuffix,       // 22
    kVersionFileSuffix,           // 23
};

/**
 * TessdataType could be updated to contain more entries, however
 * we do not expect that number to be astronomically high.
 * In order to automatically detect endianness TessdataManager will
 * flip the bits if actual_tessdata_num_entries_ is larger than
 * kMaxNumTessdataEntries.
 */
static const int kMaxNumTessdataEntries = 1000;


class TessdataManager {
 public:
  TessdataManager();
  explicit TessdataManager(FileReader reader);

  ~TessdataManager() = default;

  bool swap() const { return swap_; }
  bool is_loaded() const { return is_loaded_; }

  // Lazily loads from the the given filename. Won't actually read the file
  // until it needs it.
  void LoadFileLater(const char *data_file_name);
  /**
   * Opens and reads the given data file right now.
   * @return true on success.
   */
  bool Init(const char *data_file_name);
  // Loads from the given memory buffer as if a file, remembering name as some
  // arbitrary source id for caching.
  bool LoadMemBuffer(const char *name, const char *data, int size);
  // Overwrites a single entry of the given type.
  void OverwriteEntry(TessdataType type, const char *data, int size);

  // Saves to the given filename.
  bool SaveFile(const char* filename, FileWriter writer) const;
  // Serializes to the given vector.
  void Serialize(GenericVector<char> *data) const;
  // Resets to the initial state, keeping the reader.
  void Clear();

  // Prints a directory of contents.
  void Directory() const;

  // Returns true if the component requested is present.
  bool IsComponentAvailable(TessdataType type) const {
    return !entries_[type].empty();
  }
  // Opens the given TFile pointer to the given component type.
  // Returns false in case of failure.
  bool GetComponent(TessdataType type, TFile *fp);
  // As non-const version except it can't load the component if not already
  // loaded.
  bool GetComponent(TessdataType type, TFile *fp) const;

  // Returns the current version string.
  std::string VersionString() const;
  // Sets the version string to the given v_str.
  void SetVersionString(const std::string &v_str);

  // Returns true if the base Tesseract components are present.
  bool IsBaseAvailable() const {
    return !entries_[TESSDATA_UNICHARSET].empty() &&
           !entries_[TESSDATA_INTTEMP].empty();
  }

  // Returns true if the LSTM components are present.
  bool IsLSTMAvailable() const { return !entries_[TESSDATA_LSTM].empty(); }

  // Return the name of the underlying data file.
  const std::string& GetDataFileName() const { return data_file_name_; }

  /**
   * Reads all the standard tesseract config and data files for a language
   * at the given path and bundles them up into one binary data file.
   * Returns true if the combined traineddata file was successfully written.
   */
  bool CombineDataFiles(const char *language_data_path_prefix,
                        const char *output_filename);

  /**
   * Gets the individual components from the data_file_ with which the class was
   * initialized. Overwrites the components specified by component_filenames.
   * Writes the updated traineddata file to new_traineddata_filename.
   */
  bool OverwriteComponents(const char *new_traineddata_filename,
                            char **component_filenames,
                            int num_new_components);

  /**
   * Extracts tessdata component implied by the name of the input file from
   * the combined traineddata loaded into TessdataManager.
   * Writes the extracted component to the file indicated by the file name.
   * E.g. if the filename given is somepath/somelang.unicharset, unicharset
   * will be extracted from the data loaded into the TessdataManager and will
   * be written to somepath/somelang.unicharset.
   * @return true if the component was successfully extracted, false if the
   * component was not present in the traineddata loaded into TessdataManager.
   */
  bool ExtractToFile(const char *filename);

 private:

  // Use libarchive.
  bool LoadArchiveFile(const char *filename);

  /**
   * Fills type with TessdataType of the tessdata component represented by the
   * given file name. E.g. tessdata/eng.unicharset -> TESSDATA_UNICHARSET.
   * @return true if the tessdata component type could be determined
   * from the given file name.
   */
  static bool TessdataTypeFromFileSuffix(const char *suffix,
                                         TessdataType *type);

  /**
   * Tries to determine tessdata component file suffix from filename,
   * returns true on success.
   */
  static bool TessdataTypeFromFileName(const char *filename,
                                       TessdataType *type);

  // Name of file it came from.
  std::string data_file_name_;
  // Function to load the file when we need it.
  FileReader reader_;
  // True if the file has been loaded.
  bool is_loaded_;
  // True if the bytes need swapping.
  bool swap_;
  // Contents of each element of the traineddata file.
  GenericVector<char> entries_[TESSDATA_NUM_ENTRIES];
};

}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_TESSDATAMANAGER_H_
