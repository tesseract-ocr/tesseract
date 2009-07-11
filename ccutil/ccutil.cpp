// Copyright 2008 Google Inc. All Rights Reserved.
// Author: scharron@google.com (Samuel Charron)

#include "ccutil.h"

namespace tesseract {
CCUtil::CCUtil()
    : //// mainblk.* /////////////////////////////////////////////////////
      BOOL_MEMBER(m_print_variables, FALSE,
                  "Print initial values of all variables"),
      STRING_MEMBER(m_data_sub_dir,
                  "tessdata/", "Directory for data files")
      ////////////////////////////////////////////////////////////////////
      {

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


CCUtilMutex tprintfMutex;
} // namespace tesseract
