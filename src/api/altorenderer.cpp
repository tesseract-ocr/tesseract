// File:        altorenderer.cpp
// Description: ALTO rendering interface
// Author:      Jake Sebright

// (C) Copyright 2018
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <sstream>  // for std::stringstream
#include <tesseract/baseapi.h>
#ifdef _WIN32
# include "host.h"    // windows.h for MultiByteToWideChar, ...
#endif
#include <tesseract/renderer.h>
#include <tesseract/strngs.h> // for STRING

namespace tesseract {

/// Add coordinates to specified TextBlock, TextLine or String bounding box.
/// Add word confidence if adding to a String bounding box.
///
static void AddBoxToAlto(const ResultIterator* it, PageIteratorLevel level,
                         std::stringstream& alto_str) {
  int left, top, right, bottom;
  it->BoundingBox(level, &left, &top, &right, &bottom);

  int hpos = left;
  int vpos = top;
  int height = bottom - top;
  int width = right - left;

  alto_str << " HPOS=\"" << hpos << "\"";
  alto_str << " VPOS=\"" << vpos << "\"";
  alto_str << " WIDTH=\"" << width << "\"";
  alto_str << " HEIGHT=\"" << height << "\"";

  if (level == RIL_WORD) {
    int wc = it->Confidence(RIL_WORD);
    alto_str << " WC=\"0." << wc << "\"";
  } else {
    alto_str << ">";
  }
}

///
/// Append the ALTO XML for the beginning of the document
///
bool TessAltoRenderer::BeginDocumentHandler() {
  AppendString(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<alto xmlns=\"http://www.loc.gov/standards/alto/ns-v3#\" "
      "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
      "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
      "xsi:schemaLocation=\"http://www.loc.gov/standards/alto/ns-v3# "
      "http://www.loc.gov/alto/v3/alto-3-0.xsd\">\n"
      "\t<Description>\n"
      "\t\t<MeasurementUnit>pixel</MeasurementUnit>\n"
      "\t\t<sourceImageInformation>\n"
      "\t\t\t<fileName>");

  AppendString(title());

  AppendString(
      "</fileName>\n"
      "\t\t</sourceImageInformation>\n"
      "\t\t<OCRProcessing ID=\"OCR_0\">\n"
      "\t\t\t<ocrProcessingStep>\n"
      "\t\t\t\t<processingSoftware>\n"
      "\t\t\t\t\t<softwareName>tesseract ");
  AppendString(TessBaseAPI::Version());
  AppendString(
      "</softwareName>\n"
      "\t\t\t\t</processingSoftware>\n"
      "\t\t\t</ocrProcessingStep>\n"
      "\t\t</OCRProcessing>\n"
      "\t</Description>\n"
      "\t<Layout>\n");

  return true;
}

///
/// Append the ALTO XML for the layout of the image
///
bool TessAltoRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> text(api->GetAltoText(imagenum()));
  if (text == nullptr) return false;

  AppendString(text.get());

  return true;
}

///
/// Append the ALTO XML for the end of the document
///
bool TessAltoRenderer::EndDocumentHandler() {
  AppendString("\t</Layout>\n</alto>\n");

  return true;
}

TessAltoRenderer::TessAltoRenderer(const char* outputbase)
    : TessResultRenderer(outputbase, "xml") {}

///
/// Make an XML-formatted string with ALTO markup from the internal
/// data structures.
///
char* TessBaseAPI::GetAltoText(int page_number) {
  return GetAltoText(nullptr, page_number);
}

///
/// Make an XML-formatted string with ALTO markup from the internal
/// data structures.
///
char* TessBaseAPI::GetAltoText(ETEXT_DESC* monitor, int page_number) {
  if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(monitor) < 0))
    return nullptr;

  int lcnt = 0, tcnt = 0, bcnt = 0, wcnt = 0;

  if (input_file_ == nullptr) SetInputName(nullptr);

#ifdef _WIN32
  // convert input name from ANSI encoding to utf-8
  int str16_len =
      MultiByteToWideChar(CP_ACP, 0, input_file_->c_str(), -1, nullptr, 0);
  wchar_t* uni16_str = new WCHAR[str16_len];
  str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_->c_str(), -1,
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

  std::stringstream alto_str;
  // Use "C" locale (needed for int values larger than 999).
  alto_str.imbue(std::locale::classic());
  alto_str
      << "\t\t<Page WIDTH=\"" << rect_width_ << "\" HEIGHT=\""
      << rect_height_
      << "\" PHYSICAL_IMG_NR=\"" << page_number << "\""
      << " ID=\"page_" << page_number << "\">\n"
      << "\t\t\t<PrintSpace HPOS=\"0\" VPOS=\"0\""
      << " WIDTH=\"" << rect_width_ << "\""
      << " HEIGHT=\"" << rect_height_ << "\">\n";

  ResultIterator* res_it = GetIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      alto_str << "\t\t\t\t<ComposedBlock ID=\"cblock_" << bcnt << "\"";
      AddBoxToAlto(res_it, RIL_BLOCK, alto_str);
      alto_str << "\n";
    }

    if (res_it->IsAtBeginningOf(RIL_PARA)) {
      alto_str << "\t\t\t\t\t<TextBlock ID=\"block_" << tcnt << "\"";
      AddBoxToAlto(res_it, RIL_PARA, alto_str);
      alto_str << "\n";
    }

    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      alto_str << "\t\t\t\t\t\t<TextLine ID=\"line_" << lcnt << "\"";
      AddBoxToAlto(res_it, RIL_TEXTLINE, alto_str);
      alto_str << "\n";
    }

    alto_str << "\t\t\t\t\t\t\t<String ID=\"string_" << wcnt << "\"";
    AddBoxToAlto(res_it, RIL_WORD, alto_str);
    alto_str << " CONTENT=\"";

    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_tblock = res_it->IsAtFinalElement(RIL_PARA, RIL_WORD);
    bool last_word_in_cblock = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);


    int left, top, right, bottom;
    res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);

    do {
      const std::unique_ptr<const char[]> grapheme(
          res_it->GetUTF8Text(RIL_SYMBOL));
      if (grapheme && grapheme[0] != 0) {
        alto_str << HOcrEscape(grapheme.get()).c_str();
      }
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));

    alto_str << "\"/>";

    wcnt++;

    if (last_word_in_line) {
      alto_str << "\n\t\t\t\t\t\t</TextLine>\n";
      lcnt++;
    } else {
      int hpos = right;
      int vpos = top;
      res_it->BoundingBox(RIL_WORD, &left, &top, &right, &bottom);
      int width = left - hpos;
      alto_str << "<SP WIDTH=\"" << width << "\" VPOS=\"" << vpos
               << "\" HPOS=\"" << hpos << "\"/>\n";
    }

    if (last_word_in_tblock) {
      alto_str << "\t\t\t\t\t</TextBlock>\n";
      tcnt++;
    }

    if (last_word_in_cblock) {
      alto_str << "\t\t\t\t</ComposedBlock>\n";
      bcnt++;
    }
  }

  alto_str << "\t\t\t</PrintSpace>\n"
           << "\t\t</Page>\n";
  const std::string& text = alto_str.str();

  char* result = new char[text.length() + 1];
  strcpy(result, text.c_str());
  delete res_it;
  return result;
}

}  // namespace tesseract
