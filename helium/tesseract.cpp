// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Tesseract includes
//NOTE: Ugly ifdefs!
#ifdef EMBEDDED
#include "tessembedded.h"
#include "pageres.h"
#include "tessvars.h"
#include "control.h"
#else
#include "third_party/tesseract/ccmain/tessedit.h"
#include "third_party/tesseract/ccstruct/pageres.h"
#include "third_party/tesseract/ccmain/tessvars.h"
#include "third_party/tesseract/ccmain/control.h"
#endif

#undef LOG

// Local includes
#include "debugging.h"
#include "tesseract.h"
#include "mask.h"

extern IMAGE page_image;

using namespace helium;

const char* kArguments[3] = { "tesseract", "out", "batch" };

int Tesseract::Init(const char* datapath) {
  return api_.Init(datapath, NULL);
}

void Tesseract::ReadMask(const Mask& mask, bool flipped) {
  IMAGELINE line;
  page_image.create(mask.width(), mask.height(), 1);
  line.init(mask.width());
  bool* mask_ptr = flipped ? mask.Access(0, mask.height() - 1) : mask.data();

  for (int y = 0; y < mask.height(); ++y) {
    for (int x = 0; x < mask.width(); ++x) line.pixels[x] = !(*(mask_ptr++));
    page_image.put_line(0, y, mask.width(), &line, 0);
    if (flipped) mask_ptr -= mask.width() * 2;
  }
}

bool Tesseract::DetectBaseline(const Mask& mask,
                               int& out_offset,
                               float& out_slope,
                               bool flipped) {
  MaskThresholder* mt = new MaskThresholder(mask, flipped);
  api_.SetThresholder(mt);
  return api_.GetTextDirection(&out_offset, &out_slope);
}

void MaskToBuffer(const Mask& mask, unsigned char* buf) {
  bool* mask_ptr = mask.data();
  for (int y = 0; y < mask.height(); ++y)
    for (int x = 0; x < mask.width(); ++x)
      *buf++ = *(mask_ptr++) ? 0 : 255;
}

char* Tesseract::RecognizeText(const Mask& mask) {
  // MaskThresholder* mt = new MaskThresholder(mask, true);
  // api_.SetThresholder(mt);
  // Check with Ray on directly passing in image after fixing Otsu thresholding
  unsigned char* buf = new unsigned char[mask.width() * mask.height()];
  MaskToBuffer(mask, buf);
  api_.SetImage(buf, mask.width(), mask.height(), 1, mask.width());
  api_.Recognize(NULL);
  delete[] buf;
  return api_.GetUTF8Text();
}

void Tesseract::End() {
  api_.End();
}

tesseract::TessBaseAPI Tesseract::api_;

MaskThresholder::MaskThresholder(const Mask& mask, bool flipped)
  : mask_(mask), flipped_(flipped) {
  image_width_ = mask.width();
  image_height_ = mask.height();
  image_data_ = NULL;
  image_bytespp_ = sizeof(bool);
  image_bytespl_ = image_width_ * sizeof(bool);
  Init();
}

void MaskThresholder::ThresholdToIMAGE(IMAGE* image) {
  IMAGELINE line;
  image->create(mask_.width(), mask_.height(), 1);
  line.init(mask_.width());
  bool* mask_ptr = flipped_ ? mask_.Access(0, mask_.height() - 1) : mask_.data();

  for (int y = 0; y < mask_.height(); ++y) {
    for (int x = 0; x < mask_.width(); ++x) line.pixels[x] = !(*(mask_ptr++));
    image->put_line(0, y, mask_.width(), &line, 0);
    if (flipped_) mask_ptr -= mask_.width() * 2;
  }
}
