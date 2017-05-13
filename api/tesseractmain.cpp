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

#include <iostream>

#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "dict.h"
#include "openclwrapper.h"
#include "osdetect.h"
#include "renderer.h"
#include "strngs.h"
#include "tprintf.h"

#if defined(HAVE_TIFFIO_H) && defined(_WIN32)

#include <tiffio.h>

static void Win32WarningHandler(const char* module, const char* fmt,
                                va_list ap) {
  if (module != NULL) {
    fprintf(stderr, "%s: ", module);
  }
  fprintf(stderr, "Warning, ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}

#endif /* HAVE_TIFFIO_H &&  _WIN32 */

void PrintVersionInfo() {
  char* versionStrP;

  printf("tesseract %s\n", tesseract::TessBaseAPI::Version());

  versionStrP = getLeptonicaVersion();
  printf(" %s\n", versionStrP);
  lept_free(versionStrP);

  versionStrP = getImagelibVersions();
  printf("  %s\n", versionStrP);
  lept_free(versionStrP);

#ifdef USE_OPENCL
  cl_platform_id platform[4];
  cl_uint num_platforms;

  printf(" OpenCL info:\n");
  if (clGetPlatformIDs(4, platform, &num_platforms) == CL_SUCCESS) {
    printf("  Found %u platform(s).\n", num_platforms);
    for (unsigned n = 0; n < num_platforms; n++) {
      char info[256];
      if (clGetPlatformInfo(platform[n], CL_PLATFORM_NAME, 256, info, 0) ==
          CL_SUCCESS) {
        printf("  Platform %u name: %s.\n", n + 1, info);
      }
      if (clGetPlatformInfo(platform[n], CL_PLATFORM_VERSION, 256, info, 0) ==
          CL_SUCCESS) {
        printf("  Version: %s.\n", info);
      }
      cl_device_id devices[2];
      cl_uint num_devices;
      if (clGetDeviceIDs(platform[n], CL_DEVICE_TYPE_ALL, 2, devices,
                         &num_devices) == CL_SUCCESS) {
        printf("  Found %u device(s).\n", num_devices);
        for (unsigned i = 0; i < num_devices; ++i) {
          if (clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 256, info, 0) ==
              CL_SUCCESS) {
            printf("    Device %u name: %s.\n", i + 1, info);
          }
        }
      }
    }
    }
#endif
}

void PrintUsage(const char* program) {
  printf(
      "Usage:\n"
      "  %s --help | --help-psm | --help-oem | --version\n"
      "  %s --list-langs [--tessdata-dir PATH]\n"
      "  %s --print-parameters [options...] [configfile...]\n"
      "  %s imagename|stdin outputbase|stdout [options...] [configfile...]\n",
      program, program, program, program);
}

void PrintHelpForPSM() {
  const char* msg =
      "Page segmentation modes:\n"
      "  0    Orientation and script detection (OSD) only.\n"
      "  1    Automatic page segmentation with OSD.\n"
      "  2    Automatic page segmentation, but no OSD, or OCR.\n"
      "  3    Fully automatic page segmentation, but no OSD. (Default)\n"
      "  4    Assume a single column of text of variable sizes.\n"
      "  5    Assume a single uniform block of vertically aligned text.\n"
      "  6    Assume a single uniform block of text.\n"
      "  7    Treat the image as a single text line.\n"
      "  8    Treat the image as a single word.\n"
      "  9    Treat the image as a single word in a circle.\n"
      " 10    Treat the image as a single character.\n"
      " 11    Sparse text. Find as much text as possible in no"
      " particular order.\n"
      " 12    Sparse text with OSD.\n"
      " 13    Raw line. Treat the image as a single text line,\n"
      "\t\t\tbypassing hacks that are Tesseract-specific.\n";

  printf("%s", msg);
}

void PrintHelpForOEM() {
  const char* msg =
      "OCR Engine modes:\n"
      "  0    Original Tesseract only.\n"
      "  1    Cube only.\n"
      "  2    Tesseract + cube.\n"
      "  3    Default, based on what is available.\n";

  printf("%s", msg);
}

void PrintHelpMessage(const char* program) {
  PrintUsage(program);

  const char* ocr_options =
      "OCR options:\n"
      "  --tessdata-dir PATH   Specify the location of tessdata path.\n"
      "  --user-words PATH     Specify the location of user words file.\n"
      "  --user-patterns PATH  Specify the location of user patterns file.\n"
      "  -l LANG[+LANG]        Specify language(s) used for OCR.\n"
      "  -c VAR=VALUE          Set value for config variables.\n"
      "                        Multiple -c arguments are allowed.\n"
      "  --psm NUM             Specify page segmentation mode.\n"
      "  --oem NUM             Specify OCR Engine mode.\n"
      "NOTE: These options must occur before any configfile.\n";

  printf("\n%s\n", ocr_options);
  PrintHelpForPSM();
  PrintHelpForOEM();

  const char* single_options =
      "Single options:\n"
      "  -h, --help            Show this help message.\n"
      "  --help-psm            Show page segmentation modes.\n"
      "  --help-oem            Show OCR Engine modes.\n"
      "  -v, --version         Show version information.\n"
      "  --list-langs          List available languages for tesseract engine.\n"
      "  --print-parameters    Print tesseract parameters to stdout.\n";

  printf("\n%s", single_options);
}

void SetVariablesFromCLArgs(tesseract::TessBaseAPI* api, int argc,
                            char** argv) {
  char opt1[256], opt2[255];
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      strncpy(opt1, argv[i + 1], 255);
      opt1[255] = '\0';
      char* p = strchr(opt1, '=');
      if (!p) {
        fprintf(stderr, "Missing = in configvar assignment\n");
        exit(1);
      }
      *p = 0;
      strncpy(opt2, strchr(argv[i + 1], '=') + 1, 255);
      opt2[254] = 0;
      ++i;

      if (!api->SetVariable(opt1, opt2)) {
        fprintf(stderr, "Could not set option: %s=%s\n", opt1, opt2);
      }
    }
  }
}

