// Copyright 2008 Google Inc. All Rights Reserved.
// Author: scharron@google.com (Samuel Charron)

#include "ccutil.h"

namespace tesseract {
CCUtil::CCUtil() :
  params_(),
  STRING_INIT_MEMBER(m_data_sub_dir,
                     "tessdata/", "Directory for data files", &params_),
#ifdef __MSW32__
  STRING_INIT_MEMBER(tessedit_module_name, "tessdll.dll",
                     "Module colocated with tessdata dir", &params_),
#endif
  INT_INIT_MEMBER(ambigs_debug_level, 0, "Debug level for unichar ambiguities",
                  &params_),
  BOOL_INIT_MEMBER(use_definite_ambigs_for_classifier, 0, "Use definite"
                   " ambiguities when running character classifier", &params_),
  BOOL_INIT_MEMBER(use_ambigs_for_adaption, 0, "Use ambigs for deciding"
                   " whether to adapt to a character", &params_) {
}

CCUtil::~CCUtil() {
}


CCUtilMutex::CCUtilMutex() {
#ifdef WIN32
  mutex_ = CreateMutex(0, FALSE, 0);
#else
  pthread_mutex_init(&mutex_, NULL);
#endif
}

void CCUtilMutex::Lock() {
#ifdef WIN32
  WaitForSingleObject(mutex_, INFINITE);
#else
  pthread_mutex_lock(&mutex_);
#endif
}

void CCUtilMutex::Unlock() {
#ifdef WIN32
  ReleaseMutex(mutex_);
#else
  pthread_mutex_unlock(&mutex_);
#endif
}

CCUtilMutex tprintfMutex;  // should remain global
} // namespace tesseract
