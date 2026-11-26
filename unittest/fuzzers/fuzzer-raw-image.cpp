///////////////////////////////////////////////////////////////////////
// File:        fuzzer-raw-image.cpp
// Description: Fuzzer for layout analysis, thresholding and recognition
//              of raw images with a text header.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

// The fuzz input is a text line with parameters, followed by uncompressed
// pixel data. No image file format is decoded, so the fuzzing time is spent
// in Tesseract and not in the image decoders. Example:
//
//   psm=3 depth=8 width=448 height=144 resolution=300\n<pixel data>
//
// The line ends with the first '\n' and has at most kMaxHeaderSize bytes.
// It has any number of key=value pairs in any order, separated by spaces.
// The values are decimal numbers:
//
//   key                  values               default
//   psm                  3, 4, 5, 6, 7, 10,   6    page segmentation mode,
//                        11                        see IsUsablePageSegMode
//   depth                1, 8, 32             1    bits per pixel
//   thresholding_method  0..2                 0    Tesseract parameter with
//                                                  that name
//   width                1..kMaxSide          128  image width in pixels
//   height               1..kMaxSide          128  image height in pixels
//   resolution           0..kMaxResolution    0    image resolution in ppi,
//                                                  0 = unknown
//
// The pixel data uses the layout of the binary PBM, PGM and PPM formats:
// rows from top to bottom, pixels from left to right, and
//
//   depth=1   1 bit per pixel, most significant bit first, 1 = black,
//             each row starts with a new byte
//   depth=8   1 byte per pixel, 0 = black
//   depth=32  3 bytes per pixel (red, green, blue)
//
// Missing pixels are white, so a short input gives an image with white rows
// at the bottom. Pixel data which is not needed is ignored.
//
// Rules for changes of this format. The corpus of OSS-Fuzz and the test cases
// of its bug reports are inputs in this format, and they must keep their
// meaning:
//
// - An input which is not accepted is not run. That is the case for a missing
//   '\n', an unknown key, a key which is used twice, a value which is not
//   listed above, an image with more than kMaxPixels pixels, and an image
//   which is more than kMaxAspect times as wide as it is high. No input
//   which is in use can depend on something that is not accepted today, so
//   new keys, new values and higher limits can be added at any time.
// - A new key needs a default which gives the behaviour without the key.
// - Never change the meaning or the default of an existing key or value, and
//   never change the layout of the pixel data for an existing depth.
//
// Mutations of single bytes nearly always give a header which is not
// accepted. Therefore libFuzzer gets a custom mutator which either sets one
// of the parameters to another valid value or mutates the pixel data. The
// other fuzzing engines do not use it. They depend on the seed corpus and on
// the dictionary (fuzzer-raw-image.dict) for the header.

#ifdef HAVE_CONFIG_H
#  include "config_auto.h" // DISABLED_LEGACY_ENGINE
#endif

#include <allheaders.h>
#include <tesseract/baseapi.h>

#include <libgen.h>     // for dirname
#include <algorithm>    // for std::clamp, std::min
#include <charconv>     // for std::from_chars
#include <cstddef>      // for size_t
#include <cstdint>      // for uint8_t
#include <cstdio>       // for printf
#include <cstdlib>      // for std::getenv, setenv
#include <cstring>      // for memcpy, memmove
#include <iterator>     // for std::size
#include <random>       // for std::minstd_rand
#include <string>       // for std::string
#include <string_view>  // for std::string_view
#include <system_error> // for std::errc

// Largest size of the header line without the '\n'.
static const size_t kMaxHeaderSize = 255;

// Largest image width and height. It allows a long text line or a tall
// column in an image which respects kMaxPixels.
static const int kMaxSide = 2048;

// Largest number of pixels. The run time grows with the image area, and
// OSS-Fuzz stops a test case after 25 s. The slowest images are those with
// many small words. With ASan and _GLIBCXX_DEBUG on a 3 GHz CPU, a page full
// of 5 x 7 pixel characters needed up to 9 s with this limit and up to 14 s
// with the 512 x 256 pixels of the larger one of the fuzzer-api targets.
static const int kMaxPixels = 256 * 256;

