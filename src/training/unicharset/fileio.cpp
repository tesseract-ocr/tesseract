/**********************************************************************
 * File:        fileio.cpp
 * Description: File I/O utilities.
 * Author:      Samuel Charron
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required
 * by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
 * OF ANY KIND, either express or implied.  See the License for the specific
 * language governing permissions and limitations under the License.
 *
 **********************************************************************/

#ifdef _WIN32
#  ifndef unlink
#    include <io.h>
#  endif
#else
#  include <glob.h>
#  include <unistd.h>
#endif

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "errcode.h"
#include "fileio.h"
#include "host.h" // includes windows.h for BOOL, ...
#include "tprintf.h"

namespace tesseract {

///////////////////////////////////////////////////////////////////////////////
// File::
///////////////////////////////////////////////////////////////////////////////
FILE *File::Open(const std::string &filename, const std::string &mode) {
  return fopen(filename.c_str(), mode.c_str());
}

FILE *File::OpenOrDie(const std::string &filename, const std::string &mode) {
  FILE *stream = fopen(filename.c_str(), mode.c_str());
  if (stream == nullptr) {
    tprintf("Unable to open '%s' in mode '%s': %s\n", filename.c_str(), mode.c_str(),
            strerror(errno));
  }
  return stream;
}

void File::WriteStringToFileOrDie(const std::string &str, const std::string &filename) {
  FILE *stream = fopen(filename.c_str(), "wb");
  if (stream == nullptr) {
    tprintf("Unable to open '%s' for writing: %s\n", filename.c_str(), strerror(errno));
    return;
  }
  fputs(str.c_str(), stream);
  ASSERT_HOST(fclose(stream) == 0);
}

bool File::Readable(const std::string &filename) {
  FILE *stream = fopen(filename.c_str(), "rb");
  if (stream == nullptr) {
    return false;
  }
  fclose(stream);
  return true;
}

bool File::ReadFileToString(const std::string &filename, std::string *out) {
  FILE *stream = File::Open(filename.c_str(), "rb");
  if (stream == nullptr) {
    return false;
  }
  InputBuffer in(stream);
  *out = "";
  in.Read(out);
  return in.CloseFile();
}

std::string File::JoinPath(const std::string &prefix, const std::string &suffix) {
  return (prefix.empty() || prefix[prefix.size() - 1] == '/') ? prefix + suffix
                                                              : prefix + "/" + suffix;
}

bool File::Delete(const char *pathname) {
#if !defined(_WIN32) || defined(__MINGW32__)
  const int status = unlink(pathname);
#else
  const int status = _unlink(pathname);
#endif
  if (status != 0) {
    tprintf("ERROR: Unable to delete file '%s$: %s\n", pathname, strerror(errno));
    return false;
  }
  return true;
}

#ifdef _WIN32
bool File::DeleteMatchingFiles(const char *pattern) {
  WIN32_FIND_DATA data;
  BOOL result = TRUE;
  HANDLE handle = FindFirstFile(pattern, &data);
  bool all_deleted = true;
  if (handle != INVALID_HANDLE_VALUE) {
    for (; result; result = FindNextFile(handle, &data)) {
      all_deleted &= File::Delete(data.cFileName);
    }
    FindClose(handle);
  }
  return all_deleted;
}
#else
bool File::DeleteMatchingFiles(const char *pattern) {
  glob_t pglob;
  char **paths;
  bool all_deleted = true;
  if (glob(pattern, 0, nullptr, &pglob) == 0) {
    for (paths = pglob.gl_pathv; *paths != nullptr; paths++) {
      all_deleted &= File::Delete(*paths);
    }
    globfree(&pglob);
  }
  return all_deleted;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// InputBuffer::
///////////////////////////////////////////////////////////////////////////////
InputBuffer::InputBuffer(FILE *stream) : stream_(stream) {}

InputBuffer::InputBuffer(FILE *stream, size_t) : stream_(stream) {}

InputBuffer::~InputBuffer() {
  if (stream_ != nullptr) {
    fclose(stream_);
  }
}

bool InputBuffer::Read(std::string *out) {
  char buf[BUFSIZ + 1];
  int l;
  while ((l = fread(buf, 1, BUFSIZ, stream_)) > 0) {
    if (ferror(stream_)) {
      clearerr(stream_);
      return false;
    }
    buf[l] = 0;
    out->append(buf);
  }
  return true;
}

bool InputBuffer::CloseFile() {
  int ret = fclose(stream_);
  stream_ = nullptr;
  return ret == 0;
}

///////////////////////////////////////////////////////////////////////////////
// OutputBuffer::
///////////////////////////////////////////////////////////////////////////////

OutputBuffer::OutputBuffer(FILE *stream) : stream_(stream) {}

OutputBuffer::OutputBuffer(FILE *stream, size_t) : stream_(stream) {}

OutputBuffer::~OutputBuffer() {
  if (stream_ != nullptr) {
    fclose(stream_);
  }
}

void OutputBuffer::WriteString(const std::string &str) {
  fputs(str.c_str(), stream_);
}

bool OutputBuffer::CloseFile() {
  int ret = fclose(stream_);
  stream_ = nullptr;
  return ret == 0;
}

} // namespace tesseract
