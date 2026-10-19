// (C) Copyright 2026, Stefan Weil
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Unit tests for TBLOB/TESSLINE outline management.
//
// In particular: EliminateDuplicateOutlines must only remove an outline that
// walks the same cycle of EDGEPTs as the outline it is a duplicate of. Two
// distinct cycles can have identical bounding boxes (this happens when thin
// outlines are chopped into small pieces); deleting a TESSLINE whose loop is
// the only handle into its cycle would orphan the cycle's EDGEPTs, causing
// the memory leak reported as OSS-Fuzz issue 42525527 (Indirect-leak in
// tesseract::make_edgept).
//
// Ownership note: a TESSLINE "owns" the EDGEPT cycle its loop points into,
// and ~TESSLINE (via TESSLINE::Clear) deletes that cycle. The TBLOB therefore
// takes ownership of everything assigned to its outlines list, so these tests
// attach every cycle to a TESSLINE and let the TBLOB destructor do the
// cleanup.

#include "include_gunit.h"

#include "blobs.h"

namespace tesseract {

namespace {

// Builds a circular list of EDGEPTs from the given (x, y) points and returns
// the first node.
EDGEPT *MakeCycle(const std::vector<TPOINT> &points) {
  std::vector<EDGEPT *> pts;
  for (const auto &pos : points) {
    pts.push_back(new EDGEPT);
    pts.back()->pos = pos;
  }
  for (size_t i = 0; i < points.size(); ++i) {
    pts[i]->next = pts[(i + 1) % points.size()];
    pts[i]->prev = pts[(i + points.size() - 1) % points.size()];
  }
  return pts.front();
}

// Counts the number of EDGEPT nodes in the cycle containing start.
int CycleSize(EDGEPT *start) {
  int count = 0;
  EDGEPT *pt = start;
  do {
    pt = pt->next;
    ++count;
  } while (pt != start);
  return count;
}

// Two TESSLINEs walking the same cycle are true duplicates: only one may
// remain, and the cycle's EDGEPTs must survive for the survivor. The two
// outlines are deliberately rooted at *different* points of the cycle (as
// SPLIT::UnsplitOutlineList produces them), so SameOutlineCycle must actually
// traverse the cycle to recognize the shared points.
TEST(EliminateDuplicateOutlinesTest, SameCycleIsEliminated) {
  EDGEPT *cycle1 = MakeCycle({TPOINT(0, 0), TPOINT(0, 20), TPOINT(20, 20), TPOINT(20, 0)});

  TBLOB blob;
  auto *outline1 = new TESSLINE;
  outline1->topleft = {0, 20};
  outline1->botright = {20, 20};
  outline1->loop = cycle1;

  auto *outline2 = new TESSLINE;
  outline2->topleft = {0, 20};
  outline2->botright = {20, 20};
  outline2->loop = cycle1->next; // same cycle as outline1, different root point
  outline1->next = outline2;
  blob.outlines = outline1;

  blob.EliminateDuplicateOutlines();

  // Exactly one outline remains ...
  ASSERT_EQ(blob.outlines, outline1);
  EXPECT_EQ(outline1->next, nullptr);
  // ... and its cycle is intact and still traversable.
  EXPECT_EQ(CycleSize(outline1->loop), 4);
}

// Two TESSLINEs with the same bounding box but *different* cycles are NOT
// duplicates: both must remain, and each cycle must stay owned by its
// TESSLINE. (Regressions here leave orphaned EDGEPT cycles - the OSS-Fuzz
// 42525527 leak.)
TEST(EliminateDuplicateOutlinesTest, SameBoxDifferentCycleIsKept) {
  EDGEPT *cycle1 = MakeCycle({TPOINT(0, 0), TPOINT(0, 20), TPOINT(20, 20), TPOINT(20, 0)});
  EDGEPT *cycle2 = MakeCycle({TPOINT(21, 0), TPOINT(21, 20), TPOINT(41, 20), TPOINT(41, 0)});

  TBLOB blob;
  auto *outline1 = new TESSLINE;
  outline1->topleft = {0, 20};
  outline1->botright = {20, 20};
  outline1->loop = cycle1;

  auto *outline2 = new TESSLINE;
  outline2->topleft = {0, 20};
  outline2->botright = {20, 20};
  outline2->loop = cycle2; // different cycle, same box
  outline1->next = outline2;
  blob.outlines = outline1;

  blob.EliminateDuplicateOutlines();

  // Both outlines must survive.
  ASSERT_EQ(blob.outlines, outline1);
  ASSERT_EQ(outline1->next, outline2);
  EXPECT_EQ(outline2->next, nullptr);
  // Both cycles must remain intact.
  EXPECT_EQ(CycleSize(outline1->loop), 4);
  EXPECT_EQ(CycleSize(outline2->loop), 4);
}

} // namespace

} // namespace tesseract
