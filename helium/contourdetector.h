// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the ContourDetector, which uses a specified Tracer to
// detect closed contours on a Map. This is probably the most important step
// in Helium, as the output of the contour detection is the basis for all 
// further classification. The step can be seen as the conversion from image
// data to object data (in our case the objects are closed traces).
//
// The real tracing work is done by the Tracer, which is passed in the 
// constructor of ContourDetector. This makes it easy to exchange one Tracer
// for another or test out new types of Tracers, without reprogramming the
// steps necessary to start a trace every time.
//
#ifndef HELIUM_OUTLINEDETECTOR_H__
#define HELIUM_OUTLINEDETECTOR_H__

#include "array.h"
#include "graymap.h"
#include "types.h"

namespace helium {

class Image;
class Mask;
class Trace;
class TraceClassifier;
class Tracer;

// The ContourDetector class runs a specified Tracer at certain specified
// locations of a Map. If, for a certain location, the Tracer returns a valid
// Trace, and that Trace can be classified using a specified TraceClassifier,
// it is considered a valid contour, and added to an internal list.
class ContourDetector {
  public:
    // Constructor to initialize ContourDetector with the specified Tracer
    // and TraceClassifier.
    ContourDetector(Tracer& tracer, const TraceClassifier& classifier);
    
    // Deconstructor to delete the contours.
    ~ContourDetector();
    
    // Use the Tracer, that was specified in the constructor, to find closed
    // contours in the image. A Graymap must be provided, that acts as the
    // activation map for the detector. Together with a threshold, the 
    // activation map specifies at what coordinates to start tracing (namely,   
    // when the value of the map at those coordinates exceeds the threshold).
    // The scrap image is used to store temporary trace information in the
    // alpha channel. It must be of the same size as the activation map.
    // Calling this method multiple times will add the found contours to the
    // list of already found contours.
    void DetectContours(const GrayMap& activation, 
                        uint8 threshold,
                        Image& scrap);
    
    // Plot the found traces onto the given mask. The mask must be large enough
    // to hold the traces, and a call to DetectContours(...) must have been 
    // made to produce any results.
    void PlotTracesOnto(Mask& mask, uint8 trace_type) const;
    
    // Returns the detected contours. Note that the contours are owned by the
    // ContourDetector!
    const Array<Trace*>& contours() const {
      return contours_;
    }
    
  protected:
    const TraceClassifier& classifier_;
    Tracer& tracer_;
    Array<Trace*> contours_;
  
  private:
    ContourDetector(const ContourDetector&);
    void operator=(const ContourDetector&);
};

} // namespace

#endif  // HELIUM_OUTLINEDETECTOR_H__
