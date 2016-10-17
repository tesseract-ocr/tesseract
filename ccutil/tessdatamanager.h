///////////////////////////////////////////////////////////////////////
// File:        tessdatamanager.h
// Description: Functions to handle loading/combining tesseract data files.
// Author:      Daria Antonova
// Created:     Wed Jun 03 11:26:43 PST 2009
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

#include <stdio.h>

#include "host.h"
#include "strngs.h"
#include "tprintf.h"

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
  TESSDATA_CUBE_UNICHARSET,     // 11
  TESSDATA_CUBE_SYSTEM_DAWG,    // 12
  TESSDATA_SHAPE_TABLE,         // 13
  TESSDATA_BIGRAM_DAWG,         // 14
  TESSDATA_UNAMBIG_DAWG,        // 15
  TESSDATA_PARAMS_MODEL,        // 16

  TESSDATA_NUM_ENTRIES
};

/**
 * kTessdataFileSuffixes[i] indicates the file suffix for
 * tessdata of type i (from TessdataType enum).
 */
static const char * const kTessdataFileSuffixes[] = {
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
  kCubeUnicharsetFileSuffix,    // 11
  kCubeSystemDawgFileSuffix,    // 12
  kShapeTableFileSuffix,        // 13
  kBigramDawgFileSuffix,        // 14
  kUnambigDawgFileSuffix,       // 15
  kParamsModelFileSuffix,       // 16
};

/**
 * If kTessdataFileIsText[i] is true - the tessdata component
 * of type i (from TessdataType enum) is text, and is binary otherwise.
 */
static const bool kTessdataFileIsText[] = {
  true,                         // 0
  true,                         // 1
  true,                         // 2
  false,                        // 3
  true,                         // 4
  true,                         // 5
  false,                        // 6
  false,                        // 7
  false,                        // 8
  false,                        // 9
  false,                        // 10  // deprecated
  true,                         // 11
  false,                        // 12
  false,                        // 13
  false,                        // 14
  false,                        // 15
  true,                         // 16
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
  TessdataManager() {
    data_file_ = NULL;
    actual_tessdata_num_entries_ = 0;
    for (int i = 0; i < TESSDATA_NUM_ENTRIES; ++i) {
      offset_table_[i] = -1;
    }
  }
  ~TessdataManager() {}
  int DebugLevel() { return debug_level_; }

  /**
   * Opens the given data file and reads the offset table.
   * @return true on success.
   */
  bool Init(const char *data_file_name, int debug_level);

  // Return the name of the underlying data file.
  const STRING &GetDataFileName() const { return data_file_name_; }

  /** Returns data file pointer. */
  inline FILE *GetDataFilePtr() const { return data_file_; }

  /**
   * Returns false if there is no data of the given type.
   * Otherwise does a seek on the data_file_ to position the pointer
   * at the start of the data of the given type.
   */
  inline bool SeekToStart(TessdataType tessdata_type) {
    if (debug_level_) {
      tprintf("TessdataManager: seek to offset %lld - start of tessdata"
              "type %d (%s))\n", offset_table_[tessdata_type],
              tessdata_type, kTessdataFileSuffixes[tessdata_type]);
    }
    if (offset_table_[tessdata_type] < 0) {
      return false;
    } else {
      ASSERT_HOST(fseek(data_file_,
                        static_cast<size_t>(offset_table_[tessdata_type]),
                        SEEK_SET) == 0);
      return true;
    }
  }
  /** Returns the end offset for the given tesseract data file type. */
  inline inT64 GetEndOffset(TessdataType tessdata_type) const {
    int index = tessdata_type + 1;
    while (index < actual_tessdata_num_entries_ && offset_table_[index] == -1) {
      ++index;  // skip tessdata types not present in the combined file
    }
    if (debug_level_) {
      tprintf("TessdataManager: end offset for type %d is %lld\n",
              tessdata_type,
              (index == actual_tessdata_num_entries_) ? -1
              : offset_table_[index]);
    }
    return (index == actual_tessdata_num_entries_) ? -1 : offset_table_[index] - 1;
  }
  /** Closes data_file_ (if it was opened by Init()). */
  inline void End() {
    if (data_file_ != NULL) {
      fclose(data_file_);
      data_file_ = NULL;
    }
  }
  bool swap() const {
    return swap_;
  }

  /** Writes the number of entries and the given offset table to output_file.
   * Returns false on error.
   */
  static bool WriteMetadata(inT64 *offset_table,
                            const char *language_data_path_prefix,
                            FILE *output_file);

  /**
   * Reads all the standard tesseract config and data files for a language
   * at the given path and bundles them up into one binary data file.
   * Returns true if the combined traineddata file was successfully written.
   */
  static bool CombineDataFiles(const char *language_data_path_prefix,
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

  /**
   * Copies data from the given input file to the output_file provided.
   * If num_bytes_to_copy is >= 0, only num_bytes_to_copy is copied from
   * the input file, otherwise all the data in the input file is copied.
   */
  static void CopyFile(FILE *input_file, FILE *output_file,
                       bool newline_end, inT64 num_bytes_to_copy);

  /**
   * Fills type with TessdataType of the tessdata component represented by the
   * given file name. E.g. tessdata/eng.unicharset -> TESSDATA_UNICHARSET.
   * Sets *text_file to true if the component is in text format (e.g.
   * unicharset, unichar ambigs, config, etc).
   * @return true if the tessdata component type could be determined
   * from the given file name.
   */
  static bool TessdataTypeFromFileSuffix(const char *suffix,
                                         TessdataType *type,
                                         bool *text_file);

  /**
   * Tries to determine tessdata component file suffix from filename,
   * returns true on success.
   */
  static bool TessdataTypeFromFileName(const char *filename,
                                       TessdataType *type,
                                       bool *text_file);

 private:

  /**
   * Opens the file whose name is a concatenation of language_data_path_prefix
   * and file_suffix. Returns a file pointer to the opened file.
   */
  static FILE *GetFilePtr(const char *language_data_path_prefix,
                          const char *file_suffix, bool text_file);

  /**
   * Each offset_table_[i] contains a file offset in the combined data file
   * where the data of TessdataFileType i is stored.
   */
  inT64 offset_table_[TESSDATA_NUM_ENTRIES];
  /**
   * Actual number of entries in the tessdata table. This value can only be
   * same or smaller than TESSDATA_NUM_ENTRIES, but can never be larger,
   * since then it would be impossible to interpret the type of tessdata at
   * indices same and higher than TESSDATA_NUM_ENTRIES.
   * This parameter is used to allow for backward compatibility
   * when new tessdata types are introduced.
   */
  inT32 actual_tessdata_num_entries_;
  STRING data_file_name_;  // name of the data file.
  FILE *data_file_;  ///< pointer to the data file.
  int debug_level_;
  // True if the bytes need swapping.
  bool swap_;
};


}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_TESSDATAMANAGER_H_
