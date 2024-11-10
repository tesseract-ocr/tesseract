/**********************************************************************
 * File:        tesseract.cpp
 * Description: Main program for merge of tess and editor.
 * Author:      Ray Smith
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
#  include "config_auto.h"
#endif

#include <cerrno> // for errno
#if defined(__USE_GNU)
#  include <cfenv> // for feenableexcept
#endif
#include <climits> // for INT_MIN, INT_MAX
#include <cstdlib> // for std::getenv
#include <iostream>
#include <map>    // for std::map
#include <memory> // std::unique_ptr

#include <allheaders.h>
#include <tesseract/baseapi.h>
#include "dict.h"
#include <tesseract/renderer.h>
#include "simddetect.h"
#include "tesseractclass.h" // for AnyTessLang
#include "tprintf.h" // for tprintf

#ifdef _OPENMP
#  include <omp.h>
#endif

#if defined(HAVE_LIBARCHIVE)
#  include <archive.h>
#endif
#if defined(HAVE_LIBCURL)
#  include <curl/curl.h>
#endif

#if defined(_WIN32)
#  include <fcntl.h>
#  include <io.h>
#  if defined(HAVE_TIFFIO_H)

#    include <tiffio.h>

static void Win32ErrorHandler(const char *module, const char *fmt, va_list ap) {
  if (module != nullptr) {
    fprintf(stderr, "%s: ", module);
  }
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}

static void Win32WarningHandler(const char *module, const char *fmt, va_list ap) {
  if (module != nullptr) {
    fprintf(stderr, "%s: ", module);
  }
  fprintf(stderr, "Warning, ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, ".\n");
}

#  endif /* HAVE_TIFFIO_H */

class AutoWin32ConsoleOutputCP {
public:
  explicit AutoWin32ConsoleOutputCP(UINT codeCP) :
    oldCP_(GetConsoleOutputCP()) {
    SetConsoleOutputCP(codeCP);
  }
  ~AutoWin32ConsoleOutputCP() {
    SetConsoleOutputCP(oldCP_);
  }

private:
  UINT oldCP_;
};

static AutoWin32ConsoleOutputCP autoWin32ConsoleOutputCP(CP_UTF8);

#endif // _WIN32

using namespace tesseract;

static void PrintVersionInfo() {
  char *versionStrP;

  printf("tesseract %s\n", tesseract::TessBaseAPI::Version());

  versionStrP = getLeptonicaVersion();
  printf(" %s\n", versionStrP);
  lept_free(versionStrP);

  versionStrP = getImagelibVersions();
  printf("  %s\n", versionStrP);
  lept_free(versionStrP);

#if defined(HAVE_NEON) || defined(__aarch64__)
  if (tesseract::SIMDDetect::IsNEONAvailable())
    printf(" Found NEON\n");
#elif defined(HAVE_RVV)
  if (tesseract::SIMDDetect::IsRVVAvailable())
    printf(" Found RVV\n");
#else
  if (tesseract::SIMDDetect::IsAVX512BWAvailable()) {
    printf(" Found AVX512BW\n");
  }
  if (tesseract::SIMDDetect::IsAVX512FAvailable()) {
    printf(" Found AVX512F\n");
  }
  if (tesseract::SIMDDetect::IsAVX512VNNIAvailable()) {
    printf(" Found AVX512VNNI\n");
  }
  if (tesseract::SIMDDetect::IsAVX2Available()) {
    printf(" Found AVX2\n");
  }
  if (tesseract::SIMDDetect::IsAVXAvailable()) {
    printf(" Found AVX\n");
  }
  if (tesseract::SIMDDetect::IsFMAAvailable()) {
    printf(" Found FMA\n");
  }
  if (tesseract::SIMDDetect::IsSSEAvailable()) {
    printf(" Found SSE4.1\n");
  }
#endif
#ifdef _OPENMP
  printf(" Found OpenMP %d\n", _OPENMP);
#endif
#if defined(HAVE_LIBARCHIVE)
#  if ARCHIVE_VERSION_NUMBER >= 3002000
  printf(" Found %s\n", archive_version_details());
#  else
  printf(" Found %s\n", archive_version_string());
#  endif // ARCHIVE_VERSION_NUMBER
#endif   // HAVE_LIBARCHIVE
#if defined(HAVE_LIBCURL)
  printf(" Found %s\n", curl_version());
#endif
}

