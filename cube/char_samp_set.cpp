/**********************************************************************
 * File:        char_samp_enum.cpp
 * Description: Implementation of a Character Sample Set Class
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

#include <stdlib.h>
#include <string>
#include "char_samp_set.h"
#include "cached_file.h"

namespace tesseract {

CharSampSet::CharSampSet() {
  cnt_ = 0;
  samp_buff_ = NULL;
  own_samples_ = false;
}

CharSampSet::~CharSampSet() {
  Cleanup();
}

// free buffers and init vars
void CharSampSet::Cleanup() {
  if (samp_buff_ != NULL) {
    // only free samples if owned by class
    if (own_samples_ == true) {
      for (int samp_idx = 0; samp_idx < cnt_; samp_idx++) {
        if (samp_buff_[samp_idx] != NULL) {
          delete samp_buff_[samp_idx];
        }
      }
    }
    delete []samp_buff_;
  }
  cnt_ = 0;
  samp_buff_ = NULL;
}

// add a new sample
bool CharSampSet::Add(CharSamp *char_samp) {
  if ((cnt_ % SAMP_ALLOC_BLOCK) == 0) {
      // create an extended buffer
    CharSamp **new_samp_buff =
        reinterpret_cast<CharSamp **>(new CharSamp *[cnt_ + SAMP_ALLOC_BLOCK]);
    if (new_samp_buff == NULL) {
      return false;
    }
    // copy old contents
    if (cnt_ > 0) {
      memcpy(new_samp_buff, samp_buff_, cnt_ * sizeof(*samp_buff_));
      delete []samp_buff_;
    }
    samp_buff_ = new_samp_buff;
  }
  samp_buff_[cnt_++] = char_samp;
  return true;
}

// load char samples from file
bool CharSampSet::LoadCharSamples(FILE *fp) {
  // free existing
  Cleanup();
  // samples are created here and owned by the class
  own_samples_ = true;
  // start loading char samples
  while (feof(fp) == 0) {
    CharSamp *new_samp = CharSamp::FromCharDumpFile(fp);
    if (new_samp != NULL) {
      if (Add(new_samp) == false) {
        return false;
      }
    }
  }
  return true;
}

// creates a CharSampSet object from file
CharSampSet * CharSampSet::FromCharDumpFile(string file_name) {
  FILE *fp;
  unsigned int val32;
  // open the file
  fp = fopen(file_name.c_str(), "rb");
  if (fp == NULL) {
    return NULL;
  }
  // read and verify marker
  if (fread(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return NULL;
  }
  if (val32 != 0xfefeabd0) {
    return NULL;
  }
  // create an object
  CharSampSet *samp_set = new CharSampSet();
  if (samp_set == NULL) {
    return NULL;
  }
  if (samp_set->LoadCharSamples(fp) == false) {
    delete samp_set;
    samp_set = NULL;
  }
  fclose(fp);
  return samp_set;
}

// Create a new Char Dump file
FILE *CharSampSet::CreateCharDumpFile(string file_name) {
  FILE *fp;
  unsigned int val32;
  // create the file
  fp =  fopen(file_name.c_str(), "wb");
  if (!fp) {
    return NULL;
  }
  // read and verify marker
  val32 = 0xfefeabd0;
  if (fwrite(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return NULL;
  }
  return fp;
}

// Enumerate the Samples in the set one-by-one calling the enumertor's
  // EnumCharSamp method for each sample
bool CharSampSet::EnumSamples(string file_name, CharSampEnum *enum_obj) {
  CachedFile *fp_in;
  unsigned int val32;
  long i64_size,
    i64_pos;
  // open the file
  fp_in = new CachedFile(file_name);
  if (fp_in == NULL) {
    return false;
  }
  i64_size = fp_in->Size();
  if (i64_size < 1) {
    return false;
  }
  // read and verify marker
  if (fp_in->Read(&val32, sizeof(val32)) != sizeof(val32)) {
    return false;
  }
  if (val32 != 0xfefeabd0) {
    return false;
  }
  // start loading char samples
  while (fp_in->eof() == false) {
    CharSamp *new_samp = CharSamp::FromCharDumpFile(fp_in);
    i64_pos = fp_in->Tell();
    if (new_samp != NULL) {
      bool ret_flag = (enum_obj)->EnumCharSamp(new_samp,
                                               (100.0f * i64_pos / i64_size));
      delete new_samp;
      if (ret_flag == false) {
        break;
      }
    }
  }
  delete fp_in;
  return true;
}

}  // namespace ocrlib
