/**********************************************************************
 * File:        wordstrboxrenderer.cpp
 * Description: Renderer for creating box file with WordStr strings.
 *              based on the tsv renderer.
 *
 * (C) Copyright 2019, Google Inc.
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
#include <tesseract/renderer.h>
#include "tesseractclass.h" // for Tesseract

namespace tesseract {

/**
 * Create a UTF8 box file with WordStr strings from the internal data
 * structures. page_number is a 0-base page index that will appear in the box
 * file. Returned string must be freed with the delete [] operator.
 */

char *TessBaseAPI::GetWordStrBoxText(int page_number = 0) {
  if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(nullptr) < 0)) {
    return nullptr;
  }

  std::string wordstr_box_str;
  int left = 0, top = 0, right = 0, bottom = 0;

  bool first_line = true;

  LTRResultIterator *res_it = GetLTRIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      if (!first_line) {
        wordstr_box_str += "\n\t " + std::to_string(right + 1);
        wordstr_box_str += " " + std::to_string(image_height_ - bottom);
        wordstr_box_str += " " + std::to_string(right + 5);
        wordstr_box_str += " " + std::to_string(image_height_ - top);
        wordstr_box_str += " " + std::to_string(page_number); // row for tab for EOL
        wordstr_box_str += "\n";
      } else {
        first_line = false;
      }
      // Use bounding box for whole line for WordStr
      res_it->BoundingBox(RIL_TEXTLINE, &left, &top, &right, &bottom);
      wordstr_box_str += "WordStr " + std::to_string(left);
      wordstr_box_str += " " + std::to_string(image_height_ - bottom);
      wordstr_box_str += " " + std::to_string(right);
      wordstr_box_str += " " + std::to_string(image_height_ - top);
      wordstr_box_str += " " + std::to_string(page_number); // word
      wordstr_box_str += " #";
    }
    do {
      wordstr_box_str += std::unique_ptr<const char[]>(res_it->GetUTF8Text(RIL_WORD)).get();
      wordstr_box_str += " ";
      res_it->Next(RIL_WORD);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
  }

  if (left != 0 && top != 0 && right != 0 && bottom != 0) {
    wordstr_box_str += "\n\t " + std::to_string(right + 1);
    wordstr_box_str += " " + std::to_string(image_height_ - bottom);
    wordstr_box_str += " " + std::to_string(right + 5);
    wordstr_box_str += " " + std::to_string(image_height_ - top);
    wordstr_box_str += " " + std::to_string(page_number); // row for tab for EOL
    wordstr_box_str += "\n";
  }
  char *ret = new char[wordstr_box_str.length() + 1];
  strcpy(ret, wordstr_box_str.c_str());
  delete res_it;
  return ret;
}

/**********************************************************************
 * WordStrBox Renderer interface implementation
 **********************************************************************/
TessWordStrBoxRenderer::TessWordStrBoxRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "box") {}

bool TessWordStrBoxRenderer::AddImageHandler(TessBaseAPI *api) {
  const std::unique_ptr<const char[]> wordstrbox(api->GetWordStrBoxText(imagenum()));
  if (wordstrbox == nullptr) {
    return false;
  }

  AppendString(wordstrbox.get());

  return true;
}

} // namespace tesseract.
