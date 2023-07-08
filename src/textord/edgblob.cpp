/**********************************************************************
 * File:        edgblob.cpp (Formerly edgeloop.c)
 * Description: Functions to clean up an outline before approximation.
 * Author:      Ray Smith
 *
 *(C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0(the "License");
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

#include "edgblob.h"

#include "edgloop.h"
#include "scanedg.h"

#define BUCKETSIZE 16

namespace tesseract {

// Control parameters used in outline_complexity(), which rejects an outline
// if any one of the 3 conditions is satisfied:
//  - number of children exceeds edges_max_children_per_outline
//  - number of nested layers exceeds edges_max_children_layers
//  - joint complexity exceeds edges_children_count_limit(as in child_count())
static BOOL_VAR(edges_use_new_outline_complexity, false,
                "Use the new outline complexity module");
static INT_VAR(edges_max_children_per_outline, 10,
               "Max number of children inside a character outline");
static INT_VAR(edges_max_children_layers, 5,
               "Max layers of nested children inside a character outline");
static BOOL_VAR(edges_debug, false, "turn on debugging for this module");

static INT_VAR(edges_children_per_grandchild, 10,
               "Importance ratio for chucking outlines");
static INT_VAR(edges_children_count_limit, 45, "Max holes allowed in blob");
static BOOL_VAR(edges_children_fix, false,
                "Remove boxy parents of char-like children");
static INT_VAR(edges_min_nonhole, 12, "Min pixels for potential char in box");
static INT_VAR(edges_patharea_ratio, 40,
               "Max lensq/area for acceptable child outline");
static double_VAR(edges_childarea, 0.5, "Min area fraction of child outline");
static double_VAR(edges_boxarea, 0.875,
                  "Min area fraction of grandchild for box");

/**
 * @name OL_BUCKETS::OL_BUCKETS
 *
 * Construct an array of buckets for associating outlines into blobs.
 */

OL_BUCKETS::OL_BUCKETS(ICOORD bleft, // corners
                       ICOORD tright)
    : bxdim((tright.x() - bleft.x()) / BUCKETSIZE + 1),
      bydim((tright.y() - bleft.y()) / BUCKETSIZE + 1),
      buckets(bxdim * bydim),
      bl(bleft),
      tr(tright) {}

/**
 * @name OL_BUCKETS::operator(
 *
 * Return a pointer to a list of C_OUTLINEs corresponding to the
 * given pixel coordinates.
 */

C_OUTLINE_LIST *OL_BUCKETS::operator()( // array access
    TDimension x,                       // image coords
    TDimension y) {
  return &buckets[(y - bl.y()) / BUCKETSIZE * bxdim +
                  (x - bl.x()) / BUCKETSIZE];
}

C_OUTLINE_LIST *OL_BUCKETS::start_scan() {
  return scan_next(buckets.begin());
}

C_OUTLINE_LIST *OL_BUCKETS::scan_next() {
  return scan_next(it);
}

C_OUTLINE_LIST *OL_BUCKETS::scan_next(decltype(buckets)::iterator in_it) {
  it = std::find_if(in_it, buckets.end(), [](auto &&b) { return !b.empty(); });
  if (it == buckets.end())
    return nullptr;
  return &*it;
}

/**
 * @name OL_BUCKETS::outline_complexity
 *
 * This is the new version of count_child.
 *
 * The goal of this function is to determine if an outline and its
 * interiors could be part of a character blob.  This is done by
 * computing a "complexity" index for the outline, which is the return
 * value of this function, and checking it against a threshold.
 * The max_count is used for short-circuiting the recursion and forcing
 * a rejection that guarantees to fail the threshold test.
 * The complexity F for outline X with N children X[i] is
 *   F(X) = N + sum_i F(X[i]) * edges_children_per_grandchild
 * so each layer of nesting increases complexity exponentially.
 * An outline can be rejected as a text blob candidate if its complexity
 * is too high, has too many children(likely a container), or has too
 * many layers of nested inner loops.  This has the side-effect of
 * flattening out boxed or reversed video text regions.
 */

