// Copyright 2010 Google Inc. All Rights Reserved.
// Author: xiaotao@google.com (Xiaotao Duan)

#ifndef JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_SIMILAR_H_
#define JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_SIMILAR_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Computes signature of a given image. This signature can be used to
// compute similarity of two different images. Signature is returned
// and size is returned via output parameter size.
uint32* ComputeSignature(const uint8* const luminance,
                         const int width, const int height, int* size);

// Returns how different two given images (represented by their signatures)
// are. The input signatures must be in the same size. An integer from 0 to
// 100 is returned to indicate difference percentage of signature2
// comparing against signature1.
int Diff(const int32* const signature1, const int32* const signature2,
         const int size);

#ifdef __cplusplus
}
#endif

#endif  // JAVA_COM_GOOGLE_ANDROID_APPS_UNVEIL_JNI_IMAGEUTILS_SIMILAR_H_
