// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// input_file_buffer.h: Declarations of a class for an object that
// represents an input file buffer.
//

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
