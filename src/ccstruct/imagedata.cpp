///////////////////////////////////////////////////////////////////////
// File:        imagedata.cpp
// Description: Class to hold information about a single multi-page tiff
//              training file and its corresponding boxes or text file.
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "imagedata.h"

#include "boxread.h"    // for ReadMemBoxes
#include "rect.h"       // for TBOX
#include "scrollview.h" // for ScrollView, ScrollView::CYAN, ScrollView::NONE
#include "tprintf.h"    // for tprintf
#include "tesserrstream.h" // for tesserr

#include "helpers.h"  // for IntCastRounded, TRand, ClipToRange, Modulo
#include "serialis.h" // for TFile

#include <allheaders.h> // for pixDestroy, pixGetHeight, pixGetWidth, lept_...

#include <cinttypes>    // for PRId64
#include <fstream>      // for std::ifstream

namespace tesseract {

// Number of documents to read ahead while training. Doesn't need to be very
// large.
const int kMaxReadAhead = 8;

ImageData::ImageData() : page_number_(-1), vertical_text_(false) {}
// Takes ownership of the pix and destroys it.
ImageData::ImageData(bool vertical, Image pix)
    : page_number_(0), vertical_text_(vertical) {
  SetPix(pix);
}
ImageData::~ImageData() {
#ifdef TESSERACT_IMAGEDATA_AS_PIX
  internal_pix_.destroy();
#endif
}

// Builds and returns an ImageData from the basic data. Note that imagedata,
// truth_text, and box_text are all the actual file data, NOT filenames.
ImageData *ImageData::Build(const char *name, int page_number, const char *lang,
                            const char *imagedata, int imagedatasize,
                            const char *truth_text, const char *box_text) {
  auto *image_data = new ImageData();
  image_data->imagefilename_ = name;
  image_data->page_number_ = page_number;
  image_data->language_ = lang;
  // Save the imagedata.
  // TODO: optimize resize (no init).
  image_data->image_data_.resize(imagedatasize);
  memcpy(&image_data->image_data_[0], imagedata, imagedatasize);
  if (!image_data->AddBoxes(box_text)) {
    if (truth_text == nullptr || truth_text[0] == '\0') {
      tprintf("Error: No text corresponding to page %d from image %s!\n",
              page_number, name);
      delete image_data;
      return nullptr;
    }
    image_data->transcription_ = truth_text;
    // If we have no boxes, the transcription is in the 0th box_texts_.
    image_data->box_texts_.emplace_back(truth_text);
    // We will create a box for the whole image on PreScale, to save unpacking
    // the image now.
  } else if (truth_text != nullptr && truth_text[0] != '\0' &&
             image_data->transcription_ != truth_text) {
    // Save the truth text as it is present and disagrees with the box text.
    image_data->transcription_ = truth_text;
  }
  return image_data;
}

// Writes to the given file. Returns false in case of error.
bool ImageData::Serialize(TFile *fp) const {
  if (!fp->Serialize(imagefilename_)) {
    return false;
  }
  if (!fp->Serialize(&page_number_)) {
    return false;
  }
  if (!fp->Serialize(image_data_)) {
    return false;
  }
  if (!fp->Serialize(language_)) {
    return false;
  }
  if (!fp->Serialize(transcription_)) {
    return false;
  }
  if (!fp->Serialize(boxes_)) {
    return false;
  }
  if (!fp->Serialize(box_texts_)) {
    return false;
  }
  int8_t vertical = vertical_text_;
  return fp->Serialize(&vertical);
}

// Reads from the given file. Returns false in case of error.
bool ImageData::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(imagefilename_)) {
    return false;
  }
  if (!fp->DeSerialize(&page_number_)) {
    return false;
  }
  if (!fp->DeSerialize(image_data_)) {
    return false;
  }
  if (!fp->DeSerialize(language_)) {
    return false;
  }
  if (!fp->DeSerialize(transcription_)) {
    return false;
  }
  if (!fp->DeSerialize(boxes_)) {
    return false;
  }
  if (!fp->DeSerialize(box_texts_)) {
    return false;
  }
  int8_t vertical = 0;
  if (!fp->DeSerialize(&vertical)) {
    return false;
  }
  vertical_text_ = vertical != 0;
  return true;
}

