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
  image_data->image_data_.init_to_size(imagedatasize, 0);
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
// The return value is the scale factor that was applied to the image to
// achieve the target_height.
float ImageData::PreScale(int target_height, Pix** pix,
                          int* scaled_width, int* scaled_height,
                          GenericVector<TBOX>* boxes) const {
  int input_width = 0;
  int input_height = 0;
  Pix* src_pix = GetPix();
  ASSERT_HOST(src_pix != NULL);
  input_width = pixGetWidth(src_pix);
  input_height = pixGetHeight(src_pix);
  if (target_height == 0)
    target_height = input_height;
  float im_factor = static_cast<float>(target_height) / input_height;
  if (scaled_width != NULL)
    *scaled_width = IntCastRounded(im_factor * input_width);
  if (scaled_height != NULL)
    *scaled_height = target_height;
  if (pix != NULL) {
    // Get the scaled image.
    pixDestroy(pix);
    *pix = pixScale(src_pix, im_factor, im_factor);
    if (*pix == NULL) {
      tprintf("Scaling pix of size %d, %d by factor %g made null pix!!\n",
              input_width, input_height, im_factor);
    }
    if (scaled_width != NULL)
      *scaled_width = pixGetWidth(*pix);
    if (scaled_height != NULL)
      *scaled_height = pixGetHeight(*pix);
  }
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
  return im_factor;
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
  win->TextAttributes("Arial", kTextSize, false, false, false);
  for (int b = 0; b < boxes_.size(); ++b) {
    boxes_[b].plot(win);
    win->Text(boxes_[b].left(), height + kTextSize, box_texts_[b].string());
    TBOX scaled(boxes_[b]);
    scaled.scale(256.0 / height);
    scaled.plot(win);
  }
  // The full transcription.
  win->Pen(ScrollView::CYAN);
  win->Text(0, height + kTextSize * 2, transcription_.string());
  // Add the features.
  win->Pen(ScrollView::GREEN);
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
  image_data->init_to_size(size, 0);
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

DocumentData::DocumentData(const STRING& name)
  : document_name_(name), pages_offset_(0), total_pages_(0),
    memory_used_(0), max_memory_(0), reader_(NULL) {}

DocumentData::~DocumentData() {}

// Reads all the pages in the given lstmf filename to the cache. The reader
// is used to read the file.
bool DocumentData::LoadDocument(const char* filename, const char* lang,
                                int start_page, inT64 max_memory,
                                FileReader reader) {
  document_name_ = filename;
  lang_ = lang;
  pages_offset_ = start_page;
  max_memory_ = max_memory;
  reader_ = reader;
  return ReCachePages();
}

// Writes all the pages to the given filename. Returns false on error.
bool DocumentData::SaveDocument(const char* filename, FileWriter writer) {
  TFile fp;
  fp.OpenWrite(NULL);
  if (!pages_.Serialize(&fp) || !fp.CloseWrite(filename, writer)) {
    tprintf("Serialize failed: %s\n", filename);
    return false;
  }
  return true;
}
bool DocumentData::SaveToBuffer(GenericVector<char>* buffer) {
  TFile fp;
  fp.OpenWrite(buffer);
  return pages_.Serialize(&fp);
}

// Returns a pointer to the page with the given index, modulo the total
// number of pages, recaching if needed.
const ImageData* DocumentData::GetPage(int index) {
  index = Modulo(index, total_pages_);
  if (index < pages_offset_ || index >= pages_offset_ + pages_.size()) {
    pages_offset_ = index;
    if (!ReCachePages()) return NULL;
  }
  return pages_[index - pages_offset_];
}

// Loads as many pages can fit in max_memory_ starting at index pages_offset_.
bool DocumentData::ReCachePages() {
  // Read the file.
  TFile fp;
  if (!fp.Open(document_name_, reader_)) return false;
  memory_used_ = 0;
  if (!pages_.DeSerialize(false, &fp)) {
    tprintf("Deserialize failed: %s\n", document_name_.string());
    pages_.truncate(0);
    return false;
  }
  total_pages_ = pages_.size();
  pages_offset_ %= total_pages_;
  // Delete pages before the first one we want, and relocate the rest.
  int page;
  for (page = 0; page < pages_.size(); ++page) {
    if (page < pages_offset_) {
      delete pages_[page];
      pages_[page] = NULL;
    } else {
      ImageData* image_data = pages_[page];
      if (max_memory_ > 0 && page > pages_offset_ &&
          memory_used_ + image_data->MemoryUsed() > max_memory_)
        break;  // Don't go over memory quota unless the first image.
      if (image_data->imagefilename().length() == 0) {
        image_data->set_imagefilename(document_name_);
        image_data->set_page_number(page);
      }
      image_data->set_language(lang_);
      memory_used_ += image_data->MemoryUsed();
      if (pages_offset_ != 0) {
        pages_[page - pages_offset_] = image_data;
        pages_[page] = NULL;
      }
    }
  }
  pages_.truncate(page - pages_offset_);
  tprintf("Loaded %d/%d pages (%d-%d) of document %s\n",
          pages_.size(), total_pages_, pages_offset_,
          pages_offset_ + pages_.size(), document_name_.string());
  return !pages_.empty();
}

// Adds the given page data to this document, counting up memory.
void DocumentData::AddPageToDocument(ImageData* page) {
  pages_.push_back(page);
  memory_used_ += page->MemoryUsed();
}

// A collection of DocumentData that knows roughly how much memory it is using.
DocumentCache::DocumentCache(inT64 max_memory)
  : total_pages_(0), memory_used_(0), max_memory_(max_memory) {}
DocumentCache::~DocumentCache() {}

// Adds all the documents in the list of filenames, counting memory.
// The reader is used to read the files.
bool DocumentCache::LoadDocuments(const GenericVector<STRING>& filenames,
                                  const char* lang, FileReader reader) {
  inT64 fair_share_memory = max_memory_ / filenames.size();
  for (int arg = 0; arg < filenames.size(); ++arg) {
    STRING filename = filenames[arg];
    DocumentData* document = new DocumentData(filename);
    if (document->LoadDocument(filename.string(), lang, 0,
                               fair_share_memory, reader)) {
      AddToCache(document);
    } else {
      tprintf("Failed to load image %s!\n", filename.string());
      delete document;
    }
  }
  tprintf("Loaded %d pages, total %gMB\n",
          total_pages_, memory_used_ / 1048576.0);
  return total_pages_ > 0;
}

// Adds document to the cache, throwing out other documents if needed.
bool DocumentCache::AddToCache(DocumentData* data) {
  inT64 new_memory = data->memory_used();
  memory_used_ += new_memory;
  documents_.push_back(data);
  total_pages_ += data->NumPages();
  // Delete the first item in the array, and other pages of the same name
  // while memory is full.
  while (memory_used_ >= max_memory_ && max_memory_ > 0) {
    tprintf("Memory used=%lld vs max=%lld, discarding doc of size %lld\n",
            memory_used_ , max_memory_, documents_[0]->memory_used());
    memory_used_ -= documents_[0]->memory_used();
    total_pages_ -= documents_[0]->NumPages();
    documents_.remove(0);
  }
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

// Returns a page by serial number, selecting them in a round-robin fashion
// from all the documents.
const ImageData* DocumentCache::GetPageBySerial(int serial) {
  int document_index = serial % documents_.size();
  return documents_[document_index]->GetPage(serial / documents_.size());
}

}  // namespace tesseract.
