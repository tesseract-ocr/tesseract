/**********************************************************************
 * File:        statistc.c  (Formerly stats.c)
 * Description: Simple statistical package for integer values.
 * Author:					Ray Smith
 * Created:					Mon Feb 04 16:56:05 GMT 1991
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

#include          "mfcpch.h"     //precompiled headers
#include          <string.h>
#include          <math.h>
#include          <stdlib.h>
#include          "memry.h"
//#include                                      "ipeerr.h"
#include          "tprintf.h"
#include          "statistc.h"

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#define SEED1       0x1234       //default seeds
#define SEED2       0x5678
#define SEED3       0x9abc

/**********************************************************************
 * STATS::STATS
 *
 * Construct a new stats element by allocating and zeroing the memory.
 **********************************************************************/

STATS::STATS(            //constructor
             inT32 min,  //min of range
             inT32 max   //max of range
            ) {

  if (max <= min) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Illegal range for stats, Min=%d, Max=%d",min,max);*/
    min = 0;
    max = 1;
  }
  rangemin = min;                //setup
  rangemax = max;
  buckets = (inT32 *) alloc_mem ((max - min) * sizeof (inT32));
  if (buckets != NULL)
    this->clear ();              //zero it
  /*   else
     err.log(RESULT_NO_MEMORY,E_LOC,ERR_PRIMITIVES,
     ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
     "No memory for stats, Min=%d, Max=%d",min,max); */
}


STATS::STATS() {  //constructor
  rangemax = 0;                  //empty
  rangemin = 0;
  buckets = NULL;
}


/**********************************************************************
 * STATS::set_range
 *
 * Alter the range on an existing stats element.
 **********************************************************************/

bool STATS::set_range(            //constructor
                      inT32 min,  //min of range
                      inT32 max   //max of range
                     ) {

  if (max <= min) {
    return false;
  }
  rangemin = min;                //setup
  rangemax = max;
  if (buckets != NULL)
    free_mem(buckets);  //no longer want it
  buckets = (inT32 *) alloc_mem ((max - min) * sizeof (inT32));
  /*	if (buckets==NULL)
      return err.log(RESULT_NO_MEMORY,E_LOC,ERR_PRIMITIVES,
          ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
          "No memory for stats, Min=%d, Max=%d",min,max);*/

  this->clear ();                //zero it
  return true;
}


/**********************************************************************
 * STATS::clear
 *
 * Clear out the STATS class by zeroing all the buckets.
 **********************************************************************/

void STATS::clear() {  //clear out buckets
  total_count = 0;
  if (buckets != NULL)
    memset (buckets, 0, (rangemax - rangemin) * sizeof (inT32));
  //zero it
}


/**********************************************************************
 * STATS::~STATS
 *
 * Destructor for a stats class.
 **********************************************************************/

STATS::~STATS (                  //destructor
) {
  if (buckets != NULL) {
    free_mem(buckets);
    buckets = NULL;
  }
}


/**********************************************************************
 * STATS::add
 *
 * Add a set of samples to (or delete from) a pile.
 **********************************************************************/

void STATS::add(              //add sample
                inT32 value,  //bucket
                inT32 count   //no to add
               ) {
  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return;
  }
  if (value <= rangemin)
    buckets[0] += count;         //silently clip to range
  else if (value >= rangemax)
    buckets[rangemax - rangemin - 1] += count;
  else
                                 //add count to cell
    buckets[value - rangemin] += count;
  total_count += count;          //keep count of total
}


/**********************************************************************
 * STATS::mode
 *
 * Find the mode of a stats class.
 **********************************************************************/

inT32 STATS::mode() {  //get mode of samples
  inT32 index;                   //current index
  inT32 max;                     //max cell count
  inT32 maxindex;                //index of max

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return rangemin;
  }
  for (max = 0, maxindex = 0, index = rangemax - rangemin - 1; index >= 0;
  index--) {
    if (buckets[index] > max) {
      max = buckets[index];      //find biggest
      maxindex = index;
    }
  }
  return maxindex + rangemin;    //index of biggest
}


