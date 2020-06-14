///////////////////////////////////////////////////////////////////////
// File:        imagedata.h
// Description: Class to hold information about a single image and its
//              corresponding boxes or text file.
// Author:      Ray Smith
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

#include <mutex>                // for std::mutex
#include <thread>               // for std::thread
#include <tesseract/genericvector.h>      // for GenericVector, PointerVector, FileReader
#include "points.h"             // for FCOORD
#include <tesseract/strngs.h>   // for STRING

class ScrollView;
class TBOX;
struct Pix;

namespace tesseract {

class TFile;

// Amount of padding to apply in output pixels in feature mode.
const int kFeaturePadding = 2;
// Number of pixels to pad around text boxes.
const int kImagePadding = 4;

// Enum to determine the caching and data sequencing strategy.
enum CachingStrategy {
  // Reads all of one file before moving on to the next. Requires samples to be
  // shuffled across files. Uses the count of samples in the first file as
  // the count in all the files to achieve high-speed random access. As a
  // consequence, if subsequent files are smaller, they get entries used more
  // than once, and if subsequent files are larger, some entries are not used.
  // Best for larger data sets that don't fit in memory.
  CS_SEQUENTIAL,
  // Reads one sample from each file in rotation. Does not require shuffled
  // samples, but is extremely disk-intensive. Samples in smaller files also
  // get used more often than samples in larger files.
  // Best for smaller data sets that mostly fit in memory.
  CS_ROUND_ROBIN,
};

class WordFeature {
 public:
  WordFeature();
  WordFeature(const FCOORD& fcoord, uint8_t dir);

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
  int16_t x_;
  uint8_t y_;
  uint8_t dir_;
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
  ImageData(bool vertical, Pix* pix);
  ~ImageData();

  // Builds and returns an ImageData from the basic data. Note that imagedata,
  // truth_text, and box_text are all the actual file data, NOT filenames.
  static ImageData* Build(const char* name, int page_number, const char* lang,
                          const char* imagedata, int imagedatasize,
                          const char* truth_text, const char* box_text);

  // Writes to the given file. Returns false in case of error.
  bool Serialize(TFile* fp) const;
  // Reads from the given file. Returns false in case of error.
  bool DeSerialize(TFile* fp);
  // As DeSerialize, but only seeks past the data - hence a static method.
  static bool SkipDeSerialize(TFile* fp);

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
  const GenericVector<STRING>& box_texts() const {
    return box_texts_;
  }
  const STRING& box_text(int index) const {
    return box_texts_[index];
  }
  // Saves the given Pix as a PNG-encoded string and destroys it.
  // In case of missing PNG support in Leptonica use PNM format,
  // which requires more memory.
  void SetPix(Pix* pix);
  // Returns the Pix image for *this. Must be pixDestroyed after use.
  Pix* GetPix() const;
  // Gets anything and everything with a non-nullptr pointer, prescaled to a
  // given target_height (if 0, then the original image height), and aligned.
  // Also returns (if not nullptr) the width and height of the scaled image.
  // The return value is the scaled Pix, which must be pixDestroyed after use,
  // and scale_factor (if not nullptr) is set to the scale factor that was applied
  // to the image to achieve the target_height.
  Pix* PreScale(int target_height, int max_height, float* scale_factor,
                int* scaled_width, int* scaled_height,
                GenericVector<TBOX>* boxes) const;

  int MemoryUsed() const;

  // Draws the data in a new window.
  void Display() const;

  // Adds the supplied boxes and transcriptions that correspond to the correct
  // page number.
  void AddBoxes(const GenericVector<TBOX>& boxes,
                const GenericVector<STRING>& texts,
                const GenericVector<int>& box_pages);