// As DeSerialize, but only seeks past the data - hence a static method.
bool ImageData::SkipDeSerialize(TFile *fp) {
  if (!fp->DeSerializeSkip()) {
    return false;
  }
  int32_t page_number;
  if (!fp->DeSerialize(&page_number)) {
    return false;
  }
  if (!fp->DeSerializeSkip()) {
    return false;
  }
  if (!fp->DeSerializeSkip()) {
    return false;
  }
  if (!fp->DeSerializeSkip()) {
    return false;
  }
  if (!fp->DeSerializeSkip(sizeof(TBOX))) {
    return false;
  }
  int32_t number;
  if (!fp->DeSerialize(&number)) {
    return false;
  }
  for (int i = 0; i < number; i++) {
    if (!fp->DeSerializeSkip()) {
      return false;
    }
  }
  int8_t vertical = 0;
  return fp->DeSerialize(&vertical);
}

// Saves the given Pix as a PNG-encoded string and destroys it.
// In case of missing PNG support in Leptonica use PNM format,
// which requires more memory.
void ImageData::SetPix(Image pix) {
#ifdef TESSERACT_IMAGEDATA_AS_PIX
  internal_pix_ = pix;
#else
  SetPixInternal(pix, &image_data_);
#endif
}

// Returns the Pix image for *this. Must be pixDestroyed after use.
Image ImageData::GetPix() const {
#ifdef TESSERACT_IMAGEDATA_AS_PIX
#  ifdef GRAPHICS_DISABLED
  /* The only caller of this is the scaling functions to prescale the
   * source. Thus we can just return a new pointer to the same data. */
  return internal_pix_.clone();
#  else
  /* pixCopy always does an actual copy, so the caller can modify the
   * changed data. */
  return internal_pix_.copy();
#  endif
#else
  return GetPixInternal(image_data_);
#endif
}

// Gets anything and everything with a non-nullptr pointer, prescaled to a
// given target_height (if 0, then the original image height), and aligned.
// Also returns (if not nullptr) the width and height of the scaled image.
// The return value is the scaled Pix, which must be pixDestroyed after use,
// and scale_factor (if not nullptr) is set to the scale factor that was applied
// to the image to achieve the target_height.
Image ImageData::PreScale(int target_height, int max_height,
                          float *scale_factor, int *scaled_width,
                          int *scaled_height, std::vector<TBOX> *boxes) const {
  int input_width = 0;
  int input_height = 0;
  Image src_pix = GetPix();
  ASSERT_HOST(src_pix != nullptr);
  input_width = pixGetWidth(src_pix);
  input_height = pixGetHeight(src_pix);
  if (target_height == 0) {
    target_height = std::min(input_height, max_height);
  }
  float im_factor = static_cast<float>(target_height) / input_height;
  if (scaled_width != nullptr) {
    *scaled_width = IntCastRounded(im_factor * input_width);
  }
  if (scaled_height != nullptr) {
    *scaled_height = target_height;
  }
  // Get the scaled image.
  Image pix = pixScale(src_pix, im_factor, im_factor);
  if (pix == nullptr) {
    tprintf("Scaling pix of size %d, %d by factor %g made null pix!!\n",
            input_width, input_height, im_factor);
    src_pix.destroy();
    return nullptr;
  }
  if (scaled_width != nullptr) {
    *scaled_width = pixGetWidth(pix);
  }
  if (scaled_height != nullptr) {
    *scaled_height = pixGetHeight(pix);
  }
  src_pix.destroy();
  if (boxes != nullptr) {
    // Get the boxes.
    boxes->clear();
    for (auto box : boxes_) {
      box.scale(im_factor);
      boxes->push_back(box);
    }
    if (boxes->empty()) {
      // Make a single box for the whole image.
      TBOX box(0, 0, im_factor * input_width, target_height);
      boxes->push_back(box);
    }
  }
  if (scale_factor != nullptr) {
    *scale_factor = im_factor;
  }
  return pix;
}

int ImageData::MemoryUsed() const {
  return image_data_.size();
}

#ifndef GRAPHICS_DISABLED

