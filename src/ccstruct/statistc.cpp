/**********************************************************************
 * File:        statistc.cpp  (Formerly stats.c)
 * Description: Simple statistical package for integer values.
 * Author:      Ray Smith
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

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "statistc.h"

#include "errcode.h"
#include "scrollview.h"
#include "tprintf.h"

#include "helpers.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace tesseract {

/**********************************************************************
 * STATS::STATS
 *
 * Construct a new stats element by allocating and zeroing the memory.
 **********************************************************************/
STATS::STATS(int32_t min_bucket_value, int32_t max_bucket_value) {
  if (max_bucket_value < min_bucket_value) {
    min_bucket_value = 0;
    max_bucket_value = 1;
  }
  rangemin_ = min_bucket_value; // setup
  rangemax_ = max_bucket_value;
  buckets_ = new int32_t[1 + rangemax_ - rangemin_];
  clear();
}

/**********************************************************************
 * STATS::set_range
 *
 * Alter the range on an existing stats element.
 **********************************************************************/
bool STATS::set_range(int32_t min_bucket_value, int32_t max_bucket_value) {
  if (max_bucket_value < min_bucket_value) {
    return false;
  }
  if (rangemax_ - rangemin_ != max_bucket_value - min_bucket_value) {
    delete[] buckets_;
    buckets_ = new int32_t[1 + max_bucket_value - min_bucket_value];
  }
  rangemin_ = min_bucket_value; // setup
  rangemax_ = max_bucket_value;
  clear(); // zero it
  return true;
}

/**********************************************************************
 * STATS::clear
 *
 * Clear out the STATS class by zeroing all the buckets.
 **********************************************************************/
void STATS::clear() { // clear out buckets
  total_count_ = 0;
  if (buckets_ != nullptr) {
    memset(buckets_, 0, (1 + rangemax_ - rangemin_) * sizeof(buckets_[0]));
  }
}

/**********************************************************************
 * STATS::~STATS
 *
 * Destructor for a stats class.
 **********************************************************************/
STATS::~STATS() {
  delete[] buckets_;
}

/**********************************************************************
 * STATS::add
 *
 * Add a set of samples to (or delete from) a pile.
 **********************************************************************/
void STATS::add(int32_t value, int32_t count) {
  if (buckets_ != nullptr) {
    value = ClipToRange(value, rangemin_, rangemax_);
    buckets_[value - rangemin_] += count;
    total_count_ += count; // keep count of total
  }
}

/**********************************************************************
 * STATS::mode
 *
 * Find the mode of a stats class.
 **********************************************************************/
int32_t STATS::mode() const { // get mode of samples
  if (buckets_ == nullptr) {
    return rangemin_;
  }
  int32_t max = buckets_[0]; // max cell count
  int32_t maxindex = 0;      // index of max
  for (int index = rangemax_ - rangemin_; index > 0; --index) {
    if (buckets_[index] > max) {
      max = buckets_[index]; // find biggest
      maxindex = index;
    }
  }
  return maxindex + rangemin_; // index of biggest
}

/**********************************************************************
 * STATS::mean
 *
 * Find the mean of a stats class.
 **********************************************************************/
double STATS::mean() const { // get mean of samples
  if (buckets_ == nullptr || total_count_ <= 0) {
    return static_cast<double>(rangemin_);
  }
  int64_t sum = 0;
  for (int index = rangemax_ - rangemin_; index >= 0; --index) {
    sum += static_cast<int64_t>(index) * buckets_[index];
  }
  return static_cast<double>(sum) / total_count_ + rangemin_;
}

/**********************************************************************
 * STATS::sd
 *
 * Find the standard deviation of a stats class.
 **********************************************************************/
double STATS::sd() const { // standard deviation
  if (buckets_ == nullptr || total_count_ <= 0) {
    return 0.0;
  }
  int64_t sum = 0;
  double sqsum = 0.0;
  for (int index = rangemax_ - rangemin_; index >= 0; --index) {
    sum += static_cast<int64_t>(index) * buckets_[index];
    sqsum += static_cast<double>(index) * index * buckets_[index];
  }
  double variance = static_cast<double>(sum) / total_count_;
  variance = sqsum / total_count_ - variance * variance;
  if (variance > 0.0) {
    return sqrt(variance);
  }
  return 0.0;
}