void PrintLangsList(tesseract::TessBaseAPI* api) {
  GenericVector<STRING> languages;
  api->GetAvailableLanguagesAsVector(&languages);
  printf("List of available languages (%d):\n", languages.size());
  for (int index = 0; index < languages.size(); ++index) {
    STRING& string = languages[index];
    printf("%s\n", string.string());
  }
  api->End();
}

void PrintBanner() {
  tprintf("Tesseract Open Source OCR Engine v%s with Leptonica\n",
          tesseract::TessBaseAPI::Version());
}

/**
 * We have 2 possible sources of pagesegmode: a config file and
 * the command line. For backwards compatibility reasons, the
 * default in tesseract is tesseract::PSM_SINGLE_BLOCK, but the
 * default for this program is tesseract::PSM_AUTO. We will let
 * the config file take priority, so the command-line default
 * can take priority over the tesseract default, so we use the
 * value from the command line only if the retrieved mode
 * is still tesseract::PSM_SINGLE_BLOCK, indicating no change
 * in any config file. Therefore the only way to force
 * tesseract::PSM_SINGLE_BLOCK is from the command line.
 * It would be simpler if we could set the value before Init,
 * but that doesn't work.
 */
void FixPageSegMode(tesseract::TessBaseAPI* api,
                    tesseract::PageSegMode pagesegmode) {
  if (api->GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK)
    api->SetPageSegMode(pagesegmode);
}

