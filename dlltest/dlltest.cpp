/**********************************************************************
 * File:        dlltest.cpp
 * Description: Main program to test the tessdll interface.
 * Author:		  Ray Smith
 * Created:			Wed May 16 15:17:46 PDT 2007
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

#include "stdafx.h"
#include "imgs.h"
#include "tessdll.h"

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage:%s imagename outputname\n", argv[0]);
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
  if (image.get_bpp() != 1) {
    fprintf(stderr, "Image is not binary!\n");
    exit(1);
  }

  TessDllAPI api("eng");
  api.BeginPageUpright(image.get_xsize(), image.get_ysize(), image.get_buffer());
  ETEXT_DESC* output = api.Recognize_all_Words();
  
  FILE* fp = fopen(argv[2],"w");
  if (fp == NULL) {
    fprintf(stderr, "Can't create %s\n", argv[2]);
    exit(1);
  }

  for (int i = 0; i < output->count; ++i) {
    const EANYCODE_CHAR* ch = &output->text[i];
    for (int b = 0; b < ch->blanks; ++b)
      fprintf(fp, "\n");
    fprintf(fp, "%C[%x](%d,%d)->(%d,%d)\n",
      ch->char_code, ch->char_code,
      ch->left, ch->bottom, ch->right, ch->top);
    if (ch->formatting & 64)
      fprintf(fp, "<nl>\n\n");
    if (ch->formatting & 128)
      fprintf(fp, "<para>\n\n");
  }

  return 0;
}
