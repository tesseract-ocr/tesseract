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

#include <tesseract/baseapi.h> // for TessBaseAPI
#include <locale>              // for std::locale::classic
#include <memory>              // for std::unique_ptr
#include <sstream>             // for std::stringstream
#ifdef _WIN32
#  include "host.h" // windows.h for MultiByteToWideChar, ...
#endif
#include <tesseract/renderer.h>
#include "tesseractclass.h" // for Tesseract

namespace tesseract {

/**
 * Gets the block orientation at the current iterator position.
 */
static tesseract::Orientation GetBlockTextOrientation(const PageIterator *it) {
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
static void AddBaselineCoordsTohOCR(const PageIterator *it,
                                    PageIteratorLevel level,
                                    std::stringstream &hocr_str) {
  tesseract::Orientation orientation = GetBlockTextOrientation(it);
  if (orientation != ORIENTATION_PAGE_UP) {
    hocr_str << "; textangle " << 360 - orientation * 90;
    return;
  }

  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);

  // Try to get the baseline coordinates at this level.
  int x1, y1, x2, y2;
  if (!it->Baseline(level, &x1, &y1, &x2, &y2)) {
    return;
  }
  // Following the description of this field of the hOCR spec, we convert the
  // baseline coordinates so that "the bottom left of the bounding box is the
  // origin".
  x1 -= left;
  x2 -= left;
  y1 -= bottom;
  y2 -= bottom;

  // Now fit a line through the points so we can extract coefficients for the
  // equation:  y = p1 x + p0
  if (x1 == x2) {
    // Problem computing the polynomial coefficients.
    return;
  }
  double p1 = (y2 - y1) / static_cast<double>(x2 - x1);
  double p0 = y1 - p1 * x1;

  hocr_str << "; baseline " << round(p1 * 1000.0) / 1000.0 << " "
           << round(p0 * 1000.0) / 1000.0;
}

static void AddBoxTohOCR(const ResultIterator *it, PageIteratorLevel level,
                         std::stringstream &hocr_str) {
  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);
  // This is the only place we use double quotes instead of single quotes,
  // but it may too late to change for consistency
  hocr_str << " title=\"bbox " << left << " " << top << " " << right << " "
           << bottom;
  // Add baseline coordinates & heights for textlines only.
  if (level == RIL_TEXTLINE) {
    AddBaselineCoordsTohOCR(it, level, hocr_str);
    // add custom height measures
    float row_height, descenders, ascenders; // row attributes
    it->RowAttributes(&row_height, &descenders, &ascenders);
    // TODO(rays): Do we want to limit these to a single decimal place?
    hocr_str << "; x_size " << row_height << "; x_descenders " << -descenders
             << "; x_ascenders " << ascenders;
  }
  hocr_str << "\">";
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
char *TessBaseAPI::GetHOCRText(int page_number) {
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
char *TessBaseAPI::GetHOCRText(ETEXT_DESC *monitor, int page_number) {
  if (tesseract_ == nullptr ||
      (page_res_ == nullptr && Recognize(monitor) < 0)) {
    return nullptr;
  }

  int lcnt = 1, bcnt = 1, pcnt = 1, wcnt = 1, scnt = 1, tcnt = 1, ccnt = 1;
  int page_id = page_number + 1; // hOCR uses 1-based page numbers.
  bool para_is_ltr = true;       // Default direction is LTR
  const char *paragraph_lang = nullptr;
  bool font_info = false;
  bool hocr_boxes = false;
  GetBoolVariable("hocr_font_info", &font_info);
  GetBoolVariable("hocr_char_boxes", &hocr_boxes);

  if (input_file_.empty()) {
    SetInputName(nullptr);
  }

#ifdef _WIN32
  // convert input name from ANSI encoding to utf-8
  int str16_len =
      MultiByteToWideChar(CP_ACP, 0, input_file_.c_str(), -1, nullptr, 0);
  wchar_t *uni16_str = new WCHAR[str16_len];
  str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_.c_str(), -1, uni16_str,
                                  str16_len);
  int utf8_len = WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, nullptr,
                                     0, nullptr, nullptr);
  char *utf8_str = new char[utf8_len];
  WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, utf8_str, utf8_len,
                      nullptr, nullptr);
  input_file_ = utf8_str;
  delete[] uni16_str;
  delete[] utf8_str;
