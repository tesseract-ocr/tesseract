///////////////////////////////////////////////////////////////////////
// File:        renderer.cpp
// Description: Rendering interface to inject into TessBaseAPI
//
// (C) Copyright 2011, Google Inc.
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

#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include <cstring>
#include <memory>  // std::unique_ptr
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include <tesseract/renderer.h>

namespace tesseract {

/**********************************************************************
 * Base Renderer interface implementation
 **********************************************************************/
TessResultRenderer::TessResultRenderer(const char *outputbase,
                                       const char* extension)
    : file_extension_(extension),
      title_(""), imagenum_(-1),
      fout_(stdout),
      next_(nullptr),
      happy_(true) {
  if (strcmp(outputbase, "-") && strcmp(outputbase, "stdout")) {
    STRING outfile = STRING(outputbase) + STRING(".") + STRING(file_extension_);
    fout_ = fopen(outfile.c_str(), "wb");
    if (fout_ == nullptr) {
      happy_ = false;
    }
  }
}

TessResultRenderer::~TessResultRenderer() {
  if (fout_ != nullptr) {
    if (fout_ != stdout)
      fclose(fout_);
    else
      clearerr(fout_);
  }
  delete next_;
}

void TessResultRenderer::insert(TessResultRenderer* next) {
  if (next == nullptr) return;

  TessResultRenderer* remainder = next_;
  next_ = next;
  if (remainder) {
    while (next->next_ != nullptr) {
      next = next->next_;
    }
    next->next_ = remainder;
  }
}

bool TessResultRenderer::BeginDocument(const char* title) {
  if (!happy_) return false;
  title_ = title;
  imagenum_ = -1;
  bool ok = BeginDocumentHandler();
  if (next_) {
    ok = next_->BeginDocument(title) && ok;
  }
  return ok;
}

bool TessResultRenderer::AddImage(TessBaseAPI* api) {
  if (!happy_) return false;
  ++imagenum_;
  bool ok = AddImageHandler(api);
  if (next_) {
    ok = next_->AddImage(api) && ok;
  }
  return ok;
}

bool TessResultRenderer::EndDocument() {
  if (!happy_) return false;
  bool ok = EndDocumentHandler();
  if (next_) {
    ok = next_->EndDocument() && ok;
  }
  return ok;
}

void TessResultRenderer::AppendString(const char* s) {
  AppendData(s, strlen(s));
}

void TessResultRenderer::AppendData(const char* s, int len) {
  if (!tesseract::Serialize(fout_, s, len)) happy_ = false;
  fflush(fout_);
}

bool TessResultRenderer::BeginDocumentHandler() {
  return happy_;
}

bool TessResultRenderer::EndDocumentHandler() {
  return happy_;
}


/**********************************************************************
 * UTF8 Text Renderer interface implementation
 **********************************************************************/
TessTextRenderer::TessTextRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "txt") {
}

bool TessTextRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> utf8(api->GetUTF8Text());
  if (utf8 == nullptr) {
    return false;
  }

  AppendString(utf8.get());

  const char* pageSeparator = api->GetStringVariable("page_separator");
  if (pageSeparator != nullptr && *pageSeparator != '\0') {
    AppendString(pageSeparator);
  }

  return true;
}

/**********************************************************************
 * TSV Text Renderer interface implementation
 **********************************************************************/
TessTsvRenderer::TessTsvRenderer(const char* outputbase)
    : TessResultRenderer(outputbase, "tsv") {
  font_info_ = false;
}

TessTsvRenderer::TessTsvRenderer(const char* outputbase, bool font_info)
    : TessResultRenderer(outputbase, "tsv") {
  font_info_ = font_info;
}

bool TessTsvRenderer::BeginDocumentHandler() {
  // Output TSV column headings
  AppendString(
      "level\tpage_num\tblock_num\tpar_num\tline_num\tword_"
      "num\tleft\ttop\twidth\theight\tconf\ttext\n");
  return true;
}

bool TessTsvRenderer::EndDocumentHandler() { return true; }

bool TessTsvRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> tsv(api->GetTSVText(imagenum()));
  if (tsv == nullptr) return false;

  AppendString(tsv.get());

  return true;
}

/**********************************************************************
 * UNLV Text Renderer interface implementation
 **********************************************************************/
TessUnlvRenderer::TessUnlvRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "unlv") {
}

bool TessUnlvRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> unlv(api->GetUNLVText());
  if (unlv == nullptr) return false;

  AppendString(unlv.get());

  return true;
}

/**********************************************************************
 * BoxText Renderer interface implementation
 **********************************************************************/
TessBoxTextRenderer::TessBoxTextRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "box") {
}

bool TessBoxTextRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> text(api->GetBoxText(imagenum()));
  if (text == nullptr) return false;

  AppendString(text.get());

  return true;
}

#ifndef DISABLED_LEGACY_ENGINE

/**********************************************************************
 * Osd Text Renderer interface implementation
 **********************************************************************/
TessOsdRenderer::TessOsdRenderer(const char* outputbase)
    : TessResultRenderer(outputbase, "osd") {}

bool TessOsdRenderer::AddImageHandler(TessBaseAPI* api) {
  char* osd = api->GetOsdText(imagenum());
  if (osd == nullptr) return false;

  AppendString(osd);
  delete[] osd;

  return true;
}

#endif // ndef DISABLED_LEGACY_ENGINE

}  // namespace tesseract
