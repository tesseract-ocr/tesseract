///////////////////////////////////////////////////////////////////////
// File:        publictypes.h
// Description: Types used in both the API and internally
// Author:      Ray Smith
// Created:     Wed Mar 03 09:22:53 PST 2010
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

#ifndef TESSERACT_CCSTRUCT_PUBLICTYPES_H_
#define TESSERACT_CCSTRUCT_PUBLICTYPES_H_

// This file contains types that are used both by the API and internally
// to Tesseract. In order to decouple the API from Tesseract and prevent cyclic
// dependencies, THIS FILE SHOULD NOT DEPEND ON ANY OTHER PART OF TESSERACT.
// Restated: It is OK for low-level Tesseract files to include publictypes.h,
// but not for the low-level tesseract code to include top-level API code.
// This file should not use other Tesseract types, as that would drag
// their includes into the API-level.
// API-level code should include apitypes.h in preference to this file.

/** Number of printers' points in an inch. The unit of the pointsize return. */
constexpr int kPointsPerInch = 72;
/**
 * Minimum believable resolution. Used as a default if there is no other
 * information, as it is safer to under-estimate than over-estimate.
 */
constexpr int kMinCredibleResolution = 70;
/** Maximum believable resolution.  */
constexpr int kMaxCredibleResolution = 2400;
/**
 * Ratio between median blob size and likely resolution. Used to estimate
 * resolution when none is provided. This is basically 1/usual text size in
 * inches.  */
constexpr int kResolutionEstimationFactor = 10;

/**
 * Possible types for a POLY_BLOCK or ColPartition.
 * Must be kept in sync with kPBColors in polyblk.cpp and PTIs*Type functions
 * below, as well as kPolyBlockNames in publictypes.cpp.
 * Used extensively by ColPartition, and POLY_BLOCK.
*/
enum PolyBlockType {
  PT_UNKNOWN,        // Type is not yet known. Keep as the first element.
  PT_FLOWING_TEXT,   // Text that lives inside a column.
  PT_HEADING_TEXT,   // Text that spans more than one column.
  PT_PULLOUT_TEXT,   // Text that is in a cross-column pull-out region.
  PT_EQUATION,       // Partition belonging to an equation region.
  PT_INLINE_EQUATION,  // Partition has inline equation.
  PT_TABLE,          // Partition belonging to a table region.
  PT_VERTICAL_TEXT,  // Text-line runs vertically.
  PT_CAPTION_TEXT,   // Text that belongs to an image.
  PT_FLOWING_IMAGE,  // Image that lives inside a column.
  PT_HEADING_IMAGE,  // Image that spans more than one column.
  PT_PULLOUT_IMAGE,  // Image that is in a cross-column pull-out region.
  PT_HORZ_LINE,      // Horizontal Line.
  PT_VERT_LINE,      // Vertical Line.
  PT_NOISE,          // Lies outside of any column.
  PT_COUNT
};

/** Returns true if PolyBlockType is of horizontal line type */
inline bool PTIsLineType(PolyBlockType type) {
  return type == PT_HORZ_LINE || type == PT_VERT_LINE;
}
/** Returns true if PolyBlockType is of image type */
inline bool PTIsImageType(PolyBlockType type) {
  return type == PT_FLOWING_IMAGE || type == PT_HEADING_IMAGE ||
         type == PT_PULLOUT_IMAGE;
}
/** Returns true if PolyBlockType is of text type */
inline bool PTIsTextType(PolyBlockType type) {
  return type == PT_FLOWING_TEXT || type == PT_HEADING_TEXT ||
         type == PT_PULLOUT_TEXT || type == PT_TABLE ||
         type == PT_VERTICAL_TEXT || type == PT_CAPTION_TEXT ||
         type == PT_INLINE_EQUATION;
}
// Returns true if PolyBlockType is of pullout(inter-column) type
inline bool PTIsPulloutType(PolyBlockType type) {
  return type == PT_PULLOUT_IMAGE || type == PT_PULLOUT_TEXT;
}

/** String name for each block type. Keep in sync with PolyBlockType. */
extern const char* kPolyBlockNames[];

