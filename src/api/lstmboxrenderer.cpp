/**********************************************************************
 * File:        lstmboxrenderer.cpp
 * Description: Renderer for creating box file for LSTM training.
 *              based on the tsv renderer.
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


#include <locale>     // for std::locale::classic
#include <memory>     // for std::unique_ptr
#include <sstream>    // for std::stringstream
#include "baseapi.h"  // for TessBaseAPI
#include "renderer.h"
#include "tesseractclass.h"  // for Tesseract

namespace tesseract {

/**
 * Create a UTF8 box file for LSTM training from the internal data structures.
 * page_number is a 0-base page index that will appear in the box file.
 * Returned string must be freed with the delete [] operator.
 */
 
char* TessBaseAPI::GetLSTMBOXText(int page_number) {
  if (tesseract_ == nullptr || (page_res_ == nullptr && Recognize(nullptr) < 0))
    return nullptr;

  STRING lstm_box_str("");

  int page_num = page_number;
  bool first_word = true;

  LTRResultIterator* res_it = GetLTRIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_SYMBOL)) {
      res_it->Next(RIL_SYMBOL);
      continue;
    }
  
  int left, top, right, bottom;
  
  if (!first_word) {
    if (res_it->IsAtBeginningOf(RIL_WORD)) {
        lstm_box_str.add_str_int("  ", left);
        lstm_box_str.add_str_int(" ", image_height_ - bottom);
        lstm_box_str.add_str_int(" ", right + 2);
        lstm_box_str.add_str_int(" ", image_height_ - top);
        lstm_box_str.add_str_int(" ", page_num);  // level 5 - word
        lstm_box_str += "\n";  // end of row for word
    }
    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
        lstm_box_str.add_str_int("\t ", left);
        lstm_box_str.add_str_int(" ", image_height_ - bottom);
        lstm_box_str.add_str_int(" ", right  + 5);
        lstm_box_str.add_str_int(" ", image_height_ - top);
        lstm_box_str.add_str_int(" ", page_num);  // level 4 - line
        lstm_box_str += "\n";  // end of row for line
    }
  }
  first_word=false;
  res_it->BoundingBox(RIL_SYMBOL, &left, &top, &right, &bottom);
    
  do {
        lstm_box_str +=std::unique_ptr<const char[]>(res_it->GetUTF8Text(RIL_SYMBOL)).get();
        res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_SYMBOL));
    
    lstm_box_str.add_str_int(" ", left);
    lstm_box_str.add_str_int(" ", image_height_ - bottom);
    lstm_box_str.add_str_int(" ", right);
    lstm_box_str.add_str_int(" ", image_height_ - top);
    lstm_box_str.add_str_int(" ", page_num);  // level 6 - symbol
    lstm_box_str += "\n";  // end of row
    
  }

  char* ret = new char[lstm_box_str.length() + 1];
  strcpy(ret, lstm_box_str.string());
  delete res_it;
  return ret;
}

/**********************************************************************
 * LSTMBOX Renderer interface implementation
 **********************************************************************/
TessLSTMBOXRenderer::TessLSTMBOXRenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "box") {
}

bool TessLSTMBOXRenderer::AddImageHandler(TessBaseAPI* api) {
  const std::unique_ptr<const char[]> lstmbox(api->GetLSTMBOXText(imagenum()));
  if (lstmbox == nullptr) return false;

  AppendString(lstmbox.get());

  return true;
}

}  // namespace tesseract.
