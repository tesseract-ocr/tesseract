///////////////////////////////////////////////////////////////////////
// File:        tessdll.h
// Description: Windows dll interface for Tesseract.
// Author:      Glen Wernersbach
// Created:     Tue May 15 10:30:01 PDT 2007
//
// (C) Copyright 2007, Jetsoftdev.
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


#ifndef __cplusplus
typedef BOOL bool;
#endif /* __cplusplus */

#include "ocrclass.h"


#ifdef __cplusplus

#include "baseapi.h"


//This is an exposed C++
class TESSDLL_API TessDllAPI : public tesseract::TessBaseAPI
{
 public:
  //lang is the code of the language for which the data will be loaded.
  //(Codes follow ISO 639-3.) If it is NULL, english (eng) will be loaded.
  TessDllAPI(const char* lang = NULL) ;
  ~TessDllAPI ();

  //xsize should be the width of line in bytes times 8
  //ysize is the height
  //pass through a buffer of bytes for a 1 bit per pixel bitmap
  //BeginPage assumes the first memory address is the bottom of the image
  //BeginPageUpright assumes the first memory address is the top of the image
  int BeginPage(uinT32 xsize,uinT32 ysize,unsigned char *buf);
  int BeginPageUpright(uinT32 xsize,uinT32 ysize,unsigned char *buf);

  // This could probably be combined with about in a one function bpp=1
  int BeginPage(uinT32 xsize,uinT32 ysize,unsigned char *buf,uinT8 bpp);
  int BeginPageUpright(uinT32 xsize,uinT32 ysize,unsigned char *buf, uinT8 bpp);
  void EndPage();

  //This allows you to extract one word or section from the bitmap or
  //the whole page
  //To extract the whole page just enter zeros for left, right, top, bottom
  //Note: getting one word at time is not yet optimized for speed.
  //limit of 32000 character can be returned
  //see ocrclass.h for a decription of the ETEXT_DESC file
  ETEXT_DESC *Recognize_a_Block(uinT32 left,uinT32 right,
                                uinT32 top,uinT32 bottom);
  ETEXT_DESC *Recognize_all_Words(void);

 private:
  int ProcessPagePass1();

  unsigned char *membuf;
};

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TESSDLL_API
#ifdef TESSDLL_EXPORTS
#define TESSDLL_API __declspec(dllexport)
#elif defined(TESSDLL_IMPORTS)
#define TESSDLL_API __declspec(dllimport)
#else
#define TESSDLL_API
#endif
#endif


//The functions below provide a c wrapper to a global recognize class object

//xsize should be the width of line in bytes times 8
//ysize is the height
//pass through a buffer of bytes for a 1 bit per pixel bitmap
//BeginPage assumes the first memory address is the bottom of the image (MS DIB format)
//BeginPageUpright assumes the first memory address is the top of the image (TIFF format)
//lang is the code of the language for which the data will be loaded.
//(Codes follow ISO 639-3.) If it is NULL, english (eng) will be loaded.
TESSDLL_API int __cdecl TessDllBeginPage(uinT32 xsize,uinT32 ysize,
                                         unsigned char *buf);

TESSDLL_API int __cdecl TessDllBeginPageLang(uinT32 xsize,uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang);
TESSDLL_API int __cdecl TessDllBeginPageUpright(uinT32 xsize,uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang);
//Added in version 2.0 to allow users to specify bytes per pixel to do
//1 for binary biptmap
//8 for gray
//24 bit for color RGB
TESSDLL_API int __cdecl TessDllBeginPageBPP(uinT32 xsize,uinT32 ysize,
                                         unsigned char *buf,uinT8 bpp);

TESSDLL_API int __cdecl TessDllBeginPageLangBPP(uinT32 xsize,uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang,uinT8 bpp);
TESSDLL_API int __cdecl TessDllBeginPageUprightBPP(uinT32 xsize,uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang,uinT8 bpp);

TESSDLL_API void __cdecl TessDllEndPage(void);

//This allows you to extract one word or section from the bitmap or
//the whole page
//To extract the whole page just enter zeros for left, right, top, bottom
//Note: getting one word at time is not yet optimized for speed.
//limit of 32000 character can be returned
//see ocrclass.h for a decription of the ETEXT_DESC file
TESSDLL_API ETEXT_DESC * __cdecl TessDllRecognize_a_Block(uinT32 left,
                                                          uinT32 right,
                                                          uinT32 top,
                                                          uinT32 bottom);
TESSDLL_API ETEXT_DESC * __cdecl TessDllRecognize_all_Words();

//This will release any memory associated with the recognize class object
TESSDLL_API void __cdecl TessDllRelease();

#ifdef __cplusplus
}
#endif
