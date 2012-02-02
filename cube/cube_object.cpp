/**********************************************************************
 * File:        cube_object.cpp
 * Description: Implementation of the Cube Object Class
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

#include <math.h>
#include "cube_object.h"
#include "cube_utils.h"
#include "word_list_lang_model.h"

namespace tesseract {
CubeObject::CubeObject(CubeRecoContext *cntxt, CharSamp *char_samp) {
  Init();
  char_samp_ = char_samp;
  cntxt_ = cntxt;
}

CubeObject::CubeObject(CubeRecoContext *cntxt, IMAGE *img,
                       int left, int top, int wid, int hgt) {
  Init();
  char_samp_ = CubeUtils::CharSampleFromImg(img, left, top, wid, hgt);
  own_char_samp_ = true;
  cntxt_ = cntxt;
}

CubeObject::CubeObject(CubeRecoContext *cntxt, Pix *pix,
                       int left, int top, int wid, int hgt) {
  Init();
  char_samp_ = CubeUtils::CharSampleFromPix(pix, left, top, wid, hgt);
  own_char_samp_ = true;
  cntxt_ = cntxt;
}

// Data member initialization function
void CubeObject::Init() {
  char_samp_ = NULL;
  own_char_samp_ = false;
  alt_list_ = NULL;
  srch_obj_ = NULL;
  deslanted_alt_list_ = NULL;
  deslanted_srch_obj_ = NULL;
  deslanted_ = false;
  deslanted_char_samp_ = NULL;
  beam_obj_ = NULL;
  deslanted_beam_obj_ = NULL;
  cntxt_ = NULL;
}

// Cleanup function
void CubeObject::Cleanup() {
  if (alt_list_ != NULL) {
    delete alt_list_;
    alt_list_ = NULL;
  }

  if (deslanted_alt_list_ != NULL) {
    delete deslanted_alt_list_;
    deslanted_alt_list_ = NULL;
  }
}

CubeObject::~CubeObject() {
  if (char_samp_ != NULL && own_char_samp_ == true) {
    delete char_samp_;
    char_samp_ = NULL;
  }

  if (srch_obj_ != NULL) {
    delete srch_obj_;
    srch_obj_ = NULL;
  }

  if (deslanted_srch_obj_ != NULL) {
    delete deslanted_srch_obj_;
    deslanted_srch_obj_ = NULL;
  }

  if (beam_obj_ != NULL) {
    delete beam_obj_;
    beam_obj_ = NULL;
  }

  if (deslanted_beam_obj_ != NULL) {
    delete deslanted_beam_obj_;
    deslanted_beam_obj_ = NULL;
  }

  if (deslanted_char_samp_ != NULL) {
    delete deslanted_char_samp_;
    deslanted_char_samp_ = NULL;
  }

  Cleanup();
}

// Actually do the recognition using the specified language mode. If none
// is specified, the default language model in the CubeRecoContext is used.
// Returns the sorted list of alternate answers
// The Word mode determines whether recognition is done as a word or a phrase
WordAltList *CubeObject::Recognize(LangModel *lang_mod, bool word_mode) {
  if (char_samp_ == NULL) {
    return NULL;
  }

  // clear alt lists
  Cleanup();

  // no specified language model, use the one in the reco context
  if (lang_mod == NULL) {
    lang_mod = cntxt_->LangMod();
  }

  // normalize if necessary
  if (cntxt_->SizeNormalization()) {
    Normalize();
  }

  // assume not de-slanted by default
  deslanted_ = false;

  // create a beam search object
  if (beam_obj_ == NULL) {
    beam_obj_ = new BeamSearch(cntxt_, word_mode);
    if (beam_obj_ == NULL) {
      fprintf(stderr, "Cube ERROR (CubeObject::Recognize): could not construct "
              "BeamSearch\n");
      return NULL;
    }
  }

  // create a cube search object
  if (srch_obj_ == NULL) {
    srch_obj_ = new CubeSearchObject(cntxt_, char_samp_);
    if (srch_obj_ == NULL) {
      fprintf(stderr, "Cube ERROR (CubeObject::Recognize): could not construct "
              "CubeSearchObject\n");
      return NULL;
    }
  }

  // run a beam search against the tesslang model
  alt_list_ = beam_obj_->Search(srch_obj_, lang_mod);

  // deslant (if supported by language) and re-reco if probability is low enough
  if (cntxt_->HasItalics() == true &&
      (alt_list_ == NULL || alt_list_->AltCount() < 1 ||
       alt_list_->AltCost(0) > CubeUtils::Prob2Cost(kMinProbSkipDeslanted))) {

    if (deslanted_beam_obj_ == NULL) {
      deslanted_beam_obj_ = new BeamSearch(cntxt_);
      if (deslanted_beam_obj_ == NULL) {
        fprintf(stderr, "Cube ERROR (CubeObject::Recognize): could not "
                "construct deslanted BeamSearch\n");
        return false;
      }
    }

    if (deslanted_srch_obj_ == NULL) {
      deslanted_char_samp_ = char_samp_->Clone();
      if (deslanted_char_samp_ == NULL) {
        fprintf(stderr, "Cube ERROR (CubeObject::Recognize): could not "
                "construct deslanted CharSamp\n");
        return NULL;
      }

      if (deslanted_char_samp_->Deslant() == false) {
        return NULL;
      }

      deslanted_srch_obj_ = new CubeSearchObject(cntxt_, deslanted_char_samp_);
      if (deslanted_srch_obj_ == NULL) {
        fprintf(stderr, "Cube ERROR (CubeObject::Recognize): could not "
                "construct deslanted CubeSearchObject\n");
        return NULL;
      }
    }

    // run a beam search against the tesslang model
    deslanted_alt_list_ = deslanted_beam_obj_->Search(deslanted_srch_obj_,
                                                      lang_mod);
    // should we use de-slanted altlist?
    if (deslanted_alt_list_ != NULL &&  deslanted_alt_list_->AltCount() > 0) {
      if (alt_list_ == NULL || alt_list_->AltCount() < 1 ||
          deslanted_alt_list_->AltCost(0) < alt_list_->AltCost(0)) {
        deslanted_ = true;
        return deslanted_alt_list_;
      }
    }
  }

  return alt_list_;
}

// Recognize the member char sample as a word
WordAltList *CubeObject::RecognizeWord(LangModel *lang_mod) {
  return Recognize(lang_mod, true);
}

// Recognize the member char sample as a word
WordAltList *CubeObject::RecognizePhrase(LangModel *lang_mod) {
  return Recognize(lang_mod, false);
}

// Computes the cost of a specific string. This is done by performing
// recognition of a language model that allows only the specified word
int CubeObject::WordCost(const char *str) {
  WordListLangModel *lang_mod = new WordListLangModel(cntxt_);
  if (lang_mod == NULL) {
    return WORST_COST;
  }

  if (lang_mod->AddString(str) == false) {
    delete lang_mod;
    return WORST_COST;
  }

  // run a beam search against the single string wordlist model
  WordAltList *alt_list = RecognizeWord(lang_mod);
  delete lang_mod;

  int cost = WORST_COST;
  if (alt_list != NULL) {
    if (alt_list->AltCount() > 0) {
      cost = alt_list->AltCost(0);
    }
  }

  return cost;
}

// Recognizes a single character and returns the list of results.
CharAltList *CubeObject::RecognizeChar() {
  if (char_samp_ == NULL) return NULL;
  CharAltList* alt_list = NULL;
  CharClassifier *char_classifier = cntxt_->Classifier();
  ASSERT_HOST(char_classifier != NULL);
  alt_list = char_classifier->Classify(char_samp_);
  return alt_list;
}

// Normalize the input word bitmap to have a minimum aspect ratio
bool CubeObject::Normalize() {
  // create a cube search object
  CubeSearchObject *srch_obj = new CubeSearchObject(cntxt_, char_samp_);
  if (srch_obj == NULL) {
    return false;
  }
  // Perform over-segmentation
  int seg_cnt = srch_obj->SegPtCnt();
  // Only perform normalization if segment count is large enough
  if (seg_cnt < kMinNormalizationSegmentCnt) {
    delete srch_obj;
    return true;
  }
  // compute the mean AR of the segments
  double ar_mean = 0.0;
  for (int seg_idx = 0; seg_idx <= seg_cnt; seg_idx++) {
    CharSamp *seg_samp = srch_obj->CharSample(seg_idx - 1, seg_idx);
    if (seg_samp != NULL && seg_samp->Width() > 0) {
      ar_mean += (1.0 * seg_samp->Height() / seg_samp->Width());
    }
  }
  ar_mean /= (seg_cnt + 1);
  // perform normalization if segment AR is too high
  if (ar_mean > kMinNormalizationAspectRatio) {
    // scale down the image in the y-direction to attain AR
    CharSamp *new_samp = char_samp_->Scale(char_samp_->Width(),
                                           2.0 * char_samp_->Height() / ar_mean,
                                           false);
    if (new_samp != NULL) {
      // free existing char samp if owned
      if (own_char_samp_) {
        delete char_samp_;
      }
      // update with new scaled charsamp and set ownership flag
      char_samp_ = new_samp;
      own_char_samp_ = true;
    }
  }
  delete srch_obj;
  return true;
}
}
