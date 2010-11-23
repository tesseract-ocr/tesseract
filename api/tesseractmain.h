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

#include          "params.h"
#include          "blobs.h"
#include          "notdll.h"

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
