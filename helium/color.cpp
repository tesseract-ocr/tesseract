// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "color.h"
#include "mathfunctions.h"
#include "types.h"

// C includes
#include <stdlib.h>

using namespace helium;

uint16 helium::Hue(const Color& color) {
  float r = static_cast<float>(Red(color));
  float g = static_cast<float>(Green(color));
  float b = static_cast<float>(Blue(color));

  if ((r == g) && (g == b)) return 0;  // undefined

  if (r > g) {
    if (g >= b)        // red > green > blue
      return static_cast<uint16>(60.0 * ((g - b) / (r - b)));
    else {
      if (b > r)          // blue > red > green
        return static_cast<uint16>(60.0 * ((r - g) / (b - g)) + 240.0);
      else                      // red > blue > green
        return static_cast<uint16>(60.0 * ((g - b) / (r - g)) + 360.0);
    }
  } else if (g > b)    // green > blue > red
    return static_cast<uint16>(60.0 * ((b - r) / (g - r)) + 120.0);
  else                          // blue > green > red
    return static_cast<uint16>(60.0 * ((r - g) / (b - r)) + 240.0);
}

uint8 helium::Saturation(const Color& color) {
  float max = static_cast<float>(Max3(Red(color), Green(color), Blue(color)));
  float min = static_cast<float>(Min3(Red(color), Green(color), Blue(color)));
  float l = 0.5 * (min + max);

  if (min == max)
    return 0;
  else if (l <= 128.0)
    return static_cast<int>(((max - min) / (2.0 * l)) * 255.0);
  else
    return static_cast<int>(((max - min) / (510.0 - 2.0 * l)) * 255.0);
}

uint8 helium::Lightness(const Color& color) {
  float max = static_cast<float>(Max3(Red(color), Green(color), Blue(color)));
  float min = static_cast<float>(Min3(Red(color), Green(color), Blue(color)));
  float l = 0.5 * (min + max);
  return static_cast<int>(l);
}

uint8 helium::Luminance(const Color& color) {
  float lum = 0.3*Red(color) + 0.59*Green(color) + 0.11*Blue(color);
  return static_cast<int>(lum);
}

// Algorithm for conversion taken from Wikipedia.
Color helium::MakeColorFromHSL(uint16 hue, uint8 saturation, uint8 lightness) {
  float l = static_cast<float>(lightness) / 255.0;
  float s = static_cast<float>(saturation) / 255.0;
  float h = static_cast<float>(hue) / 360.0;

  float t2 = (lightness < 128) ? l * (1.0 + s) : l + s - (l * s);
  float t1 = 2.0 * l - t2;

  float comp[3] = { h + 1.0/3.0, h, h - 1.0/3.0 };

  for (unsigned i = 0; i < 3; i++)
    if (comp[i] < 0.0) comp[i] += 1.0; else if (comp[i] > 1.0) comp[i] -= 1.0;

  for (unsigned i = 0; i < 3; i++)
    if (comp[i] < 1.0/6.0)
      comp[i] = t1 + ((t2 - t1) * 6.0 * comp[i]);
    else if (comp[i] < 0.5)
      comp[i] = t2;
    else if (comp[i] < 2.0/3.0)
      comp[i] = t1 + ((t2 -t1) * (2.0/3.0 - comp[i]) * 6.0);
    else
      comp[i] = t1;

  return MakeColor(static_cast<int>(comp[0] * 255.0),
                   static_cast<int>(comp[1] * 255.0),
                   static_cast<int>(comp[2] * 255.0));
}

uint16 helium::Distance(const Color& a, const Color& b) {
  return  (abs(Red(a) - Red(b))
          + abs(Green(a) - Green(b))
          + abs(Blue(a) - Blue(b))) / 3;
}

uint16 helium::EuclideanDistance(const Color& a, const Color& b) {
  return SquaredRoot(Square(Red(a) - Red(b))
                    + Square(Green(a) - Green(b))
                    + Square(Blue(a) - Blue(b)));
}

uint16 helium::HueDistance(const Color& a, const Color& b) {
  int hue1 = Hue(a);
  int hue2 = Hue(b);

  return Min3(abs(hue1 - hue2),
              abs((hue1 + 360) - hue2),
              abs(hue1 - (hue2 + 360)));
}

Color helium::ColorDifference(const Color& a, const Color& b) {
  return MakeColor(abs(Red(a) - Red(b)),
                   abs(Green(a) - Green(b)),
                   abs(Blue(a) - Blue(b)));
}

