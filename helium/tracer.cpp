// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "debugging.h"
#include "helium_image.h"
#include "trace.h"
#include "tracer.h"

using namespace helium;

const uint8 kMark = 0x01;

Tracer::Tracer() : scrap_(NULL), trace_map_(NULL) {
}

void Tracer::SetScrap(Image* scrap) {
  scrap_ = scrap;
  ASSERT(scrap_);
  
  int w = scrap->width();
  neighbor_[0] = 1;
  neighbor_[1] = w + 1;
  neighbor_[2] = w;
  neighbor_[3] = w - 1;
  neighbor_[4] = -1;
  neighbor_[5] = -w - 1;
  neighbor_[6] = -w;
  neighbor_[7] = -w + 1;
}

void Tracer::Engrave(const Trace& trace, Color* scrap_ptr) const {
  for (unsigned i = 0; i < trace.size(); i++) {
    SetAlphaAt(scrap_ptr, kMark);
    scrap_ptr += neighbor_[trace.ValueAt(i)];
  }
  SetAlphaAt(scrap_ptr, kMark);
}
