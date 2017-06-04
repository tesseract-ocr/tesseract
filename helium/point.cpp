// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "point.h"
#include "mathfunctions.h"

using namespace helium;

unsigned helium::Distance(const Point& p, const Point& q) {
  return SquaredRoot(Square(p.x - q.x) + Square(p.y - q.y));
}

Point helium::Middle(const Point& p, const Point& q) {
  return Point((p.x + q.x) / 2, (p.y + q.y) / 2);
}