/**********************************************************************
 * STATS::ile
 *
 * Returns the fractile value such that frac fraction (in [0,1]) of samples
 * has a value less than the return value.
 **********************************************************************/
double STATS::ile(double frac) const {
  if (buckets_ == nullptr || total_count_ == 0) {
    return static_cast<double>(rangemin_);
  }
#if 0
  // TODO(rays) The existing code doesn't seem to be doing the right thing
  // with target a double but this substitute crashes the code that uses it.
  // Investigate and fix properly.
  int target = IntCastRounded(frac * total_count_);
  target = ClipToRange(target, 1, total_count_);
#else
  double target = frac * total_count_;
  target = ClipToRange(target, 1.0, static_cast<double>(total_count_));
#endif
  int sum = 0;
  int index = 0;
  for (index = 0; index <= rangemax_ - rangemin_ && sum < target; sum += buckets_[index++]) {
    ;
  }
  if (index > 0) {
    ASSERT_HOST(buckets_[index - 1] > 0);
    return rangemin_ + index - static_cast<double>(sum - target) / buckets_[index - 1];
  } else {
    return static_cast<double>(rangemin_);
  }
}

/**********************************************************************
 * STATS::min_bucket
 *
 * Find REAL minimum bucket - ile(0.0) isn't necessarily correct
 **********************************************************************/
int32_t STATS::min_bucket() const { // Find min
  if (buckets_ == nullptr || total_count_ == 0) {
    return rangemin_;
  }
  int32_t min = 0;
  for (min = 0; (min <= rangemax_ - rangemin_) && (buckets_[min] == 0); min++) {
    ;
  }
  return rangemin_ + min;
}

/**********************************************************************
 * STATS::max_bucket
 *
 * Find REAL maximum bucket - ile(1.0) isn't necessarily correct
 **********************************************************************/

int32_t STATS::max_bucket() const { // Find max
  if (buckets_ == nullptr || total_count_ == 0) {
    return rangemin_;
  }
  int32_t max;
  for (max = rangemax_ - rangemin_; max > 0 && buckets_[max] == 0; max--) {
    ;
  }
  return rangemin_ + max;
}

/**********************************************************************
 * STATS::median
 *
 * Finds a more useful estimate of median than ile(0.5).
 *
 * Overcomes a problem with ile() - if the samples are, for example,
 * 6,6,13,14 ile(0.5) return 7.0 - when a more useful value would be midway
 * between 6 and 13 = 9.5
 **********************************************************************/
double STATS::median() const { // get median
  if (buckets_ == nullptr) {
    return static_cast<double>(rangemin_);
  }
  double median = ile(0.5);
  int median_pile = static_cast<int>(floor(median));
  if ((total_count_ > 1) && (pile_count(median_pile) == 0)) {
    int32_t min_pile;
    int32_t max_pile;
    /* Find preceding non zero pile */
    for (min_pile = median_pile; pile_count(min_pile) == 0; min_pile--) {
      ;
    }
    /* Find following non zero pile */
    for (max_pile = median_pile; pile_count(max_pile) == 0; max_pile++) {
      ;
    }
    median = (min_pile + max_pile) / 2.0;
  }
  return median;
}

/**********************************************************************
 * STATS::local_min
 *
 * Return true if this point is a local min.
 **********************************************************************/
bool STATS::local_min(int32_t x) const {
  if (buckets_ == nullptr) {
    return false;
  }
  x = ClipToRange(x, rangemin_, rangemax_) - rangemin_;
  if (buckets_[x] == 0) {
    return true;
  }
  int32_t index; // table index
  for (index = x - 1; index >= 0 && buckets_[index] == buckets_[x]; --index) {
    ;
  }
  if (index >= 0 && buckets_[index] < buckets_[x]) {
    return false;
  }
  for (index = x + 1; index <= rangemax_ - rangemin_ && buckets_[index] == buckets_[x]; ++index) {
    ;
  }
  if (index <= rangemax_ - rangemin_ && buckets_[index] < buckets_[x]) {
    return false;
  } else {
    return true;
  }
}

