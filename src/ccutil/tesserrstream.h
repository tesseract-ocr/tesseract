// File:        tesserrstream.h
// Description: C++ stream which enhances tprintf
// Author:      Stefan Weil
//
// (C) Copyright 2024
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESSERACT_CCUTIL_TESSERRSTREAM_H
#define TESSERACT_CCUTIL_TESSERRSTREAM_H

#include "tprintf.h"
#include <tesseract/export.h> // for TESS_API

#include <ostream> // for std::ostream

namespace tesseract {

class TessStreamBuf : public std::streambuf {
public:
  TessStreamBuf() = default;

protected:
  virtual int_type overflow(int_type c) override {
    if (c != EOF) {
      if (debugfp == nullptr) {
        debugfp = get_debugfp();
      }
      if (fputc(c, debugfp) == EOF) {
        return EOF;
      }
    }
    return c;
  }

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    if (debugfp == nullptr) {
      debugfp = get_debugfp();
    }
    return fwrite(s, 1, n, debugfp);
  }

private:
  FILE *debugfp = nullptr;
};

class TessErrStream : public std::ostream {
private:
  TessStreamBuf buf;

public:
  TessErrStream() : std::ostream(nullptr), buf() {
    rdbuf(&buf);
  }
};

extern TESS_API TessErrStream tesserr;

} // namespace tesseract

#endif // TESSERACT_CCUTIL_TESSERRSTREAM_H
