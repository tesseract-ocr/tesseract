// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// input_file_buffer.h: Declarations of a class for an object that
// represents an input file buffer.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INPUT_FILE_BUFFER_H
#define INPUT_FILE_BUFFER_H

#include <stdio.h>
#include <string>
#ifdef USE_STD_NAMESPACE
using std::string;
#endif

namespace tesseract {
class InputFileBuffer {
  public:
    explicit InputFileBuffer(const string &file_name);
    virtual ~InputFileBuffer();
    int Read(void *buffer, int bytes_to_read);

  protected:
    string file_name_;
    FILE *fp_;
};
}

#endif  // INPUT_FILE_BUFFER_H__
