#ifndef TESSERACT_CCUTIL_VERSION_H_
#define TESSERACT_CCUTIL_VERSION_H_

#define TESSERACT_VERSION_STR "4.00.00alpha"
#define TESSERACT_VERSION 0x040000
#define MAKE_VERSION(major, minor, patch) \
  (((major) << 16) | ((minor) << 8) | (patch))

#endif  // TESSERACT_CCUTIL_VERSION_H_
