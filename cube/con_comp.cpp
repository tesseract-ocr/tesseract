/**********************************************************************
 * File:        con_comp.cpp
 * Description: Implementation of a Connected Component class
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

#include <stdlib.h>
#include <string.h>
#include "con_comp.h"
#include "cube_const.h"

namespace tesseract {

ConComp::ConComp() {
  head_ = NULL;
  tail_ = NULL;
  left_ = 0;
  top_ = 0;
  right_ = 0;
  bottom_ = 0;
  left_most_ = false;
  right_most_ = false;
  id_ = -1;
  pt_cnt_ = 0;
}

ConComp::~ConComp() {
  if (head_ != NULL) {
    ConCompPt *pt_ptr = head_;
    while (pt_ptr != NULL) {
      ConCompPt *pptNext = pt_ptr->Next();
      delete pt_ptr;
      pt_ptr = pptNext;
    }
    head_ = NULL;
  }
}

// adds a pt to the conn comp and updates its boundaries
bool ConComp::Add(int x, int y) {
  ConCompPt *pt_ptr = new ConCompPt(x, y);

  if (head_ == NULL) {
    left_ = x;
    right_ = x;
    top_ = y;
    bottom_ = y;

    head_ = pt_ptr;
  } else {
    left_ = left_ <= x ? left_ : x;
    top_ = top_ <= y ? top_ : y;
    right_ = right_ >= x ? right_ : x;
    bottom_ = bottom_ >= y ? bottom_ : y;
  }

  if (tail_ != NULL) {
    tail_->SetNext(pt_ptr);
  }

  tail_ = pt_ptr;
  pt_cnt_++;
  return true;
}

// merges two connected components
bool ConComp::Merge(ConComp *concomp) {
  if (head_ == NULL || tail_ == NULL ||
      concomp->head_ == NULL || concomp->tail_ == NULL) {
    return false;
  }

  tail_->SetNext(concomp->head_);
  tail_ = concomp->tail_;
  left_ = left_ <= concomp->left_ ? left_ : concomp->left_;
  top_ = top_ <= concomp->top_ ? top_ : concomp->top_;
  right_ = right_ >= concomp->right_ ? right_ : concomp->right_;
  bottom_ = bottom_ >= concomp->bottom_ ? bottom_ : concomp->bottom_;
  pt_cnt_ += concomp->pt_cnt_;

  concomp->head_ = NULL;
  concomp->tail_ = NULL;

  return true;
}

// Creates the x-coord density histogram after spreading
// each x-coord position by the HIST_WND_RATIO fraction of the
// height of the ConComp, but limited to max_hist_wnd
int *ConComp::CreateHistogram(int max_hist_wnd) {
  int wid = right_ - left_ + 1,
    hgt = bottom_ - top_ + 1,
    hist_wnd = static_cast<int>(hgt * HIST_WND_RATIO);

  if (hist_wnd > max_hist_wnd) {
      hist_wnd = max_hist_wnd;
  }

  // alloc memo for histogram
  int *hist_array = new int[wid];

  memset(hist_array, 0, wid * sizeof(*hist_array));

  // compute windowed histogram
  ConCompPt *pt_ptr = head_;

  while (pt_ptr != NULL) {
    int x = pt_ptr->x() - left_,
      xw = x - hist_wnd;

    for (int xdel = -hist_wnd; xdel <= hist_wnd; xdel++, xw++) {
      if (xw >= 0 && xw < wid) {
        hist_array[xw]++;
      }
    }

    pt_ptr = pt_ptr->Next();
  }

  return hist_array;
}

// find out the seg pts by looking for local minima in the histogram
int *ConComp::SegmentHistogram(int *hist_array, int *seg_pt_cnt) {
  // init
  (*seg_pt_cnt) = 0;

  int wid = right_ - left_ + 1,
    hgt = bottom_ - top_ + 1;

  int *x_seg_pt = new int[wid];

  int seg_pt_wnd = static_cast<int>(hgt * SEG_PT_WND_RATIO);

  if (seg_pt_wnd > 1) {
    seg_pt_wnd = 1;
  }

  for (int x = 2; x < (wid - 2); x++) {
    if (hist_array[x] < hist_array[x - 1] &&
        hist_array[x] < hist_array[x - 2] &&
        hist_array[x] <= hist_array[x + 1] &&
        hist_array[x] <= hist_array[x + 2]) {
      x_seg_pt[(*seg_pt_cnt)++] = x;
      x += seg_pt_wnd;
    } else if (hist_array[x] <= hist_array[x - 1] &&
               hist_array[x] <= hist_array[x - 2] &&
               hist_array[x] < hist_array[x + 1] &&
               hist_array[x] < hist_array[x + 2]) {
      x_seg_pt[(*seg_pt_cnt)++] = x;
      x += seg_pt_wnd;
    }
  }

  // no segments, nothing to do
  if ((*seg_pt_cnt) == 0) {
    delete []x_seg_pt;
    return NULL;
  }

  return x_seg_pt;
}

// segments a concomp based on pixel density histogram local minima
// if there were none found, it returns NULL
// this is more useful than creating a clone of itself
ConComp **ConComp::Segment(int max_hist_wnd, int *concomp_cnt) {
  // init
  (*concomp_cnt) = 0;

  // No pts
  if (head_ == NULL) {
    return NULL;
  }

  int seg_pt_cnt = 0;

  // create the histogram
  int *hist_array = CreateHistogram(max_hist_wnd);
  if (hist_array == NULL) {
    return NULL;
  }

  int *x_seg_pt = SegmentHistogram(hist_array, &seg_pt_cnt);

  // free histogram
  delete []hist_array;

  // no segments, nothing to do
  if (seg_pt_cnt == 0) {
    delete []x_seg_pt;
    return NULL;
  }

  // create concomp array
  ConComp **concomp_array = new ConComp *[seg_pt_cnt + 1];

  for (int concomp = 0; concomp <= seg_pt_cnt; concomp++) {
    concomp_array[concomp] = new ConComp();

    // split concomps inherit the ID this concomp
    concomp_array[concomp]->SetID(id_);
  }

  // set the left and right most attributes of the
  // appropriate concomps
  concomp_array[0]->left_most_ = true;
  concomp_array[seg_pt_cnt]->right_most_ = true;

  // assign pts to concomps
  ConCompPt *pt_ptr = head_;
  while (pt_ptr != NULL) {
    int seg_pt;

    // find the first seg-pt that exceeds the x value
    // of the pt
    for (seg_pt = 0; seg_pt < seg_pt_cnt; seg_pt++) {
      if ((x_seg_pt[seg_pt] + left_) > pt_ptr->x()) {
        break;
      }
    }

    // add the pt to the proper concomp
    if (concomp_array[seg_pt]->Add(pt_ptr->x(), pt_ptr->y()) == false) {
      delete []x_seg_pt;
      delete []concomp_array;
      return NULL;
    }

    pt_ptr = pt_ptr->Next();
  }

  delete []x_seg_pt;

  (*concomp_cnt) = (seg_pt_cnt + 1);

  return concomp_array;
}

// Shifts the co-ordinates of all points by the specified x & y deltas
void ConComp::Shift(int dx, int dy) {
  ConCompPt *pt_ptr = head_;

  while (pt_ptr != NULL) {
    pt_ptr->Shift(dx, dy);
    pt_ptr = pt_ptr->Next();
  }

  left_ += dx;
  right_ += dx;
  top_ += dy;
  bottom_ += dy;
}

}  // namespace tesseract
