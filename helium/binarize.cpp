// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)

// Local includes
#include "binarize.h"
#include "mathfunctions.h"

// C includes
#include <stdlib.h>

using namespace helium;

Mask Binarize::Maximum(const GrayMap& map, uint8 threshold) {
  Mask mask(map.width(), map.height());
  bool* mask_ptr = mask.Access(1, 1);
  uint8* map_ptr = map.Access(1, 1);
  int r = map.width();
  
  // Find zero crossings by comparing signs of two neighbors
  for (unsigned y = 1; y < map.height() - 1; y++) {
    for (unsigned x = 1; x < map.width() - 1; x++) {
      int val = *(map_ptr);
      int s1 = Min(val - *(map_ptr - 1), val - *(map_ptr + 1));
      int s2 = Min(val - *(map_ptr - r), val - *(map_ptr + r));
      int s3 = Min(val - *(map_ptr - r - 1), val - *(map_ptr + r + 1));
      int s4 = Min(val - *(map_ptr - r + 1), val - *(map_ptr + r - 1));
      *mask_ptr = (val > threshold) && (Max(Max(s1, s2), Max(s3, s4)) > 0);
      ++map_ptr;
      ++mask_ptr;
    }
    map_ptr += 2;
    mask_ptr += 2;
  }
    
  return mask;
}

Mask Binarize::Threshold(const GrayMap& map, uint8 threshold) {
  Mask mask(map.width(), map.height());
  bool* mask_ptr = mask.data();
  
  for (uint8* map_ptr = map.data(); map_ptr <= map.DataEnd(); map_ptr++)
    *(mask_ptr++) = (*map_ptr >= threshold);
    
  return mask;
}

Mask Binarize::ZeroCrossings(const GrayMap& map, uint8 min_slope) {
  Mask mask(map.width(), map.height());
  bool* mask_ptr = mask.data();
  uint8* map_ptr = map.data();
  unsigned row = map.width();
  
  // Find zero crossings by comparing signs of two neighbors
  for (unsigned y = 0; y < map.height() - 1; y++) {
    for (unsigned x = 0; x < map.width() - 1; x++) {
      bool is_positive = (*map_ptr) >= 127;
      
      if ((*(map_ptr + 1) >= 127) ^ is_positive)
        *(mask_ptr++) = abs(*map_ptr - *(map_ptr + 1)) >= min_slope;
      else if ((*(map_ptr + row) >= 127) ^ is_positive)
        *(mask_ptr++) = abs(*map_ptr - *(map_ptr + row)) >= min_slope;
      else
        *(mask_ptr++) = false;
        
      map_ptr++;
    }
    map_ptr++;
    mask_ptr++;
  }
    
  return mask;
}