// Largest ratio of image width to image height. With PSM_SINGLE_LINE and
// PSM_SINGLE_CHAR the whole image is one text line, and the LSTM recognizer
// scales a text line to a fixed height, so a flat image gives a very long
// line. With ASan and _GLIBCXX_DEBUG, a blank image of 2048 x 1 pixels needed
// 34 s, 2048 x 8 pixels 3 s and 2048 x 32 pixels 2 s.
static const int kMaxAspect = 64;

// The resolution covers 0 (unknown) and values on both sides of the range
// which Tesseract accepts (kMinCredibleResolution..kMaxCredibleResolution).
static const int kMaxResolution = 9999;

// Parameters from the header with their default values.
struct Parameters {
  int psm = tesseract::PSM_SINGLE_BLOCK;
  int depth = 1;
  int thresholding_method = 0;
  int width = 128;
  int height = 128;
  int resolution = 0;
};

// Keys of the header with the range of their values.
static const struct {
  std::string_view key;
  int Parameters::*value;
  int min;
  int max;
} kKeys[] = {
    {"psm", &Parameters::psm, 0, tesseract::PSM_COUNT - 1},
    {"depth", &Parameters::depth, 1, 32},
    {"thresholding_method", &Parameters::thresholding_method, 0, 2},
    {"width", &Parameters::width, 1, kMaxSide},
    {"height", &Parameters::height, 1, kMaxSide},
    {"resolution", &Parameters::resolution, 0, kMaxResolution},
};

// Returns true for the page segmentation modes which the fuzzer uses.
//
// The modes with OSD are missing because they need osd.traineddata.
// PSM_AUTO_ONLY is missing because it is the same as PSM_AUTO for
// GetUTF8Text.
//
// PSM_SINGLE_WORD, PSM_CIRCLE_WORD and PSM_RAW_LINE are missing because they
// make a single word from all blobs of the image. The time which the legacy
// engine needs for a word (Tesseract::split_and_recog_word) grows much faster
// than the number of its blobs. A checkerboard of 2 x 2 pixel squares needed
// 19 s for 256 x 32 pixels with PSM_RAW_LINE, and 9 s for 512 x 32 pixels
// and 58 s for 1024 x 32 pixels with PSM_SINGLE_WORD.
static bool IsUsablePageSegMode(int mode) {
  switch (mode) {
    case tesseract::PSM_AUTO:
    case tesseract::PSM_SINGLE_COLUMN:
    case tesseract::PSM_SINGLE_BLOCK_VERT_TEXT:
    case tesseract::PSM_SINGLE_BLOCK:
    case tesseract::PSM_SINGLE_LINE:
    case tesseract::PSM_SINGLE_CHAR:
    case tesseract::PSM_SPARSE_TEXT:
      return true;
    default:
      return false;
  }
}

// Gets the parameters from the header line (without the '\n').
// Returns false if the header is not accepted.
static bool ParseHeader(std::string_view header, Parameters &parameters) {
  unsigned seen = 0;
  while (!header.empty()) {
    const auto token = header.substr(0, header.find(' '));
    header.remove_prefix(std::min(token.size() + 1, header.size()));
    if (token.empty()) {
      continue;
    }
    const auto equals = token.find('=');
    if (equals == std::string_view::npos) {
      return false;
    }
    const auto key = token.substr(0, equals);
    const char *first = token.data() + equals + 1;
    const char *last = token.data() + token.size();
    int value = 0;
    const auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
      return false;
    }
    unsigned index = 0;
    while (index < std::size(kKeys) && key != kKeys[index].key) {
      index++;
    }
    if (index == std::size(kKeys) || (seen & (1u << index)) != 0 ||
        value < kKeys[index].min || value > kKeys[index].max) {
      return false;
    }
    seen |= 1u << index;
    parameters.*kKeys[index].value = value;
  }
  return IsUsablePageSegMode(parameters.psm) &&
         (parameters.depth == 1 || parameters.depth == 8 ||
          parameters.depth == 32) &&
         parameters.width * parameters.height <= kMaxPixels &&
         parameters.width <= kMaxAspect * parameters.height;
}

