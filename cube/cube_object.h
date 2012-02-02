/**********************************************************************
 * File:        cube_object.h
 * Description: Declaration of the Cube Object Class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
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

// The CubeObject class is the main class used to perform recognition of
// a specific char_samp as a single word.
// To recognize a word, a CubeObject is constructed for this word.
// A Call to RecognizeWord is then issued specifying the language model that
// will be used during recognition. If none is specified, the default language
// model in the CubeRecoContext is used. The CubeRecoContext is passed at
// construction time
//
// The typical usage pattern for Cube is shown below:
//
//         // Create and initialize Tesseract object and get its
//         // CubeRecoContext object (note that Tesseract object owns it,
//         // so it will be freed when the Tesseract object is freed).
//         tesseract::Tesseract *tess_obj =  new tesseract::Tesseract();
//         tess_obj->init_tesseract(data_path, lang, tesseract::OEM_CUBE_ONLY);
//         CubeRecoContext *cntxt = tess_obj->GetCubeRecoContext();
//         CHECK(cntxt != NULL) << "Unable to create a Cube reco context";
//         .
//         .
//         .
//         // Do this to recognize a word in pix whose co-ordinates are
//         // (left,top,width,height)
//         tesseract::CubeObject *cube_obj;
//         cube_obj = new tesseract::CubeObject(cntxt, pix,
//                                              left, top, width, height);
//
//         // Get back Cube's list of answers
//         tesseract::WordAltList *alt_list = cube_obj->RecognizeWord();
//         CHECK(alt_list != NULL && alt_list->AltCount() > 0);
//
//         // Get the string and cost of every alternate
//         for (int alt = 0; alt < alt_list->AltCount(); alt++) {
//           // Return the result as a UTF-32 string
//           string_32 res_str32 = alt_list->Alt(alt);
//           // Convert to UTF8 if need-be
//           string res_str;
//           CubeUtils::UTF32ToUTF8(res_str32.c_str(), &res_str);
//           // Get the string cost. This should get bigger as you go deeper
//           // in the list
//           int cost = alt_list->AltCost(alt);
//         }
//
//         // Call this once you are done recognizing this word
//         delete cube_obj;
//
//         // Call this once you are done recognizing all words with
//         // for the current language
//         delete tess_obj;
//
// Note that if the language supports "Italics" (see the CubeRecoContext), the
// RecognizeWord function attempts to de-slant the word.

#ifndef CUBE_OBJECT_H
#define CUBE_OBJECT_H

#include "img.h"
#include "char_samp.h"
#include "word_altlist.h"
#include "beam_search.h"
#include "cube_search_object.h"
#include "tess_lang_model.h"
#include "cube_reco_context.h"

namespace tesseract {

// minimum aspect ratio needed to normalize a char_samp before recognition
static const float kMinNormalizationAspectRatio = 3.5;
// minimum probability a top alt choice must meet before having
// deslanted processing applied to it
static const float kMinProbSkipDeslanted = 0.25;

class CubeObject {
 public:
  // Different flavors of constructor. They just differ in the way the
  // word image is specified
  CubeObject(CubeRecoContext *cntxt, CharSamp *char_samp);
  CubeObject(CubeRecoContext *cntxt, IMAGE *img,
             int left, int top, int wid, int hgt);
  CubeObject(CubeRecoContext *cntxt, Pix *pix,
             int left, int top, int wid, int hgt);
  ~CubeObject();

  // Perform the word recognition using the specified language mode. If none
  // is specified, the default language model in the CubeRecoContext is used.
  // Returns the sorted list of alternate word answers
  WordAltList *RecognizeWord(LangModel *lang_mod = NULL);
  // Same as RecognizeWord but recognizes as a phrase
  WordAltList *RecognizePhrase(LangModel *lang_mod = NULL);
  // Computes the cost of a specific string. This is done by performing
  // recognition of a language model that allows only the specified word.
  // The alternate list(s) will be permanently modified.
  int WordCost(const char *str);
  // Recognizes a single character and returns the list of results.
  CharAltList *RecognizeChar();

  // Returns the BeamSearch object that resulted from the last call to
  // RecognizeWord
  inline BeamSearch *BeamObj() const {
    return (deslanted_ == true ? deslanted_beam_obj_ : beam_obj_);
  }
  // Returns the WordAltList object that resulted from the last call to
  // RecognizeWord
  inline WordAltList *AlternateList() const {
    return (deslanted_ == true ? deslanted_alt_list_ : alt_list_);
  }
  // Returns the CubeSearchObject object that resulted from the last call to
  // RecognizeWord
  inline CubeSearchObject *SrchObj() const {
    return (deslanted_ == true ? deslanted_srch_obj_ : srch_obj_);
  }
  // Returns the CharSamp object that resulted from the last call to
  // RecognizeWord. Note that this object is not necessarily identical to the
  // one passed at construction time as normalization might have occurred
  inline CharSamp *CharSample() const {
    return (deslanted_ == true ? deslanted_char_samp_ : char_samp_);
  }

  // Set the ownership of the CharSamp
  inline void SetCharSampOwnership(bool own_char_samp) {
    own_char_samp_ = own_char_samp;
  }

 protected:
  // Normalize the CharSamp if its aspect ratio exceeds the below constant.
  bool Normalize();

 private:
  // minimum segment count needed to normalize a char_samp before recognition
  static const int kMinNormalizationSegmentCnt = 4;

  // Data member initialization function
  void Init();
  // Free alternate lists.
  void Cleanup();
  // Perform the actual recognition using the specified language mode. If none
  // is specified, the default language model in the CubeRecoContext is used.
  // Returns the sorted list of alternate answers. Called by both
  // RecognizerWord (word_mode is true) or RecognizePhrase (word mode is false)
  WordAltList *Recognize(LangModel *lang_mod, bool word_mode);

  CubeRecoContext *cntxt_;
  BeamSearch *beam_obj_;
  BeamSearch *deslanted_beam_obj_;
  bool offline_mode_;
  bool own_char_samp_;
  bool deslanted_;
  CharSamp *char_samp_;
  CharSamp *deslanted_char_samp_;
  CubeSearchObject *srch_obj_;
  CubeSearchObject *deslanted_srch_obj_;
  WordAltList *alt_list_;
  WordAltList *deslanted_alt_list_;
};
}

#endif  // CUBE_OBJECT_H