 private:
  // Saves the given Pix as a PNG-encoded string and destroys it.
  // In case of missing PNG support in Leptonica use PNM format,
  // which requires more memory.
  static void SetPixInternal(Pix* pix, GenericVector<char>* image_data);
  // Returns the Pix image for the image_data. Must be pixDestroyed after use.
  static Pix* GetPixInternal(const GenericVector<char>& image_data);
  // Parses the text string as a box file and adds any discovered boxes that
  // match the page number. Returns false on error.
  bool AddBoxes(const char* box_text);

 private:
  STRING imagefilename_;             // File to read image from.
  int32_t page_number_;              // Page number if multi-page tif or -1.
#ifdef TESSERACT_IMAGEDATA_AS_PIX
  Pix *internal_pix_;
#endif
  GenericVector<char> image_data_;   // PNG/PNM file data.
  STRING language_;                  // Language code for image.
  STRING transcription_;             // UTF-8 ground truth of image.
  GenericVector<TBOX> boxes_;        // If non-empty boxes of the image.
  GenericVector<STRING> box_texts_;  // String for text in each box.
  bool vertical_text_;               // Image has been rotated from vertical.
};

// A collection of ImageData that knows roughly how much memory it is using.
class DocumentData {
 public:
  explicit DocumentData(const STRING& name);
  ~DocumentData();

  // Reads all the pages in the given lstmf filename to the cache. The reader
  // is used to read the file.
  bool LoadDocument(const char* filename, int start_page, int64_t max_memory,
                    FileReader reader);
  // Sets up the document, without actually loading it.
  void SetDocument(const char* filename, int64_t max_memory, FileReader reader);
  // Writes all the pages to the given filename. Returns false on error.
  bool SaveDocument(const char* filename, FileWriter writer);
  bool SaveToBuffer(GenericVector<char>* buffer);

  // Adds the given page data to this document, counting up memory.
  void AddPageToDocument(ImageData* page);

  const STRING& document_name() const {
    std::lock_guard<std::mutex> lock(general_mutex_);
    return document_name_;
  }
  int NumPages() const {
    std::lock_guard<std::mutex> lock(general_mutex_);
    return total_pages_;
  }
  size_t PagesSize() const {
    return pages_.size();
  }
  int64_t memory_used() const {
    std::lock_guard<std::mutex> lock(general_mutex_);
    return memory_used_;
  }
  // If the given index is not currently loaded, loads it using a separate
  // thread. Note: there are 4 cases:
  // Document uncached: IsCached() returns false, total_pages_ < 0.
  // Required page is available: IsPageAvailable returns true. In this case,
  // total_pages_ > 0 and
  // pages_offset_ <= index%total_pages_ <= pages_offset_+pages_.size()
  // Pages are loaded, but the required one is not.
  // The requested page is being loaded by LoadPageInBackground. In this case,
  // index == pages_offset_. Once the loading starts, the pages lock is held
  // until it completes, at which point IsPageAvailable will unblock and return
  // true.
  void LoadPageInBackground(int index);
  // Returns a pointer to the page with the given index, modulo the total
  // number of pages. Blocks until the background load is completed.
  const ImageData* GetPage(int index);
  // Returns true if the requested page is available, and provides a pointer,
  // which may be nullptr if the document is empty. May block, even though it
  // doesn't guarantee to return true.
  bool IsPageAvailable(int index, ImageData** page);
  // Takes ownership of the given page index. The page is made nullptr in *this.
  ImageData* TakePage(int index) {
    std::lock_guard<std::mutex> lock(pages_mutex_);
    ImageData* page = pages_[index];
    pages_[index] = nullptr;
    return page;
  }
  // Returns true if the document is currently loaded or in the process of
  // loading.
  bool IsCached() const { return NumPages() >= 0; }
  // Removes all pages from memory and frees the memory, but does not forget
  // the document metadata. Returns the memory saved.
  int64_t UnCache();
  // Shuffles all the pages in the document.
  void Shuffle();

