/**********************************************************************
 * File:        hocrrenderer.cpp
 * Description: Simple API for calling tesseract.
 * Author:      Ray Smith (original code from baseapi.cpp)
 * Author:      Stefan Weil (moved to separate file and cleaned code)
 *
 * (C) Copyright 2006, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include <memory>     // for std::unique_ptr
#include "baseapi.h"  // for TessBaseAPI
#include "renderer.h"
#include "tesseractclass.h"  // for Tesseract

namespace tesseract {

/**
 * Gets the block orientation at the current iterator position.
 */
static tesseract::Orientation GetBlockTextOrientation(const PageIterator* it) {
  tesseract::Orientation orientation;
  tesseract::WritingDirection writing_direction;
  tesseract::TextlineOrder textline_order;
  float deskew_angle;
  it->Orientation(&orientation, &writing_direction, &textline_order,
                  &deskew_angle);
  return orientation;
}

/**
 * Fits a line to the baseline at the given level, and appends its coefficients
 * to the hOCR string.
 * NOTE: The hOCR spec is unclear on how to specify baseline coefficients for
 * rotated textlines. For this reason, on textlines that are not upright, this
 * method currently only inserts a 'textangle' property to indicate the rotation
 * direction and does not add any baseline information to the hocr string.
 */
static void AddBaselineCoordsTohOCR(const PageIterator* it,
                                    PageIteratorLevel level, STRING* hocr_str) {
  tesseract::Orientation orientation = GetBlockTextOrientation(it);
  if (orientation != ORIENTATION_PAGE_UP) {
    hocr_str->add_str_int("; textangle ", 360 - orientation * 90);
    return;
  }

  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);

  // Try to get the baseline coordinates at this level.
  int x1, y1, x2, y2;
  if (!it->Baseline(level, &x1, &y1, &x2, &y2)) return;
  // Following the description of this field of the hOCR spec, we convert the
  // baseline coordinates so that "the bottom left of the bounding box is the
  // origin".
  x1 -= left;
  x2 -= left;
  y1 -= bottom;
  y2 -= bottom;

  // Now fit a line through the points so we can extract coefficients for the
  // equation:  y = p1 x + p0
  double p1 = 0;
  double p0 = 0;
  if (x1 == x2) {
    // Problem computing the polynomial coefficients.
    return;
  }
  p1 = (y2 - y1) / static_cast<double>(x2 - x1);
  p0 = y1 - static_cast<double>(p1 * x1);

  hocr_str->add_str_double("; baseline ", round(p1 * 1000.0) / 1000.0);
  hocr_str->add_str_double(" ", round(p0 * 1000.0) / 1000.0);
}

static void AddIdTohOCR(STRING* hocr_str, const std::string base, int num1,
                        int num2) {
  const size_t BUFSIZE = 64;
  char id_buffer[BUFSIZE];
  if (num2 >= 0) {
    snprintf(id_buffer, BUFSIZE - 1, "%s_%d_%d", base.c_str(), num1, num2);
  } else {
    snprintf(id_buffer, BUFSIZE - 1, "%s_%d", base.c_str(), num1);
  }
  id_buffer[BUFSIZE - 1] = '\0';
  *hocr_str += " id='";
  *hocr_str += id_buffer;
  *hocr_str += "'";
}

static void AddIdTohOCR(STRING* hocr_str, const std::string base, int num1,
                        int num2, int num3) {
  const size_t BUFSIZE = 64;
  char id_buffer[BUFSIZE];
  snprintf(id_buffer, BUFSIZE - 1, "%s_%d_%d_%d", base.c_str(), num1, num2,
           num3);
  id_buffer[BUFSIZE - 1] = '\0';
  *hocr_str += " id='";
  *hocr_str += id_buffer;
  *hocr_str += "'";
}

static void AddBoxTohOCR(const ResultIterator* it, PageIteratorLevel level,
                         STRING* hocr_str) {
  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);
  // This is the only place we use double quotes instead of single quotes,
  // but it may too late to change for consistency
  hocr_str->add_str_int(" title=\"bbox ", left);
  hocr_str->add_str_int(" ", top);
  hocr_str->add_str_int(" ", right);
  hocr_str->add_str_int(" ", bottom);
  // Add baseline coordinates & heights for textlines only.
  if (level == RIL_TEXTLINE) {
    AddBaselineCoordsTohOCR(it, level, hocr_str);
    // add custom height measures
    float row_height, descenders, ascenders;  // row attributes
    it->RowAttributes(&row_height, &descenders, &ascenders);
    // TODO(rays): Do we want to limit these to a single decimal place?
    hocr_str->add_str_double("; x_size ", row_height);
    hocr_str->add_str_double("; x_descenders ", descenders * -1);
    hocr_str->add_str_double("; x_ascenders ", ascenders);
  }
  *hocr_str += "\">";
}

