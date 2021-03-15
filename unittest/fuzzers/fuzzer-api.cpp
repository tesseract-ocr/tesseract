#include <allheaders.h>
#include <tesseract/baseapi.h>

#include <libgen.h> // for dirname
#include <cstdio>   // for printf
#include <cstdlib>  // for std::getenv, std::setenv
#include <string>   // for std::string

#ifndef TESSERACT_FUZZER_WIDTH
#  define TESSERACT_FUZZER_WIDTH 100
#endif

#ifndef TESSERACT_FUZZER_HEIGHT
#  define TESSERACT_FUZZER_HEIGHT 100
#endif

class BitReader {
private:
  uint8_t const *data;
  size_t size;
  size_t shift;

public:
  BitReader(const uint8_t *data, size_t size) : data(data), size(size), shift(0) {}

  int Read(void) {
    if (size == 0) {
      return 0;
    }

    const int ret = ((*data) >> shift) & 1;

    shift++;
    if (shift >= 8) {
      shift = 0;
      data++;
      size--;
    }

    return ret;
  }
};

static tesseract::TessBaseAPI *api = nullptr;

extern "C" int LLVMFuzzerInitialize(int * /*pArgc*/, char ***pArgv) {
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

  return 0;
}

static PIX *createPix(BitReader &BR, const size_t width, const size_t height) {
  Pix *pix = pixCreate(width, height, 1);

  if (pix == nullptr) {
    printf("pix creation failed\n");
    abort();
  }

  for (size_t i = 0; i < width; i++) {
    for (size_t j = 0; j < height; j++) {
      pixSetPixel(pix, i, j, BR.Read());
    }
  }

  return pix;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  BitReader BR(data, size);

  auto pix = createPix(BR, TESSERACT_FUZZER_WIDTH, TESSERACT_FUZZER_HEIGHT);

  api->SetImage(pix);

  char *outText = api->GetUTF8Text();

  pixDestroy(&pix);
  delete[] outText;

  return 0;
}
