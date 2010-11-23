/**********************************************************************
 * File:        char_samp_set.h
 * Description: Declaration of a Character Sample Set Class
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

// The CharSampSet set encapsulates a set of CharSet objects typically
// but not necessarily loaded from a file
// It provides methods to load samples from File, Create a new file and
// Add new char samples to the set

#ifndef CHAR_SAMP_SET_H
#define CHAR_SAMP_SET_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "char_samp.h"
#include "char_samp_enum.h"
#include "char_set.h"

namespace tesseract {

// chunks of samp pointers to allocate
#define SAMP_ALLOC_BLOCK 10000

class CharSampSet {
 public:
  CharSampSet();
  ~CharSampSet();
  // return sample count
  int SampleCount() const { return cnt_; }
  // returns samples buffer
  CharSamp ** Samples() const { return samp_buff_; }
  // Create a CharSampSet set object from a file
  static CharSampSet *FromCharDumpFile(string file_name);
  // Enumerate the Samples in the set one-by-one calling the enumertor's
  // EnumCharSamp method for each sample
  static bool EnumSamples(string file_name, CharSampEnum *enumerator);
  // Create a new Char Dump file
  static FILE *CreateCharDumpFile(string file_name);
  // Add a new sample to the set
  bool Add(CharSamp *char_samp);

 private:
   // sample count
  int cnt_;
  // the char samp array
  CharSamp **samp_buff_;
  // Are the samples owned by the set or not.
  // Determines whether we should cleanup in the end
  bool own_samples_;
  // Cleanup
  void Cleanup();
  // Load character samples from a file
  bool LoadCharSamples(FILE *fp);
};
}

#endif  // CHAR_SAMP_SET_H
