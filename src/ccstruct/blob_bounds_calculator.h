///////////////////////////////////////////////////////////////////////
// File:        blob_bounds_calculator.h
// Description: Module for calculation of blob bounds from LSTM data
// Author:      Povilas Kanapickas
//
// (C) Copyright 2022, Povilas Kanapickas <povilas@radix.lt>
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

#ifndef TESSERACT_CCSTRUCT_BLOB_BOUNDS_CALCULATOR_H
#define TESSERACT_CCSTRUCT_BLOB_BOUNDS_CALCULATOR_H

#include <iosfwd>
#include <limits>
#include <optional>
#include <vector>

namespace tesseract {

/* This file contains an implementation of an algorithm for improving character
   positions when using LSTM models. LSTM model output produces only approximate
   character positions without boundary data. Matching it to the blobs that
   comprise the characters is non-trivial task, because the character positions
   in the LTSM output have drift that is large enough for simple algorithms such
   "pick nearest blobs" to produce large amounts of errors.

   It can be noticed that while LSTM model output produces only approximate
   character positions, the regular segmenter is pretty good. Most of the blob
   boundaries correspond to boundaries of characters and most significant errors
   are occasional blobs that correspond to multiple characters or multiple blobs
   that correspond to a single character.

   Thus the basic idea of the algorithm is to treat the output of the regular
   segmenter as a template to which LSTM model output is matched. The selection
   of best match is done by assigning each unwanted property a cost and
   then minimizing the total cost of the solution. The algorithm uses the
   following costs:

    - cost for merging multiple blobs to represent a character
    - cost for splitting a blob to represent multiple characters
    - cost for difference between the positions of the blobs and characters
      that they are matched to.

   The cost of difference between positions is computed not by simply
   accumulating the sum of all position differences, but by only taking into
   account additional difference of each character compared to previous
   character. This way the algorithm does not attempt to "optimize" out of
   place characters by adding unneeded blob merges and splits.

   The optimization problem is solved by dynamic programming techniques by
   noticing that assigning specific blobs to a character leaves us with a
   slightly smaller problem.

   The approach is to place the first character in all potential positions
   and record the outcomes. Then for each of these outcomes attempts are made
   to place the second character at all potential positions and so on.
   Whenever there are multiple decision paths to arrive to a situation when the
   end of a specific character is at the same position, the path with the
   lowest cost is picked and others are ignored.
*/

// Represents a character boundary in terms of index of a box in a list and
// potentially partition within that box.
struct CharBoundaryByBoxIndex {
  // The index of the box following the boundary.
  unsigned index = 0;

  // The location of the boundary within the box. split_count == 0 means that
  // the boundary is just before the box. Otherwise, the location is
  // (split_index / split_count) position within the preceding box.
  unsigned split_index = 0;
  unsigned split_count = 0;

  bool operator==(const CharBoundaryByBoxIndex& other) const {
    return index == other.index &&
        split_index == other.split_index &&
        split_count == other.split_count;
  }

  bool operator!=(const CharBoundaryByBoxIndex& other) const {
    return !(*this == other);
  }
};

std::ostream& operator<<(std::ostream& out, const CharBoundaryByBoxIndex& d);


// Represents a placement of a specific character at specific location.
struct CharacterPlaceDecision {
  // Index of the placement decision of the previous character.
  unsigned prev_index;
  // Whether the character had any boxes assigned to it. If not, then the
  // data stored in `begin` in not defined.
  bool has_boxes = false;
  // Placement of the start of a character in the input box list.
  CharBoundaryByBoxIndex begin;
  // Placement of the end of a character in the input box list.
  CharBoundaryByBoxIndex end;
  // The difference of positions between the center of the previous character
  // and the center of the assigned boxes
  double prev_pos_diff = 0;
  // The cost incurred so far
  double cost = 0;
};

std::ostream& operator<<(std::ostream& out, const CharacterPlaceDecision& d);


// Represents a set of placement decisions for a specific character
struct CharacterPlaceDecisions {
  std::vector<CharacterPlaceDecision> decisions;
  // minimum cost across all decisions
  double min_cost = std::numeric_limits<double>::infinity();

  // Adds a character placement decision.
  void add_place(unsigned prev_index, bool has_boxes,
                 CharBoundaryByBoxIndex begin, CharBoundaryByBoxIndex end,
                 double prev_pos_diff, double cost, double max_cost_diff);
};

// Represents bounds of a box in X direction
struct BoxBoundaries {
  int begin = 0;
  int end = 0;

  double middle() const { return (double(begin) + end) / 2; }
};


// Represents resulting character boundaries. The exact X positions are
// provided as well as which input blobs the character corresponds to, which
// allows computing correct boundaries in the Y axis.
struct CharacterBoundaries {
  int begin_x = 0;

  // Inclusive index of the beginning box.
  unsigned begin_box_index = 0;

  int end_x = 0;

