///////////////////////////////////////////////////////////////////////
// File:        fontstyle.h
// Description: Estimation of font style attributes (bold, italic) from
//              the image of a word.
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

#ifndef TESSERACT_CCMAIN_FONTSTYLE_H_
#define TESSERACT_CCMAIN_FONTSTYLE_H_

#include "image.h"

#include <tesseract/export.h> // for TESS_API

namespace tesseract {

struct FontInfo;

// Shape of the text in a binary image, measured without any knowledge of the
// characters.
struct TextShape {
  // Horizontal shear which makes the strokes of the text vertical. It is the
  // tangent of the slant angle and positive for text which leans to the right,
  // so about 0 for upright and 0.1 to 0.3 for italic text.
  float slant = 0.0f;
  // False if the strongest strokes of the text have a slant outside the range
  // of upright and italic text, which is from -0.1 to 0.35. That is the case
  // for text with mainly diagonal strokes, for example "wavy". The slant is 0
  // then.
  bool has_slant = false;
  // Typical width of the strokes in pixels, measured along the pixel rows.
  float stroke_width = 0.0f;
};

// Measures the shape of the text in the given image, which must have a depth
// of 1 bit. The set pixels are the text. Returns false and leaves the shape
// untouched if the image is not usable or contains no text.
TESS_API
bool MeasureTextShape(Image pix, TextShape *shape);

// Returns the entry of a table of fonts without a name for the given style.
// The entries have the font id -1 and stay valid until the program ends.
// They are used for words which get their font attributes from an estimation
// instead of from a font classifier.
TESS_API
const FontInfo *NamelessFont(bool bold, bool italic);

} // namespace tesseract

#endif // TESSERACT_CCMAIN_FONTSTYLE_H_
