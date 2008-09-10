// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

#include "histogram.h"
#include "shape.h"
#include "trace.h"

using namespace helium;

const int kMaxInt = ~(0x01 << (sizeof(int)*8 - 1));

Shape::Shape(const Trace& trace, const Histogram& histogram)
  : trace_(trace),
    bounds_(trace.bounds()),
    histogram_(histogram),
    trace_class_(trace.type()),
    needs_clustering_(false),
    was_subtracted_(false),
    flag_(false),
    left_neighbor_(NULL),
    right_neighbor_(NULL),
    parent_(NULL) {
}

float Shape::SlopeTo(const Shape& other) const {
  float x_diff = other.bounds().center().x - bounds_.center().x;
  float y_diff = other.bounds().center().y - bounds_.center().y;
  return y_diff / x_diff;
}
