//

#ifndef TESSERACT_CCUTIL_TESSIO_H_
#define TESSERACT_CCUTIL_TESSIO_H_

#include "config_auto.h"

class CLASS_PRUNER_STRUCT;
struct INT_FEATURE_STRUCT;
struct INT_PROTO_STRUCT;
struct PROTO_SET_STRUCT;

#ifdef WORDS_BIGENDIAN

static inline void convert2le(uint16_t &val) {
    val = (((val & 0x00ff) << 8) |
              ((val & 0xff00) >> 8));
}

static inline void convert2le(uint32_t &val) {
    val = (((val & 0x000000ffU) << 24) |
              ((val & 0x0000ff00U) <<  8) |
              ((val & 0x00ff0000U) >>  8) |
              ((val & 0xff000000U) >> 24));
}

static inline void convert2le(uint64_t &val) {
    val = (((val & 0x00000000000000ffULL) << 56) |
              ((val & 0x000000000000ff00ULL) << 40) |
              ((val & 0x0000000000ff0000ULL) << 24) |
              ((val & 0x00000000ff000000ULL) <<  8) |
              ((val & 0x000000ff00000000ULL) >>  8) |
              ((val & 0x0000ff0000000000ULL) >> 24) |
              ((val & 0x00ff000000000000ULL) >> 40) |
              ((val & 0xff00000000000000ULL) >> 56));
}

#else // WORDS_BIGENDIAN

static inline void convert2le(uint16_t &val) {
}

static inline void convert2le(uint32_t &val) {
}

static inline void convert2le(uint64_t &val) {
}

#endif // WORDS_BIGENDIAN

namespace tesseract {
struct FontInfo;
struct FontSet;
bool fread(FontInfo* data, FILE* f, size_t n = 1);
bool fread(FontSet* data, FILE* f, size_t n = 1);
} // namespace tesseract

bool fread(CLASS_PRUNER_STRUCT* data, FILE* f, size_t n = 1);
bool fread(INT_FEATURE_STRUCT* data, FILE* f, size_t n = 1);
bool fread(INT_PROTO_STRUCT* data, FILE* f, size_t n = 1);
bool fread(PROTO_SET_STRUCT* data, FILE* f, size_t n = 1);

#if 0
static inline bool fread(uint8_t* data, FILE* f, size_t n = 1) {
  return fread(data, sizeof(*data), n, f) == n;
}

static inline bool fread(uint16_t* data, FILE* f, size_t n = 1) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

static inline bool fread(uint32_t* data, FILE* f, size_t n = 1) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

static inline bool fread(uint64_t* data, FILE* f, size_t n = 1) {
  size_t m = fread(data, sizeof(*data), n, f);
  for (size_t i = 0; i < m; i++)
    convert2le(data[i]);
  return n == m;
}

static inline bool fread(int8_t* data, FILE* f, size_t n = 1) {
  return fread((uint8_t *)data, f, n);
}

static inline bool fread(int16_t* data, FILE* f, size_t n = 1) {
  return fread((uint16_t *)data, f, n);
}

static inline bool fread(int32_t* data, FILE* f, size_t n = 1) {
  return fread((uint32_t *)data, f, n);
}

static inline bool fread(int64_t* data, FILE* f, size_t n = 1) {
  return fread((uint64_t *)data, f, n) ;
}

static inline bool fread(char* data, FILE* f, size_t n = 1) {
  return fread(data, sizeof(*data), n, f) == n;
}

static inline bool fread(float* data, FILE* f, size_t n = 1) {
  return fread(data, sizeof(*data), n, f) == n;
}
#else

bool fread(uint8_t* data, FILE* f, size_t n = 1);
bool fread(uint16_t* data, FILE* f, size_t n = 1);
bool fread(uint32_t* data, FILE* f, size_t n = 1) ;
bool fread(uint64_t* data, FILE* f, size_t n = 1);
bool fread(int8_t* data, FILE* f, size_t n = 1);
bool fread(int16_t* data, FILE* f, size_t n = 1);
bool fread(int32_t* data, FILE* f, size_t n = 1);
bool fread(int64_t* data, FILE* f, size_t n = 1);
bool fread(char* data, FILE* f, size_t n = 1);
bool fread(float* data, FILE* f, size_t n = 1);

#endif

#endif // TESSERACT_CCUTIL_TESSIO_H_