 private:
  // Sets the value of total_pages_ behind a mutex.
  void set_total_pages(int total) {
    std::lock_guard<std::mutex> lock(general_mutex_);
    total_pages_ = total;
  }
  void set_memory_used(int64_t memory_used) {
    std::lock_guard<std::mutex> lock(general_mutex_);
    memory_used_ = memory_used;
  }
  // Locks the pages_mutex_ and Loads as many pages can fit in max_memory_
  // starting at index pages_offset_.
  bool ReCachePages();

 private:
  // A name for this document.
  STRING document_name_;
  // A group of pages that corresponds in some loose way to a document.
  PointerVector<ImageData> pages_;
  // Page number of the first index in pages_.
  int pages_offset_;
  // Total number of pages in document (may exceed size of pages_.)
  int total_pages_;
  // Total of all pix sizes in the document.
  int64_t memory_used_;
  // Max memory to use at any time.
  int64_t max_memory_;
  // Saved reader from LoadDocument to allow re-caching.
  FileReader reader_;
  // Mutex that protects pages_ and pages_offset_ against multiple parallel
  // loads, and provides a wait for page.
  std::mutex pages_mutex_;
  // Mutex that protects other data members that callers want to access without
  // waiting for a load operation.
  mutable std::mutex general_mutex_;

  // Thread which loads document.
  std::thread thread;
};

// A collection of DocumentData that knows roughly how much memory it is using.
// Note that while it supports background read-ahead, it assumes that a single
// thread is accessing documents, ie it is not safe for multiple threads to
// access different documents in parallel, as one may de-cache the other's
// content.
class DocumentCache {
 public:
  explicit DocumentCache(int64_t max_memory);
  ~DocumentCache();

  // Deletes all existing documents from the cache.
  void Clear() {
    documents_.clear();
    num_pages_per_doc_ = 0;
  }
  // Adds all the documents in the list of filenames, counting memory.
  // The reader is used to read the files.
  bool LoadDocuments(const GenericVector<STRING>& filenames,
                     CachingStrategy cache_strategy, FileReader reader);

  // Adds document to the cache.
  bool AddToCache(DocumentData* data);

  // Finds and returns a document by name.
  DocumentData* FindDocument(const STRING& document_name) const;

  // Returns a page by serial number using the current cache_strategy_ to
  // determine the mapping from serial number to page.
  const ImageData* GetPageBySerial(int serial) {
    if (cache_strategy_ == CS_SEQUENTIAL)
      return GetPageSequential(serial);
    else
      return GetPageRoundRobin(serial);
  }

  const PointerVector<DocumentData>& documents() const {
    return documents_;
  }
  // Returns the total number of pages in an epoch. For CS_ROUND_ROBIN cache
  // strategy, could take a long time.
  int TotalPages();

 private:
  // Returns a page by serial number, selecting them in a round-robin fashion
  // from all the documents. Highly disk-intensive, but doesn't need samples
  // to be shuffled between files to begin with.
  const ImageData* GetPageRoundRobin(int serial);
  // Returns a page by serial number, selecting them in sequence from each file.
  // Requires the samples to be shuffled between the files to give a random or
  // uniform distribution of data. Less disk-intensive than GetPageRoundRobin.
  const ImageData* GetPageSequential(int serial);

  // Helper counts the number of adjacent cached neighbour documents_ of index
  // looking in direction dir, ie index+dir, index+2*dir etc.
  int CountNeighbourDocs(int index, int dir);

  // A group of pages that corresponds in some loose way to a document.
  PointerVector<DocumentData> documents_;
  // Strategy to use for caching and serializing data samples.
  CachingStrategy cache_strategy_;
  // Number of pages in the first document, used as a divisor in
  // GetPageSequential to determine the document index.
  int num_pages_per_doc_;
  // Max memory allowed in this cache.
  int64_t max_memory_;
};

}  // namespace tesseract


#endif  // TESSERACT_IMAGE_IMAGEDATA_H_
