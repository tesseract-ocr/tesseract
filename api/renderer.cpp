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

#include <string.h>
#include "baseapi.h"
#include "genericvector.h"
#include "renderer.h"

namespace tesseract {

/**********************************************************************
 * Base Renderer interface implementation
 **********************************************************************/
TessResultRenderer::TessResultRenderer(const char *outputbase,
                                       const char* extension)
    : file_extension_(extension),
      title_(""), imagenum_(-1),
      fout_(stdout),
      next_(NULL),
      happy_(true) {
  if (strcmp(outputbase, "-") && strcmp(outputbase, "stdout")) {
    STRING outfile = STRING(outputbase) + STRING(".") + STRING(file_extension_);
    fout_ = fopen(outfile.string(), "wb");
    if (fout_ == NULL) {
      happy_ = false;
    }
  }
}

TessResultRenderer::~TessResultRenderer() {
 if (fout_ != stdout)
    fclose(fout_);
  else
    clearerr(fout_);
  delete next_;
}

void TessResultRenderer::insert(TessResultRenderer* next) {
  if (next == NULL) return;

  TessResultRenderer* remainder = next_;
  next_ = next;
  if (remainder) {
    while (next->next_ != NULL) {
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
  int n = fwrite(s, 1, len, fout_);
  if (n != len) happy_ = false;
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
  char* utf8 = api->GetUTF8Text();
  if (utf8 == NULL) {
    return false;
  }

  AppendString(utf8);
  delete[] utf8;

  bool pageBreak = false;
  api->GetBoolVariable("include_page_breaks", &pageBreak);
  const char* pageSeparator = api->GetStringVariable("page_separator");
  if (pageBreak) {
    AppendString(pageSeparator);
  }

  return true;
}

/**********************************************************************
 * HOcr Text Renderer interface implementation
 **********************************************************************/
TessHOcrRenderer::TessHOcrRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "hocr") {
    font_info_ = false;
}

TessHOcrRenderer::TessHOcrRenderer(const char *outputbase, bool font_info)
    : TessResultRenderer(outputbase, "hocr") {
    font_info_ = font_info;
}

bool TessHOcrRenderer::BeginDocumentHandler() {
  AppendString(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
      "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
      "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" "
      "lang=\"en\">\n <head>\n  <title>");
  AppendString(title());
  AppendString(
      "</title>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;"
      "charset=utf-8\" />\n"
      "  <meta name='ocr-system' content='tesseract " TESSERACT_VERSION_STR
              "' />\n"
      "  <meta name='ocr-capabilities' content='ocr_page ocr_carea ocr_par"
      " ocr_line ocrx_word");
  if (font_info_)
    AppendString(
      " ocrp_lang ocrp_dir ocrp_font ocrp_fsize ocrp_wconf");
  AppendString(
      "'/>\n"
      "</head>\n<body>\n");

  return true;
}

bool TessHOcrRenderer::EndDocumentHandler() {
  AppendString(" </body>\n</html>\n");

  return true;
}

bool TessHOcrRenderer::AddImageHandler(TessBaseAPI* api) {
  char* hocr = api->GetHOCRText(imagenum());
  if (hocr == NULL) return false;

  AppendString(hocr);
  delete[] hocr;

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
  char* tsv = api->GetTSVText(imagenum());
  if (tsv == NULL) return false;

  AppendString(tsv);
  delete[] tsv;

  return true;
}

/**********************************************************************
 * UNLV Text Renderer interface implementation
 **********************************************************************/
TessUnlvRenderer::TessUnlvRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "unlv") {
}

bool TessUnlvRenderer::AddImageHandler(TessBaseAPI* api) {
  char* unlv = api->GetUNLVText();
  if (unlv == NULL) return false;

  AppendString(unlv);
  delete[] unlv;

  return true;
}

/**********************************************************************
 * BoxText Renderer interface implementation
 **********************************************************************/
TessBoxTextRenderer::TessBoxTextRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "box") {
}

bool TessBoxTextRenderer::AddImageHandler(TessBaseAPI* api) {
  char* text = api->GetBoxText(imagenum());
  if (text == NULL) return false;

  AppendString(text);
  delete[] text;

  return true;
}

/**********************************************************************
 * Osd Text Renderer interface implementation
 **********************************************************************/
TessOsdRenderer::TessOsdRenderer(const char* outputbase)
    : TessResultRenderer(outputbase, "osd") {}

bool TessOsdRenderer::AddImageHandler(TessBaseAPI* api) {
  char* osd = api->GetOsdText(imagenum());
  if (osd == NULL) return false;

  AppendString(osd);
  delete[] osd;

  return true;
}

}  // namespace tesseract
