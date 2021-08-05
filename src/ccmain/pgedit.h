///////////////////////////////////////////////////////////////////////
// File:        pgedit.h
// Description: Page structure file editor
// Author:      Joern Wanke
//
// (C) Copyright 2007, Google Inc.
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

#ifndef PGEDIT_H
#define PGEDIT_H

#include "params.h"     // for INT_VAR_H, IntParam, STRING_VAR_H, StringParam
#include "scrollview.h" // for SVEvent (ptr only), SVEventHandler, ScrollView

namespace tesseract {

class BLOCK_LIST;
class PAGE_RES;

class Tesseract;

#ifndef GRAPHICS_DISABLED
// A small event handler class to process incoming events to
// this window.
class PGEventHandler : public SVEventHandler {
public:
  PGEventHandler(tesseract::Tesseract *tess) : tess_(tess) {}
  void Notify(const SVEvent *sve) override;

private:
  tesseract::Tesseract *tess_;
};
#endif // !GRAPHICS_DISABLED

extern BLOCK_LIST *current_block_list;
extern STRING_VAR_H(editor_image_win_name);
extern INT_VAR_H(editor_image_xpos);
extern INT_VAR_H(editor_image_ypos);
extern INT_VAR_H(editor_image_word_bb_color);
extern INT_VAR_H(editor_image_blob_bb_color);
extern STRING_VAR_H(editor_word_name);
extern INT_VAR_H(editor_word_xpos);
extern INT_VAR_H(editor_word_ypos);
extern INT_VAR_H(editor_word_height);
extern INT_VAR_H(editor_word_width);

} // namespace tesseract

#endif
