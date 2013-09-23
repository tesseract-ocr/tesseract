/**********************************************************************
 * File:        char_samp_enum.cpp
 * Description: Implementation of a Character Set Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

#include <string>

#include "char_set.h"
#include "cube_utils.h"
#include "tessdatamanager.h"

namespace tesseract {

CharSet::CharSet() {
  class_cnt_ = 0;
  class_strings_ = NULL;
  unicharset_map_ = NULL;
  init_ = false;

  // init hash table
  memset(hash_bin_size_, 0, sizeof(hash_bin_size_));
}

CharSet::~CharSet() {
  if (class_strings_ != NULL) {
    for (int cls = 0; cls < class_cnt_; cls++) {
      if (class_strings_[cls] != NULL) {
        delete class_strings_[cls];
      }
    }
    delete []class_strings_;
    class_strings_ = NULL;
  }
  delete []unicharset_map_;
}

// Creates CharSet object by reading the unicharset from the
// TessDatamanager, and mapping Cube's unicharset to Tesseract's if
// they differ.
CharSet *CharSet::Create(TessdataManager *tessdata_manager,
                         UNICHARSET *tess_unicharset) {
  CharSet *char_set = new CharSet();
  if (char_set == NULL) {
    return NULL;
  }

  // First look for Cube's unicharset; if not there, use tesseract's
  bool cube_unicharset_exists;
  if (!(cube_unicharset_exists =
        tessdata_manager->SeekToStart(TESSDATA_CUBE_UNICHARSET)) &&
      !tessdata_manager->SeekToStart(TESSDATA_UNICHARSET)) {
    fprintf(stderr, "Cube ERROR (CharSet::Create): could not find "
            "either cube or tesseract unicharset\n");
    return NULL;
  }
  FILE *charset_fp = tessdata_manager->GetDataFilePtr();
  if (!charset_fp) {
    fprintf(stderr, "Cube ERROR (CharSet::Create): could not load "
            "a unicharset\n");
    return NULL;
  }

  // If we found a cube unicharset separate from tesseract's, load it and
  // map its unichars to tesseract's; if only one unicharset exists,
  // just load it.
  bool loaded;
  if (cube_unicharset_exists) {
    char_set->cube_unicharset_.load_from_file(charset_fp);
    loaded = tessdata_manager->SeekToStart(TESSDATA_CUBE_UNICHARSET);
    loaded = loaded && char_set->LoadSupportedCharList(
        tessdata_manager->GetDataFilePtr(), tess_unicharset);
    char_set->unicharset_ = &char_set->cube_unicharset_;
  } else {
    loaded = char_set->LoadSupportedCharList(charset_fp, NULL);
    char_set->unicharset_ = tess_unicharset;
  }
  if (!loaded) {
    delete char_set;
    return NULL;
  }

  char_set->init_ = true;
  return char_set;
}

// Load the list of supported chars from the given data file pointer.
bool CharSet::LoadSupportedCharList(FILE *fp, UNICHARSET *tess_unicharset) {
  if (init_)
    return true;

  char str_line[256];
  // init hash table
  memset(hash_bin_size_, 0, sizeof(hash_bin_size_));
  // read the char count
  if (fgets(str_line, sizeof(str_line), fp) == NULL) {
    fprintf(stderr, "Cube ERROR (CharSet::InitMemory): could not "
            "read char count.\n");
    return false;
  }
  class_cnt_ = atoi(str_line);
  if  (class_cnt_ < 2) {
    fprintf(stderr, "Cube ERROR (CharSet::InitMemory): invalid "
            "class count: %d\n", class_cnt_);
    return false;
  }
  // memory for class strings
  class_strings_ = new string_32*[class_cnt_];
  if (class_strings_ == NULL) {
    fprintf(stderr, "Cube ERROR (CharSet::InitMemory): could not "
            "allocate memory for class strings.\n");
    return false;
  }
  // memory for unicharset map
  if (tess_unicharset) {
    unicharset_map_ = new int[class_cnt_];
    if (unicharset_map_ == NULL) {
      fprintf(stderr, "Cube ERROR (CharSet::InitMemory): could not "
              "allocate memory for unicharset map.\n");
      return false;
    }
  }

  // Read in character strings and add to hash table
  for (int class_id = 0; class_id < class_cnt_; class_id++) {
    // Read the class string
    if (fgets(str_line, sizeof(str_line), fp) == NULL) {
      fprintf(stderr, "Cube ERROR (CharSet::ReadAndHashStrings): "
              "could not read class string with class_id=%d.\n", class_id);
      return false;
    }
    // Terminate at space if any
    char *p = strchr(str_line, ' ');
    if (p != NULL)
      *p = '\0';
    // Convert to UTF32 and store
    string_32 str32;
    // Convert NULL to a space
    if (strcmp(str_line, "NULL") == 0) {
      strcpy(str_line, " ");
    }
    CubeUtils::UTF8ToUTF32(str_line, &str32);
    class_strings_[class_id] = new string_32(str32);
    if (class_strings_[class_id] == NULL) {
      fprintf(stderr, "Cube ERROR (CharSet::ReadAndHashStrings): could not "
              "allocate memory for class string with class_id=%d.\n", class_id);
      return false;
    }

    // Add to hash-table
    int hash_val = Hash(reinterpret_cast<const char_32 *>(str32.c_str()));
    if (hash_bin_size_[hash_val] >= kMaxHashSize) {
      fprintf(stderr, "Cube ERROR (CharSet::LoadSupportedCharList): hash "
              "table is full.\n");
      return false;
    }
    hash_bins_[hash_val][hash_bin_size_[hash_val]++] = class_id;

    if (tess_unicharset != NULL) {
      // Add class id to unicharset map
      UNICHAR_ID tess_id = tess_unicharset->unichar_to_id(str_line);
      if (tess_id == INVALID_UNICHAR_ID) {
        tess_unicharset->unichar_insert(str_line);
        tess_id = tess_unicharset->unichar_to_id(str_line);
      }
      ASSERT_HOST(tess_id != INVALID_UNICHAR_ID);
      unicharset_map_[class_id] = tess_id;
    }
  }
  return true;
}

}  // tesseract