namespace tesseract {
/**
 *  +------------------+  Orientation Example:
 *  | 1 Aaaa Aaaa Aaaa |  ====================
 *  | Aaa aa aaa aa    |  To left is a diagram of some (1) English and
 *  | aaaaaa A aa aaa. |  (2) Chinese text and a (3) photo credit.
 *  |                2 |
 *  |   #######  c c C |  Upright Latin characters are represented as A and a.
 *  |   #######  c c c |  '<' represents a latin character rotated
 *  | < #######  c c c |      anti-clockwise 90 degrees.
 *  | < #######  c   c |
 *  | < #######  .   c |  Upright Chinese characters are represented C and c.
 *  | 3 #######      c |
 *  +------------------+  NOTA BENE: enum values here should match goodoc.proto

 * If you orient your head so that "up" aligns with Orientation,
 * then the characters will appear "right side up" and readable.
 *
 * In the example above, both the English and Chinese paragraphs are oriented
 * so their "up" is the top of the page (page up).  The photo credit is read
 * with one's head turned leftward ("up" is to page left).
 *
 * The values of this enum match the convention of Tesseract's osdetect.h
*/
enum Orientation {
  ORIENTATION_PAGE_UP = 0,
  ORIENTATION_PAGE_RIGHT = 1,
  ORIENTATION_PAGE_DOWN = 2,
  ORIENTATION_PAGE_LEFT = 3,
};

/**
 * The grapheme clusters within a line of text are laid out logically
 * in this direction, judged when looking at the text line rotated so that
 * its Orientation is "page up".
 *
 * For English text, the writing direction is left-to-right.  For the
 * Chinese text in the above example, the writing direction is top-to-bottom.
*/
enum WritingDirection {
  WRITING_DIRECTION_LEFT_TO_RIGHT = 0,
  WRITING_DIRECTION_RIGHT_TO_LEFT = 1,
  WRITING_DIRECTION_TOP_TO_BOTTOM = 2,
};

/**
 * The text lines are read in the given sequence.
 *
 * In English, the order is top-to-bottom.
 * In Chinese, vertical text lines are read right-to-left.  Mongolian is
 * written in vertical columns top to bottom like Chinese, but the lines
 * order left-to right.
 *
 * Note that only some combinations make sense.  For example,
 * WRITING_DIRECTION_LEFT_TO_RIGHT implies TEXTLINE_ORDER_TOP_TO_BOTTOM
*/
enum TextlineOrder {
  TEXTLINE_ORDER_LEFT_TO_RIGHT = 0,
  TEXTLINE_ORDER_RIGHT_TO_LEFT = 1,
  TEXTLINE_ORDER_TOP_TO_BOTTOM = 2,
};

/**
 * Possible modes for page layout analysis. These *must* be kept in order
 * of decreasing amount of layout analysis to be done, except for OSD_ONLY,
 * so that the inequality test macros below work.
*/
enum PageSegMode {
  PSM_OSD_ONLY,       ///< Orientation and script detection only.
  PSM_AUTO_OSD,       ///< Automatic page segmentation with orientation and
                      ///< script detection. (OSD)
  PSM_AUTO_ONLY,      ///< Automatic page segmentation, but no OSD, or OCR.
  PSM_AUTO,           ///< Fully automatic page segmentation, but no OSD.
  PSM_SINGLE_COLUMN,  ///< Assume a single column of text of variable sizes.
  PSM_SINGLE_BLOCK_VERT_TEXT,  ///< Assume a single uniform block of vertically
                               ///< aligned text.
  PSM_SINGLE_BLOCK,   ///< Assume a single uniform block of text. (Default.)
  PSM_SINGLE_LINE,    ///< Treat the image as a single text line.
  PSM_SINGLE_WORD,    ///< Treat the image as a single word.
  PSM_CIRCLE_WORD,    ///< Treat the image as a single word in a circle.
  PSM_SINGLE_CHAR,    ///< Treat the image as a single character.
  PSM_SPARSE_TEXT,    ///< Find as much text as possible in no particular order.
  PSM_SPARSE_TEXT_OSD,  ///< Sparse text with orientation and script det.
  PSM_RAW_LINE,       ///< Treat the image as a single text line, bypassing
                      ///< hacks that are Tesseract-specific.

