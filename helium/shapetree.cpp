// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include <stdio.h>
#include "debugging.h"
#include "shape.h"
#include "shapetree.h"
#include "stack.h"
#include "trace.h"

using namespace helium;

ShapeTree::ShapeTree() : shapes_(4) {
}

ShapeTree::~ShapeTree() {
  ClearShapes();
}

void ShapeTree::ClearShapes() {
  for (unsigned i = 0; i < shapes_.size(); i++) delete shapes_.ValueAt(i);
  shapes_.DeleteValues();
}

// Converts the given list of Traces to Shapes, and sorts them into the
// Shape tree. The input image must be specified for color analysis.
//
// The conversion is done in multiple steps:
//  1) Color histograms are extracted from the Trace interior in the given
//     Image.
//  2) A new Shape is created from the Trace and the color information.
//  3) The parents of the Shapes are set by finding bounding boxes that enclose
//     one another.
//  4) The color histograms of child Shapes are subtracted from their parent
//     Shapes, as they are considered holes. This requires us to subtract
//     every child only once for its direct parent, and we must work from
//     the outside in, to not implicitly subtract a hole multiple times.
//  5) Call Done() on the histograms to calculate the expected value and
//     variance, and to deallocate memory used for the histogram data.
void ShapeTree::ConvertTraces(const Array<Trace*>& traces, Image& image) {
  // Make all the shapes
  for (unsigned i = 0; i < traces.size(); i++) {
    // Create shape from trace
    Trace* cur_trace = traces.ValueAt(i);
    Histogram trace_hist = cur_trace->ExtractHistogramFrom(image);
    Shape *shape = new Shape(*cur_trace, trace_hist);
    shape->set_id(i);
    shapes_.Add(shape);
  }
  ASSERT(traces.size() == shapes_.size());

  // Setup parent hierarchy
  SetShapeParents();

  // Process holes
  Stack<Shape*> shape_stack;
  for (unsigned i = 0; i < shapes_.size(); i++) {
    // Push all parents onto stack -> outmost parent at the top!
    shape_stack.Clear();
    for (Shape* cur_shape = shapes_.ValueAt(i);
         cur_shape;
         cur_shape = cur_shape->parent()) {
      shape_stack.Push(cur_shape);
      if (cur_shape->was_subtracted()) break;
    }

    // Subtract histograms of children for each parent
    while (shape_stack.Size() >= 2) {
      Shape* cur_shape = shape_stack.Pop();
      Shape* child_shape = shape_stack.Top();
      bool valid = cur_shape->histogram().Subtract(child_shape->histogram());
      if (!valid) {
        cur_shape->set_flag(true);
        child_shape->set_flag(true);
      }
      cur_shape->set_was_subtracted(true);
    }
  }

  // Finish up histograms
  for (unsigned i = 0; i < shapes_.size(); i++) {
    Shape* cur_shape = shapes_.ValueAt(i);
    cur_shape->histogram().Done();
  }
}

void ShapeTree::SetShapeParents() {
  for (unsigned i = 0; i < shapes_.size(); i++) {
    Shape* outer = shapes_.ValueAt(i);
    // Find all largest enclosed shapes
    for (unsigned j = 0; j < shapes_.size(); j++) {
      if (j == i) continue;
      Shape* inner = shapes_.ValueAt(j);
      if (outer->Contains(*inner)) {
        Shape* inner_parent = inner->parent();
        // Change inner parent, if it doesn't have one, or if the one it has
        // is larger than the current candidate
        if (!inner_parent || inner_parent->Contains(*outer)) {
          inner->set_parent(outer);
        }
      }
    }
  }
}

void ShapeTree::PaintShapes(Image& dest) const {
  Color blue = MakeColor(0, 0, 255);
  Color red = MakeColor(255, 0, 0);
  for (unsigned i = 0; i < shapes_.size(); i++) {
    Shape* shape = shapes_.ValueAt(i);
    shape->trace().FillTraceOnto(dest, shape->histogram().Expected());
    dest.DrawBox(shape->bounds(), blue);
    if (shape->flag()) {
      shape->trace().FillTraceOnto(dest, red);
    }
  }
}

