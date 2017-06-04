// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the declaration of the Mask class, used for maps of 
// boolean values. 
// Note, that the current implementation does not pack the bits.
//
#ifndef HELIUM_MASK_H__
#define HELIUM_MASK_H__

#include "map.h"

namespace helium {

  // The Mask class is just a Map of bool's, that provides extra boolean 
  // operations on other Masks.
  class Mask : public Map<bool> {
    public:
      // Constructor to create empty mask with no data.
      Mask();
      
      // Constructor to allocate a mask of the specified dimenions.
      Mask(unsigned width, unsigned height);
      
      // Constructor to wrap a boolean map in a Mask object.
      Mask(unsigned width, unsigned height, bool* data);
      
      // Apply a logical OR operation of every pixel of the specified mask with
      // the receiver. Both masks must be of the same size!
      void BitOr(const Mask& other);
      
      // Apply a logical AND operation of every pixel of the specified mask with
      // the receiver. Both masks must be of the same size!
      void BitAnd(const Mask& other);
  };
  
} // namespace

#endif  // HELIUM_MASK_H__
