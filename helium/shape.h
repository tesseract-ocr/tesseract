// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Shape class. While the Trace class is a data
// structure for contour information, Shapes are all about the colors inside
// of the contour. They do not contain any information to the actual Trace
// that surrounds them, other than its bounding box. This is all the
// information that is required for clustering and binarization. In fact, once
// we have evaluated the Shapes, the Traces can be discarded and removed from
// memory.
//
#ifndef HELIUM_SHAPE_H__
#define HELIUM_SHAPE_H__

#include "box.h"
#include "point.h"
#include "histogram.h"
#include "trace.h"

namespace helium {

// The Shape class contains a number of important members, including the
// bounding box of the Shape, the histogram of the Shape's colors, the Shape's
// class (Text, Barcode, ...) the Shape's left and right neighbors (set by the
// Clusterer), and the Shape's parent (set by the ShapeTree). It also contains
// flags that are important for the creation process (in ShapeTree), and the
// clustering (in Clusterer).
// Note that the Shape's histogram contains only the expected and variance
// values, since the actual histogram data is deallocated, once the important
// values have been calculated (to save memory).
class Shape {
  public:
    // Constructor to create a new Shape with the given bounding box,
    // histogram and trace class.
    Shape(const Trace& trace, const Histogram& histogram);

    // Accessor to the histogram of the Shape (const access).
    inline const Histogram& histogram() const {
      return histogram_;
    }

    // Accessor to the histogram of the Shape (non-const access).
    inline Histogram& histogram() {
      return histogram_;
    }

    inline uint8 trace_class() const {
      return trace_class_;
    }

    // Returns the left neighbor of the Shape, or NULL if none has been set.
    inline Shape* left_neighbor() const {
      return left_neighbor_;
    }

    // Returns the right neighbor of the Shape, or NULL if none has been set.
    inline Shape* right_neighbor() const {
      return right_neighbor_;
    }

    // Set the left neighbor of the Shape. Set to NULL to specify no neighbor.
    inline void set_left_neighbor(Shape* shape) {
      left_neighbor_ = shape;
    }

    // Set the right neighbor of the Shape. Set to NULL to specify no neighbor.
    inline void set_right_neighbor(Shape* shape) {
      right_neighbor_ = shape;
    }

    // Returns the bounding box of the shape.
    inline const Box& bounds() const {
      return bounds_;
    }

    // Returns true, if the Shape requires clustering.
    inline bool needs_clustering() const {
      return needs_clustering_;
    }

    // Returns true, if the Shape has been identified as a hole, and its
    // histogram was subtracted from another.
    inline bool was_subtracted() const {
      return was_subtracted_;
    }

    // Set flag, whether this Shape needs clustering.
    inline void set_needs_clustering(bool flag) {
      needs_clustering_ = flag;
    }

    // Set flag, whether this Shape's histogram has been subtracted from
    // another.
    inline void set_was_subtracted(bool flag) {
      was_subtracted_ = flag;
    }

    // Set the parent of this Shape or NULL if it is top level.
    inline void set_parent(Shape* shape) {
      parent_ = shape;
    }

    // Returns the parent of this Shape, or NULL if it is top level.
    inline Shape* parent() const {
      return parent_;
    }

    inline void set_id(int32 id) {
      id_ = id;
    }

    inline int32 id() const {
      return id_;
    }

    inline const Trace& trace() const {
      return trace_;
    }

    inline bool Contains(const Shape& shape) const {
      return trace_.Contains(shape.trace());
    }

    inline void set_flag(bool flag) {
      flag_ = flag;
    }

    inline bool flag() const {
      return flag_;
    }

    // Returns the slope of the center of the receiver to the center of the
    // specified Shape. This is useful for analyzing the flow of Shapes across
    // the image.
    float SlopeTo(const Shape& other) const;

  protected:
    const Trace& trace_;
    Box bounds_;
    Histogram histogram_;
    uint8 trace_class_;
    bool needs_clustering_;
    bool was_subtracted_;
    bool flag_;
    Shape* left_neighbor_;
    Shape* right_neighbor_;
    Shape* parent_;
    int32 id_;

  private:
    // Disallow copying
    Shape(const Shape&);
    void operator=(const Shape&);
};

} // namespace

#endif  // HELIUM_SHAPE_H__