/**********************************************************************
 * STATS::mean
 *
 * Find the mean of a stats class.
 **********************************************************************/

float STATS::mean() {  //get mean of samples
  inT32 index;                   //current index
  inT32 sum;                     //sum of cells

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return (float) rangemin;
  }
  for (sum = 0, index = rangemax - rangemin - 1; index >= 0; index--) {
                                 //sum all buckets
    sum += index * buckets[index];
  }
  if (total_count > 0)
                                 //mean value
    return (float) sum / total_count + rangemin;
  else
    return (float) rangemin;     //no mean
}


/**********************************************************************
 * STATS::sd
 *
 * Find the standard deviation of a stats class.
 **********************************************************************/

float STATS::sd() {  //standard deviation
  inT32 index;                   //current index
  inT32 sum;                     //sum of cells
  inT32 sqsum;                   //sum of squares
  float variance;

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return (float) 0.0;
  }
  for (sum = 0, sqsum = 0, index = rangemax - rangemin - 1; index >= 0;
  index--) {
                                 //sum all buckets
    sum += index * buckets[index];
                                 //and squares
    sqsum += index * index * buckets[index];
  }
  if (total_count > 0) {
    variance = sum / ((float) total_count);
    variance = sqsum / ((float) total_count) - variance * variance;
    return (float) sqrt (variance);
  }
  else
    return (float) 0.0;
}


/**********************************************************************
 * STATS::ile
 *
 * Find an arbitrary %ile of a stats class.
 **********************************************************************/

float STATS::ile(            //percentile
                 float frac  //fraction to find
                ) {
  inT32 index;                   //current index
  inT32 sum;                     //sum of cells
  float target;                  //target value

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return (float) rangemin;
  }
  target = frac * total_count;
  if (target <= 0)
    target = (float) 1;
  if (target > total_count)
    target = (float) total_count;
  for (sum = 0, index = 0; index < rangemax - rangemin
    && sum < target; sum += buckets[index], index++);
  if (index > 0)
    return rangemin + index - (sum - target) / buckets[index - 1];
  //better than just ints
  else
    return (float) rangemin;
}


/**********************************************************************
 * STATS::median
 *
 * Finds a more usefule estimate of median than ile(0.5).
 *
 * Overcomes a problem with ile() - if the samples are, for example,
 * 6,6,13,14 ile(0.5) return 7.0 - when a more useful value would be midway
 * between 6 and 13 = 9.5
 **********************************************************************/

float STATS::median() {  //get median
  float median;
  inT32 min_pile;
  inT32 median_pile;
  inT32 max_pile;

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return (float) rangemin;
  }
  median = (float) ile ((float) 0.5);
  median_pile = (inT32) floor (median);
  if ((total_count > 1) && (pile_count (median_pile) == 0)) {
    /* Find preceeding non zero pile */
    for (min_pile = median_pile; pile_count (min_pile) == 0; min_pile--);
    /* Find following non zero pile */
    for (max_pile = median_pile; pile_count (max_pile) == 0; max_pile++);
    median = (float) ((min_pile + max_pile) / 2.0);
  }
  return median;
}


/**********************************************************************
 * STATS::smooth
 *
 * Apply a triangular smoothing filter to the stats.
 * This makes the modes a bit more useful.
 * The factor gives the height of the triangle, i.e. the weight of the
 * centre.
 **********************************************************************/

void STATS::smooth(              //smooth samples
                   inT32 factor  //size of triangle
                  ) {
  inT32 entry;                   //bucket index
  inT32 offset;                  //from entry
  inT32 entrycount;              //no of entries
  inT32 bucket;                  //new smoothed pile
                                 //output stats
  STATS result(rangemin, rangemax);

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return;
  }
  if (factor < 2)
    return;                      //is a no-op
  entrycount = rangemax - rangemin;
  for (entry = 0; entry < entrycount; entry++) {
                                 //centre weight
    bucket = buckets[entry] * factor;
    for (offset = 1; offset < factor; offset++) {
      if (entry - offset >= 0)
        bucket += buckets[entry - offset] * (factor - offset);
      if (entry + offset < entrycount)
        bucket += buckets[entry + offset] * (factor - offset);
    }
    result.add (entry + rangemin, bucket);
  }
  total_count = result.total_count;
  memcpy (buckets, result.buckets, entrycount * sizeof (inT32));
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

