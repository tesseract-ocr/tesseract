// SPDX-License-Identifier: Apache-2.0
// File:        ltrresultiterator.h
// Description: Iterator for tesseract results in strict left-to-right
//              order that avoids using tesseract internal data structures.
// Author:      Ray Smith
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

#ifndef TESSERACT_CCMAIN_LTR_RESULT_ITERATOR_H_
#define TESSERACT_CCMAIN_LTR_RESULT_ITERATOR_H_

#include "export.h"       // for TESS_API
#include "pageiterator.h" // for PageIterator
#include "publictypes.h"  // for PageIteratorLevel
#include "unichar.h"      // for StrongScriptDirection

namespace tesseract {

class BLOB_CHOICE_IT;
class PAGE_RES;
class WERD_RES;

class Tesseract;

// Class to iterate over tesseract results, providing access to all levels
// of the page hierarchy, without including any tesseract headers or having
// to handle any tesseract structures.
// WARNING! This class points to data held within the TessBaseAPI class, and
// therefore can only be used while the TessBaseAPI class still exists and
// has not been subjected to a call of Init, SetImage, Recognize, Clear, End
// DetectOS, or anything else that changes the internal PAGE_RES.
// See tesseract/publictypes.h for the definition of PageIteratorLevel.
// See also base class PageIterator, which contains the bulk of the interface.
// LTRResultIterator adds text-specific methods for access to OCR output.

class TESS_API LTRResultIterator : public PageIterator {
  friend class ChoiceIterator;

public:
  // page_res and tesseract come directly from the BaseAPI.
  // The rectangle parameters are copied indirectly from the Thresholder,
  // via the BaseAPI. They represent the coordinates of some rectangle in an
  // original image (in top-left-origin coordinates) and therefore the top-left
  // needs to be added to any output boxes in order to specify coordinates
  // in the original image. See TessBaseAPI::SetRectangle.
  // The scale and scaled_yres are in case the Thresholder scaled the image
  // rectangle prior to thresholding. Any coordinates in tesseract's image
  // must be divided by scale before adding (rect_left, rect_top).
  // The scaled_yres indicates the effective resolution of the binary image
  // that tesseract has been given by the Thresholder.
  // After the constructor, Begin has already been called.
  LTRResultIterator(PAGE_RES *page_res, Tesseract *tesseract, int scale,
                    int scaled_yres, int rect_left, int rect_top,
                    int rect_width, int rect_height);

  ~LTRResultIterator() override;

  // LTRResultIterators may be copied! This makes it possible to iterate over
  // all the objects at a lower level, while maintaining an iterator to
  // objects at a higher level. These constructors DO NOT CALL Begin, so
  // iterations will continue from the location of src.
  // TODO: For now the copy constructor and operator= only need the base class
  // versions, but if new data members are added, don't forget to add them!

  // ============= Moving around within the page ============.

  // See PageIterator.

  // ============= Accessing data ==============.

  // Returns the null terminated UTF-8 encoded text string for the current
  // object at the given level. Use delete [] to free after use.
  char *GetUTF8Text(PageIteratorLevel level) const;

  // Set the string inserted at the end of each text line. "\n" by default.
  void SetLineSeparator(const char *new_line);

  // Set the string inserted at the end of each paragraph. "\n" by default.
  void SetParagraphSeparator(const char *new_para);

  // Returns the mean confidence of the current object at the given level.
  // The number should be interpreted as a percent probability. (0.0f-100.0f)
  float Confidence(PageIteratorLevel level) const;

  // ============= Functions that refer to words only ============.

  // Returns the font attributes of the current word. If iterating at a higher
  // level object than words, eg textlines, then this will return the
  // attributes of the first word in that textline.
  // The actual return value is a string representing a font name. It points
  // to an internal table and SHOULD NOT BE DELETED. Lifespan is the same as
  // the iterator itself, ie rendered invalid by various members of
  // TessBaseAPI, including Init, SetImage, End or deleting the TessBaseAPI.
  // Pointsize is returned in printers points (1/72 inch.)
  const char *WordFontAttributes(bool *is_bold, bool *is_italic,
                                 bool *is_underlined, bool *is_monospace,
                                 bool *is_serif, bool *is_smallcaps,
                                 int *pointsize, int *font_id) const;

  // Return the name of the language used to recognize this word.
  // On error, nullptr.  Do not delete this pointer.
  const char *WordRecognitionLanguage() const;

  // Return the overall directionality of this word.
  StrongScriptDirection WordDirection() const;

