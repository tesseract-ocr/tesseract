///////////////////////////////////////////////////////////////////////
// File:        resultiterator.h
// Description: Iterator for tesseract results that is capable of
//              iterating in proper reading order over Bi Directional
//              (e.g. mixed Hebrew and English) text.
// Author:      David Eger
// Created:     Fri May 27 13:58:06 PST 2011
//
// (C) Copyright 2011, Google Inc.
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

#ifndef TESSERACT_CCMAIN_RESULT_ITERATOR_H__
#define TESSERACT_CCMAIN_RESULT_ITERATOR_H__

#include "platform.h"
#include "ltrresultiterator.h"

template <typename T> class GenericVector;
template <typename T> class GenericVectorEqEq;
class BLOB_CHOICE_IT;
class WERD_RES;
class STRING;

namespace tesseract {

class Tesseract;

class TESS_API ResultIterator : public LTRResultIterator {
 public:
  static ResultIterator *StartOfParagraph(const LTRResultIterator &resit);

  /**
   * ResultIterator is copy constructible!
   * The default copy constructor works just fine for us.
   */
  virtual ~ResultIterator() {}

  // ============= Moving around within the page ============.
  /** 
   * Moves the iterator to point to the start of the page to begin 
   * an iteration.
   */
  virtual void Begin();

  /**
   * Moves to the start of the next object at the given level in the
   * page hierarchy in the appropriate reading order and returns false if
   * the end of the page was reached.
   * NOTE that RIL_SYMBOL will skip non-text blocks, but all other
   * PageIteratorLevel level values will visit each non-text block once.
   * Think of non text blocks as containing a single para, with a single line,
   * with a single imaginary word.
   * Calls to Next with different levels may be freely intermixed.
   * This function iterates words in right-to-left scripts correctly, if
   * the appropriate language has been loaded into Tesseract.
   */
  virtual bool Next(PageIteratorLevel level);

  /**
   * IsAtBeginningOf() returns whether we're at the logical beginning of the
   * given level.  (as opposed to ResultIterator's left-to-right top-to-bottom
   * order).  Otherwise, this acts the same as PageIterator::IsAtBeginningOf().
   * For a full description, see pageiterator.h
   */
  virtual bool IsAtBeginningOf(PageIteratorLevel level) const;

  /**
   * Implement PageIterator's IsAtFinalElement correctly in a BiDi context.
   * For instance, IsAtFinalElement(RIL_PARA, RIL_WORD) returns whether we
   * point at the last word in a paragraph.  See PageIterator for full comment.
  */
  virtual bool IsAtFinalElement(PageIteratorLevel level,
                                PageIteratorLevel element) const;

  // ============= Accessing data ==============.

  /**
   * Returns the null terminated UTF-8 encoded text string for the current
   * object at the given level. Use delete [] to free after use.
  */
  virtual char* GetUTF8Text(PageIteratorLevel level) const;

  /**
   * Return whether the current paragraph's dominant reading direction
   * is left-to-right (as opposed to right-to-left).
  */
  bool ParagraphIsLtr() const;

  // ============= Exposed only for testing =============.

  /**
   * Yields the reading order as a sequence of indices and (optional)
   * meta-marks for a set of words (given left-to-right).
   * The meta marks are passed as negative values:
   *   kMinorRunStart  Start of minor direction text.
   *   kMinorRunEnd    End of minor direction text.
   *   kComplexWord    The next indexed word contains both left-to-right and
   *                    right-to-left characters and was treated as neutral.
   *
   * For example, suppose we have five words in a text line,
   * indexed [0,1,2,3,4] from the leftmost side of the text line.
   * The following are all believable reading_orders:
   *
   * Left-to-Right (in ltr paragraph):
   *     { 0, 1, 2, 3, 4 }
   * Left-to-Right (in rtl paragraph):
   *     { kMinorRunStart, 0, 1, 2, 3, 4, kMinorRunEnd }
   * Right-to-Left (in rtl paragraph):
   *     { 4, 3, 2, 1, 0 }
   * Left-to-Right except for an RTL phrase in words 2, 3 in an ltr paragraph:
   *     { 0, 1, kMinorRunStart, 3, 2, kMinorRunEnd, 4 }
   */
  static void CalculateTextlineOrder(
      bool paragraph_is_ltr,
      const GenericVector<StrongScriptDirection> &word_dirs,
      GenericVectorEqEq<int> *reading_order);

