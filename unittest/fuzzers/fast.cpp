#include <tesseract/baseapi.h>
#include "leptonica/allheaders.h"

#include <libgen.h>     // for dirname
#include <cstdio>       // for printf
#include <cstdlib>      // for std::getenv, std::setenv
#include <string>       // for std::string

#ifndef TESSERACT_FUZZER_WIDTH
#define TESSERACT_FUZZER_WIDTH 100
#endif

#ifndef TESSERACT_FUZZER_HEIGHT
#define TESSERACT_FUZZER_HEIGHT 100
#endif

static unsigned tesseractFuzzerHeight = TESSERACT_FUZZER_HEIGHT;
static unsigned tesseractFuzzerWidth = TESSERACT_FUZZER_WIDTH;

static tesseract::TessBaseAPI* api = nullptr;

extern "C" int LLVMFuzzerInitialize(int* /*pArgc*/, char*** pArgv) {
  if (std::getenv("TESSDATA_PREFIX") == nullptr) {
    std::string binary_path = *pArgv[0];
    const std::string filepath = dirname(&binary_path[0]);

    const std::string tessdata_path = filepath + "/" + "tessdata";
    if (setenv("TESSDATA_PREFIX", tessdata_path.c_str(), 1) != 0) {
      printf("Setenv failed\n");
      std::abort();
    }
  }

  api = new tesseract::TessBaseAPI();
  if (api->Init(nullptr, "eng") != 0) {
    printf("Cannot initialize API\n");
    abort();
  }

  /* Silence output */
  api->SetVariable("debug_file", "/dev/null");

  // Get width and height from executable name.
  std::string argv0 = *pArgv[0];
  const std::string filename = basename(&argv0[0]);
  sscanf(filename.c_str(), "fuzzer-api-%ux%u",
                  &tesseractFuzzerWidth, &tesseractFuzzerHeight);
  printf("Fuzzing with image size %u x %u\n",
         tesseractFuzzerWidth, tesseractFuzzerHeight);

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  unsigned depth = 1;
  unsigned width = tesseractFuzzerWidth;
  if (size * 8 < width) {
    width = size / 8;
  }
  if (width == 0) {
    return 0;
  }
  unsigned wpl = (width * depth + 31) / 32;
  unsigned height = size / (4 * wpl); 
  if (height == 0) {
    return 0;
  }
  auto pix = pixCreateNoInit(width, height, depth);
  if (pix == nullptr) {
    printf("pix creation failed\n");
    abort();
  }
  memcpy(pixGetData(pix), data, 4 * wpl * height);

  api->SetImage(pix);

  char* outText = api->GetUTF8Text();

  pixDestroy(&pix);
  delete[] outText;

  return 0;
}

#ifdef NEEDS_MAIN
int main(int argc, char* argv[]) {
  LLVMFuzzerInitialize(&argc, &argv);
  if (argc == 2) {
    FILE* f = fopen(argv[1], "rb");
    if (f) {
      fseek(f, 0, SEEK_END);
      long size = ftell(f);
      fseek(f, 0, SEEK_SET);
      auto data = new uint8_t[size];
      fread(data, 1, size, f);
      fclose(f);
      LLVMFuzzerTestOneInput(data, size);
      delete[] data;
    }
  }
  return 0;
}
#endif
