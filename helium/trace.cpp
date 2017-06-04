// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "debugging.h"
#include "helium_image.h"
#include "trace.h"

using namespace helium;

// Neighborhood map          0, 1, 2,  3,  4,  5,  6,  7
const int8 kNeighborX[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };
const int8 kNeighborY[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };

const int kMaxInt = ~(0x01 << (sizeof(int)*8 - 1));

const uint8 kMark = 0x02;

inline bool MovingDown(uint8 dir, bool old_flag) {
  if ((dir >= 1) && (dir <= 3))
    return true;
  else if ((dir >= 5) && (dir <= 7))
    return false;
  else
    return old_flag;
}

inline bool VDirectionChanged(uint8 dir, bool down_flag) {
  return ((dir >= 1) && (dir <= 3) && !down_flag)
      || ((dir >= 5) && (dir <= 7) && down_flag);
}

Trace::Trace()
  : Array<uint8>(16),
    type_(0),
    bounds_(0, 0, 0, 0),
    start_(0, 0) {
}

Trace::Trace(const Trace& other)
  : Array<uint8>(other),
    type_(other.type_),
    bounds_(other.bounds_),
    start_(other.start_) {
}

Trace* Trace::Copy() const {
  return new Trace(*this);
}

void Trace::CloseAt(Point p) {
  int min_x = kMaxInt;
  int min_y = kMaxInt;
  int max_x = 0;
  int max_y = 0;
  Point orig_p = p;

  for (unsigned i = 0; i < size(); i++) {
    if (p.x < min_x) min_x = p.x;
    if (p.y < min_y) min_y = p.y;
    if (p.x > max_x) max_x = p.x;
    if (p.y > max_y) max_y = p.y;

    p.x += kNeighborX[ValueAt(i)];
    p.y += kNeighborY[ValueAt(i)];
  }

  // Bug: we don't need to subtract 1 from min_x,min_y
  // min_x--;
  // min_y--;
  ASSERT((max_x >= min_x) && (max_y >= min_y));
  Point origin(min_x, min_y);
  bounds_.set_origin(origin);
  bounds_.set_width(max_x - min_x + 1);
  bounds_.set_height(max_y - min_y + 1);
  start_ = p;
  ASSERT(p.x == orig_p.x && p.y == orig_p.y);
}

void Trace::PlotOnto(Mask& mask) const {
  unsigned w = mask.width();
  int neighbor[8] = { 1, w + 1, w, w - 1, -1, -w - 1, -w , -w + 1 };
  bool* mask_ptr = mask.Access(start_);
  for (unsigned i = 0; i < size(); i++) {
    (*mask_ptr) = 1;
    mask_ptr += neighbor[ValueAt(i)];
  }
  (*mask_ptr) = 1;
}

bool Trace::GetFinalVerticalMove() const {
  for (int i = size() - 1; i >= 0; i--) {
    uint8 dir = ValueAt(i);
    if ((dir >= 5) && (dir <= 7))
      return false;
    else if ((dir >= 1) && (dir <= 3))
      return true;
  }
  return false;
}

// This method marks the left and right edges of the trace to simplify the
// process of extracting pixels from within the trace. The marks are written
// to the alpha channel of the given Image, and must be plotted in such a way
// that every mark signals entering or exiting the inside of the Trace.
// Scanning the inner pixels then becomes trivial, as one must only scan the
// area of the trace in the image, line by line, and at every mark
// invert a flag that specifies whether to extract the next pixel or not.
//
// To find these trace borders, the algorithm follows the trace marking the
// points it has visited, and paying attention to the following special cases:
//
//  1) Horizontal moves
//
//    |
//    +------------
//
// We do not mark the current location as a boundry if the last move was
// horizontal. (In the upper example, the moves markes with '-' will not get
// marked.
//
//  2) Change of vertical direction
//
//    |             |
//    +-------------+
//
// However, in the this example, we need to mark the second '+', even though
// the last move was horizontal. This is due to the change in the vertical
// direction, that makes this point critical to the trace. Therefore, if there
// is a change in the vertical direction, we mark the point, at which the
// direction changes as well.
//
//  3) Spikes
//
//    +-------------+
//    |             |
//    |    |        |
//    +---/ \-------+
//
// In this special case, we must not mark the tip of the spike, as this would
// suggest that we have left the interior of the trace. Therefore, if we
// detect a spike of this manner, we do not mark the coordinates at the tip of
// such a spike.
void Trace::MarkHorizontalBoundries(Image& image) const {
  unsigned w = image.width();
  int neighbor[8] = { 1, w + 1, w, w - 1, -1, -w - 1, -w , -w + 1 };
  Color* image_ptr = image.Access(start_);

  uint8 dir, last_dir = ValueAt(size() - 1);
  bool moving_down = MovingDown(last_dir, GetFinalVerticalMove());

  for (unsigned i = 0; i < size(); i++) {
    dir = ValueAt(i);

    bool spike_up = (last_dir >= 5) && (last_dir <= 7)
                    && (dir >= 1) && (dir <= 3);

    bool spike_dn = (last_dir >= 1) && (last_dir <= 3)
                    && (dir >= 5) && (dir <= 7);


    if (!spike_up && !spike_dn) {               // Ignore spikes
      if ((last_dir != 0) && (last_dir != 4))   // Ignore horizontal moves, ...
        SetAlphaAt(image_ptr, kMark);
      else if (VDirectionChanged(dir, moving_down)) // ...unless they lead to
        SetAlphaAt(image_ptr, kMark);               // a change in vertical
    }                                               // direction.

    // Move in image
    image_ptr += neighbor[dir];

    moving_down = MovingDown(dir, moving_down);
    last_dir = dir;
  }
}