#endif

  std::stringstream hocr_str;
  // Use "C" locale (needed for double values x_size and x_descenders).
  hocr_str.imbue(std::locale::classic());
  // Use 8 digits for double values.
  hocr_str.precision(8);
  hocr_str << "  <div class='ocr_page'"
           << " id='"
           << "page_" << page_id << "'"
           << " title='image \"";
  if (!input_file_.empty()) {
    hocr_str << HOcrEscape(input_file_.c_str());
  } else {
    hocr_str << "unknown";
  }

  hocr_str << "\"; bbox " << rect_left_ << " " << rect_top_ << " "
           << rect_width_ << " " << rect_height_ << "; ppageno " << page_number
           << "; scan_res " << GetSourceYResolution() << " "
           << GetSourceYResolution() << "'>\n";

  std::unique_ptr<ResultIterator> res_it(GetIterator());
  while (!res_it->Empty(RIL_BLOCK)) {
    int left, top, right, bottom;
    auto block_type = res_it->BlockType();
    switch (block_type) {
      case PT_FLOWING_IMAGE:
      case PT_HEADING_IMAGE:
      case PT_PULLOUT_IMAGE: {
        // Handle all kinds of images.
        res_it.get()->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom);
        hocr_str << "   <div class='ocr_photo' id='block_" << page_id << '_'
                 << bcnt++ << "' title=\"bbox " << left << " " << top << " "
                 << right << " " << bottom << "\"></div>\n";
        res_it->Next(RIL_BLOCK);
        continue;
      }
      case PT_HORZ_LINE:
      case PT_VERT_LINE:
        // Handle horizontal and vertical lines.
        res_it.get()->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom);
        hocr_str << "   <div class='ocr_separator' id='block_" << page_id << '_'
                 << bcnt++ << "' title=\"bbox " << left << " " << top << " "
                 << right << " " << bottom << "\"></div>\n";
        res_it->Next(RIL_BLOCK);
        continue;
      case PT_NOISE:
        tprintf("TODO: Please report image which triggers the noise case.\n");
        ASSERT_HOST(false);
      default:
        break;
    }

    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    // Open any new block/paragraph/textline.
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      para_is_ltr = true; // reset to default direction
      hocr_str << "   <div class='ocr_carea'"
               << " id='"
               << "block_" << page_id << "_" << bcnt << "'";
      AddBoxTohOCR(res_it.get(), RIL_BLOCK, hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_PARA)) {
      hocr_str << "\n    <p class='ocr_par'";
      para_is_ltr = res_it->ParagraphIsLtr();
      if (!para_is_ltr) {
        hocr_str << " dir='rtl'";
      }
      hocr_str << " id='"
               << "par_" << page_id << "_" << pcnt << "'";
      paragraph_lang = res_it->WordRecognitionLanguage();
      if (paragraph_lang) {
        hocr_str << " lang='" << paragraph_lang << "'";
      }
      AddBoxTohOCR(res_it.get(), RIL_PARA, hocr_str);
    }
    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      hocr_str << "\n     <span class='";
      switch (block_type) {
        case PT_HEADING_TEXT:
          hocr_str << "ocr_header";
          break;
        case PT_PULLOUT_TEXT:
          hocr_str << "ocr_textfloat";
          break;
        case PT_CAPTION_TEXT:
          hocr_str << "ocr_caption";
          break;
        case PT_FLOWING_IMAGE:
        case PT_HEADING_IMAGE:
        case PT_PULLOUT_IMAGE:
          ASSERT_HOST(false);
          break;
        default:
          hocr_str << "ocr_line";
      }
      hocr_str << "' id='"
               << "line_" << page_id << "_" << lcnt << "'";
      AddBoxTohOCR(res_it.get(), RIL_TEXTLINE, hocr_str);
    }

    // Now, process the word...
    int32_t lstm_choice_mode = tesseract_->lstm_choice_mode;
    std::vector<std::vector<std::vector<std::pair<const char *, float>>>>
        *rawTimestepMap = nullptr;
    std::vector<std::vector<std::pair<const char *, float>>> *CTCMap = nullptr;
    if (lstm_choice_mode) {
      CTCMap = res_it->GetBestLSTMSymbolChoices();
      rawTimestepMap = res_it->GetRawLSTMTimesteps();
    }
    hocr_str << "\n      <span class='ocrx_word'"
             << " id='"
             << "word_" << page_id << "_" << wcnt << "'";
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
    const char *font_name =
        res_it->WordFontAttributes(&bold, &italic, &underlined, &monospace,
                                   &serif, &smallcaps, &pointsize, &font_id);
    hocr_str << " title='bbox " << left << " " << top << " " << right << " "
             << bottom << "; x_wconf "
             << static_cast<int>(res_it->Confidence(RIL_WORD));
    if (font_info) {
      if (font_name) {
        hocr_str << "; x_font " << HOcrEscape(font_name).c_str();
      }
      hocr_str << "; x_fsize " << pointsize;
    }
    hocr_str << "'";
    const char *lang = res_it->WordRecognitionLanguage();
    if (lang && (!paragraph_lang || strcmp(lang, paragraph_lang))) {
      hocr_str << " lang='" << lang << "'";
    }
    switch (res_it->WordDirection()) {
      // Only emit direction if different from current paragraph direction
      case DIR_LEFT_TO_RIGHT:
        if (!para_is_ltr) {
          hocr_str << " dir='ltr'";
        }
        break;
      case DIR_RIGHT_TO_LEFT:
        if (para_is_ltr) {
          hocr_str << " dir='rtl'";
        }
        break;
      case DIR_MIX:
      case DIR_NEUTRAL:
      default: // Do nothing.
        break;
    }
    hocr_str << ">";
    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_para = res_it->IsAtFinalElement(RIL_PARA, RIL_WORD);
    bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);
    if (bold) {
      hocr_str << "<strong>";
    }
    if (italic) {
      hocr_str << "<em>";
    }
    do {
      const std::unique_ptr<const char[]> grapheme(
          res_it->GetUTF8Text(RIL_SYMBOL));
      if (grapheme && grapheme[0] != 0) {
        if (hocr_boxes) {
          res_it->BoundingBox(RIL_SYMBOL, &left, &top, &right, &bottom);
          hocr_str << "\n       <span class='ocrx_cinfo' title='x_bboxes "
                   << left << " " << top << " " << right << " " << bottom
                   << "; x_conf " << res_it->Confidence(RIL_SYMBOL) << "'>";
        }
        hocr_str << HOcrEscape(grapheme.get()).c_str();
        if (hocr_boxes) {
          hocr_str << "</span>";
          tesseract::ChoiceIterator ci(*res_it);
          if (lstm_choice_mode == 1 && ci.Timesteps() != nullptr) {
            std::vector<std::vector<std::pair<const char *, float>>> *symbol =
                ci.Timesteps();
            hocr_str << "\n        <span class='ocr_symbol'"
                     << " id='"
                     << "symbol_" << page_id << "_" << wcnt << "_" << scnt
                     << "'>";
            for (const auto &timestep : *symbol) {
              hocr_str << "\n         <span class='ocrx_cinfo'"
                       << " id='"
                       << "timestep" << page_id << "_" << wcnt << "_" << tcnt
                       << "'>";
              for (auto conf : timestep) {
                hocr_str << "\n          <span class='ocrx_cinfo'"
                         << " id='"
                         << "choice_" << page_id << "_" << wcnt << "_" << ccnt
                         << "'"
                         << " title='x_confs " << int(conf.second * 100) << "'>"
                         << HOcrEscape(conf.first).c_str() << "</span>";
                ++ccnt;
              }
              hocr_str << "</span>";
              ++tcnt;
            }
            hocr_str << "\n        </span>";
            ++scnt;
          } else if (lstm_choice_mode == 2) {
            hocr_str << "\n        <span class='ocrx_cinfo'"
                     << " id='"
                     << "lstm_choices_" << page_id << "_" << wcnt << "_" << tcnt
                     << "'>";
            do {
              const char *choice = ci.GetUTF8Text();
              float choiceconf = ci.Confidence();
              if (choice != nullptr) {
                hocr_str << "\n         <span class='ocrx_cinfo'"
                         << " id='"
                         << "choice_" << page_id << "_" << wcnt << "_" << ccnt
                         << "'"
                         << " title='x_confs " << choiceconf << "'>"
                         << HOcrEscape(choice).c_str() << "</span>";
                ccnt++;
              }
            } while (ci.Next());
            hocr_str << "\n        </span>";
            tcnt++;
          }
        }
      }
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    if (italic) {
      hocr_str << "</em>";
    }
    if (bold) {
      hocr_str << "</strong>";
    }
    // If the lstm choice mode is required it is added here
    if (lstm_choice_mode == 1 && !hocr_boxes && rawTimestepMap != nullptr) {
      for (const auto &symbol : *rawTimestepMap) {
        hocr_str << "\n       <span class='ocr_symbol'"
                 << " id='"
                 << "symbol_" << page_id << "_" << wcnt << "_" << scnt << "'>";
        for (const auto &timestep : symbol) {
          hocr_str << "\n        <span class='ocrx_cinfo'"
                   << " id='"
                   << "timestep" << page_id << "_" << wcnt << "_" << tcnt
                   << "'>";
          for (auto &&conf : timestep) {
            hocr_str << "\n         <span class='ocrx_cinfo'"
                     << " id='"
                     << "choice_" << page_id << "_" << wcnt << "_" << ccnt
                     << "'"
                     << " title='x_confs " << int(conf.second * 100) << "'>"
                     << HOcrEscape(conf.first).c_str() << "</span>";
            ++ccnt;
          }
          hocr_str << "</span>";
          ++tcnt;
        }
        hocr_str << "</span>";
        ++scnt;
      }
    } else if (lstm_choice_mode == 2 && !hocr_boxes && CTCMap != nullptr) {
      for (const auto &timestep : *CTCMap) {
        if (timestep.size() > 0) {
          hocr_str << "\n       <span class='ocrx_cinfo'"
                   << " id='"
                   << "lstm_choices_" << page_id << "_" << wcnt << "_" << tcnt
                   << "'>";
          for (auto &j : timestep) {
            float conf = 100 - tesseract_->lstm_rating_coefficient * j.second;
            if (conf < 0.0f) {
              conf = 0.0f;
            }
            if (conf > 100.0f) {
              conf = 100.0f;
            }
            hocr_str << "\n        <span class='ocrx_cinfo'"
                     << " id='"
                     << "choice_" << page_id << "_" << wcnt << "_" << ccnt
                     << "'"
                     << " title='x_confs " << conf << "'>"
                     << HOcrEscape(j.first).c_str() << "</span>";
            ccnt++;
          }
          hocr_str << "</span>";
          tcnt++;
        }
      }
    }
    // Close ocrx_word.
    if (hocr_boxes || lstm_choice_mode > 0) {
      hocr_str << "\n      ";
    }
    hocr_str << "</span>";
    tcnt = 1;
    ccnt = 1;
    wcnt++;
    // Close any ending block/paragraph/textline.
    if (last_word_in_line) {
      hocr_str << "\n     </span>";
      lcnt++;
    }
    if (last_word_in_para) {
      hocr_str << "\n    </p>\n";
      pcnt++;
      para_is_ltr = true; // back to default direction
    }
    if (last_word_in_block) {
      hocr_str << "   </div>\n";
      bcnt++;
    }
  }
  hocr_str << "  </div>\n";

  const std::string &text = hocr_str.str();
  char *result = new char[text.length() + 1];
  strcpy(result, text.c_str());
  return result;
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
      "  <meta http-equiv=\"Content-Type\" content=\"text/html;"
      "charset=utf-8\"/>\n"
      "  <meta name='ocr-system' content='tesseract " TESSERACT_VERSION_STR
      "' />\n"
      "  <meta name='ocr-capabilities' content='ocr_page ocr_carea ocr_par"
      " ocr_line ocrx_word ocrp_wconf");
  if (font_info_) {
    AppendString(" ocrp_lang ocrp_dir ocrp_font ocrp_fsize");
  }
  AppendString(
      "'/>\n"
      " </head>\n"
      " <body>\n");

  return true;
}

bool TessHOcrRenderer::EndDocumentHandler() {
  AppendString(" </body>\n</html>\n");

  return true;
}

bool TessHOcrRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> hocr(api->GetHOCRText(imagenum()));
  if (hocr == nullptr) {
    return false;
  }

  AppendString(hocr.get());

  return true;
}

} // namespace tesseract