int32_t OL_BUCKETS::outline_complexity(C_OUTLINE *outline, // parent outline
                                       int32_t max_count,  // max output
                                       int16_t depth       // recurion depth
) {
  TDimension xmin, xmax;    // coord limits
  TDimension ymin, ymax;
  C_OUTLINE *child;         // current child
  int32_t child_count;      // no of children
  int32_t grandchild_count; // no of grandchildren
  C_OUTLINE_IT child_it;    // search iterator

  TBOX olbox = outline->bounding_box();
  xmin = (olbox.left() - bl.x()) / BUCKETSIZE;
  xmax = (olbox.right() - bl.x()) / BUCKETSIZE;
  ymin = (olbox.bottom() - bl.y()) / BUCKETSIZE;
  ymax = (olbox.top() - bl.y()) / BUCKETSIZE;
  child_count = 0;
  grandchild_count = 0;
  if (++depth > edges_max_children_layers) { // nested loops are too deep
    return max_count + depth;
  }

  for (auto yindex = ymin; yindex <= ymax; yindex++) {
    for (auto xindex = xmin; xindex <= xmax; xindex++) {
      child_it.set_to_list(&buckets[yindex * bxdim + xindex]);
      if (child_it.empty()) {
        continue;
      }
      for (child_it.mark_cycle_pt(); !child_it.cycled_list();
           child_it.forward()) {
        child = child_it.data();
        if (child == outline || !(*child < *outline)) {
          continue;
        }
        child_count++;

        if (child_count > edges_max_children_per_outline) { // too fragmented
          if (edges_debug) {
            tprintf(
                "Discard outline on child_count=%d > "
                "max_children_per_outline=%d\n",
                child_count,
                static_cast<int32_t>(edges_max_children_per_outline));
          }
          return max_count + child_count;
        }

        // Compute the "complexity" of each child recursively
        int32_t remaining_count = max_count - child_count - grandchild_count;
        if (remaining_count > 0) {
          grandchild_count += edges_children_per_grandchild *
                              outline_complexity(child, remaining_count, depth);
        }
        if (child_count + grandchild_count > max_count) { // too complex
          if (edges_debug) {
            tprintf(
                "Discard outline on child_count=%d + grandchild_count=%d "
                "> max_count=%d\n",
                child_count, grandchild_count, max_count);
          }
          return child_count + grandchild_count;
        }
      }
    }
  }
  return child_count + grandchild_count;
}

/**
 * @name OL_BUCKETS::count_children
 *
 * Find number of descendants of this outline.
 */
