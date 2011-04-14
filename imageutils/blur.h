// Copyright 2010 Google Inc. All Rights Reserved.
// Author: xiaotao@google.com (Xiaotao Duan)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_BLUR_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_BLUR_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Detects whether a given luminance matrix is blurred or not.
// The input matrix size if width * height. 1 is returned when
// input image is blurred along with blur confidence and extent
// returned through output value blur and extent.
int IsBlurred(const uint8* const luminance, const int width, const int height,
              float* const blur, float* const extent);

#ifdef __cplusplus
}
#endif

#endif  // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_BLUR_H_
