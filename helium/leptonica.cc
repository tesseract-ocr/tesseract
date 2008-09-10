// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "leptonica.h"
#include "image.h"
#include "graymap.h"
#include "imageenhancer.h"

// C includes
#include <stdlib.h>

using namespace helium;


// TODO: CONVERT IF NECESSARY!!!
Image Leptonica::PixToImage(const Pix* cpix) {
  Pix* pix = const_cast<Pix*>(cpix);
  if (pixGetDepth(pix) == 8) {
    GrayMap gmap = PixToGrayMap(pix);
    return Image::FromGrayMap(gmap);
  }
  // Else must be 32-bit
  ASSERT(pixGetDepth(pix) == 32);
  Image out = Image(pixGetWidth(pix), pixGetHeight(pix));
  memcpy(out.data(), pix->data, out.width() * out.height() * 4);
  return out;
}

Pix* Leptonica::ImageToPix(const Image& image) {
  Pix* pix = reinterpret_cast<Pix*>(calloc(sizeof(Pix), 1));
  pix->w = image.width();
  pix->h = image.height();
  pix->d = 32;
  pix->wpl = image.width();
  pix->data = reinterpret_cast<l_uint32*>(
                malloc(image.width() * image.height() * 4)
              );
  pix->refcount = 1;
  memcpy(pix->data, image.data(), image.width() * image.height() * 4);
  return pix;
}

// TODO: Return pointer to avoid unnecessary copies.
GrayMap Leptonica::PixToGrayMap(const Pix *cpix) {
  Pix* pix = const_cast<Pix*>(cpix);
  ASSERT(pixGetDepth(pix) == 8);
  GrayMap out(pixGetWidth(pix), pixGetHeight(pix));
  uint8 *wpt = out.data();
  for (int y = 0; y < out.height(); ++y) {
    l_uint32* lrpt = pixGetData(pix) + y * pixGetWpl(pix);
    for (int x = 0; x < out.width(); ++x) {
      *wpt++ = static_cast<uint8>(GET_DATA_BYTE(lrpt, x));
    }
  }
  // A direct memcpy does not work
  // memcpy(out.data(), pixGetData(pix), out.width() * out.height());
  return out;
}

Pix* Leptonica::GrayMapToPix(const GrayMap& graymap) {
  Pix* pix = pixCreate(graymap.width(), graymap.height(), 8);
  uint8 *rpt = graymap.data();
  for (int y = 0; y < graymap.height(); ++y) {
    l_uint32 *lwpt = pixGetData(pix) + y * pixGetWpl(pix);
    for (int x = 0; x < graymap.width(); ++x) {
      SET_DATA_BYTE(lwpt, x, *rpt++);
    }
  }
  return pix;
}

Mask* Leptonica::PixToMask(const Pix* cpix) {
  Pix* pix = const_cast<Pix*>(cpix);
  ASSERT(pixGetDepth(pix) == 1);
  Mask* mask = new Mask(pixGetWidth(pix), pixGetHeight(pix));
  bool* mask_ptr = mask->data();
  for (int y = 0; y < mask->height(); ++y) {
    l_uint32 *lrpt = pixGetData(pix) + y * pixGetWpl(pix);
    for (int x = 0; x < mask->width(); ++x) {
      *(mask_ptr++) = GET_DATA_BIT(lrpt, x);
    }
  }
  return mask;
}

Pix* Leptonica::MaskToPix(const Mask& mask) {
  Pix* pix = pixCreate(mask.width(), mask.height(), 1);
  bool* mask_ptr = mask.data();
  for (int y = 0; y < mask.height(); ++y) {
    l_uint32 *lwpt = pixGetData(pix) + y * pixGetWpl(pix);
    for (int x = 0; x < mask.width(); ++x) {
      if (*(mask_ptr++))
        SET_DATA_BIT(lwpt, x);
    }
  }
  return pix;
}

void Leptonica::DisplayImage(const Image& image) {
  Pix *pix = ImageToPix(image);
  pixDisplay(pix, 0, 0);
  pixDestroy(&pix);
}

void Leptonica::DisplayGrayMap(const GrayMap& graymap) {
  DisplayImage(Image::FromGrayMap(graymap));
}

void Leptonica::DisplayMask(const Mask& mask) {
  Pix *pix = MaskToPix(mask);
  pixDisplay(pix, 0, 0);
  pixDestroy(&pix);
}

void Leptonica::SaveImage(const Image& image, const char* filename) {
  Pix *pix = ImageToPix(image);
  pixWrite(filename, pix, IFF_JFIF_JPEG);
  pixDestroy(&pix);
}