  static const int kMinorRunStart;
  static const int kMinorRunEnd;
  static const int kComplexWord;

 protected:
  /**
   * We presume the data associated with the given iterator will outlive us.
   * NB: This is private because it does something that is non-obvious:
   *   it resets to the beginning of the paragraph instead of staying wherever
   *   resit might have pointed.
   */
  TESS_LOCAL explicit ResultIterator(const LTRResultIterator &resit);

 private:
  /**
   * Calculates the current paragraph's dominant writing direction.
   * Typically, members should use current_paragraph_ltr_ instead.
   */
  bool CurrentParagraphIsLtr() const;

  /**
   * Returns word indices as measured from resit->RestartRow() = index 0
   * for the reading order of words within a textline given an iterator
   * into the middle of the text line.
   * In addition to non-negative word indices, the following negative values
   * may be inserted:
   *   kMinorRunStart  Start of minor direction text.
   *   kMinorRunEnd    End of minor direction text.
   *   kComplexWord    The previous word contains both left-to-right and
   *                   right-to-left characters and was treated as neutral.
   */
  void CalculateTextlineOrder(bool paragraph_is_ltr,
                              const LTRResultIterator &resit,
                              GenericVectorEqEq<int> *indices) const;
  /** Same as above, but the caller's ssd gets filled in if ssd != NULL. */
  void CalculateTextlineOrder(bool paragraph_is_ltr,
                              const LTRResultIterator &resit,
                              GenericVector<StrongScriptDirection> *ssd,
                              GenericVectorEqEq<int> *indices) const;

  /**
   * What is the index of the current word in a strict left-to-right reading
   * of the row?
   */
  int LTRWordIndex() const;

  /**
   * Given an iterator pointing at a word, returns the logical reading order
   * of blob indices for the word.
   */
  void CalculateBlobOrder(GenericVector<int> *blob_indices) const;

  /** Precondition: current_paragraph_is_ltr_ is set. */
  void MoveToLogicalStartOfTextline();

  /**
   * Precondition: current_paragraph_is_ltr_ and in_minor_direction_ 
   * are set.
   */
  void MoveToLogicalStartOfWord();

  /** Are we pointing at the final (reading order) symbol of the word? */
  bool IsAtFinalSymbolOfWord() const;

  /** Are we pointing at the first (reading order) symbol of the word? */
  bool IsAtFirstSymbolOfWord() const;

  /**
   * Append any extra marks that should be appended to this word when printed.
   * Mostly, these are Unicode BiDi control characters.
   */
  void AppendSuffixMarks(STRING *text) const;

  /** Appends the current word in reading order to the given buffer.*/
  void AppendUTF8WordText(STRING *text) const;

  /**
   * Appends the text of the current text line, *assuming this iterator is
   * positioned at the beginning of the text line*  This function
   * updates the iterator to point to the first position past the text line.
   * Each textline is terminated in a single newline character.
   * If the textline ends a paragraph, it gets a second terminal newline.
   */
  void IterateAndAppendUTF8TextlineText(STRING *text);

  /**
   * Appends the text of the current paragraph in reading order
   * to the given buffer.
   * Each textline is terminated in a single newline character, and the
   * paragraph gets an extra newline at the end.
   */
  void AppendUTF8ParagraphText(STRING *text) const;

  /** Returns whether the bidi_debug flag is set to at least min_level. */
  bool BidiDebug(int min_level) const;

  bool current_paragraph_is_ltr_;

  /**
   * Is the currently pointed-at character at the beginning of
   * a minor-direction run?
   */
  bool at_beginning_of_minor_run_;

  /** Is the currently pointed-at character in a minor-direction sequence? */
  bool in_minor_direction_;
};

}  // namespace tesseract.

#endif  // TESSERACT_CCMAIN_RESULT_ITERATOR_H__
