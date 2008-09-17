// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
#include "textclassifier.h"

using namespace helium;

// Currently, the Classify method only checks whether the text is too small 
// to qualify.
void TextClassifier::Classify(Trace& trace) const {
  const Box& bounds = trace.bounds();
  if ((bounds.width() < 10) && (bounds.height() < 10)) {
    trace.set_type(TRACECLASS_UNKNOWN);
    return;
  }
  trace.set_type(TRACECLASS_TEXT);
}
