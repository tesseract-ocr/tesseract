/**********************************************************************
 * File:        werd.h
 * Description: Code for the WERD class.
 * Author:      Ray Smith
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

#ifndef WERD_H
#define WERD_H

#include "elst2.h"
#include "params.h"
#include "stepblob.h"

#include <bitset>

namespace tesseract {

enum WERD_FLAGS {
  W_SEGMENTED,          ///< correctly segmented
  W_ITALIC,             ///< italic text
  W_BOLD,               ///< bold text
  W_BOL,                ///< start of line
  W_EOL,                ///< end of line
  W_NORMALIZED,         ///< flags
  W_SCRIPT_HAS_XHEIGHT, ///< x-height concept makes sense.
  W_SCRIPT_IS_LATIN,    ///< Special case latin for y. splitting.
  W_DONT_CHOP,          ///< fixed pitch chopped
  W_REP_CHAR,           ///< repeated character
  W_FUZZY_SP,           ///< fuzzy space
  W_FUZZY_NON,          ///< fuzzy nonspace
  W_INVERSE             ///< white on black
};

enum DISPLAY_FLAGS {
  /* Display flags bit number allocations */
  DF_BOX,          ///< Bounding box
  DF_TEXT,         ///< Correct ascii
  DF_POLYGONAL,    ///< Polyg approx
  DF_EDGE_STEP,    ///< Edge steps
  DF_BN_POLYGONAL, ///< BL normalisd polyapx
  DF_BLAMER        ///< Blamer information
};

class ROW; // forward decl

class TESS_API WERD : public ELIST2_LINK {
public:
  WERD() = default;
  // WERD constructed with:
  //   blob_list - blobs of the word (we take this list's contents)
  //   blanks - number of blanks before the word
  //   text - correct text (outlives WERD)
  WERD(C_BLOB_LIST *blob_list, uint8_t blanks, const char *text);

  // WERD constructed from:
  //   blob_list - blobs in the word
  //   clone - werd to clone flags, etc from.
  WERD(C_BLOB_LIST *blob_list, WERD *clone);

  // Construct a WERD from a single_blob and clone the flags from this.
  // W_BOL and W_EOL flags are set according to the given values.
  WERD *ConstructFromSingleBlob(bool bol, bool eol, C_BLOB *blob);

  ~WERD() = default;

  // assignment
  WERD &operator=(const WERD &source);

  // This method returns a new werd constructed using the blobs in the input
  // all_blobs list, which correspond to the blobs in this werd object. The
  // blobs used to construct the new word are consumed and removed from the
  // input all_blobs list.
  // Returns nullptr if the word couldn't be constructed.
  // Returns original blobs for which no matches were found in the output list
  // orphan_blobs (appends).
  WERD *ConstructWerdWithNewBlobs(C_BLOB_LIST *all_blobs, C_BLOB_LIST *orphan_blobs);

  // Accessors for reject / DUFF blobs in various formats
  C_BLOB_LIST *rej_cblob_list() { // compact format
    return &rej_cblobs;
  }

  // Accessors for good blobs in various formats.
  C_BLOB_LIST *cblob_list() { // get compact blobs
    return &cblobs;
  }

  uint8_t space() const { // access function
    return blanks;
  }
  void set_blanks(uint8_t new_blanks) {
    blanks = new_blanks;
  }
  int script_id() const {
    return script_id_;
  }
  void set_script_id(int id) {
    script_id_ = id;
  }

  // Returns the (default) bounding box including all the dots.
  TBOX bounding_box() const; // compute bounding box
  // Returns the bounding box including the desired combination of upper and
  // lower noise/diacritic elements.
  TBOX restricted_bounding_box(bool upper_dots, bool lower_dots) const;
  // Returns the bounding box of only the good blobs.
  TBOX true_bounding_box() const;

  const char *text() const {
    return correct.c_str();
  }
  void set_text(const char *new_text) {
    correct = new_text;
  }

  bool flag(WERD_FLAGS mask) const {
    return flags[mask];
  }
  void set_flag(WERD_FLAGS mask, bool value) {
    flags.set(mask, value);
  }

  bool display_flag(uint8_t flag) const {
    return disp_flags[flag];
  }
  void set_display_flag(uint8_t flag, bool value) {
    disp_flags.set(flag, value);
  }

  WERD *shallow_copy(); // shallow copy word

  // reposition word by vector
  void move(const ICOORD vec);

  // join other's blobs onto this werd, emptying out other.
  void join_on(WERD *other);

  // copy other's blobs onto this word, leaving other intact.
  void copy_on(WERD *other);

  // tprintf word metadata (but not blob innards)
  void print() const;

#ifndef GRAPHICS_DISABLED
  // plot word on window in a uniform colour
  void plot(ScrollView *window, ScrollView::Color colour);

  // Get the next color in the (looping) rainbow.
  static ScrollView::Color NextColor(ScrollView::Color colour);

  // plot word on window in a rainbow of colours
  void plot(ScrollView *window);

  // plot rejected blobs in a rainbow of colours
  void plot_rej_blobs(ScrollView *window);
#endif // !GRAPHICS_DISABLED

  // Removes noise from the word by moving small outlines to the rej_cblobs
  // list, based on the size_threshold.
  void CleanNoise(float size_threshold);

  // Extracts all the noise outlines and stuffs the pointers into the given
  // vector of outlines. Afterwards, the outlines vector owns the pointers.
  void GetNoiseOutlines(std::vector<C_OUTLINE *> *outlines);
  // Adds the selected outlines to the indcated real blobs, and puts the rest
  // back in rej_cblobs where they came from. Where the target_blobs entry is
  // nullptr, a run of wanted outlines is put into a single new blob.
  // Ownership of the outlines is transferred back to the word. (Hence
  // vector and not PointerVector.)
  // Returns true if any new blob was added to the start of the word, which
  // suggests that it might need joining to the word before it, and likewise
  // sets make_next_word_fuzzy true if any new blob was added to the end.
  bool AddSelectedOutlines(const std::vector<bool> &wanted,
                           const std::vector<C_BLOB *> &target_blobs,
                           const std::vector<C_OUTLINE *> &outlines, bool *make_next_word_fuzzy);

private:
  uint8_t blanks = 0;     // no of blanks
  std::bitset<16> flags;  // flags about word
  std::bitset<16> disp_flags; // display flags
  int16_t script_id_ = 0; // From unicharset.
  std::string correct;    // correct text
  C_BLOB_LIST cblobs;     // compacted blobs
  C_BLOB_LIST rej_cblobs; // DUFF blobs
};

ELIST2IZEH(WERD)

} // namespace tesseract

#include "ocrrow.h" // placed here due to

namespace tesseract {

// compare words by increasing order of left edge, suitable for qsort(3)
int word_comparator(const void *word1p, const void *word2p);

} // namespace tesseract

#endif