  // Exclusive index of the ending box. If box data is invalid,
  // begin_box_index == end_box_index
  unsigned end_box_index = 0;

  bool operator==(const CharacterBoundaries& other) const;
};

std::ostream& operator<<(std::ostream& out, const CharacterBoundaries& bounds);


struct BoxBoundariesCalculatorConfig
{
  // The cost of each merging of two input boxes.
  double merge_cost = 2;

  // The cost of each split of two input boxes.
  double split_cost = 2;

  // The cost of each box that is not attributed to any symbol
  double box_with_no_symbol_cost = 2.2;

  // The cost of each symbol that has no boxes
  double symbol_with_no_box_cost = 2.2;

  // The cost of difference between the center the symbol and the center of
  // the input box. This cost is only incurred whenever subsequent character
  // "moves" in wrong direction. The total cost is computed by multiplying
  // the multiplier and the difference of positions relative to the average
  // width of input boxes.
  double pos_diff_cost = 1;

  // Defines which boxes to potentially consider for symbol. The number is
  // relative to the average width of input boxes.
  double max_pos_diff = 2;

  // Defines the maximum difference between minimum and maximum cost for all
  // placements of a character.
  double max_character_cost_diff = 5;
};

// See the description of the algorithm at the top of the file.
class BoxBoundariesCalculator {
public:
  // Constructs the calculator for blob boundaries computed by regular
  // segmenter.
  BoxBoundariesCalculator(const std::vector<BoxBoundaries>& bounds,
                          const BoxBoundariesCalculatorConfig& config);

  // Computes improved character positions given LSTM model output. For the
  // purposes of character positioning only the center coordinate is used.
  // The start and end coordinates are used only as a fallback when the data
  // does not match any input blobs.
  std::vector<CharacterBoundaries>
    calculate_bounds(const std::vector<BoxBoundaries>& symbols);

private:

  // This function takes all possible combinations of box boundaries between
  // start_bound and symbol_max_box, computes the costs of each option and adds
  // them to next_decisions array. The number of possibilities is approximately
  // (symbol_max_box - start_bound.index) * 2. The number is twice the number
  // of available boxes in range because we may want to split each box with
  // subsequent symbol.
  void try_decisions_from_prev_decision(CharacterPlaceDecisions& next_decisions,
                                        unsigned prev_decision_index,
                                        CharBoundaryByBoxIndex start_bound,
                                        double prev_decision_pos_diff,
                                        double prev_decision_cost,
                                        const BoxBoundaries& symbol,
                                        unsigned symbol_max_box);

  void try_decision_from_prev_decision(CharacterPlaceDecisions& next_decisions,
                                       unsigned prev_decision_index,
                                       CharBoundaryByBoxIndex start_bound,
                                       CharBoundaryByBoxIndex end_bound,
                                       double prev_decision_pos_diff,
                                       double prev_decision_cost,
                                       const BoxBoundaries& symbol);

  double get_box_pos_begin(CharBoundaryByBoxIndex bound);
  double get_box_pos_end(CharBoundaryByBoxIndex bound);

  double get_box_split_pos(const BoxBoundaries& b, unsigned split_index,
                           unsigned split_count)
  {
    return b.begin + (b.end - b.begin) * double(split_index) / split_count;
  }

  static int farthest_decision_index(const CharacterPlaceDecisions& decisions);

  std::pair<unsigned, unsigned>
    possible_boxes_for_symbol(const BoxBoundaries& symbol);


  // Goes through the decisions and adds costs for all boxes that have not
  // been added to a symbol.
  void add_costs_for_remaining_boxes(CharacterPlaceDecisions& decisions);

  // Goes through the final decisions and picks full path of the best placement
  // decision.
  std::vector<CharacterPlaceDecision> pick_best_decision_path(
    std::vector<CharacterPlaceDecisions>& decisions);

  // When constructing decisions we didn't care to update split sizes of
  // blobs when splitting more than once. As a result, splitting a blob into 4
  // parts splits at 0.5, 0.66 and 0.75 of the blob whereas the correct
  // splits are at 0.25, 0.5, 0.75. We assume this does not matter when
  // computing the costs, but for positions of the characters we need to
  // produce exact results.
  void fix_decisions_split_count(std::vector<CharacterPlaceDecision>& decisions);

  std::vector<CharacterBoundaries>
      decisions_to_results(const std::vector<BoxBoundaries>& symbols,
                           const std::vector<CharacterPlaceDecision>& decisions);

  // Finds the best decision from the final decisions. The best decision is
  // such that it has minimum cost among decisions that end at an proper box
  // boundary.
  static int get_best_end_decision(const CharacterPlaceDecisions& decisions);

private:
  std::vector<BoxBoundaries> bounds_;
  BoxBoundariesCalculatorConfig config_;
  double average_box_width_ = 0;
};

} // namespace tesseract

#endif // TESSERACT_CCSTRUCT_BLOB_BOUNDS_CALCULATOR_H
