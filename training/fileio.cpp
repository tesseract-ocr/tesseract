/**********************************************************************
 * File:        fileio.cpp
 * Description: File I/O utilities.
 * Author:      Samuel Charron
 * Created:     Tuesday, July 9, 2013
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
#include <windows.h>
#ifndef unlink
#include <io.h>
#endif
#else
#include <glob.h>
#include <unistd.h>
#endif

#include <stdlib.h>
#include <cstdio>
#include <string>

#include "fileio.h"
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
    tprintf("Unable to open '%s' in mode '%s'\n", filename.c_str(),
            mode.c_str());
  }
  return stream;
}

void File::WriteStringToFileOrDie(const string& str,
                                  const string& filename) {
  FILE* stream = fopen(filename.c_str(), "wb");
  if (stream == NULL) {
    tprintf("Unable to open '%s' for writing\n", filename.c_str());
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
  in.Read(out);
  return in.CloseFile();
}

string File::JoinPath(const string& prefix, const string& suffix) {
  return (prefix.empty() || prefix[prefix.size() - 1] == '/')
             ? prefix + suffix
             : prefix + "/" + suffix;
}

bool File::Delete(const char* pathname) {
  const int status = unlink(pathname);
  if (status != 0) {
    tprintf("ERROR: Unable to delete file %s\n", pathname);
    return false;
  }
  return true;
}

#ifdef _WIN32
bool File::DeleteMatchingFiles(const char* pattern) {
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
bool File::DeleteMatchingFiles(const char* pattern) {
  glob_t pglob;
  char **paths;
  bool all_deleted = true;
  if (glob(pattern, 0, NULL, &pglob) == 0) {
    for (paths = pglob.gl_pathv; *paths != NULL; paths++) {
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

bool InputBuffer::Read(string* out) {
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