/**********************************************************************
 * STATS::smooth
 *
 * Apply a triangular smoothing filter to the stats.
 * This makes the modes a bit more useful.
 * The factor gives the height of the triangle, i.e. the weight of the
 * centre.
 **********************************************************************/
void STATS::smooth(int32_t factor) {
  if (buckets_ == nullptr || factor < 2) {
    return;
  }
  STATS result(rangemin_, rangemax_);
  int entrycount = 1 + rangemax_ - rangemin_;
  for (int entry = 0; entry < entrycount; entry++) {
    // centre weight
    int count = buckets_[entry] * factor;
    for (int offset = 1; offset < factor; offset++) {
      if (entry - offset >= 0) {
        count += buckets_[entry - offset] * (factor - offset);
      }
      if (entry + offset < entrycount) {
        count += buckets_[entry + offset] * (factor - offset);
      }
    }
    result.add(entry + rangemin_, count);
  }
  total_count_ = result.total_count_;
  memcpy(buckets_, result.buckets_, entrycount * sizeof(buckets_[0]));
}

/**********************************************************************
 * STATS::cluster
 *
 * Cluster the samples into max_cluster clusters.
 * Each call runs one iteration. The array of clusters must be
 * max_clusters+1 in size as cluster 0 is used to indicate which samples
 * have been used.
 * The return value is the current number of clusters.
 **********************************************************************/

int32_t STATS::cluster(float lower, // thresholds
                       float upper,
                       float multiple,       // distance threshold
                       int32_t max_clusters, // max no to make
                       STATS *clusters) {    // array of clusters
  bool new_cluster;                          // added one
  float *centres;                            // cluster centres
  int32_t entry;                             // bucket index
  int32_t cluster;                           // cluster index
  int32_t best_cluster;                      // one to assign to
  int32_t new_centre = 0;                    // residual mode
  int32_t new_mode;                          // pile count of new_centre
  int32_t count;                             // pile to place
  float dist;                                // from cluster
  float min_dist;                            // from best_cluster
  int32_t cluster_count;                     // no of clusters

  if (buckets_ == nullptr || max_clusters < 1) {
    return 0;
  }
  centres = new float[max_clusters + 1];
  for (cluster_count = 1;
       cluster_count <= max_clusters && clusters[cluster_count].buckets_ != nullptr &&
       clusters[cluster_count].total_count_ > 0;
       cluster_count++) {
    centres[cluster_count] = static_cast<float>(clusters[cluster_count].ile(0.5));
    new_centre = clusters[cluster_count].mode();
    for (entry = new_centre - 1; centres[cluster_count] - entry < lower && entry >= rangemin_ &&
                                 pile_count(entry) <= pile_count(entry + 1);
         entry--) {
      count = pile_count(entry) - clusters[0].pile_count(entry);
      if (count > 0) {
        clusters[cluster_count].add(entry, count);
        clusters[0].add(entry, count);
      }
    }
    for (entry = new_centre + 1; entry - centres[cluster_count] < lower && entry <= rangemax_ &&
                                 pile_count(entry) <= pile_count(entry - 1);
         entry++) {
      count = pile_count(entry) - clusters[0].pile_count(entry);
      if (count > 0) {
        clusters[cluster_count].add(entry, count);
        clusters[0].add(entry, count);
      }
    }
  }
  cluster_count--;

  if (cluster_count == 0) {
    clusters[0].set_range(rangemin_, rangemax_);
  }
  do {
    new_cluster = false;
    new_mode = 0;
    for (entry = 0; entry <= rangemax_ - rangemin_; entry++) {
      count = buckets_[entry] - clusters[0].buckets_[entry];
      // remaining pile
      if (count > 0) { // any to handle
        min_dist = static_cast<float>(INT32_MAX);
        best_cluster = 0;
        for (cluster = 1; cluster <= cluster_count; cluster++) {
          dist = entry + rangemin_ - centres[cluster];
          // find distance
          if (dist < 0) {
            dist = -dist;
          }
          if (dist < min_dist) {
            min_dist = dist; // find least
            best_cluster = cluster;
          }
        }
        if (min_dist > upper // far enough for new
            && (best_cluster == 0 || entry + rangemin_ > centres[best_cluster] * multiple ||
                entry + rangemin_ < centres[best_cluster] / multiple)) {
          if (count > new_mode) {
            new_mode = count;
            new_centre = entry + rangemin_;
          }
        }
      }
    }
    // need new and room
    if (new_mode > 0 && cluster_count < max_clusters) {
      cluster_count++;
      new_cluster = true;
      if (!clusters[cluster_count].set_range(rangemin_, rangemax_)) {
        delete[] centres;
        return 0;
      }
      centres[cluster_count] = static_cast<float>(new_centre);
      clusters[cluster_count].add(new_centre, new_mode);
      clusters[0].add(new_centre, new_mode);
      for (entry = new_centre - 1; centres[cluster_count] - entry < lower && entry >= rangemin_ &&
                                   pile_count(entry) <= pile_count(entry + 1);
           entry--) {
        count = pile_count(entry) - clusters[0].pile_count(entry);
        if (count > 0) {
          clusters[cluster_count].add(entry, count);
          clusters[0].add(entry, count);
        }
      }
      for (entry = new_centre + 1; entry - centres[cluster_count] < lower && entry <= rangemax_ &&
                                   pile_count(entry) <= pile_count(entry - 1);
           entry++) {
        count = pile_count(entry) - clusters[0].pile_count(entry);
        if (count > 0) {
          clusters[cluster_count].add(entry, count);
          clusters[0].add(entry, count);
        }
      }
      centres[cluster_count] = static_cast<float>(clusters[cluster_count].ile(0.5));
    }
  } while (new_cluster && cluster_count < max_clusters);
  delete[] centres;
  return cluster_count;
}

