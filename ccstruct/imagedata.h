///////////////////////////////////////////////////////////////////////
// File:        imagedata.h
// Description: Class to hold information about a single image and its
//              corresponding boxes or text file.
// Author:      Ray Smith
// Created:     Mon Jul 22 14:17:06 PDT 2013
//
// (C) Copyright 2013, Google Inc.
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

#ifndef TESSERACT_IMAGE_IMAGEDATA_H_
#define TESSERACT_IMAGE_IMAGEDATA_H_


#include "genericvector.h"
#include "normalis.h"
#include "rect.h"
#include "strngs.h"

struct Pix;

namespace tesseract {

// Amount of padding to apply in output pixels in feature mode.
const int kFeaturePadding = 2;
// Number of pixels to pad around text boxes.
const int kImagePadding = 4;
// Number of training images to combine into a mini-batch for training.
const int kNumPagesPerMiniBatch = 100;

class WordFeature {
 public:
  WordFeature();
  WordFeature(const FCOORD& fcoord, uinT8 dir);

  // Computes the maximum x and y value in the features.
  static void ComputeSize(const GenericVector<WordFeature>& features,
                          int* max_x, int* max_y);
  // Draws the features in the given window.
  static void Draw(const GenericVector<WordFeature>& features,
                   ScrollView* window);

  // Accessors.
  int x() const { return x_; }
  int y() const { return y_; }
  int dir() const { return dir_; }

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

 private:
  inT16 x_;
  uinT8 y_;
  uinT8 dir_;
};

// A floating-point version of WordFeature, used as an intermediate during
// scaling.
struct FloatWordFeature {
  static void FromWordFeatures(const GenericVector<WordFeature>& word_features,
                               GenericVector<FloatWordFeature>* float_features);
  // Sort function to sort first by x-bucket, then by y.
  static int SortByXBucket(const void*, const void*);

  float x;
  float y;
  float dir;
  int x_bucket;
};

// Class to hold information on a single image:
// Filename, cached image as a Pix*, character boxes, text transcription.
// The text transcription is the ground truth UTF-8 text for the image.
// Character boxes are optional and indicate the desired segmentation of
// the text into recognition units.
class ImageData {
 public:
  ImageData();
  // Takes ownership of the pix.
  explicit ImageData(Pix* pix);
  ImageData(const GenericVector<WordFeature>& features,
            const GenericVector<TBOX>& boxes,
            const GenericVector<STRING>& texts);
  ~ImageData();

  // Builds and returns an ImageData from the basic data. Note that imagedata,
  // truth_text, and box_text are all the actual file data, NOT filenames.
  static ImageData* Build(const char* name, int page_number, const char* lang,
                          const char* imagedata, int imagedatasize,
                          const char* truth_text, const char* box_text);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(FILE* fp) const;
  // Reads from the given file. Returns false in case of error.
  // If swap is true, assumes a big/little-endian swap is needed.
  bool DeSerialize(bool swap, FILE* fp);

  // Other accessors.
  const STRING& imagefilename() const {
    return imagefilename_;
  }
  void set_imagefilename(const STRING& name) {
    imagefilename_ = name;
  }
  int page_number() const {
    return page_number_;
  }
  void set_page_number(int num) {
    page_number_ = num;
  }
  const GenericVector<char>& image_data() const {
    return image_data_;
  }
  const STRING& language() const {
    return language_;
  }
  void set_language(const STRING& lang) {
    language_ = lang;
  }
  const STRING& transcription() const {
    return transcription_;
  }
  const GenericVector<TBOX>& boxes() const {
    return boxes_;
  }
  const STRING& box_text(int index) const {
    return box_texts_[index];
  }
  const GenericVector<WordFeature>& features() const {
    return features_;
  }
  bool partial_boxes() const {
    return partial_boxes_;
  }
  // Saves the given Pix as a PNG-encoded string and destroys it.
  void SetPix(Pix* pix);
  // Returns the Pix image for *this. Must be pixDestroyed after use.
  Pix* GetPix() const;
  // Saves the given Pix as a PNG-encoded string and destroys it.
  void SetPix2(Pix* pix);
  // Saves the given PNG-encoded string as the secondary image data.
  void SetPix2Data(const char* data, int size);
  // Returns the Pix image for *this. Must be pixDestroyed after use.
  Pix* GetPix2() const;
  // Gets anything and everything with a non-NULL pointer, prescaled to a
  // given target_height (if 0, then the original image height), and aligned.
  // Also returns (if not NULL) the width and height of the scaled image.
  void PreScale(int target_height, Pix** pix,
                int* scaled_width, int* scaled_height,
                GenericVector<TBOX>* boxes) const;

