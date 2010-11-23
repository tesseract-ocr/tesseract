/**********************************************************************
 * File:        con_comp.h
 * Description: Declaration of a Connected Component class
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

#ifndef CONCOMP_H
#define CONCOMP_H

// The ConComp class implements the functionality needed for a
// Connected Component object and Connected Component (ConComp) points.
// The points consituting a connected component are kept in a linked-list
// The Concomp class provided methods to:
// 1- Compare components in L2R and R2L reading orders.
// 2- Merge ConComps
// 3- Compute the windowed vertical pixel density histogram for a specific
// windows size
// 4- Segment a ConComp based on the local windowed vertical pixel
// density histogram local minima

namespace tesseract {

// Implments a ConComp point in a linked list of points
class ConCompPt {
 public:
    ConCompPt(int x, int y) {
      x_ = x;
      y_ = y;
      next_pt_ = NULL;
    }
    inline int x() { return x_; }
    inline int y() { return y_; }
    inline void Shift(int dx, int dy) {
      x_ += dx;
      y_ += dy;
    }
    inline ConCompPt * Next() { return next_pt_; }
    inline void SetNext(ConCompPt *pt) { next_pt_ = pt; }

 private:
    int x_;
    int y_;
    ConCompPt *next_pt_;
};

class ConComp {
 public:
    ConComp();
    virtual ~ConComp();
    // accessors
    inline ConCompPt *Head() { return head_; }
    inline int Left() const { return left_; }
    inline int Top() const { return top_; }
    inline int Right() const { return right_; }
    inline int Bottom() const { return bottom_; }
    inline int Width() const { return right_ - left_ + 1; }
    inline int Height() const { return bottom_ - top_ + 1; }

    // Comparer used for sorting L2R reading order
    inline static int Left2RightComparer(const void *comp1,
                                         const void *comp2) {
      return (*(reinterpret_cast<ConComp * const *>(comp1)))->left_ +
          (*(reinterpret_cast<ConComp * const *>(comp1)))->right_ -
          (*(reinterpret_cast<ConComp * const *>(comp2)))->left_ -
          (*(reinterpret_cast<ConComp * const *>(comp2)))->right_;
    }

    // Comparer used for sorting R2L reading order
    inline static int Right2LeftComparer(const void *comp1,
                                         const void *comp2) {
      return (*(reinterpret_cast<ConComp * const *>(comp2)))->right_ -
          (*(reinterpret_cast<ConComp * const *>(comp1)))->right_;
    }

    // accessors for attribues of a ConComp
    inline bool LeftMost() const { return left_most_; }
    inline bool RightMost() const { return right_most_; }
    inline void SetLeftMost(bool left_most) { left_most_ = left_most; }
    inline void SetRightMost(bool right_most) { right_most_ = right_most;
    }
    inline int ID () const { return id_; }
    inline void SetID(int id) { id_ = id; }
    inline int PtCnt () const { return pt_cnt_; }
    // Add a new pt
    bool Add(int x, int y);
    // Merge two connected components in-place
    bool Merge(ConComp *con_comp);
    // Shifts the co-ordinates of all points by the specified x & y deltas
    void Shift(int dx, int dy);
    // segments a concomp based on pixel density histogram local minima
    ConComp **Segment(int max_hist_wnd, int *concomp_cnt);
    // creates the vertical pixel density histogram of the concomp
    int *CreateHistogram(int max_hist_wnd);
    // find out the seg pts by looking for local minima in the histogram
    int *SegmentHistogram(int *hist_array, int *seg_pt_cnt);

 private:
    int id_;
    bool left_most_;
    bool right_most_;
    int left_;
    int top_;
    int right_;
    int bottom_;
    ConCompPt *head_;
    ConCompPt *tail_;
    int pt_cnt_;
};
}

#endif  // CONCOMP_H
