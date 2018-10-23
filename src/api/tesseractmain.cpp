/**********************************************************************
 * File:        tesseractmain.cpp  (Formerly tessedit.c)
 * Description: Main program for merge of tess and editor.
 * Author:      Ray Smith
 * Created:     Tue Jan 07 15:21:46 GMT 1992
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

#include <cerrno>               // for errno
#include <iostream>

#include "allheaders.h"
#include "baseapi.h"
#include "basedir.h"
#include "dict.h"
#include "openclwrapper.h"
#include "osdetect.h"
#include "renderer.h"
#include "simddetect.h"
#include "strngs.h"
#include "tprintf.h"            // for tprintf

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#if defined(HAVE_TIFFIO_H)

#include <tiffio.h>

static void Win32ErrorHandler(const char* module, const char* fmt,
                              va_list ap) {
  if (module != nullptr) {
    fprintf(stderr, "%s: ", module);
  }
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}

static void Win32WarningHandler(const char* module, const char* fmt,
                                va_list ap) {
  if (module != nullptr) {
    fprintf(stderr, "%s: ", module);
  }
  fprintf(stderr, "Warning, ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}

#endif /* HAVE_TIFFIO_H */
#endif   // _WIN32

static void PrintVersionInfo() {
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
    if (SIMDDetect::IsAVX512BWAvailable()) printf(" Found AVX512BW\n");
    if (SIMDDetect::IsAVX512FAvailable()) printf(" Found AVX512F\n");
    if (SIMDDetect::IsAVX2Available()) printf(" Found AVX2\n");
    if (SIMDDetect::IsAVXAvailable()) printf(" Found AVX\n");
    if (SIMDDetect::IsSSEAvailable()) printf(" Found SSE\n");
}

static void PrintHelpForPSM() {
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
      "       bypassing hacks that are Tesseract-specific.\n";

#ifdef DISABLED_LEGACY_ENGINE
  const char* disabled_osd_msg =
      "\nNOTE: The OSD modes are currently disabled.\n";
  printf("%s%s", msg, disabled_osd_msg);
#else
  printf("%s", msg);
#endif
}

#ifndef DISABLED_LEGACY_ENGINE
static void PrintHelpForOEM() {
  const char* msg =
      "OCR Engine modes:\n"
      "  0    Legacy engine only.\n"
      "  1    Neural nets LSTM engine only.\n"
      "  2    Legacy + LSTM engines.\n"
      "  3    Default, based on what is available.\n";

  printf("%s", msg);
}
#endif  // ndef DISABLED_LEGACY_ENGINE

static void PrintHelpExtra(const char* program) {
  printf(
      "Usage:\n"
      "  %s --help | --help-extra | --help-psm | "
#ifndef DISABLED_LEGACY_ENGINE
      "--help-oem | "
#endif
      "--version\n"
      "  %s --list-langs [--tessdata-dir PATH]\n"
      "  %s --print-parameters [options...] [configfile...]\n"
      "  %s imagename|imagelist|stdin outputbase|stdout [options...] [configfile...]\n"
      "\n"
      "OCR options:\n"
      "  --tessdata-dir PATH   Specify the location of tessdata path.\n"
      "  --user-words PATH     Specify the location of user words file.\n"
      "  --user-patterns PATH  Specify the location of user patterns file.\n"
      "  --dpi VALUE           Specify DPI for input image.\n"
      "  -l LANG[+LANG]        Specify language(s) used for OCR.\n"
      "  -c VAR=VALUE          Set value for config variables.\n"
      "                        Multiple -c arguments are allowed.\n"
      "  --psm NUM             Specify page segmentation mode.\n"
#ifndef DISABLED_LEGACY_ENGINE
      "  --oem NUM             Specify OCR Engine mode.\n"
#endif
      "NOTE: These options must occur before any configfile.\n"
      "\n",
      program, program, program, program
  );

  PrintHelpForPSM();
#ifndef DISABLED_LEGACY_ENGINE
  printf("\n");
  PrintHelpForOEM();
#endif

  printf(
      "\n"
      "Single options:\n"
      "  -h, --help            Show minimal help message.\n"
      "  --help-extra          Show extra help for advanced users.\n"
      "  --help-psm            Show page segmentation modes.\n"
#ifndef DISABLED_LEGACY_ENGINE
      "  --help-oem            Show OCR Engine modes.\n"
#endif
      "  -v, --version         Show version information.\n"
      "  --list-langs          List available languages for tesseract engine.\n"
      "  --print-parameters    Print tesseract parameters.\n"
  );
}