static void PrintHelpForPSM() {
  printf(
      "Page segmentation modes (PSM):\n"
      "  0|osd_only                Orientation and script detection (OSD) only.\n"
      "  1|auto_osd                Automatic page segmentation with OSD.\n"
      "  2|auto_only               Automatic page segmentation, but no OSD, or OCR. (not "
      "implemented)\n"
      "  3|auto                    Fully automatic page segmentation, but no OSD. (Default)\n"
      "  4|single_column           Assume a single column of text of variable sizes.\n"
      "  5|single_block_vert_text  Assume a single uniform block of vertically aligned text.\n"
      "  6|single_block            Assume a single uniform block of text.\n"
      "  7|single_line             Treat the image as a single text line.\n"
      "  8|single_word             Treat the image as a single word.\n"
      "  9|circle_word             Treat the image as a single word in a circle.\n"
      " 10|single_char             Treat the image as a single character.\n"
      " 11|sparse_text             Sparse text. Find as much text as possible in no"
      " particular order.\n"
      " 12|sparse_text_osd         Sparse text with OSD.\n"
      " 13|raw_line                Raw line. Treat the image as a single text line,\n"
      "                            bypassing hacks that are Tesseract-specific.\n"
  );

#ifdef DISABLED_LEGACY_ENGINE
  printf("\nNOTE: The OSD modes are currently disabled.\n");
#endif
}

#ifndef DISABLED_LEGACY_ENGINE
static void PrintHelpForOEM() {
  printf(
      "OCR Engine modes (OEM):\n"
      "  0|tesseract_only          Legacy engine only.\n"
      "  1|lstm_only               Neural nets LSTM engine only.\n"
      "  2|tesseract_lstm_combined Legacy + LSTM engines.\n"
      "  3|default                 Default, based on what is available.\n"
  );
}
#endif // ndef DISABLED_LEGACY_ENGINE

static void PrintHelpExtra(const char *program) {
  printf(
      "Usage:\n"
      "  %s --help | --help-extra | --help-psm | "
#ifndef DISABLED_LEGACY_ENGINE
      "--help-oem | "
#endif
      "--version\n"
      "  %s --list-langs [--tessdata-dir PATH]\n"
#ifndef DISABLED_LEGACY_ENGINE
      "  %s --print-fonts-table [options...] [configfile...]\n"
#endif  // ndef DISABLED_LEGACY_ENGINE
      "  %s --print-parameters [options...] [configfile...]\n"
      "  %s imagename|imagelist|stdin outputbase|stdout [options...] "
      "[configfile...]\n"
      "\n"
      "OCR options:\n"
      "  --tessdata-dir PATH   Specify the location of tessdata path.\n"
      "  --user-words PATH     Specify the location of user words file.\n"
      "  --user-patterns PATH  Specify the location of user patterns file.\n"
      "  --dpi VALUE           Specify DPI for input image.\n"
      "  --loglevel LEVEL      Specify logging level. LEVEL can be\n"
      "                        ALL, TRACE, DEBUG, INFO, WARN, ERROR, FATAL or OFF.\n"
      "  -l LANG[+LANG]        Specify language(s) used for OCR.\n"
      "  -c VAR=VALUE          Set value for config variables.\n"
      "                        Multiple -c arguments are allowed.\n"
      "  --psm PSM|NUM         Specify page segmentation mode.\n"
#ifndef DISABLED_LEGACY_ENGINE
      "  --oem OEM|NUM         Specify OCR Engine mode.\n"
#endif
      "NOTE: These options must occur before any configfile.\n"
      "\n",
      program, program, program, program
#ifndef DISABLED_LEGACY_ENGINE
      , program
#endif  // ndef DISABLED_LEGACY_ENGINE
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
#ifndef DISABLED_LEGACY_ENGINE
      "  --print-fonts-table   Print tesseract fonts table.\n"
#endif  // ndef DISABLED_LEGACY_ENGINE
      "  --print-parameters    Print tesseract parameters.\n");
}

static void PrintHelpMessage(const char *program) {
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
      "  --list-langs          List available languages for tesseract "
      "engine.\n",
      program, program, program);
}

