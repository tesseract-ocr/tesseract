// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the abstract base class TraceClassifier, which provides
// a common interface to all Trace classifiers. 
// A TraceClassifier should analyze a given Trace and determine whether it is
// text, a barcode or another type of object.
// Note, that this is not the only phase of classification: The much more
// complex ClusterValidator is used to verify whether or not a certain cluster 
// of characters qualify as their proposed class. Therefore TraceClassifier
// implementations should rather provide too many, than too few classified 
// Traces.
// The only current implementation, TextClassifier, determines whether a given
// trace is a text character or not (in which case it classifies the Trace as
// TRACECLASS_UNKNOWN). There is currently no classifier for barcodes.
// 
#ifndef HELIUM_TRACECLASSIFIER_H__
#define HELIUM_TRACECLASSIFIER_H__

#include "trace.h"

namespace helium {

// The list of trace classes. Every Trace must belong to exactly one class.
enum {
  TRACECLASS_UNKNOWN = 0,
  TRACECLASS_TEXT    = 1,
  TRACECLASS_BARCODE = 2
};

// This abstract base class provides a common interface to Trace classifiers.
// It consists of only one method, that takes a Trace and sets its type to the
// evaluated trace class. 
class TraceClassifier {
  public:
    // Destructor.
    virtual ~TraceClassifier() {}

    // This method classifies a given Trace, and sets its type accordingly 
    // using the set_type(uint8) method. If the Trace cannot be classified,
    // implementations should set the type to TRACECLASS_UNKNOWN.
    virtual void Classify(Trace& trace) const = 0;
};

} // namespace

#endif  // HELIUM_TRACECLASSIFIER_H__
