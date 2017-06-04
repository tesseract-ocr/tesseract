// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#ifndef HELIUM_TRACELOC_H__
#define HELIUM_TRACELOC_H__

namespace helium {

#include "point.h"

// The TraceLoc struct is used to store the location and suggested move of
// a trace. This is important for storing alternative moves during the tracing
// process.
struct TraceLoc {
  Point point;      // The point of the alternative move
  uint8 move;       // The suggested alternative move at that point
  unsigned index;   // The index of that move in the Trace.
  
  TraceLoc() : point(0, 0), index(0) {
  }
  
  TraceLoc(Point p, uint8 m, unsigned i) : point(p), move(m), index(i) {
  }
};

} // namespace

#endif // HELIUM_TRACELOC_H__