inT32 STATS::cluster(                     //cluster samples
                     float lower,         //thresholds
                     float upper,
                     float multiple,      //distance threshold
                     inT32 max_clusters,  //max no to make
                     STATS *clusters      //array of clusters
                    ) {
  BOOL8 new_cluster;             //added one
  float *centres;                //cluster centres
  inT32 entry;                   //bucket index
  inT32 cluster;                 //cluster index
  inT32 best_cluster;            //one to assign to
  inT32 new_centre = 0;          //residual mode
  inT32 new_mode;                //pile count of new_centre
  inT32 count;                   //pile to place
  float dist;                    //from cluster
  float min_dist;                //from best_cluster
  inT32 cluster_count;           //no of clusters

  if (max_clusters < 1)
    return 0;
  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return 0;
  }
  centres = (float *) alloc_mem ((max_clusters + 1) * sizeof (float));
  if (centres == NULL) {
    /*     err.log(RESULT_NO_MEMORY,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "No memory for centres"); */
    return 0;
  }
  for (cluster_count = 1; cluster_count <= max_clusters
    && clusters[cluster_count].buckets != NULL
  && clusters[cluster_count].total_count > 0; cluster_count++) {
    centres[cluster_count] =
      (float) clusters[cluster_count].ile ((float) 0.5);
    new_centre = clusters[cluster_count].mode ();
    for (entry = new_centre - 1; centres[cluster_count] - entry < lower
      && entry >= rangemin
    && pile_count (entry) <= pile_count (entry + 1); entry--) {
      count = pile_count (entry) - clusters[0].pile_count (entry);
      if (count > 0) {
        clusters[cluster_count].add (entry, count);
        clusters[0].add (entry, count);
      }
    }
    for (entry = new_centre + 1; entry - centres[cluster_count] < lower
      && entry < rangemax
    && pile_count (entry) <= pile_count (entry - 1); entry++) {
      count = pile_count (entry) - clusters[0].pile_count (entry);
      if (count > 0) {
        clusters[cluster_count].add (entry, count);
        clusters[0].add (entry, count);
      }
    }
  }
  cluster_count--;

  if (cluster_count == 0) {
    clusters[0].set_range (rangemin, rangemax);
  }
  do {
    new_cluster = FALSE;
    new_mode = 0;
    for (entry = 0; entry < rangemax - rangemin; entry++) {
      count = buckets[entry] - clusters[0].buckets[entry];
      //remaining pile
      if (count > 0) {           //any to handle
        min_dist = (float) MAX_INT32;
        best_cluster = 0;
        for (cluster = 1; cluster <= cluster_count; cluster++) {
          dist = entry + rangemin - centres[cluster];
          //find distance
          if (dist < 0)
            dist = -dist;
          if (dist < min_dist) {
            min_dist = dist;     //find least
            best_cluster = cluster;
          }
        }
        if (min_dist > upper     //far enough for new
          && (best_cluster == 0
          || entry + rangemin > centres[best_cluster] * multiple
        || entry + rangemin < centres[best_cluster] / multiple)) {
          if (count > new_mode) {
            new_mode = count;
            new_centre = entry + rangemin;
          }
        }
      }
    }
                                 //need new and room
    if (new_mode > 0 && cluster_count < max_clusters) {
      cluster_count++;
      new_cluster = TRUE;
      if (!clusters[cluster_count].set_range (rangemin, rangemax))
        return 0;
      centres[cluster_count] = (float) new_centre;
      clusters[cluster_count].add (new_centre, new_mode);
      clusters[0].add (new_centre, new_mode);
      for (entry = new_centre - 1; centres[cluster_count] - entry < lower
        && entry >= rangemin
      && pile_count (entry) <= pile_count (entry + 1); entry--) {
        count = pile_count (entry) - clusters[0].pile_count (entry);
        if (count > 0) {
          clusters[cluster_count].add (entry, count);
          clusters[0].add (entry, count);
        }
      }
      for (entry = new_centre + 1; entry - centres[cluster_count] < lower
        && entry < rangemax
      && pile_count (entry) <= pile_count (entry - 1); entry++) {
        count = pile_count (entry) - clusters[0].pile_count (entry);
        if (count > 0) {
          clusters[cluster_count].add (entry, count);
          clusters[0].add (entry, count);
        }
      }
      centres[cluster_count] =
        (float) clusters[cluster_count].ile ((float) 0.5);
    }
  }
  while (new_cluster && cluster_count < max_clusters);
  free_mem(centres);
  return cluster_count;
}


