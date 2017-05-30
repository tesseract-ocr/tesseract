///////////////////////////////////////////////////////////////////////
// File:        raiileptonica.h
// Description: RAII utilities for Leptonica types
// Author:      Raf Schietekat
//
// (C) Copyright 2017, Google Inc.
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

#ifndef TESSERACT_CCUTIL_RAIILEPTONICA_FORWARD_H_
#define TESSERACT_CCUTIL_RAIILEPTONICA_FORWARD_H_

#include <memory> // std::unique_ptr

// These are smart pointers that will manage a single reference to
// a Leptonica object, but without implying unique global ownership.
// (In most cases this is sufficient, so it would not be worthwhile
// to create a new dependency on boost for its boost::intrusive_ptr,
// and it also seems more robust to have an explicit constructor that
// consumes its argument without requiring an extra specification.)
// Use, e.g., PixPtr (almost) like its specialized std::unique_ptr base,
// relying on its deleter for calling pixDestroy() at the right time,
// and also use some of the replaced or added features (see implementation).
// Operations rawInOut() and rawOut() are strictly for specified usage,
// and provided only to avoid changing existing signatures (for now?).
// When there is a pre-existing raw pointer with assumed ownership,
// replace, e.g., the following:
//   Pix* final_pix = pixBlockconv(pix_, 1, 1);
//   pixDestroy(&pix_);
//   pix_ = final_pix;
// with just:
//   asPixPtr(pix_).reset(pixBlockconv(pix_, 1, 1));

// TODO: or make What##Ptr ancestry private,
//         with "using declarations" for whitelisted features?

// Code generation parameter usage:
// - T: the referent's type
// - What (typically same as T):
//   - for general use: What##Ptr, as##What##Ptr()
//   - only for technical use here: What##Ptr_deleter
// - what: what##Destroy(), function parameter name

// See usage below for M1 and M2, defined with common prefix M0:
#define TESSERACT_CCUTIL_RAIILEPTONICA_M0(T,What,what)                        \
    struct T;                                                                 \
    struct What##Ptr_deleter {                                                \
      /*must not throw any exceptions*/                                       \
      void operator() (T* what) const;                                        \
    };                                                                        \
    struct What##Ptr : std::unique_ptr<T, What##Ptr_deleter> {                \
     private:                                                                 \
      typedef std::unique_ptr<T, What##Ptr_deleter> super;                    \
     public:                                                                  \
      explicit What##Ptr(T* what = nullptr) : super(what) {}                  \
      /*don't visually wag the dog, e.g., pix.get() -> pix.p()*/              \
      T* get() const = delete;                                                \
      T* p() const { return super::get(); }                                   \

// Simply close off What##Ptr:
#define TESSERACT_CCUTIL_RAIILEPTONICA_M1(T,What,what)                        \
        TESSERACT_CCUTIL_RAIILEPTONICA_M0(T,What,what)                        \
    };                                                                        \

// Add to What##Ptr, then close off:
#define TESSERACT_CCUTIL_RAIILEPTONICA_M2(T,What,what)                        \
        TESSERACT_CCUTIL_RAIILEPTONICA_M0(T,What,what)                        \
      /*prefer const + detach() to non-const + release()*/                    \
      /*  (named after intrusive_ptr::detach())*/                             \
      T* detach() const;                                                      \
      /*strictly for use as an input&output argument*/                        \
      T* &rawInOut();                                                         \
      /*strictly for use as an output argument,*/                             \
      /*  right after default initialisation*/                                \
      /*well, also for Tesseract::mutable_pix_binary()...*/                   \
      T* &rawOut();                                                           \
    };                                                                        \

// without reference count, T != What:
TESSERACT_CCUTIL_RAIILEPTONICA_M1(L_Bmf,Bmf,bmf)

// with reference count, T == What:
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Box,Box,box)
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Boxa,Boxa,boxa)
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Numa,Numa,numa)
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Pix,Pix,pix)
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Pixa,Pixa,pixa)
TESSERACT_CCUTIL_RAIILEPTONICA_M2(Pta,Pta,pta)

#undef TESSERACT_CCUTIL_RAIILEPTONICA_M2
#undef TESSERACT_CCUTIL_RAIILEPTONICA_M1
#undef TESSERACT_CCUTIL_RAIILEPTONICA_M0

#endif // TESSERACT_CCUTIL_RAIILEPTONICA_FORWARD_H_
