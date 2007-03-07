/**********************************************************************
 * File:        tessedit.cpp  (Formerly tessedit.c)
 * Description: Main program for merge of tess and editor.
 * Author:					Ray Smith
 * Created:					Tue Jan 07 15:21:46 GMT 1992
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

#include "mfcpch.h"
#include "applybox.h"
#include "control.h"
#include "tessvars.h"
#include "tessedit.h"
#include "baseapi.h"
#include "pageres.h"
#include "imgs.h"
#include "varabled.h"
#include "tprintf.h"
#include "tesseractmain.h"
#include "stderr.h"
#include "notdll.h"
#include "mainblk.h"
#include "globals.h"
#include "tfacep.h"
#include "callnet.h"

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"
#define EXTERN

EXTERN BOOL_VAR (tessedit_read_image, TRUE, "Ensure the image is read");
EXTERN BOOL_VAR (tessedit_write_images, FALSE,
"Capture the image from the IPE");
EXTERN BOOL_VAR (tessedit_debug_to_screen, FALSE, "Dont use debug file");

extern INT16 XOFFSET;
extern INT16 YOFFSET;
extern int NO_BLOCK;

const ERRCODE USAGE = "Usage";
char szAppName[] = "Tessedit";   //app name

/**********************************************************************
 *  main()
 *
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
int main(int argc, char **argv) {
  STRING outfile;               //output file

  if (argc < 3) {
    USAGE.error (argv[0], EXIT,
      "%s imagename outputbase [configfile [[+|-]varfile]...]\n", argv[0]);
  }

  if (argc == 3)
    TessBaseAPI::Init(argv[0], argv[1], NULL, false, 0, argv + 2);
  else
    TessBaseAPI::Init(argv[0], argv[1], argv[3], false, argc - 4, argv + 4);

  tprintf ("Tesseract Open Source OCR Engine\n");

  IMAGE image;
#ifdef _TIFFIO_
  TIFF* tif = TIFFOpen(argv[1], "r");
  if (tif) {
    read_tiff_image(tif, &image);
    TIFFClose(tif);
  } else {
    READFAILED.error (argv[0], EXIT, argv[1]);
  }
#else
  if (image.read_header(argv[1]) < 0)
    READFAILED.error (argv[0], EXIT, argv[1]);
  if (image.read(image.get_ysize ()) < 0) {
    MEMORY_OUT.error(argv[0], EXIT, "Read of image %s",
      argv[1]);
  }
#endif
  int bytes_per_line = check_legal_image_size(image.get_xsize(),
                                              image.get_ysize(),
                                              image.get_bpp());
  char* text = TessBaseAPI::TesseractRect(image.get_buffer(), image.get_bpp()/8,
                                          bytes_per_line, 0, 0,
                                          image.get_xsize(), image.get_ysize());
  outfile = argv[2];
  outfile += ".txt";
  FILE* fp = fopen(outfile.string(), "w");
  if (fp != NULL) {
    fwrite(text, 1, strlen(text), fp);
    fclose(fp);
  }
  delete [] text;
  TessBaseAPI::End();

  return 0;                      //Normal exit
}
#else

int main(int argc, char **argv) {
  UINT16 lang;                   //language
  STRING pagefile;               //input file

  if (argc < 4) {
    USAGE.error (argv[0], EXIT,
      "%s imagename outputbase configfile [[+|-]varfile]...\n", argv[0]);
  }

  time_t t_start = time(NULL);

  init_tessembedded (argv[0], argv[2], argv[3], argc - 4, argv + 4);

  tprintf ("Tesseract Open Source OCR Engine (graphics disabled)\n");

  if (tessedit_read_image) {
#ifdef _TIFFIO_
    TIFF* tif = TIFFOpen(argv[1], "r");
    if (tif) {
      read_tiff_image(tif);
      TIFFClose(tif);
    } else
    READFAILED.error (argv[0], EXIT, argv[1]);

#else
    if (page_image.read_header (argv[1]) < 0)
      READFAILED.error (argv[0], EXIT, argv[1]);
    if (page_image.read (page_image.get_ysize ()) < 0) {
      MEMORY_OUT.error (argv[0], EXIT, "Read of image %s",
        argv[1]);
    }
#endif
  }

  pagefile = argv[1];

  BLOCK_LIST current_block_list;
  tessembedded_read_file(pagefile, &current_block_list);
  tprintf ("Done reading files.\n");

  PAGE_RES page_res(&current_block_list);

  recog_all_words(&page_res, NULL);

  current_block_list.clear();
  ResetAdaptiveClassifier();

  time_t t_end = time(NULL);
  double secs = difftime(t_end, t_start);
  tprintf ("Done. Number of seconds: %d\n", (int)secs);
  return 0;                      //Normal exit
}

#endif

int initialized = 0;

#ifdef __MSW32__
/**********************************************************************
 * WinMain
 *
 * Main function for a windows program.
 **********************************************************************/