static bool SetVariablesFromCLArgs(tesseract::TessBaseAPI &api, int argc, char **argv) {
  bool success = true;
  char opt1[256], opt2[255];
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      strncpy(opt1, argv[i + 1], 255);
      opt1[255] = '\0';
      char *p = strchr(opt1, '=');
      if (!p) {
        fprintf(stderr, "Missing = in configvar assignment\n");
        success = false;
        break;
      }
      *p = 0;
      strncpy(opt2, strchr(argv[i + 1], '=') + 1, sizeof(opt2) - 1);
      opt2[254] = 0;
      ++i;

      if (!api.SetVariable(opt1, opt2)) {
        fprintf(stderr, "Could not set option: %s=%s\n", opt1, opt2);
      }
    }
  }
  return success;
}

static void PrintLangsList(tesseract::TessBaseAPI &api) {
  std::vector<std::string> languages;
  api.GetAvailableLanguagesAsVector(&languages);
  printf("List of available languages in \"%s\" (%zu):\n",
         api.GetDatapath(), languages.size());
  for (const auto &language : languages) {
    printf("%s\n", language.c_str());
  }
  api.End();
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
static void FixPageSegMode(tesseract::TessBaseAPI &api, tesseract::PageSegMode pagesegmode) {
  if (api.GetPageSegMode() == tesseract::PSM_SINGLE_BLOCK) {
    api.SetPageSegMode(pagesegmode);
  }
}

static bool checkArgValues(int arg, const char *mode, int count) {
  if (arg >= count || arg < 0) {
    printf("Invalid %s value, please enter a symbolic %s value or a number between 0-%d\n", mode, mode, count - 1);
    return false;
  }
  return true;
}

// Convert a symbolic or numeric string to an OEM value.
static int stringToOEM(const std::string arg) {
  std::map<std::string, int> oem_map = {
    {"0", 0},
    {"1", 1},
    {"2", 2},
    {"3", 3},
    {"tesseract_only", 0},
    {"lstm_only", 1},
    {"tesseract_lstm_combined", 2},
    {"default", 3},
  };
  auto it = oem_map.find(arg);
  return it == oem_map.end() ? -1 : it->second;
}

static int stringToPSM(const std::string arg) {
  std::map<std::string, int> psm_map = {
    {"0", 0},
    {"1", 1},
    {"2", 2},
    {"3", 3},
    {"4", 4},
    {"5", 5},
    {"6", 6},
    {"7", 7},
    {"8", 8},
    {"9", 9},
    {"10", 10},
    {"11", 11},
    {"12", 12},
    {"13", 13},
    {"osd_only", 0},
    {"auto_osd", 1},
    {"auto_only", 2},
    {"auto", 3},
    {"single_column", 4},
    {"single_block_vert_text", 5},
    {"single_block", 6},
    {"single_line", 7},
    {"single_word", 8},
    {"circle_word", 9},
    {"single_char", 10},
    {"sparse_text", 11},
    {"sparse_text_osd", 12},
    {"raw_line", 13},
  };
  auto it = psm_map.find(arg);
  return it == psm_map.end() ? -1 : it->second;
}

// NOTE: arg_i is used here to avoid ugly *i so many times in this function
static bool ParseArgs(int argc, char **argv, const char **lang, const char **image,
                      const char **outputbase, const char **datapath, l_int32 *dpi,
                      bool *list_langs, bool *print_parameters, bool *print_fonts_table,
                      std::vector<std::string> *vars_vec, std::vector<std::string> *vars_values,
                      l_int32 *arg_i, tesseract::PageSegMode *pagesegmode,
                      tesseract::OcrEngineMode *enginemode) {
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
    } else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0)) {
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
    } else if (strcmp(argv[i], "--loglevel") == 0 && i + 1 < argc) {
      // Allow the log levels which are used by log4cxx.
      const std::string loglevel_string = argv[++i];
      static const std::map<const std::string, int> loglevels {
        {"ALL", INT_MIN},
        {"TRACE", 5000},
        {"DEBUG", 10000},
        {"INFO", 20000},
        {"WARN", 30000},
        {"ERROR", 40000},
        {"FATAL", 50000},
        {"OFF", INT_MAX},
      };
      try {
        auto loglevel = loglevels.at(loglevel_string);
        log_level = loglevel;
      } catch (const std::out_of_range &e) {
        // TODO: Allow numeric argument?
        tprintf("Error, unsupported --loglevel %s\n", loglevel_string.c_str());
        return false;
      }
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
      int psm = stringToPSM(argv[i + 1]);
      if (!checkArgValues(psm, "PSM", tesseract::PSM_COUNT)) {
        return false;
      }
      *pagesegmode = static_cast<tesseract::PageSegMode>(psm);
      ++i;
    } else if (strcmp(argv[i], "--oem") == 0 && i + 1 < argc) {
#ifndef DISABLED_LEGACY_ENGINE
      int oem = stringToOEM(argv[i + 1]);
      if (!checkArgValues(oem, "OEM", tesseract::OEM_COUNT)) {
        return false;
      }
      *enginemode = static_cast<tesseract::OcrEngineMode>(oem);
#endif
      ++i;
    } else if (strcmp(argv[i], "--print-parameters") == 0) {
      noocr = true;
      *print_parameters = true;
#ifndef DISABLED_LEGACY_ENGINE
    } else if (strcmp(argv[i], "--print-fonts-table") == 0) {
      noocr = true;
      *print_fonts_table = true;
#endif  // ndef DISABLED_LEGACY_ENGINE
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      // handled properly after api init
      ++i;
    } else if (*image == nullptr) {
      *image = argv[i];
    } else {
      // Unexpected argument.
      fprintf(stderr, "Error, unknown command line argument '%s'\n", argv[i]);
      return false;
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
    return false;
  }

  return true;
}