  int MemoryUsed() const;

  // Draws the data in a new window.
  void Display() const;

 private:
  // Saves the given Pix as a PNG-encoded string and destroys it.
  static void SetPixInternal(Pix* pix, GenericVector<char>* image_data);
  // Returns the Pix image for the image_data. Must be pixDestroyed after use.
  static Pix* GetPixInternal(const GenericVector<char>& image_data);
  // Parses the text string as a box file and adds any discovered boxes that
  // match the page number. Returns false on error.
  bool AddBoxes(const char* box_text);
  // Adds the supplied boxes and transcriptions that correspond to the correct
  // page number.
  void AddBoxes(const GenericVector<TBOX>& boxes,
                const GenericVector<STRING>& texts,
                const GenericVector<int>& box_pages);

 private:
  STRING imagefilename_;             // File to read image from.
  inT32 page_number_;                // Page number if multi-page tif or -1.
  GenericVector<char> image_data_;   // PNG file data.
  STRING language_;                  // Language code for image.
  STRING transcription_;             // UTF-8 ground truth of image.
  GenericVector<TBOX> boxes_;        // If non-empty boxes of the image.
  GenericVector<STRING> box_texts_;  // String for text in each box.
  GenericVector<WordFeature> features_;
  GenericVector<char> side_data_;    // PNG file data.
  bool partial_boxes_;               // Box text disagrees with transcription.
};

// A collection of ImageData that knows roughly how much memory it is using.
class DocumentData {
 public:
  explicit DocumentData(const STRING& name);
  ~DocumentData();

  // Adds all the pages in the given lstmf filename to the cache. The reader
  // is used to read the file.
  bool LoadDocument(const char* filename, const char* lang, FileReader reader);

  // Adds the given page data to this document, counting up memory.
  void AddPageToDocument(ImageData* page);

  const STRING& document_name() const {
    return document_name_;
  }
  const PointerVector<ImageData>& pages() const {
    return pages_;
  }
  inT64 memory_used() const {
    return memory_used_;
  }
  // Takes ownership of the given page index. The page is made NULL in *this.
  ImageData* TakePage(int index) {
    ImageData* page = pages_[index];
    pages_[index] = NULL;
    return page;
  }

 private:
  // A name for this document.
  STRING document_name_;
  // A group of pages that corresponds in some loose way to a document.
  PointerVector<ImageData> pages_;
  // Total of all pix sizes in the document.
  inT64 memory_used_;
};

// A collection of DocumentData that knows roughly how much memory it is using.
class DocumentCache {
 public:
  explicit DocumentCache(inT64 max_memory);
  ~DocumentCache();

  // Adds all the documents in the list of filenames, couting memory.
  // The reader is used to read the files.
  bool LoadDocuments(const GenericVector<STRING>& filenames, const char* lang,
                     FileReader reader);

  // Adds document to the cache, throwing out other documents if needed.
  bool AddToCache(DocumentData* data);

  // Finds and returns a document by name.
  DocumentData* FindDocument(const STRING& document_name) const;

  // Returns a page by serial number, selecting them in a round-robin fashion
  // from all the documents.
  const ImageData* GetPageBySerial(int serial) const;

  const PointerVector<DocumentData>& documents() const {
    return documents_;
  }
  int total_pages() const {
    return total_pages_;
  }

 private:
  // A group of pages that corresponds in some loose way to a document.
  PointerVector<DocumentData> documents_;
  // Total of all pages.
  int total_pages_;
  // Total of all memory used by the cache.
  inT64 memory_used_;
  // Max memory allowed in this cache.
  inT64 max_memory_;
};

}  // namespace tesseract


#endif  // TESSERACT_IMAGE_IMAGEDATA_H_
