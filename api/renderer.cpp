// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include <string.h>
#include "baseapi.h"
#include "genericvector.h"
#include "renderer.h"

#if !defined(VERSION)
#include "version.h"
#endif

namespace tesseract {

// Start with a 4K output buffer which should be pretty big for a page of text
// though might need to grow for other formats or multi-page documents.
static const int kInitialAlloc = 1 << 12;

/**********************************************************************
 * Base Renderer interface implementation
 **********************************************************************/
TessResultRenderer::TessResultRenderer(const char* type, const char* extension)
    : full_typename_(type), file_extension_(extension),
      title_(""), imagenum_(-1),
      output_data_(NULL),
      next_(NULL) {
  ResetData();
}

TessResultRenderer::~TessResultRenderer() {
  delete[] output_data_;
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
  ResetData();

  title_ = title;
  imagenum_ = -1;
  bool ok = BeginDocumentHandler();
  if (next_) {
    ok = next_->BeginDocument(title) && ok;
  }
  return ok;
}

bool TessResultRenderer::AddImage(TessBaseAPI* api) {
  ++imagenum_;
  bool ok = AddImageHandler(api);
  if (next_) {
    ok = next_->AddImage(api) && ok;
  }
  return ok;
}

bool TessResultRenderer::AddError(TessBaseAPI* api) {
  ++imagenum_;
  bool ok = AddErrorHandler(api);
  if (next_) {
    ok = next_->AddError(api) && ok;
  }
  return ok;
}

bool TessResultRenderer::EndDocument() {
  bool ok = EndDocumentHandler();
  if (next_) {
    ok = next_->EndDocument() && ok;
  }
  return ok;
}

bool TessResultRenderer::GetOutput(const char** data, int* data_len) const {
  *data = output_data_;
  *data_len = output_len_;
  return true;
}

void TessResultRenderer::ResetData() {
  delete[] output_data_;
  output_data_ = new char[kInitialAlloc];
  output_alloc_ = kInitialAlloc;
  output_len_ = 0;
}

void TessResultRenderer::ReserveAdditionalData(int relative_len) {
  int total = relative_len + output_len_;
  if (total <= output_alloc_)
    return;

  if (total < 2 * output_alloc_) {
    total = 2 * output_alloc_;
  }

  char* new_data = new char[total];
  memcpy(new_data, output_data_, output_len_);
  delete[] output_data_;
  output_data_ = new_data;
}

void TessResultRenderer::AppendString(const char* s) {
  AppendData(s, strlen(s));
}

void TessResultRenderer::AppendData(const char* s, int len) {
  ReserveAdditionalData(len);
  memcpy(output_data_ + output_len_, s, len);
  output_len_ += len;
}

bool TessResultRenderer::BeginDocumentHandler() {
  return true;
}

bool TessResultRenderer::AddErrorHandler(TessBaseAPI* api) {
  return true;
}

bool TessResultRenderer::EndDocumentHandler() {
  return true;
}


/**********************************************************************
 * UTF8 Text Renderer interface implementation
 **********************************************************************/
TessTextRenderer::TessTextRenderer()
    : TessResultRenderer("Text", "txt") {
}

bool TessTextRenderer::AddImageHandler(TessBaseAPI* api) {
  char* utf8 = api->GetUTF8Text();
  if (utf8 == NULL) {
    return false;
  }

  AppendString(utf8);
  delete[] utf8;

  return true;
}

/**********************************************************************
 * HOcr Text Renderer interface implementation
 **********************************************************************/
TessHOcrRenderer::TessHOcrRenderer()
    : TessResultRenderer("HOcr", "hocr") {
}

bool TessHOcrRenderer::BeginDocumentHandler() {
  AppendString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
        "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" "
        "lang=\"en\">\n <head>\n  <title>\n");
  AppendString(title());
  AppendString(
      "</title>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;"
      "charset=utf-8\" />\n"
      "  <meta name='ocr-system' content='tesseract " VERSION "' />\n"
      "  <meta name='ocr-capabilities' content='ocr_page ocr_carea ocr_par"
      " ocr_line ocrx_word'/>\n"
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
 * UNLV Text Renderer interface implementation
 **********************************************************************/
TessUnlvRenderer::TessUnlvRenderer()
    : TessResultRenderer("UNLV", "unlv") {
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
TessBoxTextRenderer::TessBoxTextRenderer()
    : TessResultRenderer("Box Text", "box") {
}

bool TessBoxTextRenderer::AddImageHandler(TessBaseAPI* api) {
  char* text = api->GetBoxText(imagenum());
  if (text == NULL) return false;

  AppendString(text);
  delete[] text;

  return true;
}

}  // namespace tesseract
