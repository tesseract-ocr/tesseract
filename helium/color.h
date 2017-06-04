// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file defines the Color type (which is just a 32-bit integer type), and
// various functions to analyze and manipulate color values. A Color in Helium
// is always an RGBA value, or ABGR, depending on how the colors are stored,
// and the endianness of the host machine.
// The function MakeColor(...) is used to create a Color. ColorSeparate(...),
// and the functions Red(Color), Green(Color) and Blue(Color) are used for
// splitting a color into its channels red, green, and blue.
// The alpha channel is generally omitted from most of the functions, and
// is intended to provide means for efficient storage inside an RGBA image.
// It can be accessed using the Alpha() and the SetAlphaAt(...) functions.
// Various functions for color comparison are available, that range in
// complexity and functionality.
// Color space conversions are generally too expensive for Helium, but a few
// functions for HSL conversion are provided, which should be used sparingly.
//
#ifndef HELIUM_COLOR_H__
#define HELIUM_COLOR_H__

#include "mathfunctions.h"
#include "types.h"

namespace helium {

// The color type.
typedef uint32 Color;

// These constants provide the offsets of each channel within a color
// (in bits). They define the structure of the color channels.
#ifndef COLORS_LITTLEENDIAN
  const uint8 kRedOffset = 24;
  const uint8 kGreenOffset = 16;
  const uint8 kBlueOffset = 8;
  const uint8 kAlphaOffset = 0;
#else
  const uint8 kRedOffset = 0;
  const uint8 kGreenOffset = 8;
  const uint8 kBlueOffset = 16;
  const uint8 kAlphaOffset = 24;
#endif

const uint32 kAlphaMask = 0x000000FF << kAlphaOffset;

// Basic color functions -------------------------------------------------------

// Access to the red component of the color
inline uint8 Red(Color color) {
  return (color >> kRedOffset) & 0x00FF;
}

// Access to the green component of the color
inline uint8 Green(Color color) {
  return (color >> kGreenOffset) & 0x00FF;
}

// Access to the blue component of the color
inline uint8 Blue(Color color) {
  return (color >> kBlueOffset) & 0x00FF;
}

// Access to the alpha component of the color
inline uint8 Alpha(Color color) {
  return (color >> kAlphaOffset) & 0x00FF;
}

// Function to create a new color with the specified red, green, and blue
// values.
inline Color MakeColor(uint8 r, uint8 g, uint8 b) {
  return (r << kRedOffset) | (g << kGreenOffset) | (b << kBlueOffset);
}

// Function to set an alpha value of a specified color in memory.
inline void SetAlphaAt(Color* ptr, uint8 value) {
  *ptr = (*ptr & (~kAlphaMask)) | (value << kAlphaOffset);
}

// Splits a given color into separate channels.
inline void ColorSeparate(Color c, int& r, int& g, int& b) {
  r = Red(c);
  g = Green(c);
  b = Blue(c);
}

// Convenience function to limit channels given as integers to the valid
// channel range 0...255.
inline void ChannelLimit(int& r, int& g, int& b) {
  if (r < 0) r = 0; else if (r > 255) r = 255;
  if (g < 0) g = 0; else if (g > 255) g = 255;
  if (b < 0) b = 0; else if (b > 255) b = 255;
}

// Converting colors to scalars ------------------------------------------------
// Returns the average of the three channels red, green, and blue.
inline uint8 Average(const Color& color) {
  return (Red(color) + Green(color) + Blue(color)) / 3;
}

// Returns the maximum channel value.
inline uint8 MaxChannel(const Color& color) {
  return Max3(Red(color), Green(color), Blue(color));
}

// Returns the minimum channel value.
inline uint8 MinChannel(const Color& color) {
  return Min3(Red(color), Green(color), Blue(color));
}

// Comparing colors ------------------------------------------------------------
// Returns the average difference of each channel of color A to
// color B. Use this when the Euclidean distance is too expensive to calculate.
uint16 Distance(const Color& a, const Color& b);

// Returns the Euclidean distance between A and B in the three-dimensional
// RGB colorspace.
uint16 EuclideanDistance(const Color& a, const Color& b);

// Calculates the hue value of each color, and returns the angle difference
// between them.
uint16 HueDistance(const Color& a, const Color& b);

// Returns the difference of the channels in color A to the ones in color B.
// The three values are returned as a Color.
Color ColorDifference(const Color& a, const Color& b);

// HSL colorspace --------------------------------------------------------------
// Calculates the hue of the color as an angle 0...359
uint16 Hue(const Color& color);

// Calculates the saturation of the color.
uint8 Saturation(const Color& color);

// Calculates the lightness of the color.
uint8 Lightness(const Color& color);

// Calculates the luminance of the color.
uint8 Luminance(const Color& color);

// Make color from HSL components
Color MakeColorFromHSL(uint16 hue, uint8 saturation, uint8 lightness);

} // namespace

#endif // HELIUM_COLOR_H__
