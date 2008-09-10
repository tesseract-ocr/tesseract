// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file defines the basic geometrical structure: Point.
//
#ifndef HELIUM_POINT_H__
#define HELIUM_POINT_H__

namespace helium {

// The Point structure consists of two integer values x and y. Note that 
// negative values are allowed!
struct Point {
  public:
    int x, y;
    
    Point() : x(0), y(0) {
    }
    
    Point(int px, int py) : x(px), y(py) {
    }
};

// Geometrical Functions -------------------------------------------------------
// Returns true, if both points have the same coordinates.
inline bool Equals(const Point& p, const Point& q) {
  return (p.x == q.x) && (p.y == q.y);
}

// Returns the Euclidean distance between the given points.
unsigned Distance(const Point& p, const Point& q);

// Returns the point that lies in the middle of the line segment from p to q.
Point Middle(const Point& p, const Point& q);

} // helium

#endif  // HELIUM_GEOMETRY_H__