// Draws the data in a new window.
void ImageData::Display() const {
  const int kTextSize = 64;
  // Draw the image.
  Image pix = GetPix();
  if (pix == nullptr) {
    return;
  }
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  auto *win = new ScrollView("Imagedata", 100, 100, 2 * (width + 2 * kTextSize),
                             2 * (height + 4 * kTextSize), width + 10,
                             height + 3 * kTextSize, true);
  win->Draw(pix, 0, height - 1);
  pix.destroy();
  // Draw the boxes.
  win->Pen(ScrollView::RED);
  win->Brush(ScrollView::NONE);
  int text_size = kTextSize;
  if (!boxes_.empty() && boxes_[0].height() * 2 < text_size) {
    text_size = boxes_[0].height() * 2;
  }
  win->TextAttributes("Arial", text_size, false, false, false);
  if (!boxes_.empty()) {
    for (unsigned b = 0; b < boxes_.size(); ++b) {
      boxes_[b].plot(win);
      win->Text(boxes_[b].left(), height + kTextSize, box_texts_[b].c_str());
    }
  } else {
    // The full transcription.
    win->Pen(ScrollView::CYAN);
    win->Text(0, height + kTextSize * 2, transcription_.c_str());
  }
  win->Update();
  win->Wait();
}

#endif

// Adds the supplied boxes and transcriptions that correspond to the correct
// page number.
void ImageData::AddBoxes(const std::vector<TBOX> &boxes,
                         const std::vector<std::string> &texts,
                         const std::vector<int> &box_pages) {
  // Copy the boxes and make the transcription.
  for (unsigned i = 0; i < box_pages.size(); ++i) {
    if (page_number_ >= 0 && box_pages[i] != page_number_) {
      continue;
    }
    transcription_ += texts[i];
    boxes_.push_back(boxes[i]);
    box_texts_.push_back(texts[i]);
  }
}

#ifndef TESSERACT_IMAGEDATA_AS_PIX
// Saves the given Pix as a PNG-encoded string and destroys it.
// In case of missing PNG support in Leptonica use PNM format,
// which requires more memory.
void ImageData::SetPixInternal(Image pix, std::vector<char> *image_data) {
  l_uint8 *data;
  size_t size;
  l_int32 ret;
  ret = pixWriteMem(&data, &size, pix, IFF_PNG);
  if (ret) {
    ret = pixWriteMem(&data, &size, pix, IFF_PNM);
  }
  pix.destroy();
  // TODO: optimize resize (no init).
  image_data->resize(size);
  memcpy(&(*image_data)[0], data, size);
  lept_free(data);
}

// Returns the Pix image for the image_data. Must be pixDestroyed after use.
Image ImageData::GetPixInternal(const std::vector<char> &image_data) {
  Image pix = nullptr;
  if (!image_data.empty()) {
    // Convert the array to an image.
    const auto *u_data =
        reinterpret_cast<const unsigned char *>(&image_data[0]);
    pix = pixReadMem(u_data, image_data.size());
  }
  return pix;
}
#endif

// Parses the text string as a box file and adds any discovered boxes that
// match the page number. Returns false on error.
bool ImageData::AddBoxes(const char *box_text) {
  if (box_text != nullptr && box_text[0] != '\0') {
    std::vector<TBOX> boxes;
    std::vector<std::string> texts;
    std::vector<int> box_pages;
    if (ReadMemBoxes(page_number_, /*skip_blanks*/ false, box_text,
                     /*continue_on_failure*/ true, &boxes, &texts, nullptr,
                     &box_pages)) {
      AddBoxes(boxes, texts, box_pages);
      return true;
    } else {
      tprintf("Error: No boxes for page %d from image %s!\n", page_number_,
              imagefilename_.c_str());
    }
  }
  return false;
}

DocumentData::DocumentData(const std::string &name)
    : document_name_(name),
      pages_offset_(-1),
      total_pages_(-1),
      memory_used_(0),
      max_memory_(0),
      reader_(nullptr) {}

DocumentData::~DocumentData() {
  if (thread.joinable()) {
    thread.join();
  }
  std::lock_guard<std::mutex> lock_p(pages_mutex_);
  std::lock_guard<std::mutex> lock_g(general_mutex_);
  for (auto data : pages_) {
    delete data;
  }
}

// Reads all the pages in the given lstmf filename to the cache. The reader
// is used to read the file.
bool DocumentData::LoadDocument(const char *filename, int start_page,
                                int64_t max_memory, FileReader reader) {
  SetDocument(filename, max_memory, reader);
  pages_offset_ = start_page;
  return ReCachePages();
}

// Sets up the document, without actually loading it.
void DocumentData::SetDocument(const char *filename, int64_t max_memory,
                               FileReader reader) {
  std::lock_guard<std::mutex> lock_p(pages_mutex_);
  std::lock_guard<std::mutex> lock(general_mutex_);
  document_name_ = filename;
  pages_offset_ = -1;
  max_memory_ = max_memory;
  reader_ = reader;
}