// Finds the header line at the start of the fuzz input and gets the
// parameters from it. Returns the offset of the pixel data, or 0 if the input
// is not accepted.
static size_t ReadHeader(const uint8_t *data, size_t size,
                         Parameters &parameters) {
  // The '\n' may be the byte after kMaxHeaderSize.
  const std::string_view start(reinterpret_cast<const char *>(data),
                               std::min(size, kMaxHeaderSize + 1));
  const auto header_size = start.find('\n');
  if (header_size == std::string_view::npos ||
      !ParseHeader(start.substr(0, header_size), parameters)) {
    return 0;
  }
  return header_size + 1;
}

// Returns the header line for the parameters. Keys with the default value
// are left out.
static std::string WriteHeader(const Parameters &parameters) {
  const Parameters defaults;
  std::string header;
  for (const auto &key : kKeys) {
    if (parameters.*key.value != defaults.*key.value) {
      if (!header.empty()) {
        header += ' ';
      }
      header += key.key;
      header += '=';
      header += std::to_string(parameters.*key.value);
    }
  }
  header += '\n';
  return header;
}

// Sets one of the parameters to another valid value.
static void MutateParameters(Parameters &parameters, std::minstd_rand &random) {
  static const int kPageSegModes[] = {
      tesseract::PSM_AUTO,
      tesseract::PSM_SINGLE_COLUMN,
      tesseract::PSM_SINGLE_BLOCK_VERT_TEXT,
      tesseract::PSM_SINGLE_BLOCK,
      tesseract::PSM_SINGLE_LINE,
      tesseract::PSM_SINGLE_CHAR,
      tesseract::PSM_SPARSE_TEXT,
  };
  static const int kDepths[] = {1, 8, 32};
  // Resolutions around the limits which Tesseract uses, and common ones.
  static const int kResolutions[] = {
      0,   1,    69,   70,   71,
      72,  96,   150,  200,  300,
      600, 1200, 2400, 2401, kMaxResolution,
  };
  // A new size is either any size, a small size or a size near the old one.
  const auto new_size = [&random](int old_size) {
    switch (random() % 3) {
      case 0:
        return 1 + static_cast<int>(random() % kMaxSide);
      case 1:
        return 1 + static_cast<int>(random() % 64);
      default:
        return std::clamp(old_size + static_cast<int>(random() % 33) - 16, 1,
                          kMaxSide);
    }
  };
  switch (random() % 6) {
    case 0:
      parameters.psm = kPageSegModes[random() % std::size(kPageSegModes)];
      break;
    case 1:
      parameters.depth = kDepths[random() % std::size(kDepths)];
      break;
    case 2:
      parameters.thresholding_method = random() % 3;
      break;
    case 3:
      // Change the height if it is needed to get an accepted image size.
      parameters.width = new_size(parameters.width);
      parameters.height = std::clamp(
          parameters.height, (parameters.width + kMaxAspect - 1) / kMaxAspect,
          kMaxPixels / parameters.width);
      break;
    case 4:
      // Change the width if it is needed to get an accepted image size.
      parameters.height = new_size(parameters.height);
      parameters.width =
          std::min({parameters.width, kMaxPixels / parameters.height,
                    kMaxAspect * parameters.height});
      break;
    default:
      parameters.resolution =
          random() % 2 != 0 ? kResolutions[random() % std::size(kResolutions)]
                            : random() % (kMaxResolution + 1);
      break;
  }
}

// Mutation function of libFuzzer.
extern "C" size_t LLVMFuzzerMutate(uint8_t *data, size_t size, size_t max_size);