// Helper tests that the current index is still part of the peak and gathers
// the data into the peak, returning false when the peak is ended.
// src_buckets[index] - used_buckets[index] is the unused part of the histogram.
// prev_count is the histogram count of the previous index on entry and is
// updated to the current index on return.
// total_count and total_value are accumulating the mean of the peak.
static bool GatherPeak(int index, const int *src_buckets, int *used_buckets, int *prev_count,
                       int *total_count, double *total_value) {
  int pile_count = src_buckets[index] - used_buckets[index];
  if (pile_count <= *prev_count && pile_count > 0) {
    // Accumulate count and index.count product.
    *total_count += pile_count;
    *total_value += index * pile_count;
    // Mark this index as used
    used_buckets[index] = src_buckets[index];
    *prev_count = pile_count;
    return true;
  } else {
    return false;
  }
}

// Finds (at most) the top max_modes modes, well actually the whole peak around
// each mode, returning them in the given modes vector as a <mean of peak,
// total count of peak> pair in order of decreasing total count.
// Since the mean is the key and the count the data in the pair, a single call
// to sort on the output will re-sort by increasing mean of peak if that is
// more useful than decreasing total count.
// Returns the actual number of modes found.
int STATS::top_n_modes(int max_modes, std::vector<KDPairInc<float, int>> &modes) const {
  if (max_modes <= 0) {
    return 0;
  }
  int src_count = 1 + rangemax_ - rangemin_;
  // Used copies the counts in buckets_ as they get used.
  STATS used(rangemin_, rangemax_);
  modes.clear();
  // Total count of the smallest peak found so far.
  int least_count = 1;
  // Mode that is used as a seed for each peak
  int max_count = 0;
  do {
    // Find an unused mode.
    max_count = 0;
    int max_index = 0;
    for (int src_index = 0; src_index < src_count; src_index++) {
      int pile_count = buckets_[src_index] - used.buckets_[src_index];
      if (pile_count > max_count) {
        max_count = pile_count;
        max_index = src_index;
      }
    }
    if (max_count > 0) {
      // Copy the bucket count to used so it doesn't get found again.
      used.buckets_[max_index] = max_count;
      // Get the entire peak.
      double total_value = max_index * max_count;
      int total_count = max_count;
      int prev_pile = max_count;
      for (int offset = 1; max_index + offset < src_count; ++offset) {
        if (!GatherPeak(max_index + offset, buckets_, used.buckets_, &prev_pile, &total_count,
                        &total_value)) {
          break;
        }
      }
      prev_pile = buckets_[max_index];
      for (int offset = 1; max_index - offset >= 0; ++offset) {
        if (!GatherPeak(max_index - offset, buckets_, used.buckets_, &prev_pile, &total_count,
                        &total_value)) {
          break;
        }
      }
      if (total_count > least_count || modes.size() < static_cast<size_t>(max_modes)) {
        // We definitely want this mode, so if we have enough discard the least.
        if (modes.size() == static_cast<size_t>(max_modes)) {
          modes.resize(max_modes - 1);
        }
        size_t target_index = 0;
        // Linear search for the target insertion point.
        while (target_index < modes.size() && modes[target_index].data() >= total_count) {
          ++target_index;
        }
        auto peak_mean = static_cast<float>(total_value / total_count + rangemin_);
        modes.insert(modes.begin() + target_index, KDPairInc<float, int>(peak_mean, total_count));
        least_count = modes.back().data();
      }
    }
  } while (max_count > 0);
  return modes.size();
}