static void PrintHelpMessage(const char* program) {
  printf(
      "Usage:\n"
      "  %s --help | --help-extra | --version\n"
      "  %s --list-langs\n"
      "  %s imagename outputbase [options...] [configfile...]\n"
      "\n"
      "OCR options:\n"
      "  -l LANG[+LANG]        Specify language(s) used for OCR.\n"
      "NOTE: These options must occur before any configfile.\n"
      "\n"
      "Single options:\n"
      "  --help                Show this help message.\n"
      "  --help-extra          Show extra help for advanced users.\n"
      "  --version             Show version information.\n"
      "  --list-langs          List available languages for tesseract engine.\n",
      program, program, program
  );
}

static void SetVariablesFromCLArgs(tesseract::TessBaseAPI* api, int argc,
                                   char** argv) {
  char opt1[256], opt2[255];
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      strncpy(opt1, argv[i + 1], 255);
      opt1[255] = '\0';
      char* p = strchr(opt1, '=');
      if (!p) {
        fprintf(stderr, "Missing = in configvar assignment\n");
        exit(EXIT_FAILURE);
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

static void PrintLangsList(tesseract::TessBaseAPI* api) {
  GenericVector<STRING> languages;
  api->GetAvailableLanguagesAsVector(&languages);
  printf("List of available languages (%d):\n", languages.size());
  for (int index = 0; index < languages.size(); ++index) {
    STRING& string = languages[index];
    printf("%s\n", string.string());
  }
  api->End();
}

static void PrintBanner() {
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
static void FixPageSegMode(tesseract::TessBaseAPI* api,
                           tesseract::PageSegMode pagesegmode) {
  if (api->GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK)
    api->SetPageSegMode(pagesegmode);
}

static void checkArgValues(int arg, const char* mode, int count) {
  if (arg >= count || arg < 0) {
    printf("Invalid %s value, please enter a number between 0-%d\n", mode, count - 1);
    exit(EXIT_SUCCESS);
  }
}

// NOTE: arg_i is used here to avoid ugly *i so many times in this function
static void ParseArgs(const int argc, char** argv, const char** lang,
                      const char** image, const char** outputbase,
                      const char** datapath, l_int32* dpi, bool* list_langs,
                      bool* print_parameters, GenericVector<STRING>* vars_vec,
                      GenericVector<STRING>* vars_values, l_int32* arg_i,
                      tesseract::PageSegMode* pagesegmode,
                      tesseract::OcrEngineMode* enginemode) {
  bool noocr = false;
  int i;
  for (i = 1; i < argc && (*outputbase == nullptr || argv[i][0] == '-'); i++) {
    if (*image != nullptr && *outputbase == nullptr) {
      // outputbase follows image, don't allow options at that position.
      *outputbase = argv[i];
    } else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
      PrintHelpMessage(argv[0]);
      noocr = true;
    } else if (strcmp(argv[i], "--help-extra") == 0) {
      PrintHelpExtra(argv[0]);
      noocr = true;
    } else if ((strcmp(argv[i], "--help-psm") == 0)) {
      PrintHelpForPSM();
      noocr = true;
#ifndef DISABLED_LEGACY_ENGINE
    } else if ((strcmp(argv[i], "--help-oem") == 0)) {
      PrintHelpForOEM();
      noocr = true;
#endif
    } else if ((strcmp(argv[i], "-v") == 0) ||
               (strcmp(argv[i], "--version") == 0)) {
      PrintVersionInfo();
      noocr = true;
    } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
      *lang = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--tessdata-dir") == 0 && i + 1 < argc) {
      *datapath = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--dpi") == 0 && i + 1 < argc) {
      *dpi = atoi(argv[i + 1]);
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
    } else if (strcmp(argv[i], "--psm") == 0 && i + 1 < argc) {
      checkArgValues(atoi(argv[i+1]), "PSM", tesseract::PSM_COUNT);
      *pagesegmode = static_cast<tesseract::PageSegMode>(atoi(argv[i + 1]));
      ++i;
    } else if (strcmp(argv[i], "--oem") == 0 && i + 1 < argc) {
#ifndef DISABLED_LEGACY_ENGINE
      int oem = atoi(argv[i + 1]);
      checkArgValues(oem, "OEM", tesseract::OEM_COUNT);
      *enginemode = static_cast<tesseract::OcrEngineMode>(oem);
#endif
      ++i;
    } else if (strcmp(argv[i], "--print-parameters") == 0) {
      noocr = true;
      *print_parameters = true;
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      // handled properly after api init
      ++i;
    } else if (*image == nullptr) {
      *image = argv[i];
    } else {
      // Unexpected argument.
      fprintf(stderr, "Error, unknown command line argument '%s'\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  *arg_i = i;

  if (*pagesegmode == tesseract::PSM_OSD_ONLY) {
    // OSD = orientation and script detection.
    if (*lang != nullptr && strcmp(*lang, "osd")) {
      // If the user explicitly specifies a language (other than osd)
      // or a script, only orientation can be detected.
      fprintf(stderr, "Warning, detects only orientation with -l %s\n", *lang);
    } else {
      // That mode requires osd.traineddata to detect orientation and script.
      *lang = "osd";
    }
  }

  if (*outputbase == nullptr && noocr == false) {
    PrintHelpMessage(argv[0]);
    exit(EXIT_FAILURE);
  }
}

static void PreloadRenderers(
    tesseract::TessBaseAPI* api,
    tesseract::PointerVector<tesseract::TessResultRenderer>* renderers,
    tesseract::PageSegMode pagesegmode, const char* outputbase) {
  if (pagesegmode == tesseract::PSM_OSD_ONLY) {
#ifndef DISABLED_LEGACY_ENGINE
    renderers->push_back(new tesseract::TessOsdRenderer(outputbase));
#endif  // ndef DISABLED_LEGACY_ENGINE
  } else {
    bool b;
    api->GetBoolVariable("tessedit_create_hocr", &b);
    if (b) {
      bool font_info;
      api->GetBoolVariable("hocr_font_info", &font_info);
      tesseract::TessHOcrRenderer* renderer =
          new tesseract::TessHOcrRenderer(outputbase, font_info);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create hOCR output file: %s\n",
                strerror(errno));
      }
    }

    api->GetBoolVariable("tessedit_create_tsv", &b);
    if (b) {
      bool font_info;
      api->GetBoolVariable("hocr_font_info", &font_info);
      tesseract::TessTsvRenderer* renderer =
          new tesseract::TessTsvRenderer(outputbase, font_info);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create TSV output file: %s\n",
                strerror(errno));
      }
    }

    api->GetBoolVariable("tessedit_create_pdf", &b);
    if (b) {
      #ifdef WIN32
        if (_setmode(_fileno(stdout), _O_BINARY) == -1)
          tprintf("ERROR: cin to binary: %s", strerror(errno));
      #endif  // WIN32
      bool textonly;
      api->GetBoolVariable("textonly_pdf", &textonly);
      tesseract::TessPDFRenderer* renderer =
        new tesseract::TessPDFRenderer(outputbase, api->GetDatapath(),
                                       textonly);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create PDF output file: %s\n",
                strerror(errno));
      }
    }

    api->GetBoolVariable("tessedit_write_unlv", &b);
    if (b) {
      api->SetVariable("unlv_tilde_crunching", "true");
      tesseract::TessUnlvRenderer* renderer =
        new tesseract::TessUnlvRenderer(outputbase);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create UNLV output file: %s\n",
                strerror(errno));
      }
    }

    api->GetBoolVariable("tessedit_create_boxfile", &b);
    if (b) {
      tesseract::TessBoxTextRenderer* renderer =
        new tesseract::TessBoxTextRenderer(outputbase);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create BOX output file: %s\n",
                strerror(errno));
      }
    }

    api->GetBoolVariable("tessedit_create_txt", &b);
    if (b || renderers->empty()) {
      tesseract::TessTextRenderer* renderer =
        new tesseract::TessTextRenderer(outputbase);
      if (renderer->happy()) {
        renderers->push_back(renderer);
      } else {
        delete renderer;
        tprintf("Error, could not create TXT output file: %s\n",
                strerror(errno));
      }
    }
  }

  if (!renderers->empty()) {
    // Since the PointerVector auto-deletes, null-out the renderers that are
    // added to the root, and leave the root in the vector.
    for (int r = 1; r < renderers->size(); ++r) {
      (*renderers)[0]->insert((*renderers)[r]);
      (*renderers)[r] = nullptr;
    }
  }
}


