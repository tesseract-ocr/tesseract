///////////////////////////////////////////////////////////////////////
// File:        tessdll.cpp
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
// tessdll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"


#include "mfcpch.h"
#include "applybox.h"
#include "control.h"
#include "tessvars.h"
#include "tessedit.h"
#include "pageres.h"
#include "imgs.h"
#include "varabled.h"
#include "tprintf.h"
#include "tesseractmain.h"
#include "stderr.h"
#include "notdll.h"



#include "tessdll.h"

#ifdef __MSW32__
extern ESHM_INFO shm;            /*info on shm */
#define TICKS       1000
#endif

extern BOOL_VARIABLE tessedit_write_ratings;
extern BOOL_VARIABLE tessedit_write_output;
extern BOOL_VARIABLE tessedit_write_raw_output;
extern BOOL_VARIABLE tessedit_write_txt_map;
extern BOOL_VARIABLE tessedit_resegment_from_boxes;

//unsigned char membuf[sizeof (ETEXT_DESC)+32000L*sizeof (EANYCODE_CHAR)];

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}


TessDllAPI::TessDllAPI(const char* lang) {
  uinT16 oldlang;  //language

  ocr_open_shm ("0", "0", "0", "0", "0", "0", &oldlang);

  Init(NULL, lang);


  if (interactive_mode) {
    debug_window_on.set_value (TRUE);
  }

  tessedit_write_ratings.set_value (TRUE);
  tessedit_write_output.set_value(FALSE);
  tessedit_write_raw_output.set_value(FALSE);
  tessedit_write_txt_map.set_value(FALSE);


  membuf = (unsigned char *) new BYTE[(sizeof (ETEXT_DESC)+32000L*sizeof (EANYCODE_CHAR))];
}

TessDllAPI::~TessDllAPI() {
  EndPage();

  End();

  if (membuf) delete []membuf;
}

int TessDllAPI::BeginPage(uinT32 xsize,uinT32 ysize,unsigned char *buf)
{
    return BeginPage(xsize,ysize,buf,1);
}

int TessDllAPI::BeginPage(uinT32 xsize,uinT32 ysize,unsigned char *buf,uinT8 bpp) {
  SetImage(buf, xsize, ysize, bpp/8, (xsize*bpp + 7)/8);
  return ProcessPagePass1();
}
int TessDllAPI::BeginPageUpright(uinT32 xsize,uinT32 ysize,unsigned char *buf)
{

    return BeginPageUpright(xsize,ysize,buf,1);
}

int TessDllAPI::BeginPageUpright(uinT32 xsize,uinT32 ysize,unsigned char *buf, uinT8 bpp) {
  SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
  SetImage(buf, xsize, ysize, bpp/8, (xsize*bpp + 7)/8);
  return ProcessPagePass1();
}

int TessDllAPI::ProcessPagePass1() {
  if (page_res_ != NULL)
    ClearResults();
  if (FindLines() != 0)
    return -1;

  page_res_ = new PAGE_RES(block_list_);

  if (page_res_)
    tesseract_->recog_all_words(page_res_, global_monitor,0L,1);

  return (page_res_!=0);
}

void TessDllAPI::EndPage() {
  ClearResults();
}


ETEXT_DESC * TessDllAPI::Recognize_all_Words(void) {
  return Recognize_a_Block(0,0,0,0);
}

ETEXT_DESC * TessDllAPI::Recognize_a_Block(uinT32 left,uinT32 right,
                                           uinT32 top,uinT32 bottom) {
  TBOX          target_word_box(ICOORD (left+400, top+400), ICOORD (right+400, bottom+400));
  int           i;


  shm.shm_size=sizeof (ETEXT_DESC)+32000L*sizeof (EANYCODE_CHAR);

  memset(membuf,0,shm.shm_size);
  shm.shm_mem=membuf;


  global_monitor = ocr_setup_monitor();

  tesseract_->recog_all_words(page_res_, global_monitor,
	                          (right==0 ? 0L : &target_word_box), 2);
/* Disabled for now
  for (i=0;i<global_monitor->count;i++) {
    global_monitor->text[i].left-=400;
    global_monitor->text[i].right-=400;
    global_monitor->text[i].bottom-=400;
    global_monitor->text[i].top-=400;
  }
*/

  global_monitor = 0L;

  return ((ETEXT_DESC *) membuf);
}