  // Returns true if the current word was found in a dictionary.
  bool WordIsFromDictionary() const;

  // Returns the number of blanks before the current word.
  int BlanksBeforeWord() const;

  // Returns true if the current word is numeric.
  bool WordIsNumeric() const;

  // Returns true if the word contains blamer information.
  bool HasBlamerInfo() const;

  // Returns the pointer to ParamsTrainingBundle stored in the BlamerBundle
  // of the current word.
  const void *GetParamsTrainingBundle() const;

  // Returns a pointer to the string with blamer information for this word.
  // Assumes that the word's blamer_bundle is not nullptr.
  const char *GetBlamerDebug() const;

  // Returns a pointer to the string with misadaption information for this word.
  // Assumes that the word's blamer_bundle is not nullptr.
  const char *GetBlamerMisadaptionDebug() const;

  // Returns true if a truth string was recorded for the current word.
  bool HasTruthString() const;

  // Returns true if the given string is equivalent to the truth string for
  // the current word.
  bool EquivalentToTruth(const char *str) const;

  // Returns a null terminated UTF-8 encoded truth string for the current word.
  // Use delete [] to free after use.
  char *WordTruthUTF8Text() const;

  // Returns a null terminated UTF-8 encoded normalized OCR string for the
  // current word. Use delete [] to free after use.
  char *WordNormedUTF8Text() const;

  // Returns a pointer to serialized choice lattice.
  // Fills lattice_size with the number of bytes in lattice data.
  const char *WordLattice(int *lattice_size) const;

  // ============= Functions that refer to symbols only ============.

  // Returns true if the current symbol is a superscript.
  // If iterating at a higher level object than symbols, eg words, then
  // this will return the attributes of the first symbol in that word.
  bool SymbolIsSuperscript() const;
  // Returns true if the current symbol is a subscript.
  // If iterating at a higher level object than symbols, eg words, then
  // this will return the attributes of the first symbol in that word.
  bool SymbolIsSubscript() const;
  // Returns true if the current symbol is a dropcap.
  // If iterating at a higher level object than symbols, eg words, then
  // this will return the attributes of the first symbol in that word.
  bool SymbolIsDropcap() const;

protected:
  const char *line_separator_;
  const char *paragraph_separator_;
};

// Class to iterate over the classifier choices for a single RIL_SYMBOL.
class TESS_API ChoiceIterator {
public:
  // Construction is from a LTRResultIterator that points to the symbol of
  // interest. The ChoiceIterator allows a one-shot iteration over the
  // choices for this symbol and after that it is useless.
  explicit ChoiceIterator(const LTRResultIterator &result_it);
  ~ChoiceIterator();

  // Moves to the next choice for the symbol and returns false if there
  // are none left.
  bool Next();

  // ============= Accessing data ==============.

  // Returns the null terminated UTF-8 encoded text string for the current
  // choice.
  // NOTE: Unlike LTRResultIterator::GetUTF8Text, the return points to an
  // internal structure and should NOT be delete[]ed to free after use.
  const char *GetUTF8Text() const;

  // Returns the confidence of the current choice depending on the used language
  // data. If only LSTM traineddata is used the value range is 0.0f - 1.0f. All
  // choices for one symbol should roughly add up to 1.0f.
  // If only traineddata of the legacy engine is used, the number should be
  // interpreted as a percent probability. (0.0f-100.0f) In this case
  // probabilities won't add up to 100. Each one stands on its own.
  float Confidence() const;

  // Returns a vector containing all timesteps, which belong to the currently
  // selected symbol. A timestep is a vector containing pairs of symbols and
  // floating point numbers. The number states the probability for the
  // corresponding symbol.
  std::vector<std::vector<std::pair<const char *, float>>> *Timesteps() const;

private:
  // clears the remaining spaces out of the results and adapt the probabilities
  void filterSpaces();
  // Pointer to the WERD_RES object owned by the API.
  WERD_RES *word_res_;
  // Iterator over the blob choices.
  BLOB_CHOICE_IT *choice_it_;
  std::vector<std::pair<const char *, float>> *LSTM_choices_ = nullptr;
  std::vector<std::pair<const char *, float>>::iterator LSTM_choice_it_;

  const int *tstep_index_;
  // regulates the rating granularity
  double rating_coefficient_;
  // leading blanks
  int blanks_before_word_;
  // true when there is lstm engine related trained data
  bool oemLSTM_;
};

} // namespace tesseract.

#endif // TESSERACT_CCMAIN_LTR_RESULT_ITERATOR_H_