// Writes all the pages to the given filename. Returns false on error.
bool DocumentData::SaveDocument(const char *filename, FileWriter writer) {
  std::lock_guard<std::mutex> lock(pages_mutex_);
  TFile fp;
  fp.OpenWrite(nullptr);
  if (!fp.Serialize(pages_) || !fp.CloseWrite(filename, writer)) {
    tprintf("Serialize failed: %s\n", filename);
    return false;
  }
  return true;
}

// Adds the given page data to this document, counting up memory.
void DocumentData::AddPageToDocument(ImageData *page) {
  std::lock_guard<std::mutex> lock(pages_mutex_);
  pages_.push_back(page);
  set_memory_used(memory_used() + page->MemoryUsed());
}

// If the given index is not currently loaded, loads it using a separate
// thread.
void DocumentData::LoadPageInBackground(int index) {
  ImageData *page = nullptr;
  if (IsPageAvailable(index, &page)) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(pages_mutex_);
    if (pages_offset_ == index) {
      return;
    }
    pages_offset_ = index;
    for (auto page : pages_) {
      delete page;
    }
    pages_.clear();
  }
  if (thread.joinable()) {
    thread.join();
  }
  // Don't run next statement asynchronously because that would
  // create too many threads on Linux (see issue #3111).
  ReCachePages();
}

// Returns a pointer to the page with the given index, modulo the total
// number of pages. Blocks until the background load is completed.
const ImageData *DocumentData::GetPage(int index) {
  ImageData *page = nullptr;
  while (!IsPageAvailable(index, &page)) {
    // If there is no background load scheduled, schedule one now.
    pages_mutex_.lock();
    bool needs_loading = pages_offset_ != index;
    pages_mutex_.unlock();
    if (needs_loading) {
      LoadPageInBackground(index);
    }
    // We can't directly load the page, or the background load will delete it
    // while the caller is using it, so give it a chance to work.
    std::this_thread::yield();
  }
  return page;
}

// Returns true if the requested page is available, and provides a pointer,
// which may be nullptr if the document is empty. May block, even though it
// doesn't guarantee to return true.
bool DocumentData::IsPageAvailable(int index, ImageData **page) {
  std::lock_guard<std::mutex> lock(pages_mutex_);
  int num_pages = NumPages();
  if (num_pages == 0 || index < 0) {
    *page = nullptr; // Empty Document.
    return true;
  }
  if (num_pages > 0) {
    index = Modulo(index, num_pages);
    if (pages_offset_ <= index &&
        static_cast<unsigned>(index) < pages_offset_ + pages_.size()) {
      *page = pages_[index - pages_offset_]; // Page is available already.
      return true;
    }
  }
  return false;
}

// Removes all pages from memory and frees the memory, but does not forget
// the document metadata.
int64_t DocumentData::UnCache() {
  std::lock_guard<std::mutex> lock(pages_mutex_);
  int64_t memory_saved = memory_used();
  for (auto page : pages_) {
    delete page;
  }
  pages_.clear();
  pages_offset_ = -1;
  set_total_pages(-1);
  set_memory_used(0);
  tprintf("Unloaded document %s, saving %" PRId64 " memory\n",
          document_name_.c_str(), memory_saved);
  return memory_saved;
}

// Shuffles all the pages in the document.
void DocumentData::Shuffle() {
  TRand random;
  // Different documents get shuffled differently, but the same for the same
  // name.
  std::hash<std::string> hasher;
  random.set_seed(static_cast<uint64_t>(hasher(document_name_)));
  int num_pages = pages_.size();
  // Execute one random swap for each page in the document.
  for (int i = 0; i < num_pages; ++i) {
    int src = random.IntRand() % num_pages;
    int dest = random.IntRand() % num_pages;
    std::swap(pages_[src], pages_[dest]);
  }
}

