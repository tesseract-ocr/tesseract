// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "contourdetector.h"
#include "debugging.h"
#include "image.h"
#include "traceclassifier.h"
#include "trace.h"
#include "tracer.h"

using namespace helium;

ContourDetector::ContourDetector(Tracer& tracer,
                                 const TraceClassifier& classifier)
  : classifier_(classifier),
    tracer_(tracer),
    contours_(4) {
}

ContourDetector::~ContourDetector() {
  for (unsigned i = 0; i < contours_.size(); i++) delete contours_.ValueAt(i);
}

void ContourDetector::DetectContours(const GrayMap& activation,
                                     uint8 threshold,
                                     Image& scrap) {
  // Init tracer
  tracer_.SetScrap(&scrap);

  // Find activation points, trace, and classify
  Point p;
  uint8* activ_ptr = activation.data();
  Color* dest_ptr = scrap.data();
  uint8 cur_id = 3;

  // Main loop over activation map
  for (p.y = 0; p.y < activation.height(); p.y++) {
    for (p.x = 0; p.x < activation.width(); p.x++) {
      if ((*activ_ptr >= threshold) && (Alpha(*dest_ptr) != 0x01)) {
        cur_id++;
        if (cur_id < 3) cur_id = 3; // id's 0, 1, 2 reserved!

        // Check with tracer if this is a good place to start
        if (tracer_.CanBeginTraceAt(p, activation)) {
          // There is a memory leak in TraceEdgeAt resulted from
          // Array::ResizeIfNecessary.  This does not fix the leak,
          // but narrows down the scope of trace.
          Trace* trace = new Trace();
          if (tracer_.TraceEdgeAt(p, *trace, cur_id)) {
            // Trace was completed, attempt to classify it
            trace->CloseAt(p);
            classifier_.Classify(*trace);

            if (trace->type() != TRACECLASS_UNKNOWN) {
              contours_.Add(trace->Copy());
              tracer_.Engrave(*trace, dest_ptr);
            }
          }
          delete trace;
        }
      }
      activ_ptr++;
      dest_ptr++;
    }
  }
}

void ContourDetector::PlotTracesOnto(Mask& mask, uint8 trace_type) const {
  mask.Clear();
  for (unsigned i = 0; i < contours_.size(); i++)
    if (contours_.ValueAt(i)->type() == trace_type)
      contours_.ValueAt(i)->PlotOnto(mask);
}
