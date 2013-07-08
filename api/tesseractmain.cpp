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

// Include automatically generated configuration file if running autoconf
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "strngs.h"
#include "tprintf.h"

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char **argv) {
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

  // Make the order of args a bit more forgiving than it used to be.
  const char* lang = "eng";
  const char* image = NULL;
  const char* output = NULL;
  bool noocr = false;
  bool list_langs = false;
  bool print_parameters = false;

  tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
  int arg = 1;
  while (arg < argc && (output == NULL || argv[arg][0] == '-')) {
    if (strcmp(argv[arg], "-l") == 0 && arg + 1 < argc) {
      lang = argv[arg + 1];
      ++arg;
    } else if (strcmp(argv[arg], "-psm") == 0 && arg + 1 < argc) {
      pagesegmode = static_cast<tesseract::PageSegMode>(atoi(argv[arg + 1]));
      ++arg;
    } else if (strcmp(argv[arg], "--print-parameters") == 0) {
      noocr = true;
      print_parameters = true;
    } else if (strcmp(argv[arg], "-c") == 0 && arg + 1 < argc) {
      // handled properly after api init
      ++arg;
    } else if (image == NULL) {
      image = argv[arg];
    } else if (output == NULL) {
      output = argv[arg];
    }
    ++arg;
  }

  if (argc == 2 && strcmp(argv[1], "--list-langs") == 0) {
    list_langs = true;
    noocr = true;
  }

  if (output == NULL && noocr == false) {
    fprintf(stderr, "Usage:%s imagename outputbase|stdout [-l lang] "
                      "[-psm pagesegmode] [-c configvar=value] "
                      "[configfile...]\n\n", argv[0]);
    fprintf(stderr,
              "pagesegmode values are:\n"
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
              "10 = Treat the image as a single character.\n");
    fprintf(stderr, "multiple -c arguments are allowed.\n");
    fprintf(stderr, "-l lang, -psm pagesegmode and any -c options must occur"
                      "before any configfile.\n\n");
    fprintf(stderr, "Single options:\n");
    fprintf(stderr, "  -v --version: version info\n");
    fprintf(stderr, "  --list-langs: list available languages for tesseract "
                      "engine\n");
    fprintf(stderr, "  --print-parameters: print tesseract parameters to the "
                      "stdout\n");
    exit(1);
  }

  tesseract::TessBaseAPI api;

  STRING tessdata_dir;
  truncate_path(argv[0], &tessdata_dir);
  api.SetOutputName(output);
  int rc = api.Init(tessdata_dir.string(), lang, tesseract::OEM_DEFAULT,
                &(argv[arg]), argc - arg, NULL, NULL, false);

  if (rc) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }

  char opt1[255], opt2[255];
  for (arg = 0; arg < argc; arg++) {
    if (strcmp(argv[arg], "-c") == 0 && arg + 1 < argc) {
      strncpy(opt1, argv[arg + 1], 255);
      *(strchr(opt1, '=')) = 0;
      strncpy(opt2, strchr(argv[arg + 1], '=') + 1, 255);
      opt2[254] = 0;
      ++arg;

      if(!api.SetVariable(opt1, opt2)) {
        fprintf(stderr, "Could not set option: %s=%s\n", opt1, opt2);
      }
    }
  }

  if (list_langs) {
     GenericVector<STRING> languages;
     api.GetAvailableLanguagesAsVector(&languages);
     fprintf(stderr, "List of available languages (%d):\n",
             languages.size());
     for (int index = 0; index < languages.size(); ++index) {
       STRING& string = languages[index];
       fprintf(stderr, "%s\n", string.string());
     }
     api.End();
     exit(0);
  }

  if (print_parameters) {
     FILE* fout = stdout;
     fprintf(stdout, "Tesseract parameters:\n");
     api.PrintVariables(fout);
     api.End();
     exit(0);
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
    fprintf(stderr, "Cannot open input file: %s\n", image);
    exit(2);
  }
  fclose(fin);

  bool output_hocr = false;
  api.GetBoolVariable("tessedit_create_hocr", &output_hocr);
  bool output_box = false;
  api.GetBoolVariable("tessedit_create_boxfile", &output_box);

  FILE* fout = stdout;
  if (strcmp(output, "-") && strcmp(output, "stdout")) {
    STRING outfile = output;
    outfile += output_hocr ? ".html" : output_box ? ".box" : ".txt";
    fout = fopen(outfile.string(), "wb");
    if (fout == NULL) {
      fprintf(stderr, "Cannot create output file %s\n", outfile.string());
      exit(1);
    }
  }

  STRING text_out;
  if (!api.ProcessPages(image, NULL, 0, &text_out)) {
    fprintf(stderr, "Error during processing.\n");
    if (fout != stdout)
      fclose(fout);
  exit(1);
  }

  fwrite(text_out.string(), 1, text_out.length(), fout);
  if (fout != stdout)
    fclose(fout);
  else
    clearerr(fout);

  return 0;                      // Normal exit
}
