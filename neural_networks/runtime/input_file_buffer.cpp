// Copyright 2008 Google Inc.
// All Rights Reserved.
// Author: ahmadab@google.com (Ahmad Abdulkader)
//
// input_file_buffer.h: Declarations of a class for an object that
// represents an input file buffer.

#include <string>
#include "input_file_buffer.h"

namespace tesseract {
// default and only contsructor
InputFileBuffer::InputFileBuffer(const string &file_name)
  : file_name_(file_name) {
  fp_ = NULL;
}

// virtual destructor
InputFileBuffer::~InputFileBuffer() {
  if (fp_ != NULL) {
    fclose(fp_);
  }
}

// Read the specified number of bytes to the specified input buffer
int InputFileBuffer::Read(void *buffer, int bytes_to_read) {
  // open the file if necessary
  if (fp_ == NULL) {
    fp_ = fopen(file_name_.c_str(), "rb");
    if (fp_ == NULL) {
      return 0;
    }
  }
  return fread(buffer, 1, bytes_to_read, fp_);
}
}
