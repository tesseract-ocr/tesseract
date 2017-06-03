///////////////////////////////////////////////////////////////////////
// File:        imagedata.h
// Description: Class to hold information about a single multi-page tiff
//              training file and its corresponding boxes or text file.
// Author:      Ray Smith
// Created:     Tue May 28 08:56:06 PST 2013
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
#include "config_auto.h"
#endif

#include "imagedata.h"

#include "allheaders.h"
#include "boxread.h"
#include "callcpp.h"
#include "helpers.h"
#include "tprintf.h"

#if defined(__MINGW32__)
# include <unistd.h>
#elif __cplusplus > 199711L   // in C++11
# include <thread>
#endif

// Number of documents to read ahead while training. Doesn't need to be very
// large.
const int kMaxReadAhead = 8;

namespace tesseract {

WordFeature::WordFeature() : x_(0), y_(0), dir_(0) {
}

WordFeature::WordFeature(const FCOORD& fcoord, uinT8 dir)
  : x_(IntCastRounded(fcoord.x())),
    y_(ClipToRange(IntCastRounded(fcoord.y()), 0, MAX_UINT8)),
    dir_(dir) {
}

// Computes the maximum x and y value in the features.
void WordFeature::ComputeSize(const GenericVector<WordFeature>& features,
                              int* max_x, int* max_y) {
  *max_x = 0;
  *max_y = 0;
  for (int f = 0; f < features.size(); ++f) {
    if (features[f].x_ > *max_x) *max_x = features[f].x_;
    if (features[f].y_ > *max_y) *max_y = features[f].y_;
  }
}

// Draws the features in the given window.
void WordFeature::Draw(const GenericVector<WordFeature>& features,
                       ScrollView* window) {
#ifndef GRAPHICS_DISABLED
  for (int f = 0; f < features.size(); ++f) {
    FCOORD pos(features[f].x_, features[f].y_);
    FCOORD dir;
    dir.from_direction(features[f].dir_);
    dir *= 8.0f;
    window->SetCursor(IntCastRounded(pos.x() - dir.x()),
                      IntCastRounded(pos.y() - dir.y()));
    window->DrawTo(IntCastRounded(pos.x() + dir.x()),
                      IntCastRounded(pos.y() + dir.y()));
  }
#endif
}

// Writes to the given file. Returns false in case of error.
bool WordFeature::Serialize(FILE* fp) const {
  if (fwrite(&x_, sizeof(x_), 1, fp) != 1) return false;
  if (fwrite(&y_, sizeof(y_), 1, fp) != 1) return false;
  if (fwrite(&dir_, sizeof(dir_), 1, fp) != 1) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool WordFeature::DeSerialize(bool swap, FILE* fp) {
  if (fread(&x_, sizeof(x_), 1, fp) != 1) return false;
  if (swap) ReverseN(&x_, sizeof(x_));
  if (fread(&y_, sizeof(y_), 1, fp) != 1) return false;
  if (fread(&dir_, sizeof(dir_), 1, fp) != 1) return false;
  return true;
}

void FloatWordFeature::FromWordFeatures(
    const GenericVector<WordFeature>& word_features,
    GenericVector<FloatWordFeature>* float_features) {
  for (int i = 0; i < word_features.size(); ++i) {
    FloatWordFeature f;
    f.x = word_features[i].x();
    f.y = word_features[i].y();
    f.dir = word_features[i].dir();
    f.x_bucket = 0;  // Will set it later.
    float_features->push_back(f);
  }
}

// Sort function to sort first by x-bucket, then by y.
/* static */
int FloatWordFeature::SortByXBucket(const void* v1, const void* v2) {
  const FloatWordFeature* f1 = reinterpret_cast<const FloatWordFeature*>(v1);
  const FloatWordFeature* f2 = reinterpret_cast<const FloatWordFeature*>(v2);
  int x_diff = f1->x_bucket - f2->x_bucket;
  if (x_diff == 0) return f1->y - f2->y;
  return x_diff;
}

ImageData::ImageData() : page_number_(-1), vertical_text_(false) {
}
// Takes ownership of the pix and destroys it.
ImageData::ImageData(bool vertical, Pix* pix)
  : page_number_(0), vertical_text_(vertical) {
  SetPix(pix);
}
ImageData::~ImageData() {
}

// Builds and returns an ImageData from the basic data. Note that imagedata,
// truth_text, and box_text are all the actual file data, NOT filenames.
ImageData* ImageData::Build(const char* name, int page_number, const char* lang,
                            const char* imagedata, int imagedatasize,
                            const char* truth_text, const char* box_text) {
  ImageData* image_data = new ImageData();
  image_data->imagefilename_ = name;
  image_data->page_number_ = page_number;
  image_data->language_ = lang;
  // Save the imagedata.
  image_data->image_data_.resize_no_init(imagedatasize);
  memcpy(&image_data->image_data_[0], imagedata, imagedatasize);
  if (!image_data->AddBoxes(box_text)) {
    if (truth_text == NULL || truth_text[0] == '\0') {
      tprintf("Error: No text corresponding to page %d from image %s!\n",
              page_number, name);
      delete image_data;
      return NULL;
    }
    image_data->transcription_ = truth_text;
    // If we have no boxes, the transcription is in the 0th box_texts_.
    image_data->box_texts_.push_back(truth_text);
    // We will create a box for the whole image on PreScale, to save unpacking
    // the image now.
  } else if (truth_text != NULL && truth_text[0] != '\0' &&
             image_data->transcription_ != truth_text) {
    // Save the truth text as it is present and disagrees with the box text.
    image_data->transcription_ = truth_text;
  }
  return image_data;
}

// Writes to the given file. Returns false in case of error.
bool ImageData::Serialize(TFile* fp) const {
  if (!imagefilename_.Serialize(fp)) return false;
  if (fp->FWrite(&page_number_, sizeof(page_number_), 1) != 1) return false;
  if (!image_data_.Serialize(fp)) return false;
  if (!transcription_.Serialize(fp)) return false;
  // WARNING: Will not work across different endian machines.
  if (!boxes_.Serialize(fp)) return false;
  if (!box_texts_.SerializeClasses(fp)) return false;
  inT8 vertical = vertical_text_;
  if (fp->FWrite(&vertical, sizeof(vertical), 1) != 1) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool ImageData::DeSerialize(bool swap, TFile* fp) {
  if (!imagefilename_.DeSerialize(swap, fp)) return false;
  if (fp->FRead(&page_number_, sizeof(page_number_), 1) != 1) return false;
  if (swap) ReverseN(&page_number_, sizeof(page_number_));
  if (!image_data_.DeSerialize(swap, fp)) return false;
  if (!transcription_.DeSerialize(swap, fp)) return false;
  // WARNING: Will not work across different endian machines.
  if (!boxes_.DeSerialize(swap, fp)) return false;
  if (!box_texts_.DeSerializeClasses(swap, fp)) return false;
  inT8 vertical = 0;
  if (fp->FRead(&vertical, sizeof(vertical), 1) != 1) return false;
  vertical_text_ = vertical != 0;
  return true;
}

// As DeSerialize, but only seeks past the data - hence a static method.
bool ImageData::SkipDeSerialize(bool swap, TFile* fp) {
  if (!STRING::SkipDeSerialize(swap, fp)) return false;
  inT32 page_number;
  if (fp->FRead(&page_number, sizeof(page_number), 1) != 1) return false;
  if (!GenericVector<char>::SkipDeSerialize(swap, fp)) return false;
  if (!STRING::SkipDeSerialize(swap, fp)) return false;
  if (!GenericVector<TBOX>::SkipDeSerialize(swap, fp)) return false;
  if (!GenericVector<STRING>::SkipDeSerializeClasses(swap, fp)) return false;
  inT8 vertical = 0;
  return fp->FRead(&vertical, sizeof(vertical), 1) == 1;
}

// Saves the given Pix as a PNG-encoded string and destroys it.
void ImageData::SetPix(Pix* pix) {
  SetPixInternal(pix, &image_data_);
}

// Returns the Pix image for *this. Must be pixDestroyed after use.
Pix* ImageData::GetPix() const {
  return GetPixInternal(image_data_);
}

// Gets anything and everything with a non-NULL pointer, prescaled to a
// given target_height (if 0, then the original image height), and aligned.
// Also returns (if not NULL) the width and height of the scaled image.
// The return value is the scaled Pix, which must be pixDestroyed after use,
// and scale_factor (if not NULL) is set to the scale factor that was applied
// to the image to achieve the target_height.
Pix* ImageData::PreScale(int target_height, int max_height, float* scale_factor,
                         int* scaled_width, int* scaled_height,
                         GenericVector<TBOX>* boxes) const {
  int input_width = 0;
  int input_height = 0;
  Pix* src_pix = GetPix();
  ASSERT_HOST(src_pix != NULL);
  input_width = pixGetWidth(src_pix);
  input_height = pixGetHeight(src_pix);
  if (target_height == 0) {
    target_height = MIN(input_height, max_height);
  }
  float im_factor = static_cast<float>(target_height) / input_height;
  if (scaled_width != NULL)
    *scaled_width = IntCastRounded(im_factor * input_width);
  if (scaled_height != NULL)
    *scaled_height = target_height;
  // Get the scaled image.
  Pix* pix = pixScale(src_pix, im_factor, im_factor);
  if (pix == NULL) {
    tprintf("Scaling pix of size %d, %d by factor %g made null pix!!\n",
            input_width, input_height, im_factor);
  }
  if (scaled_width != NULL) *scaled_width = pixGetWidth(pix);
  if (scaled_height != NULL) *scaled_height = pixGetHeight(pix);
  pixDestroy(&src_pix);
  if (boxes != NULL) {
    // Get the boxes.
    boxes->truncate(0);
    for (int b = 0; b < boxes_.size(); ++b) {
      TBOX box = boxes_[b];
      box.scale(im_factor);
      boxes->push_back(box);
    }
    if (boxes->empty()) {
      // Make a single box for the whole image.
      TBOX box(0, 0, im_factor * input_width, target_height);
      boxes->push_back(box);
    }
  }
  if (scale_factor != NULL) *scale_factor = im_factor;
  return pix;
}

int ImageData::MemoryUsed() const {
  return image_data_.size();
}

// Draws the data in a new window.
void ImageData::Display() const {
#ifndef GRAPHICS_DISABLED
  const int kTextSize = 64;
  // Draw the image.
  Pix* pix = GetPix();
  if (pix == NULL) return;
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  ScrollView* win = new ScrollView("Imagedata", 100, 100,
                                   2 * (width + 2 * kTextSize),
                                   2 * (height + 4 * kTextSize),
                                   width + 10, height + 3 * kTextSize, true);
  win->Image(pix, 0, height - 1);
  pixDestroy(&pix);
  // Draw the boxes.
  win->Pen(ScrollView::RED);
  win->Brush(ScrollView::NONE);
  int text_size = kTextSize;
  if (!boxes_.empty() && boxes_[0].height() * 2 < text_size)
    text_size = boxes_[0].height() * 2;
  win->TextAttributes("Arial", text_size, false, false, false);
  if (!boxes_.empty()) {
    for (int b = 0; b < boxes_.size(); ++b) {
      boxes_[b].plot(win);
      win->Text(boxes_[b].left(), height + kTextSize, box_texts_[b].string());
    }
  } else {
    // The full transcription.
    win->Pen(ScrollView::CYAN);
    win->Text(0, height + kTextSize * 2, transcription_.string());
  }
  win->Update();
  window_wait(win);
#endif
}

// Adds the supplied boxes and transcriptions that correspond to the correct
// page number.
void ImageData::AddBoxes(const GenericVector<TBOX>& boxes,
                         const GenericVector<STRING>& texts,
                         const GenericVector<int>& box_pages) {
  // Copy the boxes and make the transcription.
  for (int i = 0; i < box_pages.size(); ++i) {
    if (page_number_ >= 0 && box_pages[i] != page_number_) continue;
    transcription_ += texts[i];
    boxes_.push_back(boxes[i]);
    box_texts_.push_back(texts[i]);
  }
}

// Saves the given Pix as a PNG-encoded string and destroys it.
void ImageData::SetPixInternal(Pix* pix, GenericVector<char>* image_data) {
  l_uint8* data;
  size_t size;
  pixWriteMem(&data, &size, pix, IFF_PNG);
  pixDestroy(&pix);
  image_data->resize_no_init(size);
  memcpy(&(*image_data)[0], data, size);
  free(data);
}

// Returns the Pix image for the image_data. Must be pixDestroyed after use.
Pix* ImageData::GetPixInternal(const GenericVector<char>& image_data) {
  Pix* pix = NULL;
  if (!image_data.empty()) {
    // Convert the array to an image.
    const unsigned char* u_data =
        reinterpret_cast<const unsigned char*>(&image_data[0]);
    pix = pixReadMem(u_data, image_data.size());
  }
  return pix;
}

// Parses the text string as a box file and adds any discovered boxes that
// match the page number. Returns false on error.
bool ImageData::AddBoxes(const char* box_text) {
  if (box_text != NULL && box_text[0] != '\0') {
    GenericVector<TBOX> boxes;
    GenericVector<STRING> texts;
    GenericVector<int> box_pages;
    if (ReadMemBoxes(page_number_, false, box_text, &boxes,
                     &texts, NULL, &box_pages)) {
      AddBoxes(boxes, texts, box_pages);
      return true;
    } else {
      tprintf("Error: No boxes for page %d from image %s!\n",
              page_number_, imagefilename_.string());
    }
  }
  return false;
}

// Thread function to call ReCachePages.
void* ReCachePagesFunc(void* data) {
  DocumentData* document_data = reinterpret_cast<DocumentData*>(data);
  document_data->ReCachePages();
  return NULL;
}

DocumentData::DocumentData(const STRING& name)
    : document_name_(name),
      pages_offset_(-1),
      total_pages_(-1),
      memory_used_(0),
      max_memory_(0),
      reader_(NULL) {}

DocumentData::~DocumentData() {
  SVAutoLock lock_p(&pages_mutex_);
  SVAutoLock lock_g(&general_mutex_);
}

// Reads all the pages in the given lstmf filename to the cache. The reader
// is used to read the file.
bool DocumentData::LoadDocument(const char* filename, const char* lang,
                                int start_page, inT64 max_memory,
                                FileReader reader) {
  SetDocument(filename, lang, max_memory, reader);
  pages_offset_ = start_page;
  return ReCachePages();
}

// Sets up the document, without actually loading it.
void DocumentData::SetDocument(const char* filename, const char* lang,
                               inT64 max_memory, FileReader reader) {
  SVAutoLock lock_p(&pages_mutex_);
  SVAutoLock lock(&general_mutex_);
  document_name_ = filename;
  lang_ = lang;
  pages_offset_ = -1;
  max_memory_ = max_memory;
  reader_ = reader;
}

// Writes all the pages to the given filename. Returns false on error.
bool DocumentData::SaveDocument(const char* filename, FileWriter writer) {
  SVAutoLock lock(&pages_mutex_);
  TFile fp;
  fp.OpenWrite(NULL);
  if (!pages_.Serialize(&fp) || !fp.CloseWrite(filename, writer)) {
    tprintf("Serialize failed: %s\n", filename);
    return false;
  }
  return true;
}
bool DocumentData::SaveToBuffer(GenericVector<char>* buffer) {
  SVAutoLock lock(&pages_mutex_);
  TFile fp;
  fp.OpenWrite(buffer);
  return pages_.Serialize(&fp);
}

// Adds the given page data to this document, counting up memory.
void DocumentData::AddPageToDocument(ImageData* page) {
  SVAutoLock lock(&pages_mutex_);
  pages_.push_back(page);
  set_memory_used(memory_used() + page->MemoryUsed());
}

// If the given index is not currently loaded, loads it using a separate
// thread.
void DocumentData::LoadPageInBackground(int index) {
  ImageData* page = NULL;
  if (IsPageAvailable(index, &page)) return;
  SVAutoLock lock(&pages_mutex_);
  if (pages_offset_ == index) return;
  pages_offset_ = index;
  pages_.clear();
  #ifndef GRAPHICS_DISABLED
  SVSync::StartThread(ReCachePagesFunc, this);
  #endif  // GRAPHICS_DISABLED
}

// Returns a pointer to the page with the given index, modulo the total
// number of pages. Blocks until the background load is completed.
const ImageData* DocumentData::GetPage(int index) {
  ImageData* page = NULL;
  while (!IsPageAvailable(index, &page)) {
    // If there is no background load scheduled, schedule one now.
    pages_mutex_.Lock();
    bool needs_loading = pages_offset_ != index;
    pages_mutex_.Unlock();
    if (needs_loading) LoadPageInBackground(index);
    // We can't directly load the page, or the background load will delete it
    // while the caller is using it, so give it a chance to work.
#if __cplusplus > 199711L && !defined(__MINGW32__)
    std::this_thread::sleep_for(std::chrono::seconds(1));
#elif _WIN32  // MSVS
    Sleep(1000);
#else
    sleep(1);
#endif
  }
  return page;
}

// Returns true if the requested page is available, and provides a pointer,
// which may be NULL if the document is empty. May block, even though it
// doesn't guarantee to return true.
bool DocumentData::IsPageAvailable(int index, ImageData** page) {
  SVAutoLock lock(&pages_mutex_);
  int num_pages = NumPages();
  if (num_pages == 0 || index < 0) {
    *page = NULL;  // Empty Document.
    return true;
  }
  if (num_pages > 0) {
    index = Modulo(index, num_pages);
    if (pages_offset_ <= index && index < pages_offset_ + pages_.size()) {
      *page = pages_[index - pages_offset_];  // Page is available already.
      return true;
    }
  }
  return false;
}

// Removes all pages from memory and frees the memory, but does not forget
// the document metadata.
inT64 DocumentData::UnCache() {
  SVAutoLock lock(&pages_mutex_);
  inT64 memory_saved = memory_used();
  pages_.clear();
  pages_offset_ = -1;
  set_total_pages(-1);
  set_memory_used(0);
  tprintf("Unloaded document %s, saving %d memory\n", document_name_.string(),
          memory_saved);
  return memory_saved;
}

// Locks the pages_mutex_ and Loads as many pages can fit in max_memory_
// starting at index pages_offset_.
bool DocumentData::ReCachePages() {
  SVAutoLock lock(&pages_mutex_);
  // Read the file.
  set_total_pages(0);
  set_memory_used(0);
  int loaded_pages = 0;
  pages_.truncate(0);
  TFile fp;
  if (!fp.Open(document_name_, reader_) ||
      !PointerVector<ImageData>::DeSerializeSize(false, &fp, &loaded_pages) ||
      loaded_pages <= 0) {
    tprintf("Deserialize header failed: %s\n", document_name_.string());
    return false;
  }
  pages_offset_ %= loaded_pages;
  // Skip pages before the first one we want, and load the rest until max
  // memory and skip the rest after that.
  int page;
  for (page = 0; page < loaded_pages; ++page) {
    if (page < pages_offset_ ||
        (max_memory_ > 0 && memory_used() > max_memory_)) {
      if (!PointerVector<ImageData>::DeSerializeSkip(false, &fp)) break;
    } else {
      if (!pages_.DeSerializeElement(false, &fp)) break;
      ImageData* image_data = pages_.back();
      if (image_data->imagefilename().length() == 0) {
        image_data->set_imagefilename(document_name_);
        image_data->set_page_number(page);
      }
      image_data->set_language(lang_);
      set_memory_used(memory_used() + image_data->MemoryUsed());
    }
  }
  if (page < loaded_pages) {
    tprintf("Deserialize failed: %s read %d/%d pages\n",
            document_name_.string(), page, loaded_pages);
    pages_.truncate(0);
  } else {
    tprintf("Loaded %d/%d pages (%d-%d) of document %s\n", pages_.size(),
            loaded_pages, pages_offset_, pages_offset_ + pages_.size(),
            document_name_.string());
  }
  set_total_pages(loaded_pages);
  return !pages_.empty();
}

// A collection of DocumentData that knows roughly how much memory it is using.
DocumentCache::DocumentCache(inT64 max_memory)
    : num_pages_per_doc_(0), max_memory_(max_memory) {}
DocumentCache::~DocumentCache() {}

// Adds all the documents in the list of filenames, counting memory.
// The reader is used to read the files.
bool DocumentCache::LoadDocuments(const GenericVector<STRING>& filenames,
                                  const char* lang,
                                  CachingStrategy cache_strategy,
                                  FileReader reader) {
  cache_strategy_ = cache_strategy;
  inT64 fair_share_memory = 0;
  // In the round-robin case, each DocumentData handles restricting its content
  // to its fair share of memory. In the sequential case, DocumentCache
  // determines which DocumentDatas are held entirely in memory.
  if (cache_strategy_ == CS_ROUND_ROBIN)
    fair_share_memory = max_memory_ / filenames.size();
  for (int arg = 0; arg < filenames.size(); ++arg) {
    STRING filename = filenames[arg];
    DocumentData* document = new DocumentData(filename);
    document->SetDocument(filename.string(), lang, fair_share_memory, reader);
    AddToCache(document);
  }
  if (!documents_.empty()) {
    // Try to get the first page now to verify the list of filenames.
    if (GetPageBySerial(0) != NULL) return true;
    tprintf("Load of page 0 failed!\n");
  }
  return false;
}

// Adds document to the cache.
bool DocumentCache::AddToCache(DocumentData* data) {
  inT64 new_memory = data->memory_used();
  documents_.push_back(data);
  return true;
}

// Finds and returns a document by name.
DocumentData* DocumentCache::FindDocument(const STRING& document_name) const {
  for (int i = 0; i < documents_.size(); ++i) {
    if (documents_[i]->document_name() == document_name)
      return documents_[i];
  }
  return NULL;
}

// Returns the total number of pages in an epoch. For CS_ROUND_ROBIN cache
// strategy, could take a long time.
int DocumentCache::TotalPages() {
  if (cache_strategy_ == CS_SEQUENTIAL) {
    // In sequential mode, we assume each doc has the same number of pages
    // whether it is true or not.
    if (num_pages_per_doc_ == 0) GetPageSequential(0);
    return num_pages_per_doc_ * documents_.size();
  }
  int total_pages = 0;
  int num_docs = documents_.size();
  for (int d = 0; d < num_docs; ++d) {
    // We have to load a page to make NumPages() valid.
    documents_[d]->GetPage(0);
    total_pages += documents_[d]->NumPages();
  }
  return total_pages;
}

// Returns a page by serial number, selecting them in a round-robin fashion
// from all the documents. Highly disk-intensive, but doesn't need samples
// to be shuffled between files to begin with.
const ImageData* DocumentCache::GetPageRoundRobin(int serial) {
  int num_docs = documents_.size();
  int doc_index = serial % num_docs;
  const ImageData* doc = documents_[doc_index]->GetPage(serial / num_docs);
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
const ImageData* DocumentCache::GetPageSequential(int serial) {
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
    if (serial / num_pages_per_doc_ % num_docs > 0) documents_[0]->UnCache();
  }
  int doc_index = serial / num_pages_per_doc_ % num_docs;
  const ImageData* doc =
      documents_[doc_index]->GetPage(serial % num_pages_per_doc_);
  // Count up total memory. Background loading makes it more complicated to
  // keep a running count.
  inT64 total_memory = 0;
  for (int d = 0; d < num_docs; ++d) {
    total_memory += documents_[d]->memory_used();
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
    if (!documents_[offset_index]->IsCached()) return offset - dir;
  }
  return num_docs;
}

}  // namespace tesseract.
