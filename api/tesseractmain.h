/**********************************************************************
 * File:        tessedit.h  (Formerly tessedit.h)
 * Description: Main program for merge of tess and editor.
 * Author:		Ray Smith
 * Created:		Tue Jan 07 15:21:46 GMT 1992
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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

#ifndef           TESSERACTMAIN_H
#define           TESSERACTMAIN_H

#include          "varable.h"
#include          "tessclas.h"
#include          "notdll.h"

extern BOOL_VAR_H(tessedit_create_boxfile, FALSE, "Output text with boxes");
extern BOOL_VAR_H(tessedit_read_image, TRUE, "Ensure the image is read");
extern INT_VAR_H(tessedit_serial_unlv, 0,
        "0->Whole page, 1->serial no adapt, 2->serial with adapt");
extern INT_VAR_H(tessedit_page_number, -1,
        "-1 -> All pages, else specific page to process");
extern BOOL_VAR_H(tessedit_write_images, FALSE,
                  "Capture the image from the IPE");
extern BOOL_VAR_H(tessedit_debug_to_screen, FALSE, "Dont use debug file");

/**
 * run from api
 * @param arg0 program name
 * @param lang language
 */
inT32 api_main(const char *arg0,
               uinT16 lang);
/**
 * setup dummy engine info
 * @param lang user language
 * @param name of engine
 * @param version of engine
 */
inT16 setup_info(uinT16 lang,
                 const char *name,
                 const char *version);
/**
 * read dummy image info
 * @param im_out read dummy image info
 */
inT16 read_image(IMAGE *im_out);
#ifdef __MSW32__
/**
 * main for windows command line
 */
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine,
                   int nCmdShow);
/**
 * message handler
 * @param hwnd window with message
 * @param msg message type
 */
LONG WINAPI WndProc(HWND hwnd,
                    UINT msg,
                    WPARAM wParam,
                    LPARAM lParam);
/**
 * refine argument list
 * @param argc number of input arguments
 * @param argv input arguments
 * @param arglist output arguments
 */
int parse_args (int argc,
                char *argv[],
                char *arglist[]);
#endif
#endif
