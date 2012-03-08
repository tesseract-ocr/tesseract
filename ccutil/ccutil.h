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

#ifndef TESSERACT_CCUTIL_CCUTIL_H__
#define TESSERACT_CCUTIL_CCUTIL_H__

#include "ambigs.h"
#include "errcode.h"
#include "strngs.h"
#include "tessdatamanager.h"
#include "params.h"
#include "unicharset.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

namespace tesseract {

class CCUtilMutex {
 public:
  CCUtilMutex();

  void Lock();

  void Unlock();
 private:
#ifdef _WIN32
  HANDLE mutex_;
#else
  pthread_mutex_t mutex_;
#endif
};


class CCUtil {
 public:
  CCUtil();
  virtual ~CCUtil();

 public:
  // Read the arguments and set up the data path.
  void main_setup(
                  const char *argv0,        // program name
                  const char *basename      // name of image
                 );
  ParamsVectors *params() { return &params_; }

  STRING datadir;        // dir for data files
  STRING imagebasename;  // name of image
  STRING lang;
  STRING language_data_path_prefix;
  TessdataManager tessdata_manager;
  UNICHARSET unicharset;
  UnicharAmbigs unichar_ambigs;
  STRING imagefile;  // image file name
  STRING directory;  // main directory

 private:
  ParamsVectors params_;

 public:
  // Member parameters.
  // These have to be declared and initialized after params_ member, since
  // params_ should be initialized before parameters are added to it.
  STRING_VAR_H(m_data_sub_dir, "tessdata/", "Directory for data files");
  #ifdef _WIN32
  STRING_VAR_H(tessedit_module_name, WINDLLNAME,
               "Module colocated with tessdata dir");
  #endif
  INT_VAR_H(ambigs_debug_level, 0, "Debug level for unichar ambiguities");
  BOOL_VAR_H(use_definite_ambigs_for_classifier, 0,
             "Use definite ambiguities when running character classifier");
  BOOL_VAR_H(use_ambigs_for_adaption, 0,
             "Use ambigs for deciding whether to adapt to a character");
};

extern CCUtilMutex tprintfMutex;  // should remain global
}  // namespace tesseract

#endif  // TESSERACT_CCUTIL_CCUTIL_H__
