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

ImageData::ImageData() : page_number_(-1), partial_boxes_(false) {
}
// Takes ownership of the pix and destroys it.
ImageData::ImageData(Pix* pix) : page_number_(0), partial_boxes_(false) {
  SetPix(pix);
}
ImageData::ImageData(const GenericVector<WordFeature>& features,
                     const GenericVector<TBOX>& boxes,
                     const GenericVector<STRING>& texts)
  : page_number_(0), boxes_(boxes), box_texts_(texts), features_(features),
    partial_boxes_(false) {
  for (int b = 0; b < box_texts_.size(); ++b)
    transcription_ += box_texts_[b];
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
  } else if (truth_text != NULL && truth_text[0] != '\0' &&
             image_data->transcription_ != truth_text) {
    // Save the truth text as it is present and disagrees with the box text.
    image_data->transcription_ = truth_text;
    image_data->partial_boxes_ = true;
  }
  return image_data;
}

// Writes to the given file. Returns false in case of error.
bool ImageData::Serialize(FILE* fp) const {
  if (!imagefilename_.Serialize(fp)) return false;
  if (fwrite(&page_number_, sizeof(page_number_), 1, fp) != 1) return false;
  if (!image_data_.Serialize(fp)) return false;
  if (!transcription_.Serialize(fp)) return false;
  // WARNING: Will not work across different endian machines.
  if (!boxes_.Serialize(fp)) return false;
  if (!box_texts_.SerializeClasses(fp)) return false;
  if (!features_.SerializeClasses(fp)) return false;
  if (!side_data_.Serialize(fp)) return false;
  return true;
}

// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool ImageData::DeSerialize(bool swap, FILE* fp) {
  if (!imagefilename_.DeSerialize(swap, fp)) return false;
  if (fread(&page_number_, sizeof(page_number_), 1, fp) != 1) return false;
  if (swap) ReverseN(&page_number_, sizeof(page_number_));
  if (!image_data_.DeSerialize(swap, fp)) return false;
  if (!transcription_.DeSerialize(swap, fp)) return false;
  // WARNING: Will not work across different endian machines.
  if (!boxes_.DeSerialize(swap, fp)) return false;
  if (!box_texts_.DeSerializeClasses(swap, fp)) return false;
  if (!features_.DeSerializeClasses(swap, fp)) return false;
  if (!side_data_.DeSerialize(swap, fp)) return false;
  STRING box_str;
  for (int i = 0; i < box_texts_.size(); ++i) {
    box_str += box_texts_[i];
  }
  partial_boxes_ = !box_texts_.empty() && transcription_ != box_str;
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

// Saves the given Pix as a PNG-encoded string and destroys it.
void ImageData::SetPix2(Pix* pix) {
  SetPixInternal(pix, &side_data_);
}

// Saves the given PNG-encoded string as the secondary image data.
void ImageData::SetPix2Data(const char* data, int size) {
  side_data_.init_to_size(size, 0);
  memcpy(&side_data_[0], data, size);
}

// Returns the Pix image for *this. Must be pixDestroyed after use.
Pix* ImageData::GetPix2() const {
  return GetPixInternal(side_data_);
}

// Gets anything and everything with a non-NULL pointer, prescaled to a
// given target_height (if 0, then the original image height), and aligned.
// Also returns (if not NULL) the width and height of the scaled image.
void ImageData::PreScale(int target_height, Pix** pix,
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
  }
}


int ImageData::MemoryUsed() const {
  return image_data_.size() + side_data_.size() +
      features_.size() * sizeof(WordFeature);
}

// Draws the data in a new window.
void ImageData::Display() const {
  const int kTextSize = 64;
  int x_max, y_max;
  WordFeature::ComputeSize(features_, &x_max, &y_max);
  ScrollView* win = new ScrollView("Imagedata", 100, 100,
                                   2 * (x_max + 2 * kTextSize),
                                   2 * (y_max + 4 * kTextSize),
                                   x_max + 10, y_max + 3 * kTextSize, true);
  // Draw the image.
  Pix* pix = GetPix();
  int height = 256;
  if (pix != NULL) {
    height = pixGetHeight(pix);
    win->Image(pix, 0, height - 1);
    pixDestroy(&pix);
  }
  // Draw the boxes.
  win->Pen(ScrollView::RED);
  win->Brush(ScrollView::NONE);
  win->TextAttributes("Arial", kTextSize, false, false, false);
  for (int b = 0; b < boxes_.size(); ++b) {
    boxes_[b].plot(win);
    win->Text(boxes_[b].left(), y_max + kTextSize, box_texts_[b].string());
    TBOX scaled(boxes_[b]);
    scaled.scale(256.0 / height);
    scaled.plot(win);
  }
  // The full transcription.
  win->Pen(ScrollView::CYAN);
  win->Text(0, y_max + kTextSize * 2, transcription_.string());
  // Add the features.
  win->Pen(ScrollView::GREEN);
  WordFeature::Draw(features_, win);
  win->Update();
  window_wait(win);
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

DocumentData::DocumentData(const STRING& name)
  : document_name_(name), memory_used_(0) {}

DocumentData::~DocumentData() {}

// Adds all the pages in the given lstmf filename to the cache. The reader
// is used to read the file.
bool DocumentData::LoadDocument(const char* filename, const char* lang,
                                FileReader reader) {
  // Read the file.
  GenericVector<char> file_data;
  if (!(*reader)(filename, &file_data)) {
    return false;
  }
  FILE* fp = fmemopen(&file_data[0], file_data.size(), "rb");
  document_name_ = filename;
  memory_used_ = 0;
  if (!pages_.DeSerialize(false, fp)) {
    tprintf("Deserialize failed: %s\n", filename);
    fclose(fp);
    pages_.truncate(0);
    return false;
  }
  // For each element in file_content, count memory and add additional data.
  for (int i = 0; i < pages_.size(); ++i) {
    ImageData* image_data = pages_[i];
    if (image_data->imagefilename().length() == 0) {
      image_data->set_imagefilename(filename);
      image_data->set_page_number(i);
    }
    image_data->set_language(lang);
    memory_used_ += image_data->MemoryUsed();
  }
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

// Adds all the documents in the list of filenames, couting memory.
// The reader is used to read the files.
bool DocumentCache::LoadDocuments(const GenericVector<STRING>& filenames,
                                  const char* lang, FileReader reader) {
  for (int arg = 0; arg < filenames.size(); ++arg) {
    STRING filename = filenames[arg] + ".lstmf";
    DocumentData* document = new DocumentData(filename);
    if (document->LoadDocument(filename.string(), lang, reader)) {
      AddToCache(document);
      tprintf("File %d, count=%d\n", arg, document->pages().size());
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
  total_pages_ += data->pages().size();
  // Delete the first item in the array, and other pages of the same name
  // while memory is full.
  while (memory_used_ >= max_memory_ && max_memory_ > 0) {
    tprintf("Memory used=%lld vs max=%lld, discarding doc of size %lld\n",
            memory_used_ , max_memory_, documents_[0]->memory_used());
    memory_used_ -= documents_[0]->memory_used();
    total_pages_ -= documents_[0]->pages().size();
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
const ImageData* DocumentCache::GetPageBySerial(int serial) const {
  int document_index = serial % documents_.size();
  const DocumentData& doc = *documents_[document_index];
  int page_index = serial % doc.pages().size();
  return doc.pages()[page_index];
}

}  // namespace tesseract.