// Locks the pages_mutex_ and loads as many pages as will fit into max_memory_
// starting at index pages_offset_.
bool DocumentData::ReCachePages() {
  std::lock_guard<std::mutex> lock(pages_mutex_);
  // Read the file.
  set_total_pages(0);
  set_memory_used(0);
  int loaded_pages = 0;
  for (auto page : pages_) {
    delete page;
  }
  pages_.clear();
#if !defined(TESSERACT_IMAGEDATA_AS_PIX)
  auto name_size = document_name_.size();
  if (name_size > 4 && document_name_.substr(name_size - 4) == ".png") {
    // PNG image given instead of LSTMF file.
    std::string gt_name = document_name_.substr(0, name_size - 3) + "gt.txt";
    std::ifstream t(gt_name);
    std::string line;
    std::getline(t, line);
    t.close();
    ImageData *image_data = ImageData::Build(document_name_.c_str(), 0, "", nullptr, 0, line.c_str(), nullptr);
    Image image = pixRead(document_name_.c_str());
    image_data->SetPix(image);
    pages_.push_back(image_data);
    loaded_pages = 1;
    pages_offset_ %= loaded_pages;
    set_total_pages(loaded_pages);
    set_memory_used(memory_used() + image_data->MemoryUsed());
#if 0
    tprintf("Loaded %zu/%d lines (%d-%zu) of document %s\n", pages_.size(),
            loaded_pages, pages_offset_ + 1, pages_offset_ + pages_.size(),
            document_name_.c_str());
#endif
    return !pages_.empty();
  }
#endif
  TFile fp;
  if (!fp.Open(document_name_.c_str(), reader_) ||
      !fp.DeSerializeSize(&loaded_pages) || loaded_pages <= 0) {
    tprintf("Deserialize header failed: %s\n", document_name_.c_str());
    return false;
  }
  pages_offset_ %= loaded_pages;
  // Skip pages before the first one we want, and load the rest until max
  // memory and skip the rest after that.
  int page;
  for (page = 0; page < loaded_pages; ++page) {
    uint8_t non_null;
    if (!fp.DeSerialize(&non_null)) {
      break;
    }
    if (page < pages_offset_ ||
        (max_memory_ > 0 && memory_used() > max_memory_)) {
      if (non_null && !ImageData::SkipDeSerialize(&fp)) {
        break;
      }
    } else {
      ImageData *image_data = nullptr;
      if (non_null) {
        image_data = new ImageData;
        if (!image_data->DeSerialize(&fp)) {
          delete image_data;
          break;
        }
      }
      pages_.push_back(image_data);
      if (image_data->imagefilename().empty()) {
        image_data->set_imagefilename(document_name_);
        image_data->set_page_number(page);
      }
      set_memory_used(memory_used() + image_data->MemoryUsed());
    }
  }
  if (page < loaded_pages) {
    tprintf("Deserialize failed: %s read %d/%d lines\n", document_name_.c_str(),
            page, loaded_pages);
    for (auto page : pages_) {
      delete page;
    }
    pages_.clear();
  } else if (loaded_pages > 1) {
    // Avoid lots of messages for training with single line images.
    tesserr << "Loaded " << pages_.size() << '/' << loaded_pages << " lines ("
            << pages_offset_ + 1 << '-'
            << pages_offset_ + pages_.size() << ") of document "
            << document_name_ << '\n';
  }
  set_total_pages(loaded_pages);
  return !pages_.empty();
}

// A collection of DocumentData that knows roughly how much memory it is using.
DocumentCache::DocumentCache(int64_t max_memory) : max_memory_(max_memory) {}

DocumentCache::~DocumentCache() {
  for (auto *document : documents_) {
    delete document;
  }
}

// Adds all the documents in the list of filenames, counting memory.
// The reader is used to read the files.
bool DocumentCache::LoadDocuments(const std::vector<std::string> &filenames,
                                  CachingStrategy cache_strategy,
                                  FileReader reader) {
  cache_strategy_ = cache_strategy;
  int64_t fair_share_memory = 0;
  // In the round-robin case, each DocumentData handles restricting its content
  // to its fair share of memory. In the sequential case, DocumentCache
  // determines which DocumentDatas are held entirely in memory.
  if (cache_strategy_ == CS_ROUND_ROBIN) {
    fair_share_memory = max_memory_ / filenames.size();
  }
  for (const auto &filename : filenames) {
    auto *document = new DocumentData(filename);
    document->SetDocument(filename.c_str(), fair_share_memory, reader);
    AddToCache(document);
  }
  if (!documents_.empty()) {
    // Try to get the first page now to verify the list of filenames.
    if (GetPageBySerial(0) != nullptr) {
      return true;
    }
    tprintf("Load of page 0 failed!\n");
  }
  return false;
}

// Adds document to the cache.
bool DocumentCache::AddToCache(DocumentData *data) {
  documents_.push_back(data);
  return true;
}

// Finds and returns a document by name.
DocumentData *DocumentCache::FindDocument(
    const std::string &document_name) const {
  for (auto *document : documents_) {
    if (document->document_name() == document_name) {
      return document;
    }
  }
  return nullptr;
}

