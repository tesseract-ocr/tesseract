// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the MaxTracer class, used to trace along the maximum
// value of a GrayMap produced by an EdgeDetector.
//
#ifndef HELIUM_MAXTRACER_H__
#define HELIUM_MAXTRACER_H__

#include "array.h"
#include "stack.h"
#include "traceloc.h"
#include "tracer.h"

namespace helium {

// The MaxTracer class implements the abstract Tracer class, and provides a
// tracing algorithm that is based on tracing along the maximum value.
class MaxTracer : public Tracer {
  public:
    // Constructs a MaxTracer, that traces as long as the value it is at, is
    // atleast the given minimum value.
    MaxTracer(uint8 min_value);
    
  protected:
    // Returns true, if the Trace can be initiated at the suggested position.
    // The MaxTracer implementation returns true, if the value at the 
    // specified position is a local maximum. This makes sure that the trace
    // begins at a more optimal position.
    bool CanBeginTraceAt(const Point& p, const GrayMap& activation);
    
    // Begins trace at the specified point, and continues to pixel with the
    // maximum value. Following restrictions apply to the chosen pixel:
    //
    // 1) The pixel must be within the bounds of the trace map.
    // 2) The pixel's value must be above the minimum specified value.
    // 3) The move required to visit this pixel must differ no more than 1 to
    //    the last move made (example: last move 3, this move: 2, 3, or 4).
    //    See the chain-code description in trace.h for details.
    //
    // If there are more than one possible moves, these are pushed onto a
    // stack, along with the current position.
    // If the trace runs into itself, or finds no further valid move, the
    // last alternative move and position are popped from the stack, and the
    // trace resumes from there.
    // If the stack runs empty, the trace is considered invalid, and the
    // method is aborted, returning false.
    // 
    // To avoid noise, the tracer keeps an internal counter of how many
    // old traces were crossed already. If this counter exceeds a certain limit,
    // tracing is aborted, returning false.
    //
    // If the trace hits the starting point again, after making atleast 8
    // moves, tracing is stopped, and the method returns true.
    bool TraceEdgeAt(const Point& start, Trace& trace, uint8 traceid);
    
    // Convenience method for the implementation, that returns true of the
    // given coordinates are not within the bounds of the trace map.
    inline bool OutOfBounds(int x, int y) const;
    
    uint8 min_value_;
    Stack<TraceLoc> trace_stack_;
};

} // namespace

#endif  // HELIUM_MAXTRACER_H__