// NOTE: arg_i is used here to avoid ugly *i so many times in this function
void ParseArgs(const int argc, char** argv, const char** lang,
               const char** image, const char** outputbase,
               const char** datapath, bool* list_langs, bool* print_parameters,
               GenericVector<STRING>* vars_vec,
               GenericVector<STRING>* vars_values, int* arg_i,
               tesseract::PageSegMode* pagesegmode,
               tesseract::OcrEngineMode* enginemode) {
  if (argc == 1) {
    PrintHelpMessage(argv[0]);
    exit(0);
  }

  if (argc == 2) {
    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
      PrintHelpMessage(argv[0]);
      exit(0);
    }
    if ((strcmp(argv[1], "--help-psm") == 0)) {
      PrintHelpForPSM();
      exit(0);
    }
    if ((strcmp(argv[1], "--help-oem") == 0)) {
      PrintHelpForOEM();
      exit(0);
    }
    if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
      PrintVersionInfo();
      exit(0);
    }
  }

  bool noocr = false;
  int i = 1;
  while (i < argc && (*outputbase == NULL || argv[i][0] == '-')) {
    if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
      *lang = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--tessdata-dir") == 0 && i + 1 < argc) {
      *datapath = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--user-words") == 0 && i + 1 < argc) {
      vars_vec->push_back("user_words_file");
      vars_values->push_back(argv[i + 1]);
      ++i;
    } else if (strcmp(argv[i], "--user-patterns") == 0 && i + 1 < argc) {
      vars_vec->push_back("user_patterns_file");
      vars_values->push_back(argv[i + 1]);
      ++i;
    } else if (strcmp(argv[i], "--list-langs") == 0) {
      noocr = true;
      *list_langs = true;
    } else if (strcmp(argv[i], "-psm") == 0 && i + 1 < argc) {
      // The parameter -psm is deprecated and was replaced by --psm.
      // It is still supported for compatibility reasons.
      *pagesegmode = static_cast<tesseract::PageSegMode>(atoi(argv[i + 1]));
      ++i;
    } else if (strcmp(argv[i], "--psm") == 0 && i + 1 < argc) {
      *pagesegmode = static_cast<tesseract::PageSegMode>(atoi(argv[i + 1]));
      ++i;
    } else if (strcmp(argv[i], "--oem") == 0 && i + 1 < argc) {
      *enginemode = static_cast<tesseract::OcrEngineMode>(atoi(argv[i + 1]));
      ++i;
    } else if (strcmp(argv[i], "--print-parameters") == 0) {
      noocr = true;
      *print_parameters = true;
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      // handled properly after api init
      ++i;
    } else if (*image == NULL) {
      *image = argv[i];
    } else if (*outputbase == NULL) {
      *outputbase = argv[i];
    }
    ++i;
  }

  *arg_i = i;

  if (argc == 2 && strcmp(argv[1], "--list-langs") == 0) {
    *list_langs = true;
    noocr = true;
  }

  if (*outputbase == NULL && noocr == false) {
    PrintHelpMessage(argv[0]);
    exit(1);
  }
}

