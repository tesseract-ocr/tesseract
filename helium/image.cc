// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "box.h"
#include "image.h"

using namespace helium;

Image::Image() : Map<Color>() {
}

Image::Image(unsigned width, unsigned height)
  : Map<Color>(width, height) {
}

Image::Image(unsigned width, unsigned height, uint32* data)
  : Map<Color>(width, height, data) {
}

Image Image::FromGrayMap(const GrayMap& map) {
  Image image(map.width(), map.height());
  Color* image_ptr = image.data();
  for (uint8* map_ptr = map.data(); map_ptr <= map.DataEnd(); map_ptr++)
    *(image_ptr++) = MakeColor(*map_ptr, *map_ptr, *map_ptr);
  return image;
}

Image Image::FromMask(const Mask& mask) {
  Image image(mask.width(), mask.height());
  Color* image_ptr = image.data();
  for (bool* mask_ptr = mask.data(); mask_ptr <= mask.DataEnd(); mask_ptr++)
    *(image_ptr++) = (*mask_ptr) ? 0 : 0xFFFFFFFF;  // mask==true means black
  return image;
}

Image Image::FromLabeledImage(const Image& labels) {
  Image image(labels.width(), labels.height());
  Color* image_ptr = image.data();
  for (Color* l_ptr = labels.data(); l_ptr <= labels.DataEnd(); l_ptr++)
    *(image_ptr++) = MakeColorFromHSL(Alpha(*l_ptr), 196, 128);
  return image;
}

GrayMap Image::ToGrayMap(const Image& image) {
  GrayMap gray(image.width(), image.height());
  uint8* gray_ptr = gray.data();
  for (Color* image_ptr = image.data(); image_ptr <= image.DataEnd();
       image_ptr++)
    *gray_ptr++ = Luminance(*image_ptr);
  return gray;
}

void Image::DrawBox(const Box& box, Color color) {
  Color* image_ptr = Access(box.origin());
  unsigned rows = (box.height() > 0) ? box.height() - 1: 0;
  unsigned cols = (box.width() > 0) ? box.width() - 1: 0;
  unsigned y_offset = rows * width();
  for (unsigned x = 0; x < box.width(); ++x) {
    *(image_ptr) = color;
    *(image_ptr + y_offset) = color;
    ++image_ptr;
  }

  image_ptr = Access(box.origin());
  for (unsigned y = 0; y < box.height(); ++y) {
    *(image_ptr) = color;
    *(image_ptr + cols) = color;
    image_ptr += width();
  }
}

void Image::DrawBoxes(const Array<Box>& boxes, Color color, float scale) {
  for (unsigned i = 0; i < boxes.size(); i++) {
    Box scaled_box = ScaleBox(boxes.ValueAt(i), scale);
    DrawBox(scaled_box, color);
  }
}