/**********************************************************************
 * STATS::local_min
 *
 * Return TRUE if this point is a local min.
 **********************************************************************/

BOOL8 STATS::local_min(         //test minness
                       inT32 x  //of x
                      ) {
  inT32 index;                   //table index

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return FALSE;
  }
  if (x < rangemin)
    x = rangemin;
  if (x >= rangemax)
    x = rangemax - 1;
  x -= rangemin;
  if (buckets[x] == 0)
    return TRUE;
  for (index = x - 1; index >= 0 && buckets[index] == buckets[x]; index--);
  if (index >= 0 && buckets[index] < buckets[x])
    return FALSE;
  for (index = x + 1; index < rangemax - rangemin
    && buckets[index] == buckets[x]; index++);
  if (index < rangemax - rangemin && buckets[index] < buckets[x])
    return FALSE;
  else
    return TRUE;
}


/**********************************************************************
 * STATS::print
 *
 * Print a summary of the stats and optionally a dump of the table.
 **********************************************************************/

void STATS::print(            //print stats table
                  FILE *,     //Now uses tprintf instead
                  BOOL8 dump  //dump full table
                 ) {
  inT32 index;                   //table index

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return;
  }
  if (dump) {
    for (index = 0; index < rangemax - rangemin; index++) {
      tprintf ("%4d:%-3d ", rangemin + index, buckets[index]);
      if (index % 8 == 7)
        tprintf ("\n");
    }
    tprintf ("\n");
  }

  tprintf ("Total count=%d\n", total_count);
  tprintf ("Min=%d\n", (inT32) (ile ((float) 0.0)));
  tprintf ("Lower quartile=%.2f\n", ile ((float) 0.25));
  tprintf ("Median=%.2f\n", ile ((float) 0.5));
  tprintf ("Upper quartile=%.2f\n", ile ((float) 0.75));
  tprintf ("Max=%d\n", (inT32) (ile ((float) 0.99999)));
  tprintf ("Mean= %.2f\n", mean ());
  tprintf ("SD= %.2f\n", sd ());
}


/**********************************************************************
 * STATS::min_bucket
 *
 * Find REAL minimum bucket - ile(0.0) isnt necessarily correct
 **********************************************************************/

inT32 STATS::min_bucket() {  //Find min
  inT32 min;

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return rangemin;
  }

  for (min = 0; (min < rangemax - rangemin) && (buckets[min] == 0); min++);
  return rangemin + min;
}


/**********************************************************************
 * STATS::max_bucket
 *
 * Find REAL maximum bucket - ile(1.0) isnt necessarily correct
 **********************************************************************/

inT32 STATS::max_bucket() {  //Find max
  inT32 max;

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return rangemin;
  }

  for (max = rangemax - rangemin - 1;
    (max > 0) && (buckets[max] == 0); max--);
  return rangemin + max;
}


/**********************************************************************
 * STATS::short_print
 *
 * Print a summary of the stats and optionally a dump of the table.
 * ( BUT ONLY THE PART OF THE TABLE BETWEEN MIN AND MAX)
 **********************************************************************/