void PreloadRenderers(
    tesseract::TessBaseAPI* api,
    tesseract::PointerVector<tesseract::TessResultRenderer>* renderers,
    tesseract::PageSegMode pagesegmode, const char* outputbase) {
  if (pagesegmode == tesseract::PSM_OSD_ONLY) {
    renderers->push_back(new tesseract::TessOsdRenderer(outputbase));
  } else {
    bool b;
    api->GetBoolVariable("tessedit_create_hocr", &b);
    if (b) {
      bool font_info;
      api->GetBoolVariable("hocr_font_info", &font_info);
      renderers->push_back(
          new tesseract::TessHOcrRenderer(outputbase, font_info));
    }

    api->GetBoolVariable("tessedit_create_tsv", &b);
    if (b) {
      bool font_info;
      api->GetBoolVariable("hocr_font_info", &font_info);
      renderers->push_back(
          new tesseract::TessTsvRenderer(outputbase, font_info));
    }

    api->GetBoolVariable("tessedit_create_pdf", &b);
    if (b) {
      bool textonly;
      api->GetBoolVariable("textonly_pdf", &textonly);
      renderers->push_back(new tesseract::TessPDFRenderer(
          outputbase, api->GetDatapath(), textonly));
    }

    api->GetBoolVariable("tessedit_write_unlv", &b);
    if (b) {
      renderers->push_back(new tesseract::TessUnlvRenderer(outputbase));
    }

    api->GetBoolVariable("tessedit_create_boxfile", &b);
    if (b) {
      renderers->push_back(new tesseract::TessBoxTextRenderer(outputbase));
    }

    api->GetBoolVariable("tessedit_create_txt", &b);
    if (b || renderers->empty()) {
      renderers->push_back(new tesseract::TessTextRenderer(outputbase));
    }
  }

  if (!renderers->empty()) {
    // Since the PointerVector auto-deletes, null-out the renderers that are
    // added to the root, and leave the root in the vector.
    for (int r = 1; r < renderers->size(); ++r) {
      (*renderers)[0]->insert((*renderers)[r]);
      (*renderers)[r] = NULL;
    }
  }
}

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char** argv) {
  const char* lang = "eng";
  const char* image = NULL;
  const char* outputbase = NULL;
  const char* datapath = NULL;
  bool list_langs = false;
  bool print_parameters = false;
  int arg_i = 1;
  tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
  tesseract::OcrEngineMode enginemode = tesseract::OEM_DEFAULT;
  /* main() calls functions like ParseArgs which call exit().
   * This results in memory leaks if vars_vec and vars_values are
   * declared as auto variables (destructor is not called then). */
  static GenericVector<STRING> vars_vec;
  static GenericVector<STRING> vars_values;

#ifdef NDEBUG
  // Disable debugging and informational messages from Leptonica.
  setMsgSeverity(L_SEVERITY_ERROR);
#endif

#if defined(HAVE_TIFFIO_H) && defined(_WIN32)
  /* Show libtiff warnings on console (not in GUI). */
  TIFFSetWarningHandler(Win32WarningHandler);
#endif /* HAVE_TIFFIO_H &&  _WIN32 */

  ParseArgs(argc, argv, &lang, &image, &outputbase, &datapath, &list_langs,
            &print_parameters, &vars_vec, &vars_values, &arg_i, &pagesegmode,
            &enginemode);

  bool banner = false;
  if (outputbase != NULL && strcmp(outputbase, "-") &&
      strcmp(outputbase, "stdout")) {
    banner = true;
  }

  PERF_COUNT_START("Tesseract:main")

  // Call GlobalDawgCache here to create the global DawgCache object before
  // the TessBaseAPI object. This fixes the order of destructor calls:
  // first TessBaseAPI must be destructed, DawgCache must be the last object.
  tesseract::Dict::GlobalDawgCache();

  // Avoid memory leak caused by auto variable when exit() is called.
  static tesseract::TessBaseAPI api;

  api.SetOutputName(outputbase);

  int init_failed = api.Init(datapath, lang, enginemode, &(argv[arg_i]),
                             argc - arg_i, &vars_vec, &vars_values, false);
  if (init_failed) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    return EXIT_FAILURE;
  }

  SetVariablesFromCLArgs(&api, argc, argv);

  if (list_langs) {
    PrintLangsList(&api);
    return EXIT_SUCCESS;
  }

  if (print_parameters) {
     FILE* fout = stdout;
     fprintf(stdout, "Tesseract parameters:\n");
     api.PrintVariables(fout);
     api.End();
     return EXIT_SUCCESS;
  }

  FixPageSegMode(&api, pagesegmode);

  if (pagesegmode == tesseract::PSM_AUTO_ONLY) {
    int ret_val = EXIT_SUCCESS;

    Pix* pixs = pixRead(image);
    if (!pixs) {
      fprintf(stderr, "Cannot open input file: %s\n", image);
      return 2;
    }

    api.SetImage(pixs);

    tesseract::Orientation orientation;
    tesseract::WritingDirection direction;
    tesseract::TextlineOrder order;
    float deskew_angle;

    tesseract::PageIterator* it = api.AnalyseLayout();
    if (it) {
      it->Orientation(&orientation, &direction, &order, &deskew_angle);
      tprintf(
          "Orientation: %d\nWritingDirection: %d\nTextlineOrder: %d\n"
          "Deskew angle: %.4f\n",
          orientation, direction, order, deskew_angle);
    } else {
      ret_val = EXIT_FAILURE;
    }

    delete it;

    pixDestroy(&pixs);
    return ret_val;
  }

  // set in_training_mode to true when using one of these configs:
  // ambigs.train, box.train, box.train.stderr, linebox, rebox
  bool b = false;
  bool in_training_mode =
      (api.GetBoolVariable("tessedit_ambigs_training", &b) && b) ||
      (api.GetBoolVariable("tessedit_resegment_from_boxes", &b) && b) ||
      (api.GetBoolVariable("tessedit_make_boxes_from_boxes", &b) && b);

  // Avoid memory leak caused by auto variable when exit() is called.
  static tesseract::PointerVector<tesseract::TessResultRenderer> renderers;

  if (in_training_mode) {
    renderers.push_back(NULL);
  } else {
    PreloadRenderers(&api, &renderers, pagesegmode, outputbase);
  }

  if (!renderers.empty()) {
    if (banner) PrintBanner();
    bool succeed = api.ProcessPages(image, NULL, 0, renderers[0]);
    if (!succeed) {
      fprintf(stderr, "Error during processing.\n");
      return EXIT_FAILURE;
    }
  }

  PERF_COUNT_END

  return EXIT_SUCCESS;
}