static void PreloadRenderers(tesseract::TessBaseAPI &api,
                             std::vector<std::unique_ptr<TessResultRenderer>> &renderers,
                             tesseract::PageSegMode pagesegmode, const char *outputbase) {
  if (pagesegmode == tesseract::PSM_OSD_ONLY) {
#ifndef DISABLED_LEGACY_ENGINE
    renderers.push_back(std::make_unique<tesseract::TessOsdRenderer>(outputbase));
#endif // ndef DISABLED_LEGACY_ENGINE
  } else {
    bool error = false;
    bool b;
    api.GetBoolVariable("tessedit_create_hocr", &b);
    if (b) {
      bool font_info;
      api.GetBoolVariable("hocr_font_info", &font_info);
      auto renderer = std::make_unique<tesseract::TessHOcrRenderer>(outputbase, font_info);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create hOCR output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_alto", &b);
    if (b) {
      auto renderer = std::make_unique<tesseract::TessAltoRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create ALTO output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_page_xml", &b);
    if (b) {
      auto renderer = std::make_unique<tesseract::TessPAGERenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create PAGE output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_tsv", &b);
    if (b) {
      bool font_info;
      api.GetBoolVariable("hocr_font_info", &font_info);
      auto renderer = std::make_unique<tesseract::TessTsvRenderer>(outputbase, font_info);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create TSV output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_pdf", &b);
    if (b) {
#ifdef WIN32
      if (_setmode(_fileno(stdout), _O_BINARY) == -1)
        tprintf("ERROR: cin to binary: %s", strerror(errno));
#endif // WIN32
      bool textonly;
      api.GetBoolVariable("textonly_pdf", &textonly);
      auto renderer = std::make_unique<tesseract::TessPDFRenderer>(outputbase, api.GetDatapath(), textonly);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create PDF output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_write_unlv", &b);
    if (b) {
      api.SetVariable("unlv_tilde_crunching", "true");
      auto renderer = std::make_unique<tesseract::TessUnlvRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create UNLV output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_lstmbox", &b);
    if (b) {
      auto renderer = std::make_unique<tesseract::TessLSTMBoxRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create LSTM BOX output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_boxfile", &b);
    if (b) {
      auto renderer = std::make_unique<tesseract::TessBoxTextRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create BOX output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_wordstrbox", &b);
    if (b) {
      auto renderer = std::make_unique<tesseract::TessWordStrBoxRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create WordStr BOX output file: %s\n", strerror(errno));
        error = true;
      }
    }

    api.GetBoolVariable("tessedit_create_txt", &b);
    if (b || (!error && renderers.empty())) {
      // Create text output if no other output was requested
      // even if text output was not explicitly requested unless
      // there was an error.
      auto renderer = std::make_unique<tesseract::TessTextRenderer>(outputbase);
      if (renderer->happy()) {
        renderers.push_back(std::move(renderer));
      } else {
        tprintf("Error, could not create TXT output file: %s\n", strerror(errno));
      }
    }
  }

  // Null-out the renderers that are
  // added to the root, and leave the root in the vector.
  for (size_t r = 1; r < renderers.size(); ++r) {
    renderers[0]->insert(renderers[r].get());
    renderers[r].release(); // at the moment insert() is owning
  }
}

/**********************************************************************
 *  main()
 *
 **********************************************************************/

int main(int argc, char **argv) {
#if defined(__USE_GNU) && defined(HAVE_FEENABLEEXCEPT)
  // Raise SIGFPE.
#  if defined(__clang__)
  // clang creates code which causes some FP exceptions, so don't enable those.
  feenableexcept(FE_DIVBYZERO);
#  else
  feenableexcept(FE_DIVBYZERO | FE_OVERFLOW | FE_INVALID);
#  endif
#endif
  const char *lang = nullptr;
  const char *image = nullptr;
  const char *outputbase = nullptr;
  const char *datapath = nullptr;
  bool list_langs = false;
  bool print_parameters = false;
  bool print_fonts_table = false;
  l_int32 dpi = 0;
  int arg_i = 1;
  tesseract::PageSegMode pagesegmode = tesseract::PSM_AUTO;
#ifdef DISABLED_LEGACY_ENGINE
  auto enginemode = tesseract::OEM_LSTM_ONLY;
#else
  tesseract::OcrEngineMode enginemode = tesseract::OEM_DEFAULT;
#endif
  std::vector<std::string> vars_vec;
  std::vector<std::string> vars_values;

  if (std::getenv("LEPT_MSG_SEVERITY")) {
    // Get Leptonica message level from environment variable.
    setMsgSeverity(L_SEVERITY_EXTERNAL);
  } else {
    // Disable debugging and informational messages from Leptonica.
    setMsgSeverity(L_SEVERITY_ERROR);
  }

#if defined(HAVE_TIFFIO_H) && defined(_WIN32)
  /* Show libtiff errors and warnings on console (not in GUI). */
  TIFFSetErrorHandler(Win32ErrorHandler);
  TIFFSetWarningHandler(Win32WarningHandler);
#endif // HAVE_TIFFIO_H && _WIN32

  if (!ParseArgs(argc, argv, &lang, &image, &outputbase, &datapath, &dpi, &list_langs,
                 &print_parameters, &print_fonts_table, &vars_vec, &vars_values, &arg_i,
                 &pagesegmode, &enginemode)) {
    return EXIT_FAILURE;
  }

  bool in_recognition_mode = !list_langs && !print_parameters && !print_fonts_table;

  if (lang == nullptr && in_recognition_mode) {
    // Set default language model if none was given and a model file is needed.
    lang = "eng";
  }

  if (image == nullptr && in_recognition_mode) {
    return EXIT_SUCCESS;
  }

  // Call GlobalDawgCache here to create the global DawgCache object before
  // the TessBaseAPI object. This fixes the order of destructor calls:
  // first TessBaseAPI must be destructed, DawgCache must be the last object.
  tesseract::Dict::GlobalDawgCache();

  TessBaseAPI api;

  api.SetOutputName(outputbase);

  const int init_failed = api.Init(datapath, lang, enginemode, &(argv[arg_i]), argc - arg_i,
                                   &vars_vec, &vars_values, false);

  if (!SetVariablesFromCLArgs(api, argc, argv)) {
    return EXIT_FAILURE;
  }

  // SIMD settings might be overridden by config variable.
  tesseract::SIMDDetect::Update();

  if (list_langs) {
    PrintLangsList(api);
    return EXIT_SUCCESS;
  }

  if (init_failed) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    return EXIT_FAILURE;
  }

  if (print_parameters) {
    FILE *fout = stdout;
    fprintf(stdout, "Tesseract parameters:\n");
    api.PrintVariables(fout);
    api.End();
    return EXIT_SUCCESS;
  }

#ifndef DISABLED_LEGACY_ENGINE
  if (print_fonts_table) {
    FILE *fout = stdout;
    fprintf(stdout, "Tesseract fonts table:\n");
    api.PrintFontsTable(fout);
    api.End();
    return EXIT_SUCCESS;
  }
#endif  // ndef DISABLED_LEGACY_ENGINE

  FixPageSegMode(api, pagesegmode);

  if (dpi) {
    auto dpi_string = std::to_string(dpi);
    api.SetVariable("user_defined_dpi", dpi_string.c_str());
  }

  int ret_val = EXIT_SUCCESS;

  if (pagesegmode == tesseract::PSM_AUTO_ONLY) {
    Pix *pixs = pixRead(image);
    if (!pixs) {
      fprintf(stderr, "Leptonica can't process input file: %s\n", image);
      return 2;
    }

    api.SetImage(pixs);

    tesseract::Orientation orientation;
    tesseract::WritingDirection direction;
    tesseract::TextlineOrder order;
    float deskew_angle;

    const std::unique_ptr<const tesseract::PageIterator> it(api.AnalyseLayout());
    if (it) {
      // TODO: Implement output of page segmentation, see documentation
      // ("Automatic page segmentation, but no OSD, or OCR").
      it->Orientation(&orientation, &direction, &order, &deskew_angle);
      tprintf(
          "Orientation: %d\nWritingDirection: %d\nTextlineOrder: %d\n"
          "Deskew angle: %.4f\n",
          orientation, direction, order, deskew_angle);
    } else {
      ret_val = EXIT_FAILURE;
    }

    pixDestroy(&pixs);
    return ret_val;
  }

  // Set in_training_mode to true when using one of these configs:
  // ambigs.train, box.train, box.train.stderr, linebox, rebox, lstm.train.
  // In this mode no other OCR result files are written.
  bool b = false;
  bool in_training_mode = (api.GetBoolVariable("tessedit_ambigs_training", &b) && b) ||
                          (api.GetBoolVariable("tessedit_resegment_from_boxes", &b) && b) ||
                          (api.GetBoolVariable("tessedit_make_boxes_from_boxes", &b) && b) ||
                          (api.GetBoolVariable("tessedit_train_line_recognizer", &b) && b);

  if (api.GetPageSegMode() == tesseract::PSM_OSD_ONLY) {
    if (!api.tesseract()->AnyTessLang()) {
      fprintf(stderr, "Error, OSD requires a model for the legacy engine\n");
      return EXIT_FAILURE;
    }
  }
#ifdef DISABLED_LEGACY_ENGINE
  auto cur_psm = api.GetPageSegMode();
  auto osd_warning = std::string("");
  if (cur_psm == tesseract::PSM_OSD_ONLY) {
    const char *disabled_osd_msg =
        "\nERROR: The page segmentation mode 0 (OSD Only) is currently "
        "disabled.\n\n";
    fprintf(stderr, "%s", disabled_osd_msg);
    return EXIT_FAILURE;
  } else if (cur_psm == tesseract::PSM_AUTO_OSD) {
    api.SetPageSegMode(tesseract::PSM_AUTO);
    osd_warning +=
        "\nWarning: The page segmentation mode 1 (Auto+OSD) is currently "
        "disabled. "
        "Using PSM 3 (Auto) instead.\n\n";
  } else if (cur_psm == tesseract::PSM_SPARSE_TEXT_OSD) {
    api.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);
    osd_warning +=
        "\nWarning: The page segmentation mode 12 (Sparse text + OSD) is "
        "currently disabled. "
        "Using PSM 11 (Sparse text) instead.\n\n";
  }
#endif // def DISABLED_LEGACY_ENGINE

  std::vector<std::unique_ptr<TessResultRenderer>> renderers;

  if (in_training_mode) {
    renderers.push_back(nullptr);
  } else if (outputbase != nullptr) {
    PreloadRenderers(api, renderers, pagesegmode, outputbase);
  }

  if (!renderers.empty()) {
#ifdef DISABLED_LEGACY_ENGINE
    if (!osd_warning.empty()) {
      fprintf(stderr, "%s", osd_warning.c_str());
    }
#endif
    bool succeed = api.ProcessPages(image, nullptr, 0, renderers[0].get());
    if (!succeed) {
      fprintf(stderr, "Error during processing.\n");
      ret_val = EXIT_FAILURE;
    }
  }

  return ret_val;
}
