#include "fileio.h"

#include <stdlib.h>
#include <cstdio>
#include <string>

#include "tprintf.h"

namespace tesseract {

///////////////////////////////////////////////////////////////////////////////
// File::
///////////////////////////////////////////////////////////////////////////////
FILE* File::Open(const string& filename, const string& mode) {
  return fopen(filename.c_str(), mode.c_str());
}

FILE* File::OpenOrDie(const string& filename,
                      const string& mode) {
  FILE* stream = fopen(filename.c_str(), mode.c_str());
  if (stream == NULL) {
    tprintf("Unable to open '%s' in mode '%s'", filename.c_str(), mode.c_str());
  }
  return stream;
}

void File::WriteStringToFileOrDie(const string& str,
                                  const string& filename) {
  FILE* stream = fopen(filename.c_str(), "wb");
  if (stream == NULL) {
    tprintf("Unable to open '%s' for writing", filename.c_str());
    return;
  }
  fputs(str.c_str(), stream);
  ASSERT_HOST(fclose(stream) == 0);
}

bool File::Readable(const string& filename) {
  FILE* stream = fopen(filename.c_str(), "rb");
  if (stream == NULL) {
    return false;
  }
  fclose(stream);
  return true;
}

bool File::ReadFileToString(const string& filename, string* out) {
  FILE* stream = File::Open(filename.c_str(), "rb");
  if (stream == NULL)
    return false;
  InputBuffer in(stream);
  *out = "";
  string temp;
  while (in.ReadLine(&temp)) {
    *out += temp;
    *out += '\n';
  }
  return in.CloseFile();
}

void File::ReadFileToStringOrDie(const string& filename, string* out) {
  ASSERT_HOST(ReadFileToString(filename, out));
}

///////////////////////////////////////////////////////////////////////////////
// InputBuffer::
///////////////////////////////////////////////////////////////////////////////
InputBuffer::InputBuffer(FILE* stream)
  : stream_(stream) {
    fseek(stream_, 0, SEEK_END);
    filesize_ = ftell(stream_);
    fseek(stream_, 0, SEEK_SET);
}

InputBuffer::InputBuffer(FILE* stream, size_t)
  : stream_(stream) {
    fseek(stream_, 0, SEEK_END);
    filesize_ = ftell(stream_);
    fseek(stream_, 0, SEEK_SET);
}

InputBuffer::~InputBuffer() {
  if (stream_ != NULL) {
    fclose(stream_);
  }
}

bool InputBuffer::ReadLine(string* out) {
  ASSERT_HOST(stream_ != NULL);
  char* line = NULL;
  size_t line_size;
  int len = getline(&line, &line_size, stream_);

  if (len < 0)
    return false;
  if (len >= 1 && line[len - 1] == '\n')
    line[len - 1] = '\0';
  *out = string(line);
  free(line);
  return true;
}

bool InputBuffer::CloseFile() {
  int ret = fclose(stream_);
  stream_ = NULL;
  return ret == 0;
}

///////////////////////////////////////////////////////////////////////////////
// OutputBuffer::
///////////////////////////////////////////////////////////////////////////////

OutputBuffer::OutputBuffer(FILE* stream)
  : stream_(stream) {
}

OutputBuffer::OutputBuffer(FILE* stream, size_t)
  : stream_(stream) {
}

OutputBuffer::~OutputBuffer() {
  if (stream_ != NULL) {
    fclose(stream_);
  }
}

void OutputBuffer::WriteString(const string& str) {
  fputs(str.c_str(), stream_);
}

bool OutputBuffer::CloseFile() {
  int ret = fclose(stream_);
  stream_ = NULL;
  return ret == 0;
}

}  // namespace tesseract
