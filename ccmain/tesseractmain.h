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
#include          "tessembedded.h"

extern BOOL_VAR_H (tessedit_read_image, TRUE, "Ensure the image is read");
inT32 api_main(                   //run from api
               const char *arg0,  //program name
               uinT16 lang        //language
              );
inT16 setup_info(                     //setup dummy engine info
                 uinT16 lang,         //user language
                 const char *name,    //of engine
                 const char *version  //of engine
                );
inT16 read_image(               //read dummy image info
                 IMAGE *im_out  //output image
                );
#ifdef __MSW32__
int WINAPI WinMain(  //main for windows //command line
                   HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine,
                   int nCmdShow);
LONG WINAPI WndProc(            //message handler
                    HWND hwnd,  //window with message
                    UINT msg,   //message typ
                    WPARAM wParam,
                    LPARAM lParam);
int parse_args (                 /*refine arg list */
int argc,                        /*no of input args */
char *argv[],                    /*input args */
char *arglist[]                  /*output args */
);
#endif
#endif
