// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the TextClassifier class, which determines whether a 
// certain Trace is a text character or not. Currently, this is the only 
// available TraceClassifier.
// Note, that this is not the only phase of classification: The much more
// complex TextValidator (ClusterValidator subclass) is used to verify whether
// or not a certain cluster of characters qualify as text. Therefore the 
// TextClassifier implementation should rather provide too many, than too
// few classified Traces.
//
#ifndef HELIUM_TEXTCLASSIFIER_H__
#define HELIUM_TEXTCLASSIFIER_H__

#include "traceclassifier.h"

namespace helium {

// The TraceClassifier implementation: TextClassifier.
class TextClassifier : public TraceClassifier {
  public:
    // Classifies the given Trace as either text (TRACECLASS_TEXT) or unknown
    // (TRACECLASS_UNKNOWN). See the implementation for details.
    void Classify(Trace& trace) const;
};

} // namespace

#endif  // HELIUM_TEXTCLASSIFIER_H__