/**********************************************************************
 * STATS::print
 *
 * Prints a summary and table of the histogram.
 **********************************************************************/
void STATS::print() const {
  if (buckets_ == nullptr) {
    return;
  }
  int32_t min = min_bucket() - rangemin_;
  int32_t max = max_bucket() - rangemin_;

  int num_printed = 0;
  for (int index = min; index <= max; index++) {
    if (buckets_[index] != 0) {
      tprintf("%4d:%-3d ", rangemin_ + index, buckets_[index]);
      if (++num_printed % 8 == 0) {
        tprintf("\n");
      }
    }
  }
  tprintf("\n");
  print_summary();
}

/**********************************************************************
 * STATS::print_summary
 *
 * Print a summary of the stats.
 **********************************************************************/
void STATS::print_summary() const {
  if (buckets_ == nullptr) {
    return;
  }
  int32_t min = min_bucket();
  int32_t max = max_bucket();
  tprintf("Total count=%d\n", total_count_);
  tprintf("Min=%.2f Really=%d\n", ile(0.0), min);
  tprintf("Lower quartile=%.2f\n", ile(0.25));
  tprintf("Median=%.2f, ile(0.5)=%.2f\n", median(), ile(0.5));
  tprintf("Upper quartile=%.2f\n", ile(0.75));
  tprintf("Max=%.2f Really=%d\n", ile(1.0), max);
  tprintf("Range=%d\n", max + 1 - min);
  tprintf("Mean= %.2f\n", mean());
  tprintf("SD= %.2f\n", sd());
}

/**********************************************************************
 * STATS::plot
 *
 * Draw a histogram of the stats table.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void STATS::plot(ScrollView *window, // to draw in
                 float xorigin,      // bottom left
                 float yorigin,
                 float xscale,                     // one x unit
                 float yscale,                     // one y unit
                 ScrollView::Color colour) const { // colour to draw in
  if (buckets_ == nullptr) {
    return;
  }
  window->Pen(colour);

  for (int index = 0; index <= rangemax_ - rangemin_; index++) {
    window->Rectangle(xorigin + xscale * index, yorigin, xorigin + xscale * (index + 1),
                      yorigin + yscale * buckets_[index]);
  }
}
#endif

/**********************************************************************
 * STATS::plotline
 *
 * Draw a histogram of the stats table. (Line only)
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void STATS::plotline(ScrollView *window, // to draw in
                     float xorigin,      // bottom left
                     float yorigin,
                     float xscale,                     // one x unit
                     float yscale,                     // one y unit
                     ScrollView::Color colour) const { // colour to draw in
  if (buckets_ == nullptr) {
    return;
  }
  window->Pen(colour);
  window->SetCursor(xorigin, yorigin + yscale * buckets_[0]);
  for (int index = 0; index <= rangemax_ - rangemin_; index++) {
    window->DrawTo(xorigin + xscale * index, yorigin + yscale * buckets_[index]);
  }
}
#endif

} // namespace tesseract