/**
 * Make a HTML-formatted string with hOCR markup from the internal
 * data structures.
 * page_number is 0-based but will appear in the output as 1-based.
 * Image name/input_file_ can be set by SetInputName before calling
 * GetHOCRText
 * STL removed from original patch submission and refactored by rays.
 * Returned string must be freed with the delete [] operator.
 */
char* TessBaseAPI::GetHOCRText(int page_number) {
  return GetHOCRText(nullptr, page_number);
}

/**
 * Make a HTML-formatted string with hOCR markup from the internal
 * data structures.
 * page_number is 0-based but will appear in the output as 1-based.
 * Image name/input_file_ can be set by SetInputName before calling
 * GetHOCRText
 * STL removed from original patch submission and refactored by rays.
 * Returned string must be freed with the delete [] operator.
 */
char* TessBaseAPI::GetHOCRText(ETEXT_DESC* monitor, int page_number) {
  if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(monitor) < 0))
    return nullptr;

  int lcnt = 1, bcnt = 1, pcnt = 1, wcnt = 1, tcnt = 1, gcnt = 1;
  int page_id = page_number + 1;  // hOCR uses 1-based page numbers.
  bool para_is_ltr = true;        // Default direction is LTR
  const char* paragraph_lang = nullptr;
  bool font_info = false;
  GetBoolVariable("hocr_font_info", &font_info);

  STRING hocr_str("");

  if (input_file_ == nullptr) SetInputName(nullptr);

#ifdef _WIN32
  // convert input name from ANSI encoding to utf-8
  int str16_len =
      MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1, nullptr, 0);
  wchar_t* uni16_str = new WCHAR[str16_len];
  str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1,
                                  uni16_str, str16_len);
  int utf8_len = WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, nullptr,
                                     0, nullptr, nullptr);
  char* utf8_str = new char[utf8_len];
  WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, utf8_str, utf8_len,
                      nullptr, nullptr);
  *input_file_ = utf8_str;
  delete[] uni16_str;
  delete[] utf8_str;
