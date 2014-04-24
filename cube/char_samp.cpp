/**********************************************************************
 * File:        char_samp.cpp
 * Description: Implementation of a Character Bitmap Sample Class
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

#include <string.h>
#include <string>
#include "char_samp.h"
#include "cube_utils.h"

namespace tesseract {

#define MAX_LINE_LEN  1024

CharSamp::CharSamp()
    : Bmp8(0, 0) {
  left_ = 0;
  top_ = 0;
  label32_ = NULL;
  page_ = -1;
}

CharSamp::CharSamp(int wid, int hgt)
    : Bmp8(wid, hgt) {
  left_ = 0;
  top_ = 0;
  label32_ = NULL;
  page_ = -1;
}

CharSamp::CharSamp(int left, int top, int wid, int hgt)
    : Bmp8(wid, hgt)
    , left_(left)
    , top_(top) {
  label32_ = NULL;
  page_ = -1;
}

CharSamp::~CharSamp() {
  if (label32_ != NULL) {
    delete []label32_;
    label32_ = NULL;
  }
}

// returns a UTF-8 version of the string label
string CharSamp::stringLabel() const {
  string str = "";
  if (label32_ != NULL) {
    string_32 str32(label32_);
    CubeUtils::UTF32ToUTF8(str32.c_str(), &str);
  }
  return str;
}

// set a the string label using a UTF encoded string
void CharSamp::SetLabel(string str) {
  if (label32_ != NULL) {
    delete []label32_;
    label32_ = NULL;
  }
  string_32 str32;
  CubeUtils::UTF8ToUTF32(str.c_str(), &str32);
  SetLabel(reinterpret_cast<const char_32 *>(str32.c_str()));
}

// creates a CharSamp object from file
CharSamp *CharSamp::FromCharDumpFile(CachedFile *fp) {
  unsigned short left;
  unsigned short top;
  unsigned short page;
  unsigned short first_char;
  unsigned short last_char;
  unsigned short norm_top;
  unsigned short norm_bottom;
  unsigned short norm_aspect_ratio;
  unsigned int val32;

  char_32 *label32;

  // read and check 32 bit marker
  if (fp->Read(&val32, sizeof(val32)) != sizeof(val32)) {
    return NULL;
  }
  if (val32 != 0xabd0fefe) {
    return NULL;
  }
  // read label length,
  if (fp->Read(&val32, sizeof(val32)) != sizeof(val32)) {
    return NULL;
  }
  // the label is not null terminated in the file
  if (val32 > 0 && val32 < MAX_UINT32) {
    label32 = new char_32[val32 + 1];
    if (label32 == NULL) {
      return NULL;
    }
    // read label
    if (fp->Read(label32, val32 * sizeof(*label32)) !=
        (val32 * sizeof(*label32))) {
      return NULL;
    }
    // null terminate
    label32[val32] = 0;
  } else {
    label32 = NULL;
  }
  // read coordinates
  if (fp->Read(&page, sizeof(page)) != sizeof(page)) {
    return NULL;
  }
  if (fp->Read(&left, sizeof(left)) != sizeof(left)) {
    return NULL;
  }
  if (fp->Read(&top, sizeof(top)) != sizeof(top)) {
    return NULL;
  }
  if (fp->Read(&first_char, sizeof(first_char)) != sizeof(first_char)) {
    return NULL;
  }
  if (fp->Read(&last_char, sizeof(last_char)) != sizeof(last_char)) {
    return NULL;
  }
  if (fp->Read(&norm_top, sizeof(norm_top)) != sizeof(norm_top)) {
    return NULL;
  }
  if (fp->Read(&norm_bottom, sizeof(norm_bottom)) != sizeof(norm_bottom)) {
    return NULL;
  }
  if (fp->Read(&norm_aspect_ratio, sizeof(norm_aspect_ratio)) !=
      sizeof(norm_aspect_ratio)) {
    return NULL;
  }
  // create the object
  CharSamp *char_samp = new CharSamp();
  if (char_samp == NULL) {
    return NULL;
  }
  // init
  char_samp->label32_ = label32;
  char_samp->page_ = page;
  char_samp->left_ = left;
  char_samp->top_ = top;
  char_samp->first_char_ = first_char;
  char_samp->last_char_ = last_char;
  char_samp->norm_top_ = norm_top;
  char_samp->norm_bottom_ = norm_bottom;
  char_samp->norm_aspect_ratio_ = norm_aspect_ratio;
  // load the Bmp8 part
  if (char_samp->LoadFromCharDumpFile(fp) == false) {
    delete char_samp;
    return NULL;
  }
  return char_samp;
}

// Load a Char Samp from a dump file
CharSamp *CharSamp::FromCharDumpFile(FILE *fp) {
  unsigned short left;
  unsigned short top;
  unsigned short page;
  unsigned short first_char;
  unsigned short last_char;
  unsigned short norm_top;
  unsigned short norm_bottom;
  unsigned short norm_aspect_ratio;
  unsigned int val32;
  char_32 *label32;

  // read and check 32 bit marker
  if (fread(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return NULL;
  }
  if (val32 != 0xabd0fefe) {
    return NULL;
  }
  // read label length,
  if (fread(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return NULL;
  }
  // the label is not null terminated in the file
  if (val32 > 0 && val32 < MAX_UINT32) {
    label32 = new char_32[val32 + 1];
    if (label32 == NULL) {
      return NULL;
    }
    // read label
    if (fread(label32, 1, val32 * sizeof(*label32), fp) !=
        (val32 * sizeof(*label32))) {
      delete [] label32;
      return NULL;
    }
    // null terminate
    label32[val32] = 0;
  } else {
    label32 = NULL;
  }
  // read coordinates
  if (fread(&page, 1, sizeof(page), fp) != sizeof(page) ||
      fread(&left, 1, sizeof(left), fp) != sizeof(left) ||
      fread(&top, 1, sizeof(top), fp) != sizeof(top) ||
      fread(&first_char, 1, sizeof(first_char), fp) != sizeof(first_char) ||
      fread(&last_char, 1, sizeof(last_char), fp) != sizeof(last_char) ||
      fread(&norm_top, 1, sizeof(norm_top), fp) != sizeof(norm_top) ||
      fread(&norm_bottom, 1, sizeof(norm_bottom), fp) != sizeof(norm_bottom) ||
      fread(&norm_aspect_ratio, 1, sizeof(norm_aspect_ratio), fp) !=
          sizeof(norm_aspect_ratio)) {
    delete [] label32;
    return NULL;
  }
  // create the object
  CharSamp *char_samp = new CharSamp();
  if (char_samp == NULL) {
    delete [] label32;
    return NULL;
  }
  // init
  char_samp->label32_ = label32;
  char_samp->page_ = page;
  char_samp->left_ = left;
  char_samp->top_ = top;
  char_samp->first_char_ = first_char;
  char_samp->last_char_ = last_char;
  char_samp->norm_top_ = norm_top;
  char_samp->norm_bottom_ = norm_bottom;
  char_samp->norm_aspect_ratio_ = norm_aspect_ratio;
  // load the Bmp8 part
  if (char_samp->LoadFromCharDumpFile(fp) == false) {
    delete char_samp;  // It owns label32.
    return NULL;
  }
  return char_samp;
}

// returns a copy of the charsamp that is scaled to the
// specified width and height
CharSamp *CharSamp::Scale(int wid, int hgt, bool isotropic) {
  CharSamp *scaled_samp = new CharSamp(wid, hgt);
  if (scaled_samp == NULL) {
    return NULL;
  }
  if (scaled_samp->ScaleFrom(this, isotropic) == false) {
    delete scaled_samp;
    return NULL;
  }
  scaled_samp->left_ = left_;
  scaled_samp->top_ = top_;
  scaled_samp->page_ = page_;
  scaled_samp->SetLabel(label32_);
  scaled_samp->first_char_ = first_char_;
  scaled_samp->last_char_ = last_char_;
  scaled_samp->norm_top_ = norm_top_;
  scaled_samp->norm_bottom_ = norm_bottom_;
  scaled_samp->norm_aspect_ratio_ = norm_aspect_ratio_;
  return scaled_samp;
}

// Load a Char Samp from a dump file
CharSamp *CharSamp::FromRawData(int left, int top, int wid, int hgt,
                                unsigned char *data) {
  // create the object
  CharSamp *char_samp = new CharSamp(left, top, wid, hgt);
  if (char_samp == NULL) {
    return NULL;
  }
  if (char_samp->LoadFromRawData(data) == false) {
    delete char_samp;
    return NULL;
  }
  return char_samp;
}

// Saves the charsamp to a dump file
bool CharSamp::Save2CharDumpFile(FILE *fp) const {
  unsigned int val32;
  // write and check 32 bit marker
  val32 = 0xabd0fefe;
  if (fwrite(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return false;
  }
  // write label length
  val32 = (label32_ == NULL) ? 0 : LabelLen(label32_);
  if (fwrite(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return false;
  }
  // write label
  if (label32_ != NULL) {
    if (fwrite(label32_, 1, val32 * sizeof(*label32_), fp) !=
        (val32 * sizeof(*label32_))) {
      return false;
    }
  }
  // write coordinates
  if (fwrite(&page_, 1, sizeof(page_), fp) != sizeof(page_)) {
    return false;
  }
  if (fwrite(&left_, 1, sizeof(left_), fp) != sizeof(left_)) {
    return false;
  }
  if (fwrite(&top_, 1, sizeof(top_), fp) != sizeof(top_)) {
    return false;
  }
  if (fwrite(&first_char_, 1, sizeof(first_char_), fp) !=
      sizeof(first_char_)) {
    return false;
  }
  if (fwrite(&last_char_, 1, sizeof(last_char_), fp) != sizeof(last_char_)) {
    return false;
  }
  if (fwrite(&norm_top_, 1, sizeof(norm_top_), fp) != sizeof(norm_top_)) {
    return false;
  }
  if (fwrite(&norm_bottom_, 1, sizeof(norm_bottom_), fp) !=
      sizeof(norm_bottom_)) {
    return false;
  }
  if (fwrite(&norm_aspect_ratio_, 1, sizeof(norm_aspect_ratio_), fp) !=
      sizeof(norm_aspect_ratio_)) {
    return false;
  }
  if (SaveBmp2CharDumpFile(fp) == false) {
    return false;
  }
  return true;
}

// Crop the char samp such that there are no white spaces on any side.
// The norm_top_ and norm_bottom_ fields are the character top/bottom
// with respect to whatever context the character is being recognized
// in (e.g. word bounding box) normalized to a standard size of
// 255. Here they default to 0 and 255 (word box boundaries), but
// since they are context dependent, they may need to be reset by the
// calling function.
CharSamp *CharSamp::Crop() {
  // get the dimesions of the cropped img
  int cropped_left = 0;
  int cropped_top = 0;
  int cropped_wid = wid_;
  int cropped_hgt = hgt_;
  Bmp8::Crop(&cropped_left, &cropped_top,
             &cropped_wid, &cropped_hgt);

  if (cropped_wid == 0 || cropped_hgt == 0) {
    return NULL;
  }
  // create the cropped char samp
  CharSamp *cropped_samp = new CharSamp(left_ + cropped_left,
                                        top_ + cropped_top,
                                        cropped_wid, cropped_hgt);
  cropped_samp->SetLabel(label32_);
  cropped_samp->SetFirstChar(first_char_);
  cropped_samp->SetLastChar(last_char_);
  // the following 3 fields may/should be reset by the calling function
  // using context information, i.e., location of character box
  // w.r.t. the word bounding box
  cropped_samp->SetNormAspectRatio(255 *
                                   cropped_wid / (cropped_wid + cropped_hgt));
  cropped_samp->SetNormTop(0);
  cropped_samp->SetNormBottom(255);

  // copy the bitmap to the cropped img
  Copy(cropped_left, cropped_top, cropped_wid, cropped_hgt, cropped_samp);
  return cropped_samp;
}

// segment the char samp to connected components
// based on contiguity and vertical pixel density histogram
ConComp **CharSamp::Segment(int *segment_cnt, bool right_2_left,
                            int max_hist_wnd, int min_con_comp_size) const {
  // init
  (*segment_cnt) = 0;
  int concomp_cnt = 0;
  int seg_cnt = 0;
  // find the concomps of the image
  ConComp **concomp_array = FindConComps(&concomp_cnt, min_con_comp_size);
  if (concomp_cnt <= 0 || !concomp_array) {
    if (concomp_array)
      delete []concomp_array;
    return NULL;
  }
  ConComp **seg_array = NULL;
  // segment each concomp further using vertical histogram
  for (int concomp = 0; concomp < concomp_cnt; concomp++) {
    int concomp_seg_cnt = 0;
    // segment the concomp
    ConComp **concomp_seg_array = NULL;
    ConComp **concomp_alloc_seg =
        concomp_array[concomp]->Segment(max_hist_wnd, &concomp_seg_cnt);
    // no segments, add the whole concomp
    if (concomp_alloc_seg == NULL) {
      concomp_seg_cnt = 1;
      concomp_seg_array = concomp_array + concomp;
    } else {
      // delete the original concomp, we no longer need it
      concomp_seg_array = concomp_alloc_seg;
      delete concomp_array[concomp];
    }
    // add the resulting segments
    for (int seg_idx = 0; seg_idx < concomp_seg_cnt; seg_idx++) {
      // too small of a segment: ignore
      if (concomp_seg_array[seg_idx]->Width() < 2 &&
          concomp_seg_array[seg_idx]->Height() < 2) {
        delete concomp_seg_array[seg_idx];
      } else {
        // add the new segment
        // extend the segment array
        if ((seg_cnt % kConCompAllocChunk) == 0) {
          ConComp **temp_segm_array =
              new ConComp *[seg_cnt + kConCompAllocChunk];
          if (temp_segm_array == NULL) {
            fprintf(stderr, "Cube ERROR (CharSamp::Segment): could not "
                    "allocate additional connected components\n");
            delete []concomp_seg_array;
            delete []concomp_array;
            delete []seg_array;
            return NULL;
          }
          if (seg_cnt > 0) {
            memcpy(temp_segm_array, seg_array, seg_cnt * sizeof(*seg_array));
            delete []seg_array;
          }
          seg_array = temp_segm_array;
        }
        seg_array[seg_cnt++] = concomp_seg_array[seg_idx];
      }
    }  // segment
    if (concomp_alloc_seg != NULL) {
      delete []concomp_alloc_seg;
    }
  }  // concomp
  delete []concomp_array;

  // sort the concomps from Left2Right or Right2Left, based on the reading order
  if (seg_cnt > 0 && seg_array != NULL) {
    qsort(seg_array, seg_cnt, sizeof(*seg_array), right_2_left ?
        ConComp::Right2LeftComparer : ConComp::Left2RightComparer);
  }
  (*segment_cnt) = seg_cnt;
  return seg_array;
}

// builds a char samp from a set of connected components
CharSamp *CharSamp::FromConComps(ConComp **concomp_array, int strt_concomp,
                                 int seg_flags_size, int *seg_flags,
                                 bool *left_most, bool *right_most,
                                 int word_hgt) {
  int concomp;
  int end_concomp;
  int concomp_cnt = 0;
  end_concomp = strt_concomp + seg_flags_size;
  // determine ID range
  bool once = false;
  int min_id = -1;
  int max_id = -1;
  for (concomp = strt_concomp; concomp < end_concomp; concomp++) {
    if (!seg_flags || seg_flags[concomp - strt_concomp] != 0) {
      if (!once) {
        min_id = concomp_array[concomp]->ID();
        max_id = concomp_array[concomp]->ID();
        once = true;
      } else {
        UpdateRange(concomp_array[concomp]->ID(), &min_id, &max_id);
      }
      concomp_cnt++;
    }
  }
  if (concomp_cnt < 1 || !once || min_id == -1 || max_id == -1) {
    return NULL;
  }
  // alloc memo for computing leftmost and right most attributes
  int id_cnt = max_id - min_id + 1;
  bool *id_exist = new bool[id_cnt];
  bool *left_most_exist = new bool[id_cnt];
  bool *right_most_exist = new bool[id_cnt];
  if (!id_exist || !left_most_exist || !right_most_exist)
    return NULL;
  memset(id_exist, 0, id_cnt * sizeof(*id_exist));
  memset(left_most_exist, 0, id_cnt * sizeof(*left_most_exist));
  memset(right_most_exist, 0, id_cnt * sizeof(*right_most_exist));
  // find the dimensions of the charsamp
  once = false;
  int left = -1;
  int right = -1;
  int top = -1;
  int bottom = -1;
  int unq_ids = 0;
  int unq_left_most = 0;
  int unq_right_most = 0;
  for (concomp = strt_concomp; concomp < end_concomp; concomp++) {
    if (!seg_flags || seg_flags[concomp - strt_concomp] != 0) {
      if (!once) {
        left = concomp_array[concomp]->Left();
        right = concomp_array[concomp]->Right();
        top = concomp_array[concomp]->Top();
        bottom = concomp_array[concomp]->Bottom();
        once = true;
      } else {
        UpdateRange(concomp_array[concomp]->Left(),
                    concomp_array[concomp]->Right(), &left, &right);
        UpdateRange(concomp_array[concomp]->Top(),
                    concomp_array[concomp]->Bottom(), &top, &bottom);
      }
      // count unq ids, unq left most and right mosts ids
      int concomp_id = concomp_array[concomp]->ID() - min_id;
      if (!id_exist[concomp_id]) {
        id_exist[concomp_id] = true;
        unq_ids++;
      }
      if (concomp_array[concomp]->LeftMost()) {
        if (left_most_exist[concomp_id] == false) {
          left_most_exist[concomp_id] = true;
          unq_left_most++;
        }
      }
      if (concomp_array[concomp]->RightMost()) {
        if (right_most_exist[concomp_id] == false) {
          right_most_exist[concomp_id] = true;
          unq_right_most++;
        }
      }
    }
  }
  delete []id_exist;
  delete []left_most_exist;
  delete []right_most_exist;
  if (!once || left == -1 || top == -1 || right == -1 || bottom == -1) {
    return NULL;
  }
  (*left_most) = (unq_left_most >= unq_ids);
  (*right_most) = (unq_right_most >= unq_ids);
  // create the char sample object
  CharSamp *samp = new CharSamp(left, top, right - left + 1, bottom - top + 1);
  if (!samp) {
    return NULL;
  }

  // set the foreground pixels
  for (concomp = strt_concomp; concomp < end_concomp; concomp++) {
    if (!seg_flags || seg_flags[concomp - strt_concomp] != 0) {
      ConCompPt *pt_ptr = concomp_array[concomp]->Head();
      while (pt_ptr) {
        samp->line_buff_[pt_ptr->y() - top][pt_ptr->x() - left] = 0;
        pt_ptr = pt_ptr->Next();
      }
    }
  }
  return samp;
}

// clones the object
CharSamp *CharSamp::Clone() const {
  // create the cropped char samp
  CharSamp *samp = new CharSamp(left_, top_, wid_, hgt_);
  samp->SetLabel(label32_);
  samp->SetFirstChar(first_char_);
  samp->SetLastChar(last_char_);
  samp->SetNormTop(norm_top_);
  samp->SetNormBottom(norm_bottom_);
  samp->SetNormAspectRatio(norm_aspect_ratio_);
  // copy the bitmap to the cropped img
  Copy(0, 0, wid_, hgt_, samp);
  return samp;
}

// Load a Char Samp from a dump file
CharSamp *CharSamp::FromCharDumpFile(unsigned char **raw_data_ptr) {
  unsigned int val32;
  char_32 *label32;
  unsigned char *raw_data = *raw_data_ptr;

  // read and check 32 bit marker
  memcpy(&val32, raw_data, sizeof(val32));
  raw_data += sizeof(val32);
  if (val32 != 0xabd0fefe) {
    return NULL;
  }
  // read label length,
  memcpy(&val32, raw_data, sizeof(val32));
  raw_data += sizeof(val32);
  // the label is not null terminated in the file
  if (val32 > 0 && val32 < MAX_UINT32) {
    label32 = new char_32[val32 + 1];
    if (label32 == NULL) {
      return NULL;
    }
    // read label
    memcpy(label32, raw_data, val32 * sizeof(*label32));
    raw_data += (val32 * sizeof(*label32));
    // null terminate
    label32[val32] = 0;
  } else {
    label32 = NULL;
  }

  // create the object
  CharSamp *char_samp = new CharSamp();
  if (char_samp == NULL) {
    return NULL;
  }

  // read coordinates
  char_samp->label32_ = label32;
  memcpy(&char_samp->page_, raw_data, sizeof(char_samp->page_));
  raw_data += sizeof(char_samp->page_);
  memcpy(&char_samp->left_, raw_data, sizeof(char_samp->left_));
  raw_data += sizeof(char_samp->left_);
  memcpy(&char_samp->top_, raw_data, sizeof(char_samp->top_));
  raw_data += sizeof(char_samp->top_);
  memcpy(&char_samp->first_char_, raw_data, sizeof(char_samp->first_char_));
  raw_data += sizeof(char_samp->first_char_);
  memcpy(&char_samp->last_char_, raw_data, sizeof(char_samp->last_char_));
  raw_data += sizeof(char_samp->last_char_);
  memcpy(&char_samp->norm_top_, raw_data, sizeof(char_samp->norm_top_));
  raw_data += sizeof(char_samp->norm_top_);
  memcpy(&char_samp->norm_bottom_, raw_data, sizeof(char_samp->norm_bottom_));
  raw_data += sizeof(char_samp->norm_bottom_);
  memcpy(&char_samp->norm_aspect_ratio_, raw_data,
         sizeof(char_samp->norm_aspect_ratio_));
  raw_data += sizeof(char_samp->norm_aspect_ratio_);

  // load the Bmp8 part
  if (char_samp->LoadFromCharDumpFile(&raw_data) == false) {
    delete char_samp;
    return NULL;
  }

  (*raw_data_ptr) = raw_data;
  return char_samp;
}

// computes the features corresponding to the char sample
bool CharSamp::ComputeFeatures(int conv_grid_size, float *features) {
  // Create a scaled BMP
  CharSamp *scaled_bmp = Scale(conv_grid_size, conv_grid_size);
  if (!scaled_bmp) {
    return false;
  }
  // prepare input
  unsigned char *buff = scaled_bmp->RawData();
  // bitmap features
  int input;
  int bmp_size = conv_grid_size * conv_grid_size;
  for (input = 0; input < bmp_size; input++) {
    features[input] = 255.0f - (1.0f * buff[input]);
  }
  // word context features
  features[input++] = FirstChar();
  features[input++] = LastChar();
  features[input++] = NormTop();
  features[input++] = NormBottom();
  features[input++] = NormAspectRatio();
  delete scaled_bmp;
  return true;
}
}  // namespace tesseract
