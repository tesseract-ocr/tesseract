// Include automatically generated configuration file if running autoconf.
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
        "lang=\"en\">\n <head>\n  <title>\n");
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

}  // namespace tesseract
