// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the ShapeTree class, which structures the Shapes in such
// a way, that the bounding box of parent Shapes enclose the bounds of their
// child Shapes.
// ShapeTree provides a method to convert Traces to Shapes, which extracts
// color information of the Trace interior, and by examining the bounding
// boxes, sorts the resulting Shape into the Shape tree.
//
#ifndef HELIUM_SHAPETREE_H__
#define HELIUM_SHAPETREE_H__

#include "array.h"
#include "image.h"

namespace helium {

class Shape;
class Trace;

// The ShapeTree class does not actually have a typical tree structured member
// variable. Instead it contains an Array of Shapes, which internally have
// parent pointers to enclosing Shapes. The most important method,
// ConvertTraces(...), builds the Shape tree from a given set of Traces.
class ShapeTree {
  public:
    ShapeTree();

    // Destructor calls ClearShapes().
    ~ShapeTree();

    // Remove all Shapes from the internal list, and free up the memory. You
    // may convert Traces to Shapes again after calling this.
    void ClearShapes();

    // Converts the given list of Traces to Shapes, and sorts them into the
    // Shape tree. The input image must be specified for color analysis.
    // The complexity of this function is O(n^2), n being the number of
    // Traces. Details are explained in shapetree.cc!
    void ConvertTraces(const Array<Trace*>& traces, Image& image);

    // Returns the Shapes in the Shape tree.
    inline const Array<Shape*>& shapes() const {
      return shapes_;
    }

    // Paints the Shapes of the Shape tree onto the specified Image. For
    // debugging only.
    void PaintShapes(Image& dest) const;

  private:
    // This method sets the parents of the Shapes, by iterating through the
    // bounding boxes of the traces, and finding boxes that enclose others.
    void SetShapeParents();

    // The Array of Shapes. Every Shape has a parent pointer, which specifies
    // the Shape's position in the tree.
    Array<Shape*> shapes_;
};

} // namespace

#endif  // HELIUM_SHAPETREE_H__
