/**********************************************************************
 * File:        altorenderer.cpp
 * Description: ALTO rendering interface
 * Author:      Jake Sebright
 *
 * (C) Copyright 2018
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

#include "baseapi.h"
#include <memory.h>
#include "renderer.h"

namespace tesseract {

/**********************************************************************
 * Alto Text Renderer interface implementation
 **********************************************************************/
    TessAltoRenderer::TessAltoRenderer(const char *outputbase)
            : TessResultRenderer(outputbase, "xml") {
    }

    /**
    * Append the ALTO XML for the beginning of the document
    */
    bool TessAltoRenderer::BeginDocumentHandler() {
        AppendString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<alto xmlns=\"http://www.loc.gov/standards/alto/ns-v3#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.loc.gov/standards/alto/ns-v3# http://www.loc.gov/alto/v3/alto-3-0.xsd\">\n"
                "\t<Description>\n"
                "\t\t<MeasurementUnit>pixel</MeasurementUnit>\n"
                "\t\t<sourceImageInformation>\n"
                "\t\t\t<fileName>");

        AppendString(title());

        AppendString("\t\t\t</fileName>\n"
                     "\t\t</sourceImageInformation>\n"
                     "\t\t<OCRProcessing ID=\"OCR_0\">\n"
                     "\t\t\t<ocrProcessingStep>\n"
                     "\t\t\t\t<processingSoftware>\n"
                     "\t\t\t\t\t<softwareName>tesseract 4.0.0</softwareName>\n"
                     "\t\t\t\t</processingSoftware>\n"
                     "\t\t\t</ocrProcessingStep>\n"
                     "\t\t</OCRProcessing>\n"
                     "\t</Description>\n"
                     "\t<Layout>\n");

        return true;
    }

    /**
    * Append the ALTO XML for the end of the document
    */
    bool TessAltoRenderer::EndDocumentHandler() {
        AppendString("\t</Layout>\n</alto>\n");

        return true;
    }

    /**
    * Append the ALTO XML for the layout of the image
    */
    bool TessAltoRenderer::AddImageHandler(TessBaseAPI *api) {
        const std::unique_ptr<const char[]> hocr(api->GetAltoText(imagenum()));
        if (hocr == nullptr) return false;

        AppendString(hocr.get());

        return true;
    }

    /**
    * Add a unique ID to an ALTO element
    */
    static void AddIdToAlto(STRING *alto_str, const std::string base, int num1) {
        const size_t BUFSIZE = 64;
        char id_buffer[BUFSIZE];
        snprintf(id_buffer, BUFSIZE - 1, "%s_%d", base.c_str(), num1);
        id_buffer[BUFSIZE - 1] = '\0';
        *alto_str += " ID=\"";
        *alto_str += id_buffer;
        *alto_str += "\"";
    }

    /**
    * Add coordinates to specified TextBlock, TextLine, or String bounding box
    * Add word confidence if adding to a String bounding box
    */
    static void AddBoxToAlto(const ResultIterator *it, PageIteratorLevel level,
                             STRING *alto_str) {
        int left, top, right, bottom;
        it->BoundingBox(level, &left, &top, &right, &bottom);

        int hpos = left;
        int vpos = top;
        int height = bottom - top;
        int width = right - left;

        *alto_str += " HPOS=\"";
        alto_str->add_str_int("", hpos);
        *alto_str += "\"";
        *alto_str += " VPOS=\"";
        alto_str->add_str_int("", vpos);
        *alto_str += "\"";
        *alto_str += " WIDTH=\"";
        alto_str->add_str_int("", width);
        *alto_str += "\"";
        *alto_str += " HEIGHT=\"";
        alto_str->add_str_int("", height);
        *alto_str += "\"";

        if (level == RIL_WORD) {
            int wc = it->Confidence(RIL_WORD);
            *alto_str += " WC=\"0.";
            alto_str->add_str_int("", wc);
            *alto_str += "\"";
        }
        if (level != RIL_WORD) {

            *alto_str += ">";
        }
    }

    /**
     * Make an XML-formatted string with ALTO markup from the internal
     * data structures.
     */
        char *TessBaseAPI::GetAltoText(int page_number) {
            return GetAltoText(nullptr, page_number);
        }

    /**
     * Make an XML-formatted string with ALTO markup from the internal
     * data structures.
     */
        char *TessBaseAPI::GetAltoText(ETEXT_DESC *monitor, int page_number) {
            if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(monitor) < 0))
                return nullptr;

            int lcnt = 0, bcnt = 0, wcnt = 0;
            int page_id = page_number;

            STRING alto_str("");

            if (input_file_ == nullptr)
                SetInputName(nullptr);

    #ifdef _WIN32
            // convert input name from ANSI encoding to utf-8
          int str16_len =
              MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1, nullptr, 0);
          wchar_t *uni16_str = new WCHAR[str16_len];
          str16_len = MultiByteToWideChar(CP_ACP, 0, input_file_->string(), -1,
                                          uni16_str, str16_len);
          int utf8_len = WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, nullptr, 0,
                                             nullptr, nullptr);
          char *utf8_str = new char[utf8_len];
          WideCharToMultiByte(CP_UTF8, 0, uni16_str, str16_len, utf8_str,
                              utf8_len, nullptr, nullptr);
          *input_file_ = utf8_str;
          delete[] uni16_str;
          delete[] utf8_str;
    #endif

            alto_str += "\t\t<Page WIDTH=\"";
            alto_str.add_str_int("", rect_width_);
            alto_str += "\" HEIGHT=\"";
            alto_str.add_str_int("", rect_height_);
            alto_str += "\" PHYSICAL_IMG_NR=\"";
            alto_str.add_str_int("", rect_height_);
            alto_str += "\"";
            AddIdToAlto(&alto_str, "page", page_id);
            alto_str += ">\n";
            alto_str += ("\t\t\t<PrintSpace HPOS=\"0\" "
                         "VPOS=\"0\""
                         " WIDTH=\"");
            alto_str.add_str_int("", rect_width_);
            alto_str += "\" HEIGHT=\"";
            alto_str.add_str_int("", rect_height_);
            alto_str += "\">\n";

            ResultIterator *res_it = GetIterator();
            while (!res_it->Empty(RIL_BLOCK)) {
                if (res_it->Empty(RIL_WORD)) {
                    res_it->Next(RIL_WORD);
                    continue;
                }

                if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
                    alto_str += "\t\t\t\t<TextBlock ";
                    AddIdToAlto(&alto_str, "block", bcnt);
                    AddBoxToAlto(res_it, RIL_BLOCK, &alto_str);
                    alto_str += "\n";
                }

                if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {

                    alto_str += "\t\t\t\t\t<TextLine ";
                    AddIdToAlto(&alto_str, "line", lcnt);
                    AddBoxToAlto(res_it, RIL_TEXTLINE, &alto_str);
                    alto_str += "\n";
                }

                alto_str += "\t\t\t\t\t\t<String ";
                AddIdToAlto(&alto_str, "string", wcnt);
                AddBoxToAlto(res_it, RIL_WORD, &alto_str);
                alto_str += " CONTENT=\"";


                bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
                bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);

                do {
                    const std::unique_ptr<const char[]> grapheme(
                            res_it->GetUTF8Text(RIL_SYMBOL));
                    if (grapheme && grapheme[0] != 0) {
                        alto_str += HOcrEscape(grapheme.get());
                    }
                    res_it->Next(RIL_SYMBOL);
                } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));

                alto_str += "\"/>\n";

                wcnt++;

                if (last_word_in_line) {
                    alto_str += "\t\t\t\t\t</TextLine>\n";
                    lcnt++;
                }

                if (last_word_in_block) {
                    alto_str += "\t\t\t\t</TextBlock>\n";
                    bcnt++;
                }
            }

            alto_str += "\t\t\t</PrintSpace>\n";
            alto_str += "\t\t</Page>\n";

            char *ret = new char[alto_str.length() + 1];
            strcpy(ret, alto_str.string());
            delete res_it;
            return ret;
        }

    }