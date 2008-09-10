// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "box.h"
#include "third_party/leptonica/include/allheaders.h"

// Enclose all routines in helium namespace to avoid conflicts with
// leptonica Box.

namespace helium {

Box ScaleBox(const Box& box, float scale) {
  float x = box.left();
  float y = box.top();
  float w = box.width();
  float h = box.height();
  
  return Box(static_cast<int>(x * scale),
             static_cast<int>(y * scale),
             static_cast<unsigned>(w * scale),
             static_cast<unsigned>(h * scale));
}

bool Encloses(const Box& outer, const Box& inner) {
  return  (outer.left() <= inner.left())
          && (outer.top() <= inner.top())
          && (outer.right() >= inner.right())
          && (outer.bottom() >= inner.bottom());
}

bool Surrounds(const Box& outer, const Box& inner) {
  return  (outer.left() < inner.left())
          && (outer.top() < inner.top())
          && (outer.right() > inner.right())
          && (outer.bottom() > inner.bottom());
}

bool Intersect(const Box& a, const Box& b) {
  return (Max(a.left(), b.left()) < Min(a.right(), b.right()))
         && (Max(a.top(), b.top()) < Min(a.bottom(), b.bottom())); 
}

Box MinEnclosingBox(const Box& a, const Box& b) {
  Box out = Box(Min(a.left(), b.left()),
                Min(a.top(), b.top()),
                0, 0);
  out.set_right(Max(a.right(), b.right()));
  out.set_bottom(Max(a.bottom(), b.bottom()));
  return out;
}

Box Intersection(const Box& a, const Box& b) {
  Box out = Box(Max(a.left(), b.left()),
                Max(a.top(), b.top()),
                0, 0);
  
  unsigned right = Min(a.right(), b.right());
  unsigned bottom = Min(a.bottom(), b.bottom());
  
  if ((right > out.left()) && (bottom > out.top())) {
    out.set_right(right);
    out.set_bottom(bottom);
  }
  
  return out;
}

Box MinEnclosingBox(const Array<Box>& boxes) {
  ASSERT(boxes.size() > 0);
  Box bounds = boxes.ValueAt(0);
  for (unsigned i = 1; i < boxes.size(); i++) 
    bounds = MinEnclosingBox(bounds, boxes.ValueAt(i));
  return bounds;
}

void BoxSetsIntersection(const Array<Box>& boxes_a,
                                 const Array<Box>& boxes_b,
                                 Array<Box>& result) {
  for (unsigned i = 0; i < boxes_a.size(); i++) {
    for (unsigned j = 0; j < boxes_b.size(); j++) {
      Box intersect = Intersection(boxes_a.ValueAt(i), boxes_b.ValueAt(j));
      if (intersect.Area() > 0) result.Add(intersect);
    }
  }
}

void BoxesInvert(const Array<Box>& boxes,
                               Array<Box>& inversion) {
  if (boxes.size() == 0) return;
  
  Array<Box> frame_boxes(4);
  
  frame_boxes.Add(MinEnclosingBox(boxes));
  unsigned cur_index = 0;
  unsigned max_index = 0;
  
  // Find inverse of area of boxes
  for (unsigned i = 0; i < boxes.size(); i++) {
    for (unsigned j = cur_index; j <= max_index; j++) {
      Box cur_area = frame_boxes.ValueAt(j);
      CutBox(cur_area, boxes.ValueAt(i), frame_boxes);
    }
    cur_index = max_index + 1;
    max_index = frame_boxes.size() - 1;
  }
  
  // Add to output
  for (unsigned i = cur_index; i <= max_index; i++) 
    inversion.Add(frame_boxes.ValueAt(i));
}

unsigned Area(const Array<Box>& boxes) {
  if (boxes.size() == 0) return 0;
  Box min_enclosing = MinEnclosingBox(boxes);
  // Create bitmap for min_enclosing box and set all pixels in intersection
  Pix *pix = pixCreate(min_enclosing.width(), min_enclosing.height(), 1);
  for (unsigned i = 0; i < boxes.size(); i++) {
    // translate to align with min_enclosing box
    pixRasterop(pix,
                boxes.ValueAt(i).left() - min_enclosing.left(),
                boxes.ValueAt(i).top() - min_enclosing.top(),
                boxes.ValueAt(i).width(), boxes.ValueAt(i).height(),
                PIX_SET, NULL, 0, 0);
  }
  // Count set pixels
  l_int32 count = 0;
  pixCountPixels(pix, &count, NULL);
  pixDestroy(&pix);
  return static_cast<unsigned int>(count);
}

void CutBox(const Box& area, const Box& cut, Array<Box>& result) {
  if (!Intersect(cut, area)) {
    result.Add(area);
    return;
  }
    
  // First rectangle: Left
  int left = area.left();
  int top = area.top();
  int right = cut.left();
  int bottom = area.bottom();
  
  if ((left < right) && (top < bottom))
    result.Add(Box(left, top, right - left, bottom - top));
  
  // Second rectangle: Top
  left = Max(cut.left(), area.left());
  top = area.top();
  right = Min(cut.right(), area.right());
  bottom = cut.top();

  if ((left < right) && (top < bottom)) 
    result.Add(Box(left, top, right - left, bottom - top));
  
  // Third rectangle: Right
  left = cut.right();
  top = area.top();
  right = area.right();
  bottom = area.bottom();
  
  if ((left < right) && (top < bottom))
    result.Add(Box(left, top, right - left, bottom - top));
    
  // Fourth rectangle: Bottom
  left = Max(cut.left(), area.left());
  top = cut.bottom();
  right = Min(cut.right(), area.right());
  bottom = area.bottom();
  
  if ((left < right) && (top < bottom))
    result.Add(Box(left, top, right - left, bottom - top));
}

}
