// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the declaration of the 8-Bit GrayMap.
//
#ifndef HELIUM_GRAYMAP_H__
#define HELIUM_GRAYMAP_H__

#include "map.h"

namespace helium {
// A GrayMap is just a more beautiful name for an 8-bit unsigned Map.
typedef Map<uint8> GrayMap;
} // namespace

#endif // HELIUM_GRAYMAP_H__