// TODO(rays) Merge with outline_complexity.
int32_t OL_BUCKETS::count_children( // recursive count
    C_OUTLINE *outline,             // parent outline
    int32_t max_count               // max output
) {
  bool parent_box;    // could it be boxy
  TDimension xmin, xmax;    // coord limits
  TDimension ymin, ymax;
  C_OUTLINE *child;         // current child
  int32_t child_count;      // no of children
  int32_t grandchild_count; // no of grandchildren
  int32_t parent_area;      // potential box
  float max_parent_area;    // potential box
  int32_t child_area;       // current child
  int32_t child_length;     // current child
  TBOX olbox;
  C_OUTLINE_IT child_it; // search iterator

  olbox = outline->bounding_box();
  xmin = (olbox.left() - bl.x()) / BUCKETSIZE;
  xmax = (olbox.right() - bl.x()) / BUCKETSIZE;
  ymin = (olbox.bottom() - bl.y()) / BUCKETSIZE;
  ymax = (olbox.top() - bl.y()) / BUCKETSIZE;
  child_count = 0;
  grandchild_count = 0;
  parent_area = 0;
  max_parent_area = 0;
  parent_box = true;
  for (auto yindex = ymin; yindex <= ymax; yindex++) {
    for (auto xindex = xmin; xindex <= xmax; xindex++) {
      child_it.set_to_list(&buckets[yindex * bxdim + xindex]);
      if (child_it.empty()) {
        continue;
      }
      for (child_it.mark_cycle_pt(); !child_it.cycled_list();
           child_it.forward()) {
        child = child_it.data();
        if (child != outline && *child < *outline) {
          child_count++;
          if (child_count <= max_count) {
            int max_grand =
                (max_count - child_count) / edges_children_per_grandchild;
            if (max_grand > 0) {
              grandchild_count += count_children(child, max_grand) *
                                  edges_children_per_grandchild;
            } else {
              grandchild_count += count_children(child, 1);
            }
          }
          if (child_count + grandchild_count > max_count) {
            if (edges_debug) {
              tprintf("Discarding parent with child count=%d, gc=%d\n",
                      child_count, grandchild_count);
            }
            return child_count + grandchild_count;
          }
          if (parent_area == 0) {
            parent_area = outline->outer_area();
            if (parent_area < 0) {
              parent_area = -parent_area;
            }
            max_parent_area = outline->bounding_box().area() * edges_boxarea;
            if (parent_area < max_parent_area) {
              parent_box = false;
            }
          }
          if (parent_box &&
              (!edges_children_fix ||
               child->bounding_box().height() > edges_min_nonhole)) {
            child_area = child->outer_area();
            if (child_area < 0) {
              child_area = -child_area;
            }
            if (edges_children_fix) {
              if (parent_area - child_area < max_parent_area) {
                parent_box = false;
                continue;
              }
              if (grandchild_count > 0) {
                if (edges_debug) {
                  tprintf(
                      "Discarding parent of area %d, child area=%d, max%g "
                      "with gc=%d\n",
                      parent_area, child_area, max_parent_area,
                      grandchild_count);
                }
                return max_count + 1;
              }
              child_length = child->pathlength();
              if (child_length * child_length >
                  child_area * edges_patharea_ratio) {
                if (edges_debug) {
                  tprintf(
                      "Discarding parent of area %d, child area=%d, max%g "
                      "with child length=%d\n",
                      parent_area, child_area, max_parent_area, child_length);
                }
                return max_count + 1;
              }
            }
            if (child_area < child->bounding_box().area() * edges_childarea) {
              if (edges_debug) {
                tprintf(
                    "Discarding parent of area %d, child area=%d, max%g "
                    "with child rect=%d\n",
                    parent_area, child_area, max_parent_area,
                    child->bounding_box().area());
              }
              return max_count + 1;
            }
          }
        }
      }
    }
  }
  return child_count + grandchild_count;
}

/**
 * @name OL_BUCKETS::extract_children
 *
 * Find number of descendants of this outline.
 */

void OL_BUCKETS::extract_children( // recursive count
    C_OUTLINE *outline,            // parent outline
    C_OUTLINE_IT *it               // destination iterator
) {
  TDimension xmin, xmax; // coord limits
  TDimension ymin, ymax;
  TBOX olbox;
  C_OUTLINE_IT child_it; // search iterator

  olbox = outline->bounding_box();
  xmin = (olbox.left() - bl.x()) / BUCKETSIZE;
  xmax = (olbox.right() - bl.x()) / BUCKETSIZE;
  ymin = (olbox.bottom() - bl.y()) / BUCKETSIZE;
  ymax = (olbox.top() - bl.y()) / BUCKETSIZE;
  for (auto yindex = ymin; yindex <= ymax; yindex++) {
    for (auto xindex = xmin; xindex <= xmax; xindex++) {
      child_it.set_to_list(&buckets[yindex * bxdim + xindex]);
      for (child_it.mark_cycle_pt(); !child_it.cycled_list();
           child_it.forward()) {
        if (*child_it.data() < *outline) {
          it->add_after_then_move(child_it.extract());
        }
      }
    }
  }
}

/// @name extract_edges

