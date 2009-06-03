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
#include "output.h"
#include "globals.h"
#include "blread.h"
#include "tfacep.h"
#include "callnet.h"

/*
** Include automatically generated configuration file if running autoconf
*/
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif
// Includes libtiff if HAVE_LIBTIFF is defined
#ifdef HAVE_LIBTIFF
#include "tiffio.h"
#endif

#ifdef _TIFFIO_
void read_tiff_image(TIFF* tif, IMAGE* image);
#endif

#define VARDIR        "configs/" /*variables files */
                                 //config under api
#define API_CONFIG      "configs/api_config"
#define EXTERN

EXTERN BOOL_VAR (tessedit_create_boxfile, FALSE, "Output text with boxes");
EXTERN BOOL_VAR (tessedit_read_image, TRUE, "Ensure the image is read");
EXTERN INT_VAR (tessedit_serial_unlv, 0,
                "0->Whole page, 1->serial no adapt, 2->serial with adapt");
EXTERN INT_VAR (tessedit_page_number, -1,
                "-1 -> All pages, else specifc page to process");
EXTERN BOOL_VAR (tessedit_write_images, FALSE,
"Capture the image from the IPE");
EXTERN BOOL_VAR (tessedit_debug_to_screen, FALSE, "Dont use debug file");

extern inT16 XOFFSET;
extern inT16 YOFFSET;
extern int NO_BLOCK;

const int kMaxIntSize = 22;
const ERRCODE USAGE = "Usage";
char szAppName[] = "Tessedit";   //app name

void TesseractImage(const char* input_file, IMAGE* image, STRING* text_out) {
  int bytes_per_line = check_legal_image_size(image->get_xsize(),
                                              image->get_ysize(),
                                              image->get_bpp());
  if (tessedit_serial_unlv == 0) {
    char* text;
    if (tessedit_create_boxfile)
      text = TessBaseAPI::TesseractRectBoxes(image->get_buffer(),
                                             image->get_bpp()/8,
                                             bytes_per_line, 0, 0,
                                             image->get_xsize(),
                                             image->get_ysize(),
                                             image->get_ysize());
    else if (tessedit_write_unlv)
      text = TessBaseAPI::TesseractRectUNLV(image->get_buffer(),
                                            image->get_bpp()/8,
                                            bytes_per_line, 0, 0,
                                            image->get_xsize(),
                                            image->get_ysize());
    else
      text = TessBaseAPI::TesseractRect(image->get_buffer(), image->get_bpp()/8,
                                        bytes_per_line, 0, 0,
                                        image->get_xsize(), image->get_ysize());
    *text_out += text;
    delete [] text;
  } else {
    BLOCK_LIST blocks;
    STRING filename = input_file;
    int len = filename.length();
    if (len > 4 && filename[len - 4] == '.') {
      filename[len - 4] = '\0';
    }
    if (!read_unlv_file(filename, image->get_xsize(), image->get_ysize(),
                        &blocks)) {
      fprintf(stderr, "Error: Must have a unlv zone file %s to read!\n",
              filename.string());
      return;
    }
    BLOCK_IT b_it = &blocks;
    for (b_it.mark_cycle_pt(); !b_it.cycled_list(); b_it.forward()) {
      BLOCK* block = b_it.data();
      TBOX box = block->bounding_box();
      char* text = TessBaseAPI::TesseractRectUNLV(image->get_buffer(),
                                                  image->get_bpp()/8,
                                                  bytes_per_line,
                                                  box.left(),
                                                  image->get_ysize() - box.top(),
                                                  box.width(),
                                                  box.height());
      *text_out += text;
      delete [] text;
      if (tessedit_serial_unlv == 1)
        TessBaseAPI::ClearAdaptiveClassifier();
    }
  }
  if (tessedit_write_images) {
    page_image.write("tessinput.tif");
  }
}

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char **argv) {
  STRING outfile;               //output file

  if (argc < 3) {
    USAGE.error (argv[0], TESSLOG,
      "%s imagename outputbase [-l lang] [configfile [[+|-]varfile]...]\n",
      argv[0]);
	return 1;
  }
  // Find the required language.
  const char* lang = "eng";
  int arg = 3;
  if (argc >= 5 && strcmp(argv[3], "-l") == 0) {
    lang = argv[4];
    arg = 5;
  }
  // Find the basename of the input file.
  STRING infile(argv[1]);
  const char* lastdot = strrchr(argv[1], '.');
  if (lastdot != NULL) {
    infile[lastdot - argv[1]] = '\0';
  }

  if (argc == arg)
    TessBaseAPI::InitWithLanguage(argv[0], infile.string(), lang,
                                  NULL, false, 0, argv + arg);
  else
    TessBaseAPI::InitWithLanguage(argv[0], infile.string(), lang,
                                  argv[arg], false,
                                  argc - arg - 1, argv + arg + 1);
  TessBaseAPI::SetInputName(argv[1]);

  tprintf ("Tesseract Open Source OCR Engine\n");

  IMAGE image;
  STRING text_out;
#ifdef _TIFFIO_
  int len = strlen(argv[1]);
  if (len > 3 && strcmp("tif", argv[1] + len - 3) == 0) {
    // Use libtiff to read a tif file so multi-page can be handled.
    // The page number so the tiff file can be closed and reopened.
    int page_number = tessedit_page_number;
    if (page_number < 0)
      page_number = 0;
    TIFF* archive = NULL;
    do {
      // Since libtiff keeps all read images in memory we have to close the
      // file and reopen it for every page, and seek to the appropriate page.
      if (archive != NULL)
        TIFFClose(archive);
      archive = TIFFOpen(argv[1], "r");
      if (archive == NULL) {
        READFAILED.error (argv[0], EXIT, argv[1]);
        return 1;
      }
      if (page_number > 0)
        tprintf("Page %d\n", page_number);

      // Seek to the appropriate page.
      for (int i = 0; i < page_number; ++i) {
        TIFFReadDirectory(archive);
      }
      char page_str[kMaxIntSize];
      snprintf(page_str, kMaxIntSize - 1, "%d", page_number);
      TessBaseAPI::SetVariable("applybox_page", page_str);
      ++page_number;
      // Read the current page into the Tesseract image.
      IMAGE image;
      read_tiff_image(archive, &image);

      // Run tesseract on the page!
      TesseractImage(argv[1], &image, &text_out);
    // Do this while there are more pages in the tiff file.
    } while (TIFFReadDirectory(archive) &&
             (page_number <= tessedit_page_number || tessedit_page_number < 0));
    TIFFClose(archive);
  } else {
#endif
    if (image.read_header(argv[1]) < 0)
      READFAILED.error (argv[0], EXIT, argv[1]);
    if (image.read(image.get_ysize ()) < 0) {
      MEMORY_OUT.error(argv[0], EXIT, "Read of image %s",
        argv[1]);
    }
    TesseractImage(argv[1], &image, &text_out);
#ifdef _TIFFIO_
  }
#endif

  outfile = argv[2];
  outfile += ".txt";
  FILE* fp = fopen(outfile.string(), "w");
  if (fp != NULL) {
    fwrite(text_out.string(), 1, text_out.length(), fp);
    fclose(fp);
  }
  TessBaseAPI::End();

  return 0;                      //Normal exit
}

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