/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char** argv) {
  const char* lang = nullptr;
  const char* image = nullptr;
  const char* outputbase = nullptr;
  const char* datapath = nullptr;
  bool list_langs = false;
  bool print_parameters = false;
  l_int32 dpi = 0;
  int arg_i = 1;
  tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
#ifdef DISABLED_LEGACY_ENGINE
  auto enginemode = tesseract::OEM_LSTM_ONLY;
#else
  tesseract::OcrEngineMode enginemode = tesseract::OEM_DEFAULT;
#endif
  /* main() calls functions like ParseArgs which call exit().
   * This results in memory leaks if vars_vec and vars_values are
   * declared as auto variables (destructor is not called then). */
  static GenericVector<STRING> vars_vec;
  static GenericVector<STRING> vars_values;

#if !defined(DEBUG)
  // Disable debugging and informational messages from Leptonica.
  setMsgSeverity(L_SEVERITY_ERROR);
#endif

#if defined(HAVE_TIFFIO_H) && defined(_WIN32)
  /* Show libtiff errors and warnings on console (not in GUI). */
  TIFFSetErrorHandler(Win32ErrorHandler);
  TIFFSetWarningHandler(Win32WarningHandler);
#endif // HAVE_TIFFIO_H && _WIN32

  ParseArgs(argc, argv, &lang, &image, &outputbase, &datapath, &dpi,
            &list_langs, &print_parameters, &vars_vec, &vars_values, &arg_i,
            &pagesegmode, &enginemode);

  if (lang == nullptr) {
    // Set default language if none was given.
    lang = "eng";
  }

  if (image == nullptr && !list_langs && !print_parameters)
    return EXIT_SUCCESS;

  PERF_COUNT_START("Tesseract:main")

  // Call GlobalDawgCache here to create the global DawgCache object before
  // the TessBaseAPI object. This fixes the order of destructor calls:
  // first TessBaseAPI must be destructed, DawgCache must be the last object.
  tesseract::Dict::GlobalDawgCache();

  // Avoid memory leak caused by auto variable when return is called.
  static tesseract::TessBaseAPI api;

  api.SetOutputName(outputbase);

  const int init_failed = api.Init(datapath, lang, enginemode, &(argv[arg_i]),
                             argc - arg_i, &vars_vec, &vars_values, false);

  SetVariablesFromCLArgs(&api, argc, argv);

  if (list_langs) {
    PrintLangsList(&api);
    return EXIT_SUCCESS;
  }

  if (init_failed) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    return EXIT_FAILURE;
  }

  if (print_parameters) {
    FILE* fout = stdout;
    fprintf(stdout, "Tesseract parameters:\n");
    api.PrintVariables(fout);
    api.End();
    return EXIT_SUCCESS;
  }

  FixPageSegMode(&api, pagesegmode);

  if (dpi) {
    char dpi_string[255];
    snprintf(dpi_string, 254, "%d", dpi);
    api.SetVariable("user_defined_dpi", dpi_string);
  }

  if (pagesegmode == tesseract::PSM_AUTO_ONLY) {
    int ret_val = EXIT_SUCCESS;

    Pix* pixs = pixRead(image);
    if (!pixs) {
      fprintf(stderr, "Leptonica can't process input file: %s\n", image);
      return 2;
    }

    api.SetImage(pixs);

    tesseract::Orientation orientation;
    tesseract::WritingDirection direction;
    tesseract::TextlineOrder order;
    float deskew_angle;

    const tesseract::PageIterator* it = api.AnalyseLayout();
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

#ifdef DISABLED_LEGACY_ENGINE
  auto cur_psm = api.GetPageSegMode();
  auto osd_warning = std::string("");
  if (cur_psm == tesseract::PSM_OSD_ONLY) {
    const char* disabled_osd_msg =
        "\nERROR: The page segmentation mode 0 (OSD Only) is currently disabled.\n\n";
    fprintf(stderr, "%s",  disabled_osd_msg);
    return EXIT_FAILURE;
  } else if (cur_psm == tesseract::PSM_AUTO_OSD) {
      api.SetPageSegMode(tesseract::PSM_AUTO);
      osd_warning +=
          "\nWarning: The page segmentation mode 1 (Auto+OSD) is currently disabled. "
          "Using PSM 3 (Auto) instead.\n\n";
  } else if (tesseract::PSM_SPARSE_TEXT_OSD) {
      api.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
      osd_warning +=
          "\nWarning: The page segmentation mode 12 (Sparse text + OSD) is currently disabled. "
          "Using PSM 11 (Sparse text) instead.\n\n";
  }
#endif  // def DISABLED_LEGACY_ENGINE

  // Avoid memory leak caused by auto variable when exit() is called.
  static tesseract::PointerVector<tesseract::TessResultRenderer> renderers;

  if (in_training_mode) {
    renderers.push_back(nullptr);
  } else {
    PreloadRenderers(&api, &renderers, pagesegmode, outputbase);
  }

  bool banner = false;
  if (outputbase != nullptr && strcmp(outputbase, "-") &&
      strcmp(outputbase, "stdout")) {
    banner = true;
  }

  if (!renderers.empty()) {
    if (banner) PrintBanner();
#ifdef DISABLED_LEGACY_ENGINE
    if (!osd_warning.empty()) {
      fprintf(stderr, "%s",osd_warning.c_str());
    }
#endif
    bool succeed = api.ProcessPages(image, nullptr, 0, renderers[0]);
    if (!succeed) {
      fprintf(stderr, "Error during processing.\n");
      return EXIT_FAILURE;
    }
  }

  PERF_COUNT_END

  return EXIT_SUCCESS;
}

