// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the Array subclass Trace, which stores a list of
// chain-codes, that represent the movements of a Tracer.
// Traces are important for analyzing the interior of the detected contours.
// The chain-code information itself, is not of importance for the
// clustering or even the binarization phase. But it is essential to analyzing
// the color of the pixels that lie within the trace, which is an important
// aspect of the Helium detection process.
//
#ifndef HELIUM_TRACE_H__
#define HELIUM_TRACE_H__

// Local includes
#include "array.h"
#include "box.h"
#include "debugging.h"
#include "histogram.h"
#include "point.h"

namespace helium {

class Mask;

// The Trace class is a subclass of Array<uint8>, so it provides all the
// functionality of the Array class. Conceptually, a Trace is a list of
// chain-codes, which in our case is an Array of values that range from
// 0 to 7, and have the following directional interpretation:
//
//  5   6   7
//  4   X   0
//  3   2   1
//
// In addition to that, a Trace also contains the point where the trace
// originates, and the its bounding box. However, to save unnecessary
// computing, these values are only calculated, if the Trace is closed by
// calling CloseAt(const Point&).
//
// Closed Traces provide further important functionality. The
// ExtractHistogramFrom(Image&) method, creates a histogram from all the color
// pixels that lie within the Trace. This color information is used later on
// for clustering, binarization and text box refinement.
class Trace : public Array<uint8> {
  public:
    Trace();

    // Accessor method to set the type of the trace. See available types in
    // the TraceClassifier documentation.
    inline void set_type(uint8 type) {
      type_ = type;
    }

    // Accessor method to get the type of the trace. See available types in
    // the TraceClassifier documentation.
    inline uint8 type() const {
      return type_;
    }

    // Returns the bounding box of the Trace. This is only valid after the
    // Trace has been closed with CloseAt(Point).
    inline const Box& bounds() const {
      return bounds_;
    }

    // When a Tracer has run into a dead end, and needs to start at a previous
    // position, use this method to roll back to the index at that position.
    inline void RollBackTo(unsigned size) {
      ASSERT(size <= size_);
      size_ = size;
    }

    // Allocates and returns a copy of the receiver.
    Trace* Copy() const;

    // Close the trace at the given coordinates. Call this after completing
    // a closed Trace. This method calculates the Trace's bounds and sets the
    // starting point.
    void CloseAt(Point p);

    // Creates and returns a Histogram by scanning through all the pixels that
    // lie within the Trace. This may only be called on closed Traces that have
    // been closed with the CloseAt(...) method. This method does bounds
    // checking.
    Histogram ExtractHistogramFrom(Image& image) const;

    // Plots the Trace onto the given Mask. This method does no bounds checking
    // so you must make sure, that the Trace fits onto the Mask!
    void PlotOnto(Mask& mask) const;

    // This paints the Trace interior onto the specified Image with the
    // specified color. This may only be called on closed Traces that have
    // been closed with the CloseAt(...) method. This method does bounds
    // checking.
    void FillTraceOnto(Image& image, Color color) const;

    // Test if the given contour is completely contained in this contour.
    // Both contours must be non-intersecting and closed.
    bool Contains(const Trace& trace) const;

  private:
    uint8 type_;
    Box bounds_;
    Point start_;

    // The copy constructor is used internally for the Copy() method.
    Trace(const Trace& other);

    // Assignment operations are not allowed.
    void operator=(const Trace&);

    // This method marks the left and right edges of the trace to simplify the
    // process of extracting pixels from within the trace. Algorithm details
    // are found in trace.cc!
    void MarkHorizontalBoundries(Image& image) const;

    // This removes the marks for pixel extraction, so that the marks do not
    // collide with subsequent extractions.
    void RemoveMarks(Image& image) const;

    // Helper method for marking the boundries. This method returns whether the
    // last vertical move was up or down.
    bool GetFinalVerticalMove() const;
};

} // namespace

#endif  // HELIUM_TRACE_H__