// Returns the total number of pages in an epoch. For CS_ROUND_ROBIN cache
// strategy, could take a long time.
int DocumentCache::TotalPages() {
  if (cache_strategy_ == CS_SEQUENTIAL) {
    // In sequential mode, we assume each doc has the same number of pages
    // whether it is true or not.
    if (num_pages_per_doc_ == 0) {
      GetPageSequential(0);
    }
    return num_pages_per_doc_ * documents_.size();
  }
  int total_pages = 0;
  for (auto *document : documents_) {
    // We have to load a page to make NumPages() valid.
    document->GetPage(0);
    total_pages += document->NumPages();
  }
  return total_pages;
}

// Returns a page by serial number, selecting them in a round-robin fashion
// from all the documents. Highly disk-intensive, but doesn't need samples
// to be shuffled between files to begin with.
const ImageData *DocumentCache::GetPageRoundRobin(int serial) {
  int num_docs = documents_.size();
  int doc_index = serial % num_docs;
  const ImageData *doc = documents_[doc_index]->GetPage(serial / num_docs);
  for (int offset = 1; offset <= kMaxReadAhead && offset < num_docs; ++offset) {
    doc_index = (serial + offset) % num_docs;
    int page = (serial + offset) / num_docs;
    documents_[doc_index]->LoadPageInBackground(page);
  }
  return doc;
}

// Returns a page by serial number, selecting them in sequence from each file.
// Requires the samples to be shuffled between the files to give a random or
// uniform distribution of data. Less disk-intensive than GetPageRoundRobin.
const ImageData *DocumentCache::GetPageSequential(int serial) {
  int num_docs = documents_.size();
  ASSERT_HOST(num_docs > 0);
  if (num_pages_per_doc_ == 0) {
    // Use the pages in the first doc as the number of pages in each doc.
    documents_[0]->GetPage(0);
    num_pages_per_doc_ = documents_[0]->NumPages();
    if (num_pages_per_doc_ == 0) {
      tprintf("First document cannot be empty!!\n");
      ASSERT_HOST(num_pages_per_doc_ > 0);
    }
    // Get rid of zero now if we don't need it.
    if (serial / num_pages_per_doc_ % num_docs > 0) {
      documents_[0]->UnCache();
    }
  }
  int doc_index = serial / num_pages_per_doc_ % num_docs;
  const ImageData *doc =
      documents_[doc_index]->GetPage(serial % num_pages_per_doc_);
  // Count up total memory. Background loading makes it more complicated to
  // keep a running count.
  int64_t total_memory = 0;
  for (auto *document : documents_) {
    total_memory += document->memory_used();
  }
  if (total_memory >= max_memory_) {
    // Find something to un-cache.
    // If there are more than 3 in front, then serial is from the back reader
    // of a pair of readers. If we un-cache from in-front-2 to 2-ahead, then
    // we create a hole between them and then un-caching the backmost occupied
    // will work for both.
    int num_in_front = CountNeighbourDocs(doc_index, 1);
    for (int offset = num_in_front - 2;
         offset > 1 && total_memory >= max_memory_; --offset) {
      int next_index = (doc_index + offset) % num_docs;
      total_memory -= documents_[next_index]->UnCache();
    }
    // If that didn't work, the best solution is to un-cache from the back. If
    // we take away the document that a 2nd reader is using, it will put it
    // back and make a hole between.
    int num_behind = CountNeighbourDocs(doc_index, -1);
    for (int offset = num_behind; offset < 0 && total_memory >= max_memory_;
         ++offset) {
      int next_index = (doc_index + offset + num_docs) % num_docs;
      total_memory -= documents_[next_index]->UnCache();
    }
  }
  int next_index = (doc_index + 1) % num_docs;
  if (!documents_[next_index]->IsCached() && total_memory < max_memory_) {
    documents_[next_index]->LoadPageInBackground(0);
  }
  return doc;
}

// Helper counts the number of adjacent cached neighbours of index looking in
// direction dir, ie index+dir, index+2*dir etc.
int DocumentCache::CountNeighbourDocs(int index, int dir) {
  int num_docs = documents_.size();
  for (int offset = dir; abs(offset) < num_docs; offset += dir) {
    int offset_index = (index + offset + num_docs) % num_docs;
    if (!documents_[offset_index]->IsCached()) {
      return offset - dir;
    }
  }
  return num_docs;
}

} // namespace tesseract.
