// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the abstract base class Tracer, which provides the common
// interface to all tracer types. It is mainly used by the ContourDetector,
// which requires such a Tracer object at initialization.
//
#ifndef HELIUM_TRACER_H__
#define HELIUM_TRACER_H__

// Local includes
#include "color.h"
#include "point.h"

namespace helium { 

class Trace;

// The abstract base class Tracer provides a common interface to all tracer
// types. The only functionality it has to the outside is initialization.
// Everything else is done by the ContourDetector: It passes the scrap image
// to the tracer, asks whether the Tracer can start tracing at specific
// coordinates, and initiates the trace, if it returns true. Subclasses must
// implement atleast the two pure virtual methods, but will most likely 
// require methods for further initialization, such as setting the map to trace
// onto. 
class Tracer {
  public:
    Tracer();
    virtual ~Tracer() {
    }
    
    // Set the GrayMap to trace. This must be set before beginning the trace
    // process!
    inline void set_trace_map(const GrayMap* trace_map) {
      trace_map_ = trace_map;
    }
    
  protected:
    // The following methods should only be accessible to subclasses and
    // the ContourDetector!
    friend class ContourDetector;
    
    // Sets the scrap image of the tracer. The tracer is allowed to store any
    // 8 bit information in the alpha channel of the scrap image. This method
    // is called by the ContourDetector before starting the first trace.
    void SetScrap(Image* scrap);
    
    // This method allows the Tracer to intervene before a trace is started.
    // Subclass implementations should return false, if they do not wish to 
    // start a trace at the given point. The activation map is provided by
    // the ContourDetector as well.
    virtual bool CanBeginTraceAt(const Point& p, const GrayMap& activation) = 0;
    
    // Method to begin tracing at the specified point with the specified Trace
    // object and trace ID. Traces can use this id to identify themselves in
    // the scrap image. Implementations should return true, if the trace found
    // is a valid and closed, false otherwise.
    virtual bool TraceEdgeAt(const Point& start, 
                             Trace& trace, 
                             uint8 traceid) = 0;
    
    // This method plots the given trace into the alpha channel beginning at 
    // the given scrap pointer with the special ID of 1. This is used to signal  
    // that a trace has been accepted and is now permanent. Future traces can 
    // then detect when they run into a previous completed trace, and the 
    // ContourDetector does not start a trace at coordinates with that ID.
    void Engrave(const Trace& trace, Color* scrap_ptr) const;
    
    Image* scrap_;              // The scrap image
    const GrayMap* trace_map_;  // The map to trace
    int neighbor_[8];           // The offsets to move around in the image
};

} // namespace

#endif  // HELIUM_TRACER_H__
