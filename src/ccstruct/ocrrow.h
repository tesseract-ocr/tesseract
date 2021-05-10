/**********************************************************************
 * File:        ocrrow.h  (Formerly row.h)
 * Description: Code for the ROW class.
 * Author:      Ray Smith
 * Created:     Tue Oct 08 15:58:04 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef OCRROW_H
#define OCRROW_H

#include "elst.h"       // for ELIST_ITERATOR, ELISTIZEH, ELIST_LINK
#include "quspline.h"   // for QSPLINE
#include "rect.h"       // for TBOX
#include "scrollview.h" // for ScrollView, ScrollView::Color
#include "werd.h"       // for WERD_LIST

#include <cstdint> // for int16_t, int32_t
#include <cstdio>  // for FILE

namespace tesseract {

class ICOORD;
class TO_ROW;

struct PARA;

class ROW : public ELIST_LINK {
  friend void tweak_row_baseline(ROW *, double, double);

public:
  ROW() = default;
  ROW(                     // constructor
      int32_t spline_size, // no of segments
      int32_t *xstarts,    // segment boundaries
      double *coeffs,      // coefficients //ascender size
      float x_height, float ascenders,
      float descenders, // descender size
      int16_t kern,     // char gap
      int16_t space);   // word gap
  ROW(                  // constructor
      TO_ROW *row,      // textord row
      int16_t kern,     // char gap
      int16_t space);   // word gap

  WERD_LIST *word_list() { // get words
    return &words;
  }

  float base_line(        // compute baseline
      float xpos) const { // at the position
    // get spline value
    return static_cast<float>(baseline.y(xpos));
  }
  float x_height() const { // return x height
    return xheight;
  }
  void set_x_height(float new_xheight) { // set x height
    xheight = new_xheight;
  }
  int32_t kern() const { // return kerning
    return kerning;
  }
  float body_size() const { // return body size
    return bodysize;
  }
  void set_body_size(float new_size) { // set body size
    bodysize = new_size;
  }
  int32_t space() const { // return spacing
    return spacing;
  }
  float ascenders() const { // return size
    return ascrise;
  }
  float descenders() const { // return size
    return descdrop;
  }
  TBOX bounding_box() const { // return bounding box
    return bound_box;
  }
  // Returns the bounding box including the desired combination of upper and
  // lower noise/diacritic elements.
  TBOX restricted_bounding_box(bool upper_dots, bool lower_dots) const;

  void set_lmargin(int16_t lmargin) {
    lmargin_ = lmargin;
  }
  void set_rmargin(int16_t rmargin) {
    rmargin_ = rmargin;
  }
  int16_t lmargin() const {
    return lmargin_;
  }
  int16_t rmargin() const {
    return rmargin_;
  }

  void set_has_drop_cap(bool has) {
    has_drop_cap_ = has;
  }
  bool has_drop_cap() const {
    return has_drop_cap_;
  }

  void set_para(PARA *p) {
    para_ = p;
  }
  PARA *para() const {
    return para_;
  }

  void recalc_bounding_box(); // recalculate BB

  void move(             // reposition row
      const ICOORD vec); // by vector

  void print(    // print
      FILE *fp) const; // file to print on

#ifndef GRAPHICS_DISABLED
  void plot(                     // draw one
      ScrollView *window,        // window to draw in
      ScrollView::Color colour); // uniform colour
  void plot(                     // draw one
      ScrollView *window);       // in rainbow colours

  void plot_baseline(             // draw the baseline
      ScrollView *window,         // window to draw in
      ScrollView::Color colour) { // colour to draw
    // draw it
    baseline.plot(window, colour);
  }
#endif // !GRAPHICS_DISABLED
  ROW &operator=(const ROW &source);

private:
  // Copy constructor (currently unused, therefore private).
  ROW(const ROW &source) = delete;

  int32_t kerning;  // inter char gap
  int32_t spacing;  // inter word gap
  TBOX bound_box;   // bounding box
  float xheight;    // height of line
  float ascrise;    // size of ascenders
  float descdrop;   //-size of descenders
  float bodysize;   // CJK character size. (equals to
                    // xheight+ascrise by default)
  WERD_LIST words;  // words
  QSPLINE baseline; // baseline spline

  // These get set after blocks have been determined.
  bool has_drop_cap_;
  int16_t lmargin_; // Distance to left polyblock margin.
  int16_t rmargin_; // Distance to right polyblock margin.

  // This gets set during paragraph analysis.
  PARA *para_; // Paragraph of which this row is part.
};

ELISTIZEH(ROW)

} // namespace tesseract

#endif
