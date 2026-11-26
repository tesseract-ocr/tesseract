#include <allheaders.h>
#include <tesseract/baseapi.h>

#include <libgen.h>
#include <cstdint>  // for uint8_t
#include <cstdio>
#include <cstdlib>
#include <string>

static const int kMaxDim = 2048;
// The area is what bounds the run time: the number of pixels drives the work
// of both engines, and compressed inputs can expand to a large raster.  The
// OSS-Fuzz build uses ASan and -D_GLIBCXX_DEBUG with the combined legacy +
// LSTM engine, which is slow enough that 16 MP images do not fit the time
// limit.
static const int64_t kMaxArea = 1024 * 1024;

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

  /* Bound the run time for images which give a huge number of blobs (OSS-Fuzz
     issue 581720522). This is not the default, because it would also truncate
     the text of normal pages. The parameter does not exist in builds without
     the legacy engine, so the result is ignored. */
  api->SetVariable("chop_max_classifications_pass", "5000");

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0) {
    return 0;
  }

  // Read the header before decoding: a small input can declare large
  // dimensions, and pixReadMem() would decode the whole raster before the
  // harness could reject it, costing time and memory.
  l_int32 format;
  l_int32 w;
  l_int32 h;
  l_int32 bps;
  l_int32 spp;
  Pix *header = pixReadHeaderMem(data, size, &format, &w, &h, &bps, &spp);
  if (header == nullptr) {
    return 0;
  }
  pixDestroy(&header);

  // bps * spp > 32 cannot be a raster which Leptonica decodes; with the area
  // bound it also caps the size of the raster which pixReadMem() allocates.
  if (w <= 0 || h <= 0 || bps <= 0 || spp <= 0 || bps * spp > 32 || w > kMaxDim || h > kMaxDim ||
      static_cast<int64_t>(w) * h > kMaxArea) {
    return 0;
  }

  Pix *pix = pixReadMem(data, size);
  if (pix == nullptr) {
    return 0;
  }

  // The decoded image can differ from what its header declared; recheck the
  // bounds before handing it to the API.
  w = pixGetWidth(pix);
  h = pixGetHeight(pix);
  if (w <= 0 || h <= 0 || w > kMaxDim || h > kMaxDim || static_cast<int64_t>(w) * h > kMaxArea) {
    pixDestroy(&pix);
    return 0;
  }

  api->SetImage(pix);

  char *outText = api->GetUTF8Text();

  delete[] outText;
  api->Clear();
  // Clear() keeps what the adaptive classifier of the legacy engine learned
  // from earlier inputs, so a crash could depend on preceding inputs and not
  // reproduce from a single saved test case; reset the classifier as well.
  api->ClearAdaptiveClassifier();
  pixDestroy(&pix);

  return 0;
}

// Release the API on exit (called by libFuzzer). Without this, the API's
// legacy dictionary keeps references to the DAWGs of the static global DAWG
// cache (Dict::GlobalDawgCache()), which still hold count > 0 when that cache
// is destroyed during static teardown, printing spurious
// "ObjectCache: WARNING! LEAK!" lines. Deleting the API runs the normal
// ~Tesseract() -> Dict::End() path, which releases the references.
extern "C" void LLVMFuzzerDone() {
  delete api;
  api = nullptr;
}
