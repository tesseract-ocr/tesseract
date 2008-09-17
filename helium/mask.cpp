// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "mask.h"

using namespace helium;

Mask::Mask() : Map<bool>() {
}

Mask::Mask(unsigned width, unsigned height) : Map<bool>(width, height) {
}

Mask::Mask(unsigned width, unsigned height, bool* data) 
  : Map<bool>(width, height, data) {
}

// Note on bit operations: We use the double boolean operators (i.e. '&&'), 
// thus potentially saving a pointer lookup for the second operand.
void Mask::BitOr(const Mask& other) {
  ASSERT((width() == other.width()) && (height() == other.height()));
  
  bool* src_ptr = other.data();
  for (bool* dest_ptr = data(); dest_ptr < DataEnd(); ++dest_ptr) {
    *dest_ptr = (*dest_ptr) || (*src_ptr);
    ++src_ptr;
  }
}

void Mask::BitAnd(const Mask& other) {
  ASSERT((width() == other.width()) && (height() == other.height()));
  
  bool* src_ptr = other.data();
  for (bool* dest_ptr = data(); dest_ptr < DataEnd(); ++dest_ptr) {
    *dest_ptr = (*dest_ptr) && (*src_ptr);
    ++src_ptr;
  }
}
