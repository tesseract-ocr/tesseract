/**********************************************************************
 * File:        dlltest.cpp
 * Description: Main program to test the tessdll interface.
 * Author:      Ray Smith
 * Created:     Wed May 16 15:17:46 PDT 2007
 *
 * (C) Copyright 2007, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/
#define _UNICODE

#include "stdafx.h"
#include "imgs.h"
#include "unichar.h"
#include "tessdll.h"

/**********************************************************************
 *  main()
 *
 **********************************************************************/




static wchar_t *make_unicode_string(const char *utf8)
{
  int size = 0, out_index = 0;
  wchar_t *out;

  /* first calculate the size of the target string */
  int used = 0;
  int utf8_len = strlen(utf8);
  while (used < utf8_len) {
    int step = UNICHAR::utf8_step(utf8 + used);
    if (step == 0)
      break;
    used += step;
    ++size;
  }

  out = (wchar_t *) malloc((size + 1) * sizeof(wchar_t));
  if (out == NULL)
      return NULL;

  /* now convert to Unicode */
  used = 0;
  while (used < utf8_len) {
    int step = UNICHAR::utf8_step(utf8 + used);
    if (step == 0)
      break;
    UNICHAR ch(utf8 + used, step);
    out[out_index++] = ch.first_uni();
    used += step;
  }
  out[out_index] = 0;

  return out;
}


int main(int argc, char **argv) {
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "Usage:%s imagename outputname [lang]\n", argv[0]);
    exit(1);
  }


  IMAGE image;
  if (image.read_header(argv[1]) < 0) {
    fprintf(stderr, "Can't open %s\n", argv[1]);
    exit(1);
  }
  if (image.read(image.get_ysize ()) < 0) {
    fprintf(stderr, "Can't read %s\n", argv[1]);
    exit(1);
  }
 


  TessDllAPI api(argc > 3 ? argv[3] : "eng");



  api.BeginPageUpright(image.get_xsize(), image.get_ysize(), image.get_buffer(),
		       image.get_bpp());

  ETEXT_DESC* output = api.Recognize_all_Words();




  FILE* fp = fopen(argv[2],"w");
  if (fp == NULL) {
    fprintf(stderr, "Can't create %s\n", argv[2]);
    exit(1);
  }

  // It should be noted that the format for char_code for version 2.0 and beyond is UTF8
  // which means that ASCII characters will come out as one structure but other characters
  // will be returned in two or more instances of this structure with a single byte of the
  // UTF8 code in each, but each will have the same bounding box.
  // Programs which want to handle languagues with different characters sets will need to
  // handle extended characters appropriately, but *all* code needs to be prepared to
  // receive UTF8 coded characters for characters such as bullet and fancy quotes.
  int j;
  for (int i = 0; i < output->count; i = j) {
    const EANYCODE_CHAR* ch = &output->text[i];
	  unsigned char unistr[UNICHAR_LEN];
		
    for (int b = 0; b < ch->blanks; ++b)
      fprintf(fp, "\n");

    for (j = i; j < output->count; j++)
	  {
		  const EANYCODE_CHAR* unich = &output->text[j];

		  if (ch->left != unich->left || ch->right != unich->right ||
          ch->top != unich->top || ch->bottom != unich->bottom)
			  break;
		  unistr[j - i] = static_cast<unsigned char>(unich->char_code);
	  }
    unistr[j - i] = '\0';
		  
    wchar_t *utf16ch=make_unicode_string(reinterpret_cast<const char*>(unistr));
#ifndef _UNICODE
    // If we aren't in _UNICODE mode, print string only if ascii.
    if (ch->char_code <= 0x7f) {
      fprintf(fp, "%s", unistr);
#else
    // %S is a microsoft-special. Attempts to translate the Unicode
    // back to the current locale to print in 8 bit
    fprintf(fp, "%S", utf16ch);
#endif
    // Print the hex codes of the utf8 code.
    for (int x = 0; unistr[x] != '\0'; ++x)
      fprintf(fp, "[%x]", unistr[x]);
		fprintf(fp, "->");
    // Print the hex codes of the unicode.
    for (int y = 0; utf16ch[y] != 0; ++y)
      fprintf(fp, "[%x]", utf16ch[y]);
    // Print the coords.
    fprintf(fp, "(%d,%d)->(%d,%d)\n",
      ch->left, ch->bottom, ch->right, ch->top);
    if (ch->formatting & 64)
      fprintf(fp, "<nl>\n\n");
    if (ch->formatting & 128)
      fprintf(fp, "<para>\n\n");
	  free(utf16ch);
  }

  fclose(fp);

  return 0;
}
