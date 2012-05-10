/**********************************************************************
 * File:        paragraphs.h
 * Description: Paragraph Detection data structures.
 * Author:      David Eger
 * Created:     25 February 2011
 *
 * (C) Copyright 2011, Google Inc.
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

#ifndef TESSERACT_CCMAIN_PARAGRAPHS_H_
#define TESSERACT_CCMAIN_PARAGRAPHS_H_

#include "rect.h"
#include "ocrpara.h"
#include "genericvector.h"
#include "strngs.h"


class WERD;
class UNICHARSET;

namespace tesseract {

class MutableIterator;

// This structure captures all information needed about a text line for the
// purposes of paragraph detection.  It is meant to be exceedingly light-weight
// so that we can easily test paragraph detection independent of the rest of
// Tesseract.
class RowInfo {
 public:
  // Constant data derived from Tesseract output.
  STRING text;        // the full UTF-8 text of the line.
  bool ltr;           // whether the majority of the text is left-to-right
                      // TODO(eger) make this more fine-grained.

  bool has_leaders;   // does the line contain leader dots (.....)?
  bool has_drop_cap;  // does the line have a drop cap?
  int pix_ldistance;  // distance to the left pblock boundary in pixels
  int pix_rdistance;  // distance to the right pblock boundary in pixels
  float pix_xheight;  // guessed xheight for the line
  int average_interword_space; // average space between words in pixels.

  int num_words;
  TBOX lword_box;     // in normalized (horiz text rows) space
  TBOX rword_box;     // in normalized (horiz text rows) space

  STRING lword_text;   // the UTF-8 text of the leftmost werd
  STRING rword_text;   // the UTF-8 text of the rightmost werd

  //   The text of a paragraph typically starts with the start of an idea and
  // ends with the end of an idea.  Here we define paragraph as something that
  // may have a first line indent and a body indent which may be different.
  // Typical words that start an idea are:
  //   1. Words in western scripts that start with
  //      a capital letter, for example "The"
  //   2. Bulleted or numbered list items, for
  //      example "2."
  // Typical words which end an idea are words ending in punctuation marks. In
  // this vocabulary, each list item is represented as a paragraph.
  bool lword_indicates_list_item;
  bool lword_likely_starts_idea;
  bool lword_likely_ends_idea;

  bool rword_indicates_list_item;
  bool rword_likely_starts_idea;
  bool rword_likely_ends_idea;
};

// Main entry point for Paragraph Detection Algorithm.
//
// Given a set of equally spaced textlines (described by row_infos),
// Split them into paragraphs.  See http://goto/paragraphstalk
//
// Output:
//   row_owners - one pointer for each row, to the paragraph it belongs to.
//   paragraphs - this is the actual list of PARA objects.
//   models - the list of paragraph models referenced by the PARA objects.
//            caller is responsible for deleting the models.
void DetectParagraphs(int debug_level,
                      GenericVector<RowInfo> *row_infos,
                      GenericVector<PARA *> *row_owners,
                      PARA_LIST *paragraphs,
                      GenericVector<ParagraphModel *> *models);

// Given a MutableIterator to the start of a block, run DetectParagraphs on
// that block and commit the results to the underlying ROW and BLOCK structs,
// saving the ParagraphModels in models.  Caller owns the models.
// We use unicharset during the function to answer questions such as "is the
// first letter of this word upper case?"
void DetectParagraphs(int debug_level,
                      bool after_text_recognition,
                      const MutableIterator *block_start,
                      GenericVector<ParagraphModel *> *models);

}  // namespace

#endif  // TESSERACT_CCMAIN_PARAGRAPHS_H_