int WINAPI WinMain(  //main for windows //command line
                   HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine,
                   int nCmdShow) {
  WNDCLASS wc;
  HWND hwnd;
  MSG msg;

  char **argv;
  char *argsin[2];
  int argc;
  int exit_code;

  wc.style = CS_NOCLOSE | CS_OWNDC;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = NULL;               //LoadIcon (NULL, IDI_APPLICATION);
  wc.hCursor = NULL;             //LoadCursor (NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = szAppName;

  RegisterClass(&wc);

  hwnd = CreateWindow (szAppName, szAppName,
    WS_OVERLAPPEDWINDOW | WS_DISABLED,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, HWND_DESKTOP, NULL, hInstance, NULL);

  argsin[0] = strdup (szAppName);
  argsin[1] = strdup (lpszCmdLine);
  /*allocate memory for the args. There can never be more than half*/
  /*the total number of characters in the arguments.*/
  argv =
    (char **) malloc (((strlen (argsin[0]) + strlen (argsin[1])) / 2 + 1) *
    sizeof (char *));

  /*now construct argv as it should be for C.*/
  argc = parse_args (2, argsin, argv);

  //  ShowWindow (hwnd, nCmdShow);
  //  UpdateWindow (hwnd);

  if (initialized) {
    exit_code = main (argc, argv);
    free (argsin[0]);
    free (argsin[1]);
    free(argv);
    return exit_code;
  }
  while (GetMessage (&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (initialized) {
      exit_code = main (argc, argv);
      break;
    }
    else
      exit_code = msg.wParam;
  }
  free (argsin[0]);
  free (argsin[1]);
  free(argv);
  return exit_code;
}


/**********************************************************************
 * WndProc
 *
 * Function to respond to messages.
 **********************************************************************/

LONG WINAPI WndProc(            //message handler
                    HWND hwnd,  //window with message
                    UINT msg,   //message typ
                    WPARAM wParam,
                    LPARAM lParam) {
  HDC hdc;

  if (msg == WM_CREATE) {
    //
    // Create a rendering context.
    //
    hdc = GetDC (hwnd);
    ReleaseDC(hwnd, hdc);
    initialized = 1;
    return 0;
  }
  return DefWindowProc (hwnd, msg, wParam, lParam);
}


/**********************************************************************
 * parse_args
 *
 * Turn a list of args into a new list of args with each separate
 * whitespace spaced string being an arg.
 **********************************************************************/

int
parse_args (                     /*refine arg list */
int argc,                        /*no of input args */
char *argv[],                    /*input args */
char *arglist[]                  /*output args */
) {
  int argcount;                  /*converted argc */
  char *testchar;                /*char in option string */
  int arg;                       /*current argument */

  argcount = 0;                  /*no of options */
  for (arg = 0; arg < argc; arg++) {
    testchar = argv[arg];        /*start of arg */
    do {
      while (*testchar
        && (*testchar == ' ' || *testchar == '\n'
        || *testchar == '\t'))
        testchar++;              /*skip white space */
      if (*testchar) {
                                 /*new arg */
        arglist[argcount++] = testchar;
                                 /*skip to white space */
        for (testchar++; *testchar && *testchar != ' ' && *testchar != '\n' && *testchar != '\t'; testchar++);
        if (*testchar)
          *testchar++ = '\0';    /*turn to separate args */
      }
    }
    while (*testchar);
  }
  return argcount;               /*new number of args */
}
#endif
