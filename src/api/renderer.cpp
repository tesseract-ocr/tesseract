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
#  include "config_auto.h"
#endif
#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <cstring>
#include <memory>     // std::unique_ptr
#include <string>     // std::string
#include "serialis.h" // Serialize
#include "tprintf.h"

namespace tesseract {

/**********************************************************************
 * Base Renderer interface implementation
 **********************************************************************/
TessResultRenderer::TessResultRenderer(const char *outputbase, const char *extension)
    : next_(nullptr)
    , fout_(stdout)
    , file_extension_(extension)
    , title_("")
    , imagenum_(-1)
    , rendering_image_(nullptr)
    , rendering_dpi_(0)
    , happy_(true) {
  if (strcmp(outputbase, "-") && strcmp(outputbase, "stdout")) {
    std::string outfile = std::string(outputbase) + "." + extension;
    fout_ = fopen(outfile.c_str(), "wb");
    if (fout_ == nullptr) {
      happy_ = false;
    }
  }
}

TessResultRenderer::~TessResultRenderer() {
  if (fout_ != nullptr) {
    if (fout_ != stdout) {
      fclose(fout_);
    } else {
      clearerr(fout_);
    }
  }
  delete next_;
}

void TessResultRenderer::insert(TessResultRenderer *next) {
  if (next == nullptr) {
    return;
  }

  TessResultRenderer *remainder = next_;
  next_ = next;
  if (remainder) {
    while (next->next_ != nullptr) {
      next = next->next_;
    }
    next->next_ = remainder;
  }
}

bool TessResultRenderer::BeginDocument(const char *title) {
  if (!happy_) {
    return false;
  }
  title_ = title;
  imagenum_ = -1;
  bool ok = BeginDocumentHandler();
  if (next_) {
    ok = next_->BeginDocument(title) && ok;
  }
  return ok;
}

bool TessResultRenderer::AddImage(TessBaseAPI *api) {
  if (!happy_) {
    return false;
  }
  ++imagenum_;
  Pix *rendering_image_prev = rendering_image_;
  int rendering_dpi_prev = rendering_dpi_;
  bool ok = AddImageHandler(api);
  ResetRenderingState(rendering_image_prev, rendering_dpi_prev);
  if (next_) {
    ok = next_->AddImage(api) && ok;
  }
  return ok;
}

void TessResultRenderer::ResetRenderingState(Pix *rendering_image_prev,
                                             int rendering_dpi_prev) {
  if (rendering_image_ != rendering_image_prev) {
    pixDestroy(&rendering_image_);
    rendering_image_ = rendering_image_prev;
  }
  if (rendering_dpi_ != rendering_dpi_prev) {
    rendering_dpi_ = rendering_dpi_prev;
  }
}

Pix *TessResultRenderer::GetRenderingImage(TessBaseAPI *api) {
  if (!rendering_image_) {
    Pix *source_image = api->GetInputImage();
    int source_dpi = api->GetSourceYResolution();
    if (!source_image || source_dpi <= 0) {
      happy_ = false;
      return nullptr;
    }

    int rendering_dpi = GetRenderingResolution(api);
    if (rendering_dpi != source_dpi) {
      float scale = (float)rendering_dpi / (float)source_dpi;

      rendering_image_ = pixScale(source_image, scale, scale);
    } else {
      return source_image;
    }
  }
  return rendering_image_;
}

int TessResultRenderer::GetRenderingResolution(tesseract::TessBaseAPI *api) {
  if (rendering_dpi_) {
    return rendering_dpi_;
  }
  int source_dpi = api->GetSourceYResolution();
  int rendering_dpi;
  if (api->GetIntVariable("rendering_dpi", &rendering_dpi) &&
      rendering_dpi > 0 && rendering_dpi != source_dpi) {
    if (rendering_dpi < kMinCredibleResolution ||
        rendering_dpi > kMaxCredibleResolution) {
#if !defined(NDEBUG)
      tprintf(
          "Warning: User defined rendering dpi %d is outside of expected range "
          "(%d - %d)!\n",
          rendering_dpi, kMinCredibleResolution, kMaxCredibleResolution);
#endif
    }
    rendering_dpi_ = rendering_dpi;
    return rendering_dpi_;
  }
  return source_dpi;
}

bool TessResultRenderer::EndDocument() {
  if (!happy_) {
    return false;
  }
  bool ok = EndDocumentHandler();
  if (next_) {
    ok = next_->EndDocument() && ok;
  }
  return ok;
}

void TessResultRenderer::AppendString(const char *s) {
  if (s == nullptr) {
    return;
  }
  AppendData(s, strlen(s));
}

void TessResultRenderer::AppendData(const char *s, int len) {
  if (!tesseract::Serialize(fout_, s, len)) {
    happy_ = false;
  }
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
    : TessResultRenderer(outputbase, "txt") {}

bool TessTextRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> utf8(api->GetUTF8Text());
  if (utf8 == nullptr) {
    return false;
  }

  const char *pageSeparator = api->GetStringVariable("page_separator");
  if (pageSeparator != nullptr && *pageSeparator != '\0' && imagenum() > 0) {
    AppendString(pageSeparator);
  }

  AppendString(utf8.get());

  return true;
}

/**********************************************************************
 * TSV Text Renderer interface implementation
 **********************************************************************/
TessTsvRenderer::TessTsvRenderer(const char *outputbase) : TessResultRenderer(outputbase, "tsv") {
  font_info_ = false;
}

TessTsvRenderer::TessTsvRenderer(const char *outputbase, bool font_info)
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

bool TessTsvRenderer::EndDocumentHandler() {
  return true;
}

bool TessTsvRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> tsv(api->GetTSVText(imagenum()));
  if (tsv == nullptr) {
    return false;
  }

  AppendString(tsv.get());

  return true;
}

/**********************************************************************
 * UNLV Text Renderer interface implementation
 **********************************************************************/
TessUnlvRenderer::TessUnlvRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "unlv") {}

bool TessUnlvRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> unlv(api->GetUNLVText());
  if (unlv == nullptr) {
    return false;
  }

  AppendString(unlv.get());

  return true;
}

/**********************************************************************
 * BoxText Renderer interface implementation
 **********************************************************************/
TessBoxTextRenderer::TessBoxTextRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "box") {}

bool TessBoxTextRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> text(api->GetBoxText(imagenum()));
  if (text == nullptr) {
    return false;
  }

  AppendString(text.get());

  return true;
}

#ifndef DISABLED_LEGACY_ENGINE

/**********************************************************************
 * Osd Text Renderer interface implementation
 **********************************************************************/
TessOsdRenderer::TessOsdRenderer(const char *outputbase) : TessResultRenderer(outputbase, "osd") {}

bool TessOsdRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> osd(api->GetOsdText(imagenum()));
  if (osd == nullptr) {
    return false;
  }

  AppendString(osd.get());

  return true;
}

#endif // ndef DISABLED_LEGACY_ENGINE

} // namespace tesseract
