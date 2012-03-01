/**********************************************************************
 * File:        statistc.h  (Formerly stats.h)
 * Description: Class description for STATS class.
 * Author:					Ray Smith
 * Created:					Mon Feb 04 16:19:07 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
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

#ifndef TESSERACT_CCSTRUCT_STATISTC_H_
#define TESSERACT_CCSTRUCT_STATISTC_H_

#include <stdio.h>
#include "host.h"
#include "scrollview.h"

// Simple histogram-based statistics for integer values in a known
// range, such that the range is small compared to the number of samples.
class STATS {
 public:
  // The histogram buckets are in the range
  // [min_bucket_value, max_bucket_value_plus_1 - 1] i.e.
  // [min_bucket_value, max_bucket_value].
  // Any data under min_bucket value is silently mapped to min_bucket_value,
  // and likewise, any data over max_bucket_value is silently mapped to
  // max_bucket_value.
  // In the internal array, min_bucket_value maps to 0 and
  // max_bucket_value_plus_1 - min_bucket_value to the array size.
  // TODO(rays) This is ugly. Convert the second argument to
  // max_bucket_value and all the code that uses it.
  STATS(inT32 min_bucket_value, inT32 max_bucket_value_plus_1);
  STATS();  // empty for arrays

  ~STATS();

  // (Re)Sets the range and clears the counts.
  // See the constructor for info on max and min values.
  bool set_range(inT32 min_bucket_value, inT32 max_bucket_value_plus_1);

  void clear();  // empty buckets

  void add(inT32 value, inT32 count);

  // "Accessors" return various statistics on the data.
  inT32 mode() const;  // get mode of samples
  double mean() const;  // get mean of samples
  double sd() const;  // standard deviation
  // Returns the fractile value such that frac fraction (in [0,1]) of samples
  // has a value less than the return value.
  double ile(double frac) const;
  // Returns the minimum used entry in the histogram (ie the minimum of the
  // data, NOT the minimum of the supplied range, nor is it an index.)
  // Would normally be called min(), but that is a reserved word in VC++.
  inT32 min_bucket() const;  // Find min
  // Returns the maximum used entry in the histogram (ie the maximum of the
  // data, NOT the maximum of the supplied range, nor is it an index.)
  inT32 max_bucket() const;  // Find max
  // Finds a more useful estimate of median than ile(0.5).
  // Overcomes a problem with ile() - if the samples are, for example,
  // 6,6,13,14 ile(0.5) return 7.0 - when a more useful value would be midway
  // between 6 and 13 = 9.5
  double median() const;  // get median of samples
  // Returns the count of the given value.
  inT32 pile_count(inT32 value ) const {
    if (value <= rangemin_)
      return buckets_[0];
    if (value >= rangemax_ - 1)
      return buckets_[rangemax_ - rangemin_ - 1];
    return buckets_[value - rangemin_];
  }
  // Returns the total count of all buckets.
  inT32 get_total() const {
    return total_count_;        // total of all piles
  }
  // Returns true if x is a local min.
  bool local_min(inT32 x) const;

  // Apply a triangular smoothing filter to the stats.
  // This makes the modes a bit more useful.
  // The factor gives the height of the triangle, i.e. the weight of the
  // centre.
  void smooth(inT32 factor);

  // Cluster the samples into max_cluster clusters.
  // Each call runs one iteration. The array of clusters must be
  // max_clusters+1 in size as cluster 0 is used to indicate which samples
  // have been used.
  // The return value is the current number of clusters.
  inT32 cluster(float lower,         // thresholds
                float upper,
                float multiple,      // distance threshold
                inT32 max_clusters,  // max no to make
                STATS *clusters);    // array of clusters


  // Prints a summary and table of the histogram.
  void print() const;
  // Prints summary stats only of the histogram.
  void print_summary() const;

  #ifndef GRAPHICS_DISABLED
  // Draws the histogram as a series of rectangles.
  void plot(ScrollView* window,   // window to draw in
            float xorigin,   // origin of histo
            float yorigin,   // gram
            float xscale,    // size of one unit
            float yscale,    // size of one uint
            ScrollView::Color colour) const;  // colour to draw in

  // Draws a line graph of the histogram.
  void plotline(ScrollView* window,   // window to draw in
                float xorigin,   // origin of histo
                float yorigin,   // gram
                float xscale,    // size of one unit
                float yscale,    // size of one uint
                ScrollView::Color colour) const;  // colour to draw in
  #endif  // GRAPHICS_DISABLED

 private:
  inT32 rangemin_;                // min of range
  // rangemax_ is not well named as it is really one past the max.
  inT32 rangemax_;                // max of range
  inT32 total_count_;             // no of samples
  inT32* buckets_;                // array of cells
};

// Returns the nth ordered item from the array, as if they were
// ordered, but without ordering them, in linear time.
// The array does get shuffled!
inT32 choose_nth_item(inT32 index,   // index to choose
                      float *array,  // array of items
                      inT32 count);  // no of items
// Generic version uses a defined comparator (with qsort semantics).
inT32 choose_nth_item(inT32 index,   // index to choose
                      void *array,   // array of items
                      inT32 count,   // no of items
                      size_t size,   // element size
                      int (*compar)(const void*, const void*));  // comparator
// Swaps 2 entries in an array in-place.
void swap_entries(void *array,   // array of entries
                  size_t size,   // size of entry
                  inT32 index1,  // entries to swap
                  inT32 index2);

#endif  // TESSERACT_CCSTRUCT_STATISTC_H_
