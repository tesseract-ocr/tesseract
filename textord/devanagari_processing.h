// Copyright 2008 Google Inc. All Rights Reserved.
// Author: shobhitsaxena@google.com (Shobhit Saxena)

#ifndef TESSERACT_TEXTORD_DEVNAGARI_PROCESSING_H_
#define TESSERACT_TEXTORD_DEVNAGARI_PROCESSING_H_

#include "ocrblock.h"
#include "params.h"

struct Pix;
struct Box;
struct Boxa;

extern
INT_VAR_H(devanagari_split_debuglevel, 0,
          "Debug level for split shiro-rekha process.");

extern
BOOL_VAR_H(devanagari_split_debugimage, 0,
           "Whether to create a debug image for split shiro-rekha process.");

class TBOX;

namespace tesseract {

class PixelHistogram {
 public:
  PixelHistogram() {
    hist_ = NULL;
    length_ = 0;
  }

  ~PixelHistogram() {
    Clear();
  }

  void Clear() {
    if (hist_) {
      delete[] hist_;
    }
    length_ = 0;
  }

  int* hist() const {
    return hist_;
  }

  int length() const {
    return length_;
  }

  // Methods to construct histograms from images. These clear any existing data.
  void ConstructVerticalCountHist(Pix* pix);
  void ConstructHorizontalCountHist(Pix* pix);

  // This method returns the global-maxima for the histogram. The frequency of
  // the global maxima is returned in count, if specified.
  int GetHistogramMaximum(int* count) const;

 private:
  int* hist_;
  int length_;
};

class ShiroRekhaSplitter {
 public:
  enum SplitStrategy {
    NO_SPLIT = 0,   // No splitting is performed for the phase.
    MINIMAL_SPLIT,  // Blobs are split minimally.
    MAXIMAL_SPLIT   // Blobs are split maximally.
  };

  ShiroRekhaSplitter();
  virtual ~ShiroRekhaSplitter();

  // Top-level method to perform splitting based on current settings.
  // Returns true if a split was actually performed.
  // If split_for_pageseg is true, the pageseg_split_strategy_ is used for
  // splitting. If false, the ocr_split_strategy_ is used.
  bool Split(bool split_for_pageseg);

  // Clears the memory held by this object.
  void Clear();

  // Refreshes the words in the segmentation block list by using blobs in the
  // input blob list.
  // The segmentation block list must be set.
  void RefreshSegmentationWithNewBlobs(C_BLOB_LIST* new_blobs);

  // Returns true if the split strategies for pageseg and ocr are different.
  bool HasDifferentSplitStrategies() const {
    return pageseg_split_strategy_ != ocr_split_strategy_;
  }

  // This only keeps a copy of the block list pointer. At split call, the list
  // object should still be alive. This block list is used as a golden
  // segmentation when performing splitting.
  void set_segmentation_block_list(BLOCK_LIST* block_list) {
    segmentation_block_list_ = block_list;
  }

  static const int kUnspecifiedXheight = -1;

  void set_global_xheight(int xheight) {
    global_xheight_ = xheight;
  }

  void set_perform_close(bool perform) {
    perform_close_ = perform;
  }

  // Returns the image obtained from shiro-rekha splitting. The returned object
  // is owned by this class. Callers may want to clone the returned pix to keep
  // it alive beyond the life of ShiroRekhaSplitter object.
  Pix* splitted_image() {
    return splitted_image_;
  }

  // On setting the input image, a clone of it is owned by this class.
  void set_orig_pix(Pix* pix);

  // Returns the input image provided to the object. This object is owned by
  // this class. Callers may want to clone the returned pix to work with it.
  Pix* orig_pix() {
    return orig_pix_;
  }

  SplitStrategy ocr_split_strategy() const {
    return ocr_split_strategy_;
  }

  void set_ocr_split_strategy(SplitStrategy strategy) {
    ocr_split_strategy_ = strategy;
  }

  SplitStrategy pageseg_split_strategy() const {
    return pageseg_split_strategy_;
  }

  void set_pageseg_split_strategy(SplitStrategy strategy) {
    pageseg_split_strategy_ = strategy;
  }

  BLOCK_LIST* segmentation_block_list() {
    return segmentation_block_list_;
  }

  // This method dumps a debug image to the specified location.
  void DumpDebugImage(const char* filename) const;

  // This method returns the computed mode-height of blobs in the pix.
  // It also prunes very small blobs from calculation. Could be used to provide
  // a global xheight estimate for images which have the same point-size text.
  static int GetModeHeight(Pix* pix);

 private:
  // Method to perform a close operation on the input image. The xheight
  // estimate decides the size of sel used.
  static void PerformClose(Pix* pix, int xheight_estimate);

  // This method resolves the cc bbox to a particular row and returns the row's
  // xheight. This uses block_list_ if available, else just returns the
  // global_xheight_ estimate currently set in the object.
  int GetXheightForCC(Box* cc_bbox);

  // Returns a list of regions (boxes) which should be cleared in the original
  // image so as to perform shiro-rekha splitting. Pix is assumed to carry one
  // (or less) word only. Xheight measure could be the global estimate, the row
  // estimate, or unspecified. If unspecified, over splitting may occur, since a
  // conservative estimate of stroke width along with an associated multiplier
  // is used in its place. It is advisable to have a specified xheight when
  // splitting for classification/training.
  void SplitWordShiroRekha(SplitStrategy split_strategy,
                           Pix* pix,
                           int xheight,
                           int word_left,
                           int word_top,
                           Boxa* regions_to_clear);

  // Returns a new box object for the corresponding TBOX, based on the original
  // image's coordinate system.
  Box* GetBoxForTBOX(const TBOX& tbox) const;

  // This method returns y-extents of the shiro-rekha computed from the input
  // word image.
  static void GetShiroRekhaYExtents(Pix* word_pix,
                                    int* shirorekha_top,
                                    int* shirorekha_bottom,
                                    int* shirorekha_ylevel);

  Pix* orig_pix_;         // Just a clone of the input image passed.
  Pix* splitted_image_;   // Image produced after the last splitting round. The
                          // object is owned by this class.
  SplitStrategy pageseg_split_strategy_;
  SplitStrategy ocr_split_strategy_;
  Pix* debug_image_;
  // This block list is used as a golden segmentation when performing splitting.
  BLOCK_LIST* segmentation_block_list_;
  int global_xheight_;
  bool perform_close_;  // Whether a morphological close operation should be
                        // performed before CCs are run through splitting.
};

}  // namespace tesseract.

#endif  // TESSERACT_TEXTORD_DEVNAGARI_PROCESSING_H_
