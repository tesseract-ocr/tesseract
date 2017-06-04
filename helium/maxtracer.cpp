// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// Local includes
#include "debugging.h"
#include "helium_image.h"
#include "maxtracer.h"
#include "mathfunctions.h"
#include "trace.h"

// C includes
#include <stdlib.h>

using namespace helium;

// Constants -------------------------------------------------------------------
// Neighborhood map          0, 1, 2,  3,  4,  5,  6,  7
const int8 kNeighborX[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };
const int8 kNeighborY[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };

const int8 kMoveForOffset[3][3] = { { 5, 6, 7 },
                                    { 4, 8, 0 },
                                    { 3, 2, 1 } };
const unsigned kMaxNoise = 8;
const uint8 kNoLastMove = 8;

// Helper functions ------------------------------------------------------------
inline uint8 InvertMove(uint8 move) {
  return (move + 4) % 8;
}

inline uint8 Mod8(int8 m) {
  return (m < 0) ? (m + 8) : (m % 8);
}

// MaxTracer implementation ----------------------------------------------------
MaxTracer::MaxTracer(uint8 min_value)
  : Tracer(), min_value_(min_value) {
}

bool MaxTracer::CanBeginTraceAt(const Point& p, const GrayMap& activation) {
  if ((p.x <= 1) || (p.x >= (activation.width() - 1))) return false;
  if ((p.y <= 1) || (p.y >= (activation.height() - 1))) return false;
  
  uint8* edge_ptr = activation.Access(p);
  unsigned width = activation.width();
  
  bool is_h_max = (*(edge_ptr - 1) <= *edge_ptr)
                  && (*(edge_ptr + 1) <= *edge_ptr);
  bool is_v_max = (*(edge_ptr - width) <= *edge_ptr)
                  && (*(edge_ptr + width) <= *edge_ptr);
                  
  if (is_h_max && is_v_max) {
    trace_stack_.Clear();
    return true;
  }
  return false;
}

bool MaxTracer::OutOfBounds(int x, int y) const {
  return (x <= 0) || (y <= 0) 
         || (x >= trace_map_->width() - 1) || (y >= trace_map_->height() - 1);
}

uint8 MoveDistance(uint8 m1, uint8 m2) {
  // NOTE: Better way to do this???
  if (m2 > m1)
    return Min(m2 - m1, (m1 + 8) - m2);
  else
    return Min(m1 - m2, (m2 + 8) - m1);
}

bool MaxTracer::TraceEdgeAt(const Point& start, Trace& trace, uint8 traceid) {
  ASSERT(trace_stack_.Empty());
  if (OutOfBounds(start.x, start.y)) return false;
  
  const GrayMap& edgemap = static_cast<const GrayMap&>(*trace_map_);
  
  uint8 brightness[8];
  
  Point p = start;
  uint8* trace_ptr = edgemap.Access(p);
  Color* history_ptr = scrap_->Access(p);
  unsigned noise = 0;
  uint8 last_move = kNoLastMove;
  
  while(*trace_ptr > 0) {
    uint8 best_move = 8;;
    
    // Are we crossing another outline?
    uint8 trace_under = Alpha(*history_ptr);
    if (trace_under) {
      if (trace_under == 0x01) 
        return false; // Ran into complete trace
      else if (trace_under != traceid) {
        noise++;  // Yes, -> increase noise level
        if (noise > kMaxNoise) return false; 
      }
    }
    
    // Back at start?
    if ((abs(start.x - p.x) <= 1) 
    && (abs(start.y - p.y) <= 1)
    && (trace.size() > 8)) {
      trace.Add(kMoveForOffset[(start.y - p.y) + 1][(start.x - p.x) + 1]);
      return true;
    }
    
    // Find best next move
    uint8 max_value= 0;
    for (uint8 d = 0; d < 8; ++d) {
      // Check if the distance to the last move code is less than 1
      if ((MoveDistance(last_move, d) > 1) && (last_move != 8))
        brightness[d] = 0;
      // Check if the next move would be within the map bounds, and the value
      // there would be atleast the minimum threshold:
      else if ((!OutOfBounds(p.x + kNeighborX[d], p.y + kNeighborY[d]))
           && (*(trace_ptr + neighbor_[d]) >= min_value_)) {
        brightness[d] = *(trace_ptr + neighbor_[d]);
        if (brightness[d] > max_value) {
          max_value = brightness[d];
          best_move = d;
        }
      } else brightness[d] = 0; 
    }
    
    if ((best_move != 8) && (trace_under != traceid)) {
      // Push alternatives onto stack
      for (uint8 m = 0; m < 8; m++) {
        if (abs(brightness[m] - max_value) < 24) {
          TraceLoc loc(p, m, trace.size());
          trace_stack_.Push(loc);
          if (trace_stack_.Size() > 4800) return false;
        }
      }
    } else {
      // At dead end: Pop valid alternative off of stack
      do {
        if (trace_stack_.Empty()) return false;
        
        // Pop trace location from stack and continue from there
        TraceLoc loc = trace_stack_.Pop();
        trace.RollBackTo(loc.index);
        p = loc.point;
        best_move = loc.move;
        trace_ptr = edgemap.Access(p);
        history_ptr = scrap_->Access(p);
      } while (Alpha(*(history_ptr + neighbor_[best_move])) == traceid);
    }
      
    trace.Add(best_move);
       
    //LOG(ERROR) << "Moving " << (int)best_move << " (" << p.x << ", " 
    //          << p.y << ")";
    
    // Mark this spot
    SetAlphaAt(history_ptr, traceid);
    
    // Move on
    p.x += kNeighborX[best_move];
    p.y += kNeighborY[best_move];
    trace_ptr += neighbor_[best_move];
    history_ptr += neighbor_[best_move];
    
    last_move = best_move;
  }
  return false;
}