TessDllAPI *recognize=0L;
char* current_lang = 0L;

extern "C"
{

TESSDLL_API void __cdecl TessDllRelease() {
  if (recognize) delete recognize;
  recognize=0L;
}

TESSDLL_API void  * __cdecl TessDllInit(const char* lang) {
  if (recognize) TessDllRelease();

  recognize = new TessDllAPI(lang);
  if (current_lang != 0L)
    free(current_lang);
  current_lang = lang ? strdup(lang) : 0L;

  return (void*) recognize;
}

TESSDLL_API int __cdecl TessDllBeginPageBPP(uinT32 xsize,uinT32 ysize,
                                         unsigned char *buf, uinT8 bpp) {
  return TessDllBeginPageLangBPP(xsize, ysize, buf, NULL,bpp);
}

TESSDLL_API int __cdecl TessDllBeginPageLangBPP(uinT32 xsize, uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang, uinT8 bpp) {
  if (recognize==0L || (lang != 0L) != (current_lang != 0L) ||
      lang != 0L && strcmp(lang, current_lang))
    TessDllInit(lang);

  return recognize->BeginPage(xsize, ysize, buf,bpp);
}

TESSDLL_API int __cdecl TessDllBeginPageUprightBPP(uinT32 xsize, uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang, uinT8 bpp) {
  if (recognize==0L || (lang != 0L) != (current_lang != 0L) ||
      lang != 0L && strcmp(lang, current_lang))
    TessDllInit(lang);

  return recognize->BeginPageUpright(xsize, ysize, buf,bpp);
}

TESSDLL_API int __cdecl TessDllBeginPage(uinT32 xsize,uinT32 ysize,
                                         unsigned char *buf) {
  return TessDllBeginPageLangBPP(xsize, ysize, buf, NULL,1);
}

TESSDLL_API int __cdecl TessDllBeginPageLang(uinT32 xsize, uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang) {
  if (recognize==0L || (lang != 0L) != (current_lang != 0L) ||
      lang != 0L && strcmp(lang, current_lang))
    TessDllInit(lang);

  return recognize->BeginPage(xsize, ysize, buf,1);
}

TESSDLL_API int __cdecl TessDllBeginPageUpright(uinT32 xsize, uinT32 ysize,
                                             unsigned char *buf,
                                             const char* lang) {
  if (recognize==0L || (lang != 0L) != (current_lang != 0L) ||
      lang != 0L && strcmp(lang, current_lang))
    TessDllInit(lang);

  return recognize->BeginPageUpright(xsize, ysize, buf);
}

TESSDLL_API void __cdecl TessDllEndPage(void) {
  recognize->EndPage();
}

TESSDLL_API ETEXT_DESC * __cdecl TessDllRecognize_a_Block(uinT32 left,
                                                          uinT32 right,
                                                          uinT32 top,
                                                          uinT32 bottom) {
  return recognize->Recognize_a_Block(left,right,top,bottom);
}


TESSDLL_API ETEXT_DESC * __cdecl TessDllRecognize_all_Words(void) {
  return recognize->Recognize_all_Words();
}



//deprecated funtions
TESSDLL_API void __cdecl ReleaseRecognize()
{

    if (recognize) delete recognize;recognize=0L;

}




TESSDLL_API void  * __cdecl InitRecognize()
{
if (recognize) ReleaseRecognize();

recognize = new TessDllAPI();

return (void*) recognize;
}

TESSDLL_API int __cdecl CreateRecognize(uinT32 xsize,uinT32 ysize,unsigned char *buf)
{
InitRecognize();

return recognize->BeginPage(xsize,ysize,buf);

}

TESSDLL_API ETEXT_DESC * __cdecl reconize_a_word(uinT32 left,uinT32 right,uinT32 top,uinT32 bottom)
{
return recognize->Recognize_a_Block(left,right,top,bottom);
}


}