// Custom mutator for libFuzzer. It changes either a parameter or the pixel
// data, so the header is always accepted.
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *data, size_t size,
                                          size_t max_size, unsigned int seed) {
  std::minstd_rand random(seed);
  Parameters parameters;
  // An input which is not accepted is used as pixel data.
  const size_t old_header_size = ReadHeader(data, size, parameters);
  if (old_header_size == 0) {
    parameters = Parameters();
  }
  const bool mutate_parameters = random() % 4 == 0;
  if (mutate_parameters) {
    MutateParameters(parameters, random);
  }
  const std::string header = WriteHeader(parameters);
  if (header.size() >= max_size) {
    return size;
  }
  uint8_t *pixels = data + header.size();
  const size_t max_pixels_size = max_size - header.size();
  size_t pixels_size = std::min(size - old_header_size, max_pixels_size);
  memmove(pixels, data + old_header_size, pixels_size);
  if (!mutate_parameters) {
    pixels_size = LLVMFuzzerMutate(pixels, pixels_size, max_pixels_size);
  }
  memcpy(data, header.data(), header.size());
  return header.size() + pixels_size;
}

// Reads the pixel data byte by byte.
class ByteReader {
private:
  const uint8_t *data;
  size_t size;

public:
  ByteReader(const uint8_t *data, size_t size) : data(data), size(size) {}

  // Returns the next byte or fill if there is no more data.
  unsigned Read(unsigned fill) {
    if (size == 0) {
      return fill;
    }
    size--;
    return *data++;
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
  setMsgSeverity(L_SEVERITY_NONE);

  /* Bound the run time for images which give a huge number of blobs (OSS-Fuzz
     issue 581720522). This is not the default, because it would also truncate
     the text of normal pages. The parameter does not exist in builds without
     the legacy engine, so the result is ignored. */
  api->SetVariable("chop_max_classifications_pass", "5000");

  return 0;
}

static Pix *createPix(ByteReader &reader, int width, int height, int depth) {
  Pix *pix = pixCreate(width, height, depth);

  if (pix == nullptr) {
    printf("pix creation failed\n");
    abort();
  }

  l_uint32 *line = pixGetData(pix);
  const int wpl = pixGetWpl(pix);
  for (int y = 0; y < height; y++, line += wpl) {
    switch (depth) {
      case 1:
        for (int x = 0; x < width; x += 8) {
          SET_DATA_BYTE(line, x / 8, reader.Read(0x00));
        }
        break;
      case 8:
        for (int x = 0; x < width; x++) {
          SET_DATA_BYTE(line, x, reader.Read(0xff));
        }
        break;
      default:
        for (int x = 0; x < width; x++) {
          const unsigned red = reader.Read(0xff);
          const unsigned green = reader.Read(0xff);
          const unsigned blue = reader.Read(0xff);
          composeRGBPixel(red, green, blue, line + x);
        }
        break;
    }
  }

  // Clear the unused bits at the end of the rows of a 1 bpp image.
  pixSetPadBits(pix, 0);

  return pix;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  Parameters parameters;
  const size_t header_size = ReadHeader(data, size, parameters);
  if (header_size == 0) {
    // The input is not accepted. Return 0 nevertheless: libFuzzer and AFL++
    // would not add the input to the corpus for -1, but honggfuzz ends with
    // an error for any other value than 0.
    return 0;
  }

  ByteReader reader(data + header_size, size - header_size);
  auto pix =
      createPix(reader, parameters.width, parameters.height, parameters.depth);
  pixSetResolution(pix, parameters.resolution, parameters.resolution);

  api->SetPageSegMode(static_cast<tesseract::PageSegMode>(parameters.psm));
  api->SetVariable("thresholding_method",
                   std::to_string(parameters.thresholding_method).c_str());
  api->SetImage(pix);

  char *outText = api->GetUTF8Text();

  delete[] outText;

  // Release the image and the results, and forget what was learned from this
  // image, so that the result for the next image does not depend on it.
  api->Clear();
#ifndef DISABLED_LEGACY_ENGINE
  api->ClearAdaptiveClassifier();
#endif
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
