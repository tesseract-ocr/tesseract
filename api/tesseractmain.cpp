/**********************************************************************
* File:        tessedit.cpp  (Formerly tessedit.c)
* Description: Main program for merge of tess and editor.
* Author:                  Ray Smith
* Created:                 Tue Jan 07 15:21:46 GMT 1992
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

// #define USE_VLD //Uncomment for Visual Leak Detector.
#if (defined _MSC_VER && defined USE_VLD)
#include "mfcpch.h"
#include <vld.h>
#endif

// Include automatically generated configuration file if running autoconf
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
#ifdef USING_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(x) gettext(x)
#else
#define _(x) (x)
#endif

#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "strngs.h"
#include "tesseractmain.h"
#include "tprintf.h"

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char **argv) {
#ifdef USING_GETTEXT
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif
  if ((argc == 2 && strcmp(argv[1], "-v") == 0) ||
      (argc == 2 && strcmp(argv[1], "--version") == 0)) {
    char *versionStrP;

    fprintf(stderr, "tesseract %s\n", tesseract::TessBaseAPI::Version());

    versionStrP = getLeptonicaVersion();
    fprintf(stderr, " %s\n", versionStrP);
    lept_free(versionStrP);

    versionStrP = getImagelibVersions();
    fprintf(stderr, "  %s\n", versionStrP);
    lept_free(versionStrP);

    exit(0);
  }

  tesseract::TessBaseAPI api;
  STRING tessdata_dir;
  truncate_path(argv[0], &tessdata_dir);
  int rc = api.Init(tessdata_dir.string(), NULL);
  if (rc) {
    fprintf(stderr, _("Could not initialize tesseract.\n"));
    exit(1);
  }

  if (argc == 2 && strcmp(argv[1], "--list-langs") == 0) {
     GenericVector<STRING> languages;
     api.GetAvailableLanguagesAsVector(&languages);
     fprintf(stderr, _("List of available languages (%d):\n"), languages.size());
     for (int index = 0; index < languages.size(); ++index) {
       STRING& string = languages[index];
       fprintf(stderr, "%s\n", string.string());
     }
     api.Clear();
     exit(0);
  }
  api.End();

  // Make the order of args a bit more forgiving than it used to be.
  const char* lang = "eng";
  const char* image = NULL;
  const char* output = NULL;
  tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
  int arg = 1;
  while (arg < argc && (output == NULL || argv[arg][0] == '-')) {
    if (strcmp(argv[arg], "-l") == 0 && arg + 1 < argc) {
      lang = argv[arg + 1];
      ++arg;
    } else if (strcmp(argv[arg], "-psm") == 0 && arg + 1 < argc) {
      pagesegmode = static_cast<tesseract::PageSegMode>(atoi(argv[arg + 1]));
      ++arg;
    } else if (image == NULL) {
      image = argv[arg];
    } else if (output == NULL) {
      output = argv[arg];
    }
    ++arg;
  }
  if (output == NULL) {
    fprintf(stderr, _("Usage:%s imagename outputbase [-l lang] "
                      "[-psm pagesegmode] [configfile...]\n\n"), argv[0]);
    fprintf(stderr,
            _("pagesegmode values are:\n"
              "0 = Orientation and script detection (OSD) only.\n"
              "1 = Automatic page segmentation with OSD.\n"
              "2 = Automatic page segmentation, but no OSD, or OCR\n"
              "3 = Fully automatic page segmentation, but no OSD. (Default)\n"
              "4 = Assume a single column of text of variable sizes.\n"
              "5 = Assume a single uniform block of vertically aligned text.\n"
              "6 = Assume a single uniform block of text.\n"
              "7 = Treat the image as a single text line.\n"
              "8 = Treat the image as a single word.\n"
              "9 = Treat the image as a single word in a circle.\n"
              "10 = Treat the image as a single character.\n"));
    fprintf(stderr, _("-l lang and/or -psm pagesegmode must occur before any"
                      "configfile.\n\n"));
    fprintf(stderr, _("Single options:\n"));
    fprintf(stderr, _("  -v --version: version info\n"));
    fprintf(stderr, _("  --list-langs: list available languages for tesseract "
                      "engine\n"));
    exit(1);
  }


  api.SetOutputName(output);

  rc = api.Init(tessdata_dir.string(), lang, tesseract::OEM_DEFAULT,
                &(argv[arg]), argc - arg, NULL, NULL, false);
  if (rc) {
    fprintf(stderr, _("Could not initialize tesseract.\n"));
    exit(1);
  }

  // We have 2 possible sources of pagesegmode: a config file and
  // the command line. For backwards compatability reasons, the
  // default in tesseract is tesseract::PSM_SINGLE_BLOCK, but the
  // default for this program is tesseract::PSM_AUTO. We will let
  // the config file take priority, so the command-line default
  // can take priority over the tesseract default, so we use the
  // value from the command line only if the retrieved mode
  // is still tesseract::PSM_SINGLE_BLOCK, indicating no change
  // in any config file. Therefore the only way to force
  // tesseract::PSM_SINGLE_BLOCK is from the command line.
  // It would be simpler if we could set the value before Init,
  // but that doesn't work.
  if (api.GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK)
    api.SetPageSegMode(pagesegmode);
  tprintf("Tesseract Open Source OCR Engine v%s with Leptonica\n",
           tesseract::TessBaseAPI::Version());


  FILE* fin = fopen(image, "rb");
  if (fin == NULL) {
    fprintf(stderr, _("Cannot open input file: %s\n"), image);
    exit(2);
  }
  fclose(fin);

  PIX   *pixs;
  if ((pixs = pixRead(image)) == NULL) {
    fprintf(stderr, _("Unsupported image type.\n"));
    exit(3);
  }
  pixDestroy(&pixs);

  STRING text_out;
  if (!api.ProcessPages(image, NULL, 0, &text_out)) {
    fprintf(stderr, _("Error during processing.\n"));
  }
  bool output_hocr = false;
  api.GetBoolVariable("tessedit_create_hocr", &output_hocr);
  bool output_box = false;
  api.GetBoolVariable("tessedit_create_boxfile", &output_box);
  STRING outfile = output;
  outfile += output_hocr ? ".html" : output_box ? ".box" : ".txt";
  FILE* fout = fopen(outfile.string(), "wb");
  if (fout == NULL) {
    fprintf(stderr, _("Cannot create output file %s\n"), outfile.string());
    exit(1);
  }
  fwrite(text_out.string(), 1, text_out.length(), fout);
  fclose(fout);

  return 0;                      // Normal exit
}

#ifdef _WIN32

char szAppName[] = "Tesseract";   //app name
int initialized = 0;

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
  wc.hIcon = NULL;         //LoadIcon (NULL, IDI_APPLICATION);
  wc.hCursor = NULL;       //LoadCursor (NULL, IDC_ARROW);
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
  argv = (char **)malloc(((strlen(argsin[0]) + strlen(argsin[1])) / 2 + 1) *
                         sizeof(char *));

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
        HWND hwnd,              //window with message
        UINT msg,               //message typ
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
        int argc,                /*no of input args */
        char *argv[],            /*input args */
        char *arglist[]          /*output args */
        ) {
  int argcount;            /*converted argc */
  char *testchar;          /*char in option string */
  int arg;                 /*current argument */

  argcount = 0;            /*no of options */
  for (arg = 0; arg < argc; arg++) {
    testchar = argv[arg]; /*start of arg */
    do {
      while (*testchar
             && (*testchar == ' ' || *testchar == '\n'
                 || *testchar == '\t'))
        testchar++; /*skip white space */
      if (*testchar) {
        /*new arg */
        arglist[argcount++] = testchar;
        /*skip to white space */
        for (testchar++; *testchar && *testchar != ' ' && *testchar != '\n' && *testchar != '\t'; testchar++) ;
        if (*testchar)
          *testchar++ = '\0'; /*turn to separate args */
      }
    }
    while (*testchar);
  }
  return argcount;         /*new number of args */
}
#endif