void extract_edges(Image pix,      // thresholded image
                   BLOCK *block) { // block to scan
  C_OUTLINE_LIST outlines;         // outlines in block
  C_OUTLINE_IT out_it = &outlines;

  block_edges(pix, &(block->pdblk), &out_it);
  ICOORD bleft; // block box
  ICOORD tright;
  block->pdblk.bounding_box(bleft, tright);
  // make blobs
  outlines_to_blobs(block, bleft, tright, &outlines);
}

/// @name fill_buckets

static void fill_buckets(C_OUTLINE_LIST *outlines, // outlines in block
                         OL_BUCKETS *buckets       // output buckets
) {
  C_OUTLINE_IT out_it = outlines; // iterator
  C_OUTLINE_IT bucket_it;         // iterator in bucket

  for (out_it.mark_cycle_pt(); !out_it.cycled_list(); out_it.forward()) {
    auto outline = out_it.extract(); // take off list
                                     // get box
    const TBOX &ol_box(outline->bounding_box());
    bucket_it.set_to_list((*buckets)(ol_box.left(), ol_box.bottom()));
    bucket_it.add_to_end(outline);
  }
}

/**
 * @name capture_children
 *
 * Find all neighbouring outlines that are children of this outline
 * and either move them to the output list or declare this outline
 * illegal and return false.
 */

static bool capture_children(OL_BUCKETS *buckets,  // bucket sort class
                             C_BLOB_IT *reject_it, // dead grandchildren
                             C_OUTLINE_IT *blob_it // output outlines
) {
  // master outline
  auto outline = blob_it->data();
  // no of children
  int32_t child_count;
  if (edges_use_new_outline_complexity) {
    child_count =
        buckets->outline_complexity(outline, edges_children_count_limit, 0);
  } else {
    child_count = buckets->count_children(outline, edges_children_count_limit);
  }
  if (child_count > edges_children_count_limit) {
    return false;
  }

  if (child_count > 0) {
    buckets->extract_children(outline, blob_it);
  }
  return true;
}

/**
 * @name empty_buckets
 *
 * Run the edge detector over the block and return a list of blobs.
 */

static void empty_buckets(BLOCK *block,       // block to scan
                          OL_BUCKETS *buckets // output buckets
) {
  C_OUTLINE_LIST outlines; // outlines in block
                           // iterator
  C_OUTLINE_IT out_it = &outlines;
  auto start_scan = buckets->start_scan();
  if (start_scan == nullptr) {
    return;
  }
  C_OUTLINE_IT bucket_it = start_scan;
  C_BLOB_IT good_blobs = block->blob_list();
  C_BLOB_IT junk_blobs = block->reject_blobs();

  while (!bucket_it.empty()) {
    out_it.set_to_list(&outlines);
    C_OUTLINE_IT parent_it; // parent outline
    do {
      parent_it = bucket_it; // find outermost
      do {
        bucket_it.forward();
      } while (!bucket_it.at_first() &&
               !(*parent_it.data() < *bucket_it.data()));
    } while (!bucket_it.at_first());

    // move to new list
    out_it.add_after_then_move(parent_it.extract());
    // healthy blob
    bool good_blob = capture_children(buckets, &junk_blobs, &out_it);
    C_BLOB::ConstructBlobsFromOutlines(good_blob, &outlines, &good_blobs,
                                       &junk_blobs);

    if (auto l = buckets->scan_next())
      bucket_it.set_to_list(l);
    else
      break;
  }
}

/**
 * @name outlines_to_blobs
 *
 * Gather together outlines into blobs using the usual bucket sort.
 */

void outlines_to_blobs( // find blobs
    BLOCK *block,       // block to scan
    ICOORD bleft, ICOORD tright, C_OUTLINE_LIST *outlines) {
  // make buckets
  OL_BUCKETS buckets(bleft, tright);

  fill_buckets(outlines, &buckets);
  empty_buckets(block, &buckets);
}

} // namespace tesseract