void STATS::short_print(            //print stats table
                        FILE *,     //Now uses tprintf instead
                        BOOL8 dump  //dump full table
                       ) {
  inT32 index;                   //table index
  inT32 min = min_bucket ();
  inT32 max = max_bucket ();

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return;
  }
  if (dump) {
    for (index = min; index <= max; index++) {
      tprintf ("%4d:%-3d ", rangemin + index, buckets[index]);
      if ((index - min) % 8 == 7)
        tprintf ("\n");
    }
    tprintf ("\n");
  }

  tprintf ("Total count=%d\n", total_count);
  tprintf ("Min=%d Really=%d\n", (inT32) (ile ((float) 0.0)), min);
  tprintf ("Max=%d Really=%d\n", (inT32) (ile ((float) 1.1)), max);
  tprintf ("Range=%d\n", max + 1 - min);
  tprintf ("Lower quartile=%.2f\n", ile ((float) 0.25));
  tprintf ("Median=%.2f\n", ile ((float) 0.5));
  tprintf ("Upper quartile=%.2f\n", ile ((float) 0.75));
  tprintf ("Mean= %.2f\n", mean ());
  tprintf ("SD= %.2f\n", sd ());
}


/**********************************************************************
 * STATS::plot
 *
 * Draw a histogram of the stats table.
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void STATS::plot(                //plot stats table
                 ScrollView* window,  //to draw in
                 float xorigin,  //bottom left
                 float yorigin,
                 float xscale,   //one x unit
                 float yscale,   //one y unit
                 ScrollView::Color colour   //colour to draw in
                ) {
  inT32 index;                   //table index

  if (buckets == NULL) {
    /*		err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
            ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
            "Empty stats");*/
    return;
  }
  window->Pen(colour);

  for (index = 0; index < rangemax - rangemin; index++) {
    window->Rectangle( xorigin + xscale * index, yorigin,
      xorigin + xscale * (index + 1),
      yorigin + yscale * buckets[index]);
  }
}
#endif


/**********************************************************************
 * STATS::plotline
 *
 * Draw a histogram of the stats table. (Line only
 **********************************************************************/

#ifndef GRAPHICS_DISABLED
void STATS::plotline(                //plot stats table
                     ScrollView* window,  //to draw in
                     float xorigin,  //bottom left
                     float yorigin,
                     float xscale,   //one x unit
                     float yscale,   //one y unit
                     ScrollView::Color colour   //colour to draw in
                    ) {
  inT32 index;                   //table index

  if (buckets == NULL) {
    /*     err.log(RESULT_LOGICAL_ERROR,E_LOC,ERR_PRIMITIVES,
       ERR_SCROLLING,ERR_CONTINUE,ERR_ERROR,
       "Empty stats"); */
    return;
  }
  window->Pen(colour);

  window->SetCursor(xorigin, yorigin + yscale * buckets[0]);
  for (index = 0; index < rangemax - rangemin; index++) {
    window->DrawTo(xorigin + xscale * index,      yorigin + yscale * buckets[index]);
  }
}
#endif


/**********************************************************************
 * choose_nth_item
 *
 * Returns the index of what would b the nth item in the array
 * if the members were sorted, without actually sorting.
 **********************************************************************/