void Trace::RemoveMarks(Image& image) const {
  unsigned w = image.width();
  int neighbor[8] = { 1, w + 1, w, w - 1, -1, -w - 1, -w , -w + 1 };
  Color* image_ptr = image.Access(start_);

  for (unsigned i = 0; i < size(); i++) {
    SetAlphaAt(image_ptr, 0x00);
    image_ptr += neighbor[ValueAt(i)];
  }
}

Histogram Trace::ExtractHistogramFrom(Image& image) const {
  // Is this Trace valid (has it been closed)?
  ASSERT((bounds_.width() > 0) && (bounds_.height() > 0));

  // Bounds check
  ASSERT((bounds_.left() >= 0) && (bounds_.right() < image.width()));
  ASSERT((bounds_.top() >= 0) && (bounds_.bottom() < image.height()));

  // Mark the horizontal boundries
  MarkHorizontalBoundries(image);

  // Scan pixels
  Point p;
  Color* image_ptr = image.Access(bounds_.origin());
  unsigned step_v = image.width() - bounds_.width();
  Histogram histogram;

  for (p.y = 0; p.y < bounds_.height(); p.y++) {
    bool pen = false;
    for (p.x = 0; p.x < bounds_.width(); p.x++) {
      if (Alpha(*image_ptr) == kMark)
        pen = !pen;
      else if (pen)  // exclude marker point since they are on boundary
        histogram.AddColor(*image_ptr);
      image_ptr++;
    }
    image_ptr += step_v;
  }

  // Remove marks again NOTE: Is this the best way to do it???
  RemoveMarks(image);

  return histogram;
}

void Trace::FillTraceOnto(Image& image, Color color) const {
  // Is this Trace valid (has it been closed)?
  ASSERT((bounds_.width() > 0) && (bounds_.height() > 0));

  // Bounds check
  ASSERT((bounds_.left() >= 0) && (bounds_.right() < image.width()));
  ASSERT((bounds_.top() >= 0) && (bounds_.bottom() < image.height()));

  // Mark the horizontal boundries
  MarkHorizontalBoundries(image);

  // Render pixels
  Point p;
  Color* image_ptr = image.Access(bounds_.origin());
  unsigned step_v = image.width() - bounds_.width();

  for (p.y = 0; p.y < bounds_.height(); p.y++) {
    bool pen = false;
    for (p.x = 0; p.x < bounds_.width(); p.x++) {
      if (Alpha(*image_ptr) == kMark)
         pen = !pen;
      else if (pen)  // exclude marker point since they are on boundary
         *image_ptr = color;
      image_ptr++;
    }
    image_ptr += step_v;
  }

  // Remove marks again NOTE: Is this the best way to do it???
  RemoveMarks(image);
}

// In order to test whether two non-intersecting contours A contains B, we
// need the actual coordinates of boundary points, not just directions.
// The algorithm is due to Ray Smith.  Since the traces are non-intersecting
// and closed, we simply need to test if any point (x,y) in B is inside or
// outside A.  This is achieved by counting the number of times points on
// A crosses the horizontal line at y to the right of x.  The traces must
// have been 'Closed' first.
bool Trace::Contains(const Trace& trace) const {
  // First do a quick test using bounding box
  if (!Surrounds(bounds_, trace.bounds_))
    return false;
  int x = trace.start_.x;
  int y = trace.start_.y;
  int ncross = 0;
  Point p(start_.x, start_.y);
  for (int i = 0; i < size(); ++i) {
    uint8 dir = ValueAt(i);
    p.x += kNeighborX[dir];
    p.y += kNeighborY[dir];
    if (p.x <= x) continue;   // consider only one side of x
    if (p.y == y && dir >= 1 && dir <= 3)
      ncross++;  // increment count if coming down to the horizon
    if (p.y == y+1 && dir >= 5 && dir <= 7)
      ncross--;  // decrement count if rising from the horizon
  }
  return (ncross % 2);  // If A contains B, must have odd crossings
}
