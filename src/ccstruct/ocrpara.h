/////////////////////////////////////////////////////////////////////
// File:        ocrpara.h
// Description: OCR Paragraph Output Type
// Author:      David Eger
//
// (C) Copyright 2010, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

#ifndef TESSERACT_CCSTRUCT_OCRPARA_H_
#define TESSERACT_CCSTRUCT_OCRPARA_H_

#include "elst.h"

#include <tesseract/publictypes.h>

namespace tesseract {

class ParagraphModel;

struct PARA : public ELIST<PARA>::LINK {
public:
  PARA()
      : model(nullptr)
      , is_list_item(false)
      , is_very_first_or_continuation(false)
      , has_drop_cap(false) {}

  // We do not own the model, we just reference it.
  // model may be nullptr if there is not a good model for this paragraph.
  const ParagraphModel *model;

  bool is_list_item;

  // The first paragraph on a page often lacks a first line indent, but should
  // still be modeled by the same model as other body text paragraphs on the
  // page.
  bool is_very_first_or_continuation;

  // Does this paragraph begin with a drop cap?
  bool has_drop_cap;
};

ELISTIZEH(PARA)

// A geometric model of paragraph indentation and alignment.
//
// Measurements are in pixels. The meaning of the integer arguments changes
// depending upon the value of justification.  Distances less than or equal
// to tolerance apart we take as "equivalent" for the purpose of model
// matching, and in the examples below, we assume tolerance is zero.
//
// justification = LEFT:
//   margin       the "ignored" margin to the left block edge.
//   first_indent indent from the left margin to a typical first text line.
//   body_indent  indent from the left margin of a typical body text line.
//
// justification = RIGHT:
//   margin       the "ignored" margin to the right block edge.
//   first_indent indent from the right margin to a typical first text line.
//   body_indent  indent from the right margin of a typical body text line.
//
// justification = CENTER:
//   margin       ignored
//   first_indent ignored
//   body_indent  ignored
//
//  ====== Extended example, assuming each letter is ten pixels wide: =======
//
// +--------------------------------+
// |      Awesome                   | ParagraphModel(CENTER, 0, 0, 0)
// |   Centered Title               |
// | Paragraph Detection            |
// |      OCR TEAM                  |
// |  10 November 2010              |
// |                                |
// |  Look here, I have a paragraph.| ParagraphModel(LEFT, 0, 20, 0)
// |This paragraph starts at the top|
// |of the page and takes 3 lines.  |
// |  Here I have a second paragraph| ParagraphModel(LEFT, 0, 20, 0)
// |which indicates that the first  |
// |paragraph is not a continuation |
// |from a previous page, as it is  |
// |indented just like this second  |
// |paragraph.                      |
// |   Here is a block quote. It    | ParagraphModel(LEFT, 30, 0, 0)
// |   looks like the prior text    |
// |   but it  is indented  more    |
// |   and is fully justified.      |
// |  So how does one deal with     | ParagraphModel(LEFT, 0, 20, 0)
// |centered text, block quotes,    |
// |normal paragraphs, and lists    |
// |like what follows?              |
// |1. Make a plan.                 | ParagraphModel(LEFT, 0, 0, 30)
// |2. Use a heuristic, for example,| ParagraphModel(LEFT, 0, 0, 30)
// |   looking for lines where the  |
// |   first word of the next line  |
// |   would fit on the previous    |
// |   line.                        |
// |8. Try to implement the plan in | ParagraphModel(LEFT, 0, 0, 30)
// |   Python and try it out.       |
// |4. Determine how to fix the     | ParagraphModel(LEFT, 0, 0, 30)
// |   mistakes.                    |
// |5. Repeat.                      | ParagraphModel(LEFT, 0, 0, 30)
// |  For extra painful penalty work| ParagraphModel(LEFT, 0, 20, 0)
// |you can try to identify source  |
// |code.  Ouch!                    |
// +--------------------------------+
class TESS_API ParagraphModel {
public:
  ParagraphModel(tesseract::ParagraphJustification justification, int margin, int first_indent,
                 int body_indent, int tolerance)
      : justification_(justification)
      , margin_(margin)
      , first_indent_(first_indent)
      , body_indent_(body_indent)
      , tolerance_(tolerance) {
    // Make one of {first_indent, body_indent} is 0.
    int added_margin = first_indent;
    if (body_indent < added_margin) {
      added_margin = body_indent;
    }
    margin_ += added_margin;
    first_indent_ -= added_margin;
    body_indent_ -= added_margin;
  }

  ParagraphModel()
      : justification_(tesseract::JUSTIFICATION_UNKNOWN)
      , margin_(0)
      , first_indent_(0)
      , body_indent_(0)
      , tolerance_(0) {}

  // ValidFirstLine() and ValidBodyLine() take arguments describing a text line
  // in a block of text which we are trying to model:
  //   lmargin, lindent:  these add up to the distance from the leftmost ink
  //                      in the text line to the surrounding text block's left
  //                      edge.
  //   rmargin, rindent:  these add up to the distance from the rightmost ink
  //                      in the text line to the surrounding text block's right
  //                      edge.
  // The caller determines the division between "margin" and "indent", which
  // only actually affect whether we think the line may be centered.
  //
  // If the amount of whitespace matches the amount of whitespace expected on
  // the relevant side of the line (within tolerance_) we say it matches.

  // Return whether a given text line could be a first paragraph line according
  // to this paragraph model.
  bool ValidFirstLine(int lmargin, int lindent, int rindent, int rmargin) const;

  // Return whether a given text line could be a first paragraph line according
  // to this paragraph model.
  bool ValidBodyLine(int lmargin, int lindent, int rindent, int rmargin) const;

  tesseract::ParagraphJustification justification() const {
    return justification_;
  }
  int margin() const {
    return margin_;
  }
  int first_indent() const {
    return first_indent_;
  }
  int body_indent() const {
    return body_indent_;
  }
  int tolerance() const {
    return tolerance_;
  }
  bool is_flush() const {
    return (justification_ == tesseract::JUSTIFICATION_LEFT ||
            justification_ == tesseract::JUSTIFICATION_RIGHT) &&
           abs(first_indent_ - body_indent_) <= tolerance_;
  }

  // Return whether this model is likely to agree with the other model on most
  // paragraphs they are marked.
  bool Comparable(const ParagraphModel &other) const;

  std::string ToString() const;

private:
  tesseract::ParagraphJustification justification_;
  int margin_;
  int first_indent_;
  int body_indent_;
  int tolerance_;
};

} // namespace tesseract

#endif // TESSERACT_CCSTRUCT_OCRPARA_H_