  PSM_COUNT           ///< Number of enum entries.
};

/**
 * Inline functions that act on a PageSegMode to determine whether components of
 * layout analysis are enabled.
 * *Depend critically on the order of elements of PageSegMode.*
 * NOTE that arg is an int for compatibility with INT_PARAM.
*/
inline bool PSM_OSD_ENABLED(int pageseg_mode) {
  return pageseg_mode <= PSM_AUTO_OSD || pageseg_mode == PSM_SPARSE_TEXT_OSD;
}
inline bool PSM_ORIENTATION_ENABLED(int pageseg_mode) {
  return pageseg_mode <= PSM_AUTO || pageseg_mode == PSM_SPARSE_TEXT_OSD;
}
inline bool PSM_COL_FIND_ENABLED(int pageseg_mode) {
  return pageseg_mode >= PSM_AUTO_OSD && pageseg_mode <= PSM_AUTO;
}
inline bool PSM_SPARSE(int pageseg_mode) {
  return pageseg_mode == PSM_SPARSE_TEXT || pageseg_mode == PSM_SPARSE_TEXT_OSD;
}
inline bool PSM_BLOCK_FIND_ENABLED(int pageseg_mode) {
  return pageseg_mode >= PSM_AUTO_OSD && pageseg_mode <= PSM_SINGLE_COLUMN;
}
inline bool PSM_LINE_FIND_ENABLED(int pageseg_mode) {
  return pageseg_mode >= PSM_AUTO_OSD && pageseg_mode <= PSM_SINGLE_BLOCK;
}
inline bool PSM_WORD_FIND_ENABLED(int pageseg_mode) {
  return (pageseg_mode >= PSM_AUTO_OSD && pageseg_mode <= PSM_SINGLE_LINE) ||
      pageseg_mode == PSM_SPARSE_TEXT || pageseg_mode == PSM_SPARSE_TEXT_OSD;
}

/**
 * enum of the elements of the page hierarchy, used in ResultIterator
 * to provide functions that operate on each level without having to
 * have 5x as many functions.
*/
enum PageIteratorLevel {
  RIL_BLOCK,     // Block of text/image/separator line.
  RIL_PARA,      // Paragraph within a block.
  RIL_TEXTLINE,  // Line within a paragraph.
  RIL_WORD,      // Word within a textline.
  RIL_SYMBOL     // Symbol/character within a word.
};

/**
 * JUSTIFICATION_UNKNOWN
 *   The alignment is not clearly one of the other options.  This could happen
 *   for example if there are only one or two lines of text or the text looks
 *   like source code or poetry.
 *
 * NOTA BENE: Fully justified paragraphs (text aligned to both left and right
 *    margins) are marked by Tesseract with JUSTIFICATION_LEFT if their text
 *    is written with a left-to-right script and with JUSTIFICATION_RIGHT if
 *    their text is written in a right-to-left script.
 *
 * Interpretation for text read in vertical lines:
 *   "Left" is wherever the starting reading position is.
 *
 * JUSTIFICATION_LEFT
 *   Each line, except possibly the first, is flush to the same left tab stop.
 *
 * JUSTIFICATION_CENTER
 *   The text lines of the paragraph are centered about a line going
 *   down through their middle of the text lines.
 *
 * JUSTIFICATION_RIGHT
 *   Each line, except possibly the first, is flush to the same right tab stop.
 */
enum ParagraphJustification {
  JUSTIFICATION_UNKNOWN,
  JUSTIFICATION_LEFT,
  JUSTIFICATION_CENTER,
  JUSTIFICATION_RIGHT,
};

/**
 * When Tesseract/Cube is initialized we can choose to instantiate/load/run
 * only the Tesseract part, only the Cube part or both along with the combiner.
 * The preference of which engine to use is stored in tessedit_ocr_engine_mode.
 *
 * ATTENTION: When modifying this enum, please make sure to make the
 * appropriate changes to all the enums mirroring it (e.g. OCREngine in
 * cityblock/workflow/detection/detection_storage.proto). Such enums will
 * mention the connection to OcrEngineMode in the comments.
*/
enum OcrEngineMode {
  OEM_TESSERACT_ONLY,           // Run Tesseract only - fastest; deprecated
  OEM_LSTM_ONLY,                // Run just the LSTM line recognizer.
  OEM_TESSERACT_LSTM_COMBINED,  // Run the LSTM recognizer, but allow fallback
                                // to Tesseract when things get difficult.
                                // deprecated
  OEM_DEFAULT,                  // Specify this mode when calling init_*(),
                                // to indicate that any of the above modes
                                // should be automatically inferred from the
                                // variables in the language-specific config,
                                // command-line configs, or if not specified
                                // in any of the above should be set to the
                                // default OEM_TESSERACT_ONLY.
  OEM_COUNT                     // Number of OEMs
};

}  // namespace tesseract.

#endif  // TESSERACT_CCSTRUCT_PUBLICTYPES_H_