DLLSYM inT32 choose_nth_item(               //fast median
                             inT32 index,   //index to choose
                             float *array,  //array of items
                             inT32 count    //no of items
                            ) {
  static uinT16 seeds[3] = { SEED1, SEED2, SEED3 };
  //for nrand
  inT32 next_sample;             //next one to do
  inT32 next_lesser;             //space for new
  inT32 prev_greater;            //last one saved
  inT32 equal_count;             //no of equal ones
  float pivot;                   //proposed median
  float sample;                  //current sample

  if (count <= 1)
    return 0;
  if (count == 2) {
    if (array[0] < array[1]) {
      return index >= 1 ? 1 : 0;
    }
    else {
      return index >= 1 ? 0 : 1;
    }
  }
  else {
    if (index < 0)
      index = 0;                 //ensure lergal
    else if (index >= count)
      index = count - 1;
    #ifdef __UNIX__
    equal_count = (inT32) (nrand48 (seeds) % count);
    #else
    equal_count = (inT32) (rand () % count);
    #endif
    pivot = array[equal_count];
                                 //fill gap
    array[equal_count] = array[0];
    next_lesser = 0;
    prev_greater = count;
    equal_count = 1;
    for (next_sample = 1; next_sample < prev_greater;) {
      sample = array[next_sample];
      if (sample < pivot) {
                                 //shuffle
        array[next_lesser++] = sample;
        next_sample++;
      }
      else if (sample > pivot) {
        prev_greater--;
                                 //juggle
        array[next_sample] = array[prev_greater];
        array[prev_greater] = sample;
      }
      else {
        equal_count++;
        next_sample++;
      }
    }
    for (next_sample = next_lesser; next_sample < prev_greater;)
      array[next_sample++] = pivot;
    if (index < next_lesser)
      return choose_nth_item (index, array, next_lesser);
    else if (index < prev_greater)
      return next_lesser;        //in equal bracket
    else
      return choose_nth_item (index - prev_greater,
        array + prev_greater,
        count - prev_greater) + prev_greater;
  }
}


/**********************************************************************
 * choose_nth_item
 *
 * Returns the index of what would b the nth item in the array
 * if the members were sorted, without actually sorting.
 **********************************************************************/

DLLSYM inT32
choose_nth_item (                //fast median
inT32 index,                     //index to choose
void *array,                     //array of items
inT32 count,                     //no of items
size_t size,                     //element size
                                 //comparator
int (*compar) (const void *, const void *)
) {
  static uinT16 seeds[3] = { SEED1, SEED2, SEED3 };
  //for nrand
  int result;                    //of compar
  inT32 next_sample;             //next one to do
  inT32 next_lesser;             //space for new
  inT32 prev_greater;            //last one saved
  inT32 equal_count;             //no of equal ones
  inT32 pivot;                   //proposed median

  if (count <= 1)
    return 0;
  if (count == 2) {
    if (compar (array, (char *) array + size) < 0) {
      return index >= 1 ? 1 : 0;
    }
    else {
      return index >= 1 ? 0 : 1;
    }
  }
  if (index < 0)
    index = 0;                   //ensure lergal
  else if (index >= count)
    index = count - 1;
  #ifdef __UNIX__
  pivot = (inT32) (nrand48 (seeds) % count);
  #else
  pivot = (inT32) (rand () % count);
  #endif
  swap_entries (array, size, pivot, 0);
  next_lesser = 0;
  prev_greater = count;
  equal_count = 1;
  for (next_sample = 1; next_sample < prev_greater;) {
    result =
      compar ((char *) array + size * next_sample,
      (char *) array + size * next_lesser);
    if (result < 0) {
      swap_entries (array, size, next_lesser++, next_sample++);
      //shuffle
    }
    else if (result > 0) {
      prev_greater--;
      swap_entries(array, size, prev_greater, next_sample);
    }
    else {
      equal_count++;
      next_sample++;
    }
  }
  if (index < next_lesser)
    return choose_nth_item (index, array, next_lesser, size, compar);
  else if (index < prev_greater)
    return next_lesser;          //in equal bracket
  else
    return choose_nth_item (index - prev_greater,
      (char *) array + size * prev_greater,
      count - prev_greater, size,
      compar) + prev_greater;
}


/**********************************************************************
 * swap_entries
 *
 * Swap 2 entries of abitrary size in-place in a table.
 **********************************************************************/

void swap_entries(               //swap in place
                  void *array,   //array of entries
                  size_t size,   //size of entry
                  inT32 index1,  //entries to swap
                  inT32 index2) {
  char tmp;
  char *ptr1;                    //to entries
  char *ptr2;
  size_t count;                  //of bytes

  ptr1 = (char *) array + index1 * size;
  ptr2 = (char *) array + index2 * size;
  for (count = 0; count < size; count++) {
    tmp = *ptr1;
    *ptr1++ = *ptr2;
    *ptr2++ = tmp;               //tedious!
  }
}
