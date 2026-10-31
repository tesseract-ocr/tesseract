#include <allheaders.h>
#include <tesseract/baseapi.h>

#include <libgen.h>
#include <cstdio>
#include <cstdlib>
#include <string>

static const int kMaxDim = 4096;

static tesseract::TessBaseAPI *api = nullptr;

extern "C" int LLVMFuzzerInitialize(int *, char ***pArgv) {
  if (std::getenv("TESSDATA_PREFIX") == nullptr) {
    std::string binary_path = *pArgv[0];
    const std::string filepath = dirname(&binary_path[0]);

    const std::string tessdata_path = filepath + "/" + "tessdata";
    if (setenv("TESSDATA_PREFIX", tessdata_path.c_str(), 1) != 0) {
      printf("Setenv failed\n");
      std::abort();
    }
  }

  setMsgSeverity(L_SEVERITY_NONE);

  api = new tesseract::TessBaseAPI();
  if (api->Init(nullptr, "eng") != 0) {
    printf("Cannot initialize API\n");
    abort();
  }

  /* Silence output */
  api->SetVariable("debug_file", "/dev/null");

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0) {
    return 0;
  }

  Pix *pix = pixReadMem(data, size);
  if (pix == nullptr) {
    return 0;
  }

  const int w = pixGetWidth(pix);
  const int h = pixGetHeight(pix);
  if (w <= 0 || h <= 0 || w > kMaxDim || h > kMaxDim) {
    pixDestroy(&pix);
    return 0;
  }

  api->SetImage(pix);

  char *outText = api->GetUTF8Text();

  delete[] outText;
  api->Clear();
  pixDestroy(&pix);

  return 0;
}
