///////////////////////////////////////////////////////////////////////
// File:        cjkpitch.h
// Description: Code to determine fixed pitchness and the pitch if fixed,
//              for CJK text.
// Copyright 2011 Google Inc. All Rights Reserved.
// Author: takenaka@google.com (Hiroshi Takenaka)
// Created:     Mon Jun 27 12:48:35 JST 2011
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////
#ifndef CJKPITCH_H_
#define CJKPITCH_H_

#include          "blobbox.h"
#include          "notdll.h"

// Function to test "fixed-pitchness" of the input text and estimating
// character pitch parameters for it, based on CJK fixed-pitch layout
// model.
//
// This function assumes that a fixed-pitch CJK text has following
// characteristics:
//
// - Most glyphs are designed to fit within the same sized square
//   (imaginary body). Also they are aligned to the center of their
//   imaginary bodies.
// - The imaginary body is always a regular rectangle.
// - There may be some extra space between character bodies
//   (tracking).
// - There may be some extra space after punctuations.
// - The text is *not* space-delimited. Thus spaces are rare.
// - Character may consists of multiple unconnected blobs.
//
// And the function works in two passes.  On pass 1, it looks for such
// "good" blobs that has the pitch same pitch on the both side and
// looks like a complete CJK character. Then estimates the character
// pitch for every row, based on those good blobs. If we couldn't find
// enough good blobs for a row, then the pitch is estimated from other
// rows with similar character height instead.
//
// Pass 2 is an iterative process to fit the blobs into fixed-pitch
// character cells. Once we have estimated the character pitch, blobs
// that are almost as large as the pitch can be considered to be
// complete characters. And once we know that some characters are
// complete characters, we can estimate the region occupied by its
// neighbors. And so on.
//
// We repeat the process until all ambiguities are resolved. Then make
// the final decision about fixed-pitchness of each row and compute
// pitch and spacing parameters.
//
// (If a row is considered to be propotional, pitch_decision for the
// row is set to PITCH_CORR_PROP and the later phase
// (i.e. Textord::to_spacing()) should determine its spacing
// parameters)
//
// This function doesn't provide all information required by
// fixed_pitch_words() and the rows need to be processed with
// make_prop_words() even if they are fixed pitched.
void compute_fixed_pitch_cjk(ICOORD page_tr,               // top right
                             TO_BLOCK_LIST *port_blocks);  // input list

#endif  // CJKPITCH_H_