#endif

  hocr_str += "  <div class='ocr_page'";
  AddIdTohOCR(&hocr_str, "page", page_id, -1);
  hocr_str += " title='image \"";
  if (input_file_) {
    hocr_str += HOcrEscape(input_file_->string());
  } else {
    hocr_str += "unknown";
  }
  hocr_str.add_str_int("\"; bbox ", rect_left_);
  hocr_str.add_str_int(" ", rect_top_);
  hocr_str.add_str_int(" ", rect_width_);
  hocr_str.add_str_int(" ", rect_height_);
  hocr_str.add_str_int("; ppageno ", page_number);
  hocr_str += "'>\n";

  ResultIterator* res_it = GetIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    // Open any new block/paragraph/textline.
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      para_is_ltr = true;  // reset to default direction
      hocr_str += "   <div class='ocr_carea'";
      AddIdTohOCR(&hocr_str, "block", page_id, bcnt);
      AddBoxTohOCR(res_it, RIL_BLOCK, &hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_PARA)) {
      hocr_str += "\n    <p class='ocr_par'";
      para_is_ltr = res_it->ParagraphIsLtr();
      if (!para_is_ltr) {
        hocr_str += " dir='rtl'";
      }
      AddIdTohOCR(&hocr_str, "par", page_id, pcnt);
      paragraph_lang = res_it->WordRecognitionLanguage();
      if (paragraph_lang) {
        hocr_str += " lang='";
        hocr_str += paragraph_lang;
        hocr_str += "'";
      }
      AddBoxTohOCR(res_it, RIL_PARA, &hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      hocr_str += "\n     <span class='ocr_line'";
      AddIdTohOCR(&hocr_str, "line", page_id, lcnt);
      AddBoxTohOCR(res_it, RIL_TEXTLINE, &hocr_str);
    }

    // Now, process the word...
    std::vector<std::vector<std::pair<const char*, float>>>* confidencemap =
        nullptr;
    if (tesseract_->lstm_choice_mode) {
      confidencemap = res_it->GetBestLSTMSymbolChoices();
    }
    hocr_str += "\n      <span class='ocrx_word'";
    AddIdTohOCR(&hocr_str, "word", page_id, wcnt);
    int left, top, right, bottom;
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    const char* font_name;
    res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
    font_name =
        res_it->WordFontAttributes(&bold, &italic, &underlined, &monospace,
                                   &serif, &smallcaps, &pointsize, &font_id);
    hocr_str.add_str_int(" title='bbox ", left);
    hocr_str.add_str_int(" ", top);
    hocr_str.add_str_int(" ", right);
    hocr_str.add_str_int(" ", bottom);
    hocr_str.add_str_int("; x_wconf ", res_it->Confidence(RIL_WORD));
    if (font_info) {
      if (font_name) {
        hocr_str += "; x_font ";
        hocr_str += HOcrEscape(font_name);
      }
      hocr_str.add_str_int("; x_fsize ", pointsize);
    }
    hocr_str += "'";
    const char* lang = res_it->WordRecognitionLanguage();
    if (lang && (!paragraph_lang || strcmp(lang, paragraph_lang))) {
      hocr_str += " lang='";
      hocr_str += lang;
      hocr_str += "'";
    }
    switch (res_it->WordDirection()) {
      // Only emit direction if different from current paragraph direction
      case DIR_LEFT_TO_RIGHT:
        if (!para_is_ltr) hocr_str += " dir='ltr'";
        break;
      case DIR_RIGHT_TO_LEFT:
        if (para_is_ltr) hocr_str += " dir='rtl'";
        break;
      case DIR_MIX:
      case DIR_NEUTRAL:
      default:  // Do nothing.
        break;
    }
    hocr_str += ">";
    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_para = res_it->IsAtFinalElement(RIL_PARA, RIL_WORD);
    bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);
    if (bold) hocr_str += "<strong>";
    if (italic) hocr_str += "<em>";
    do {
      const std::unique_ptr<const char[]> grapheme(
          res_it->GetUTF8Text(RIL_SYMBOL));
      if (grapheme && grapheme[0] != 0) {
        hocr_str += HOcrEscape(grapheme.get());
      }
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    if (italic) hocr_str += "</em>";
    if (bold) hocr_str += "</strong>";
    // If the lstm choice mode is required it is added here
    if (tesseract_->lstm_choice_mode == 1 && confidencemap != nullptr) {
      for (size_t i = 0; i < confidencemap->size(); i++) {
        hocr_str += "\n       <span class='ocrx_cinfo'";
        AddIdTohOCR(&hocr_str, "timestep", page_id, wcnt, tcnt);
        hocr_str += ">";
        std::vector<std::pair<const char*, float>> timestep =
            (*confidencemap)[i];
        for (std::pair<const char*, float> conf : timestep) {
          hocr_str += "<span class='ocr_glyph'";
          AddIdTohOCR(&hocr_str, "choice", page_id, wcnt, gcnt);
          hocr_str.add_str_int(" title='x_confs ", int(conf.second * 100));
          hocr_str += "'";
          hocr_str += ">";
          hocr_str += conf.first;
          hocr_str += "</span>";
          gcnt++;
        }
        hocr_str += "</span>";
        tcnt++;
      }
    } else if (tesseract_->lstm_choice_mode == 2 && confidencemap != nullptr) {
      for (size_t i = 0; i < confidencemap->size(); i++) {
        std::vector<std::pair<const char*, float>> timestep =
            (*confidencemap)[i];
        if (timestep.size() > 0) {
          hocr_str += "\n       <span class='ocrx_cinfo'";
          AddIdTohOCR(&hocr_str, "lstm_choices", page_id, wcnt, tcnt);
          hocr_str += " chosen='";
          hocr_str += timestep[0].first;
          hocr_str += "'>";
          for (size_t j = 1; j < timestep.size(); j++) {
            hocr_str += "<span class='ocr_glyph'";
            AddIdTohOCR(&hocr_str, "choice", page_id, wcnt, gcnt);
            hocr_str.add_str_int(" title='x_confs ",
                                 int(timestep[j].second * 100));
            hocr_str += "'";
            hocr_str += ">";
            hocr_str += timestep[j].first;
            hocr_str += "</span>";
            gcnt++;
          }
          hocr_str += "</span>";
          tcnt++;
        }
      }
    }
    hocr_str += "</span>";
    tcnt = 1;
    gcnt = 1;
    wcnt++;
    // Close any ending block/paragraph/textline.
    if (last_word_in_line) {
      hocr_str += "\n     </span>";
      lcnt++;
    }
    if (last_word_in_para) {
      hocr_str += "\n    </p>\n";
      pcnt++;
      para_is_ltr = true;  // back to default direction
    }
    if (last_word_in_block) {
      hocr_str += "   </div>\n";
      bcnt++;
    }
  }
  hocr_str += "  </div>\n";

  char* ret = new char[hocr_str.length() + 1];
  strcpy(ret, hocr_str.string());
  delete res_it;
  return ret;
}

/**********************************************************************
 * HOcr Text Renderer interface implementation
 **********************************************************************/
TessHOcrRenderer::TessHOcrRenderer(const char* outputbase)
    : TessResultRenderer(outputbase, "hocr") {
  font_info_ = false;
}

TessHOcrRenderer::TessHOcrRenderer(const char* outputbase, bool font_info)
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
      "  <meta name='ocr-system' content='tesseract " PACKAGE_VERSION
      "' />\n"
      "  <meta name='ocr-capabilities' content='ocr_page ocr_carea ocr_par"
      " ocr_line ocrx_word ocrp_wconf");
  if (font_info_) AppendString(" ocrp_lang ocrp_dir ocrp_font ocrp_fsize");
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
  const std::unique_ptr<const char[]> hocr(api->GetHOCRText(imagenum()));
  if (hocr == nullptr) return false;

  AppendString(hocr.get());

  return true;
}

}  // namespace tesseract
