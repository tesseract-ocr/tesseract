///////////////////////////////////////////////////////////////////////
// File:        ccutil.h
// Description: ccutil class.
// Author:      Samuel Charron
//
// (C) Copyright 2006, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCUTIL_CCUTIL_H_
#define TESSERACT_CCUTIL_CCUTIL_H_

#ifndef _WIN32
#  include <pthread.h>
#  include <semaphore.h>
#endif

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif

#ifndef DISABLED_LEGACY_ENGINE
#  include "ambigs.h"
#endif
#include "errcode.h"
#ifdef _WIN32
#  include "host.h" // windows.h for HANDLE, ...
#endif
#include "params.h"
#include "unicharset.h"

namespace tesseract {

class TESS_API CCUtil {
public:
  CCUtil();
  virtual ~CCUtil();

public:
  // Read the arguments and set up the data path.
  void main_setup(const std::string &argv0,   // program name
                  const std::string &basename // name of image
  );
  ParamsVectors *params() {
    return &params_;
  }

  std::string datadir;       // dir for data files
  std::string imagebasename; // name of image
  std::string lang;
  std::string language_data_path_prefix;
  UNICHARSET unicharset;
#ifndef DISABLED_LEGACY_ENGINE
  UnicharAmbigs unichar_ambigs;
#endif
  std::string imagefile; // image file name
  std::string directory; // main directory

private:
  ParamsVectors params_;

public:
  // Member parameters.
  // These have to be declared and initialized after params_ member, since
  // params_ should be initialized before parameters are added to it.
  INT_VAR_H(ambigs_debug_level);
  BOOL_VAR_H(use_ambigs_for_adaption);
};

} // namespace tesseract

#endif // TESSERACT_CCUTIL_CCUTIL_H_
