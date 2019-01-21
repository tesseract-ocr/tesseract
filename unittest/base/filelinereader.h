// Copyright 2010-2014 Google
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_BASE_FILELINEREADER_H_
#define OR_TOOLS_BASE_FILELINEREADER_H_

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <string>

#include "base/callback.h"
#include "base/integral_types.h"
#include "base/file.h"

namespace {
// The FileLineReader class will read a text file specified by
// 'filename' line by line.  Each line will be cleaned with respect to
// termination ('\n' and '\r').  The line callback will be called in
// sequence on each line.
class FileLineReader {
 public:
  // Creates a file line reader object that will read the file 'filename'
  // line by line.
  explicit FileLineReader(const char* const filename);

  ~FileLineReader();

  // Sets the line callback and takes ownership.
  void set_line_callback(Callback1<char*>* const callback);
  // Reloads the file line by line.
  void Reload();
  // Indicates if the file was loaded successfully.
  bool loaded_successfully() const;

 private:
  const char* filename_;
  std::unique_ptr<Callback1<char*> > line_callback_;
  bool loaded_successfully_;
};
}  // namespace 
#endif  // OR_TOOLS_BASE_FILELINEREADER_H_
