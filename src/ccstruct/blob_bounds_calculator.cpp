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

#include "blob_bounds_calculator.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace tesseract {

std::ostream& operator<<(std::ostream& out, const CharBoundaryByBoxIndex& d) {
  out << "CharBoundaryByBoxIndex{ "
      << d.index << ", "
      << d.split_index << " " << d.split_count << " }";
  return out;
}

std::ostream& operator<<(std::ostream& out, const CharacterPlaceDecision& d) {
  out << "CharacterPlaceDecision{"
      << " prev_index: " << d.prev_index
      << " has_boxes: " << d.has_boxes
      << " begin: " << d.begin
      << " end: " << d.end
      << " prev_pos_diff: " << d.prev_pos_diff
      << " cost: " << d.cost
      << " }";
  return out;
}

void CharacterPlaceDecisions::add_place(unsigned prev_index, bool has_boxes,
                                        CharBoundaryByBoxIndex begin,
                                        CharBoundaryByBoxIndex end,
                                        double prev_pos_diff,
                                        double cost, double max_cost_diff) {
  if (cost > min_cost + max_cost_diff) {
    return;
  }

  int replace_existing_decision_index = -1;
  for (std::size_t i = 0; i < decisions.size(); ++i) {
    if (decisions[i].end == end) {
      if (cost < decisions[i].cost) {
        replace_existing_decision_index = i;
        break;
      } else {
        // existing decision is better
        return;
      }
    }
  }

  CharacterPlaceDecision new_decision{prev_index, has_boxes, begin, end,
                                      prev_pos_diff, cost};
  if (replace_existing_decision_index >= 0) {
      decisions[replace_existing_decision_index] = new_decision;
  } else {
      decisions.push_back(new_decision);
  }

  if (cost < min_cost) {
    min_cost = cost;

    // Remove all decisions that no longer satisfy maximum cost difference
    // requirement.
    auto last_it = std::remove_if(decisions.begin(), decisions.end(),
                                  [=](const auto& d) {
      return d.cost > min_cost + max_cost_diff;
    });
    decisions.erase(last_it, decisions.end());
  }
}

bool CharacterBoundaries::operator==(const CharacterBoundaries& other) const {
  return begin_x == other.begin_x &&
          begin_box_index == other.begin_box_index &&
          end_x == other.end_x &&
          end_box_index == other.end_box_index;
}

std::ostream& operator<<(std::ostream& out, const CharacterBoundaries& bounds) {
  out << "CharacterBoundaries{" << bounds.begin_x << ", "
      << bounds.begin_box_index << ", "
      << bounds.end_x << ", "
      << bounds.end_box_index << "}";
  return out;
}

BoxBoundariesCalculator::BoxBoundariesCalculator(
    const std::vector<BoxBoundaries>& bounds,
    const BoxBoundariesCalculatorConfig& config) :
  bounds_{bounds},
  config_{config}
{
  if (!bounds_.empty()) {
    double width_sum = 0;
    for (const auto& b : bounds) {
      width_sum += b.end - b.begin;
    }
    average_box_width_ = width_sum / static_cast<double>(bounds.size());
  }
}

std::vector<CharacterBoundaries>
  BoxBoundariesCalculator::calculate_bounds(const std::vector<BoxBoundaries>& symbols)
{
  std::vector<CharacterPlaceDecisions> decisions;
  decisions.resize(symbols.size());

  // The initial state
  CharacterPlaceDecisions init_decisions;
  init_decisions.add_place(0, true, {0, 0, 0}, {0, 0, 0}, 0, 0,
                           config_.max_character_cost_diff);

  for (std::size_t is = 0; is != symbols.size(); ++is) {
    const auto& symbol = symbols[is];
    const auto& prev_decisions = is == 0 ? init_decisions : decisions[is - 1];
    auto& next_decisions = decisions[is];

    auto [symbol_min_box, symbol_max_box] = possible_boxes_for_symbol(symbol);

    unsigned prev_farthest_index = farthest_decision_index(prev_decisions);
    const auto& prev_farthest_decision =
        prev_decisions.decisions[prev_farthest_index];

    if (symbol_min_box == symbol_max_box) {
      // There are no boxes for the current symbol. Select the previous
      // decision which went farthest and was at box boundary.
      //
      // We ignore everything that affects the cost for this symbol because the
      // cost will be the same for all decision paths, thus will not affect
      // which decision path is ultimately selected.
      auto new_cost = prev_farthest_decision.cost +
          config_.symbol_with_no_box_cost;

      // We reset prev_pos_diff as we are effectively starting over.
      next_decisions.add_place(prev_farthest_index, false, {{}, 0, 0},
                               prev_farthest_decision.end,
                               0, new_cost,
                               config_.max_character_cost_diff);
      continue;
    }

    if (prev_farthest_decision.end.index < symbol_min_box) {
      // There are boxes that can't be attributed to any of the symbols because
      // they are too far away. In this case we pick the previous decision path
      // that went farthest and force the first box to be attributed to the
      // symbol.
      //
      // We ignore everything that affects the cost for this symbol because the
      // cost will be the same for all decision paths, thus will not affect
      // which decision path is ultimately selected.

      auto boxes_with_no_symbols =
          symbol_min_box - prev_farthest_decision.end.index;

      auto new_cost = prev_farthest_decision.cost +
          config_.box_with_no_symbol_cost * boxes_with_no_symbols;

      // We reset prev_pos_diff as we are effectively starting over.
      try_decisions_from_prev_decision(next_decisions, prev_farthest_index,
                                       {symbol_min_box, 0, 0},
                                       0, new_cost,
                                       symbol, symbol_max_box);
      continue;
    }

    for (std::size_t i_d = 0; i_d < prev_decisions.decisions.size(); ++i_d) {
      const auto& prev_decision = prev_decisions.decisions[i_d];
      try_decisions_from_prev_decision(next_decisions, i_d,
                                       prev_decision.end,
                                       prev_decision.prev_pos_diff,
                                       prev_decision.cost,
                                       symbol, symbol_max_box);
    }
  }

  add_costs_for_remaining_boxes(decisions.back());
  auto best_decision_path = pick_best_decision_path(decisions);
  fix_decisions_split_count(best_decision_path);
  return decisions_to_results(symbols, best_decision_path);
}

void BoxBoundariesCalculator::try_decisions_from_prev_decision(
    CharacterPlaceDecisions& next_decisions,
    unsigned prev_decision_index,
    CharBoundaryByBoxIndex start_bound,
    double prev_decision_pos_diff,
    double prev_decision_cost,
    const BoxBoundaries& symbol, unsigned symbol_max_box)
{
  if (start_bound.split_index > 0) {
    // attempt to split the start box once again
    try_decision_from_prev_decision(next_decisions, prev_decision_index,
                                    start_bound,
                                    {start_bound.index,
                                     start_bound.split_index + 1,
                                     start_bound.split_count + 1},
                                    prev_decision_pos_diff, prev_decision_cost,
                                    symbol);
    // attempt to take the remaining split of the start box
    try_decision_from_prev_decision(next_decisions, prev_decision_index,
                                    start_bound, {start_bound.index, 0, 0},
                                    prev_decision_pos_diff, prev_decision_cost,
                                    symbol);
  }
  for (unsigned end_box = start_bound.index + 1;
       end_box <= symbol_max_box; ++end_box) {
    // try one or more full boxes
    try_decision_from_prev_decision(next_decisions, prev_decision_index,
                                    start_bound, {end_box, 0, 0},
                                    prev_decision_pos_diff, prev_decision_cost,
                                    symbol);
    // try zero or more full boxes and a split box
    try_decision_from_prev_decision(next_decisions, prev_decision_index,
                                    start_bound, {end_box, 1, 2},
                                    prev_decision_pos_diff, prev_decision_cost,
                                    symbol);
  }
}

void BoxBoundariesCalculator::try_decision_from_prev_decision(
    CharacterPlaceDecisions& next_decisions,
    unsigned prev_decision_index,
    CharBoundaryByBoxIndex start_bound, CharBoundaryByBoxIndex end_bound,
    double prev_decision_pos_diff,
    double prev_decision_cost,
    const BoxBoundaries& symbol)
{
  // The following computes the additional cost of the decision. The
  // following rules are used:
  //
  //  - The center of the resulting merged boxes that we assign to the symbol
  //    is just the middle between the start and end boundaries. We don't use
  //    anything like weighted averages because presumably the boxes actually
  //    represent a single symbol and were split into parts due to bad quality
  //    input or a segmenter error. Instead we just consider whole area as a
  //    single box.
  //
  //  - In case of split box, the boundary position is computed according to
  //    the currently known split factor without taking into account that
  //    future decisions may split the box further. In theory we could go back
  //    to previous decisions and adjust the cost, but this is not currently
  //    implemented.
  double cost = prev_decision_cost;

  bool is_split = end_bound.split_index != 0;
  if (is_split) {
    cost += config_.split_cost;
  }

  unsigned merge_count = end_bound.index - start_bound.index;
  if (start_bound.split_index == 0) {
    merge_count--;
  }

  cost += config_.merge_cost * merge_count;

  double merged_box_center = (get_box_pos_begin(start_bound) +
                              get_box_pos_end(end_bound)) / 2;
  double symbol_center = symbol.middle();

  double pos_diff = symbol_center - merged_box_center;
  double pos_diff_for_cost = 0;

  if (pos_diff < 0 && pos_diff < prev_decision_pos_diff) {
    if (prev_decision_pos_diff < 0) {
      pos_diff_for_cost = prev_decision_pos_diff - pos_diff;
    } else {
      pos_diff_for_cost = -pos_diff;
    }
  }

  if (pos_diff > 0 && pos_diff > prev_decision_pos_diff) {
    if (prev_decision_pos_diff > 0) {
      pos_diff_for_cost = pos_diff - prev_decision_pos_diff;
    } else {
      pos_diff_for_cost = pos_diff;
    }
  }

  cost += config_.pos_diff_cost * pos_diff_for_cost / average_box_width_;

  next_decisions.add_place(prev_decision_index, true, start_bound, end_bound,
                           pos_diff, cost, config_.max_character_cost_diff);
}


double BoxBoundariesCalculator::get_box_pos_begin(CharBoundaryByBoxIndex bound)
{
  if (bound.split_index == 0) {
    return bounds_[bound.index].begin;
  }
  assert(bound.index > 0);
  return get_box_split_pos(bounds_[bound.index - 1],
                           bound.split_index, bound.split_count);
}

double BoxBoundariesCalculator::get_box_pos_end(CharBoundaryByBoxIndex bound)
{
  assert(bound.index > 0);

  if (bound.split_index == 0) {
    return bounds_[bound.index - 1].end;
  }
  return get_box_split_pos(bounds_[bound.index - 1],
                           bound.split_index, bound.split_count);
}


int BoxBoundariesCalculator::farthest_decision_index(
    const CharacterPlaceDecisions& decisions)
{
    unsigned best_decision = 0;
    unsigned max_box_index = 0;
    double best_decision_cost = std::numeric_limits<double>::infinity();

    for (std::size_t i = 0; i < decisions.decisions.size(); ++i) {
      const auto& decision = decisions.decisions[i];

      if (decision.end.split_index == 0) {
        if ((decision.end.index == max_box_index &&
             decision.cost < best_decision_cost) ||
            decision.end.index < max_box_index) {
          max_box_index = decision.end.index;
          best_decision_cost = decision.cost;
          best_decision = i;
        }
      }
    }
    return best_decision;
}

std::pair<unsigned, unsigned>
  BoxBoundariesCalculator::possible_boxes_for_symbol(const BoxBoundaries& symbol)
{
    auto min = symbol.begin - config_.max_pos_diff * average_box_width_;
    auto max = symbol.end + config_.max_pos_diff * average_box_width_;

    auto range_begin = std::partition_point(bounds_.begin(), bounds_.end(),
                                            [min](const auto& b){
      return b.middle() < min;
    });

    auto range_end = std::partition_point(range_begin, bounds_.end(),
                                          [max](const auto& b){
      return b.middle() < max;
    });

    if (range_begin == bounds_.end()) {
        return { 0, 0 };
    }
    return { std::distance(bounds_.begin(), range_begin),
             std::distance(bounds_.begin(), range_end) };
}

void BoxBoundariesCalculator::add_costs_for_remaining_boxes(
    CharacterPlaceDecisions& decisions) {

  for (auto& decision : decisions.decisions) {
    if (decision.end.split_index != 0) {
      // We don't care about decisions that don't end on a box boundary.
      continue;
    }
    assert(decision.end.index > 0);

    auto unused_boxes = bounds_.size() - decision.end.index;
    decision.cost += unused_boxes * config_.box_with_no_symbol_cost;
  }
}

std::vector<CharacterPlaceDecision>
  BoxBoundariesCalculator::pick_best_decision_path(
    std::vector<CharacterPlaceDecisions>& decisions) {

  std::vector<CharacterPlaceDecision> result;
  result.resize(decisions.size());

  unsigned next_best_decision = get_best_end_decision(decisions.back());
  for (int i = decisions.size(); i > 0; --i) {
    int curr_index = i - 1;
    const auto& curr_decisions = decisions[curr_index];
    const auto& curr_best_decision = curr_decisions.decisions[next_best_decision];
    next_best_decision = curr_best_decision.prev_index;

    result[curr_index] = curr_best_decision;
  }

  return result;
}

void BoxBoundariesCalculator::fix_decisions_split_count(
    std::vector<CharacterPlaceDecision>& decisions) {
  unsigned last_box_index = std::numeric_limits<unsigned>::max();
  unsigned last_box_split_count = 0;

  auto adjust_index = [&](CharBoundaryByBoxIndex& index) {
    // The box indexes are always increasing and the last index with nonzero
    // split_count contains the largest split_count that we must apply to the
    // rest of indexes with nonzero split_count and the same box index.
    // Note that we iterate backwards in the loop below, so the order reverses
    // here.
    if (index.index == last_box_index) {
      if (index.split_count != 0) {
        last_box_split_count = index.split_count;
      }
      index.split_count = last_box_split_count;
    } else {
      last_box_index = index.index;
      last_box_split_count = index.split_count;
    }
  };

  for (auto it = decisions.rbegin(); it != decisions.rend(); it++) {
    if (it->has_boxes) {
      adjust_index(it->end);
      adjust_index(it->begin);
    }
  }
}

std::vector<CharacterBoundaries> BoxBoundariesCalculator::decisions_to_results(
    const std::vector<BoxBoundaries>& symbols,
    const std::vector<CharacterPlaceDecision>& decisions)
{
  std::vector<CharacterBoundaries> results;
  results.resize(symbols.size());

  for (int i = decisions.size(); i > 0; --i) {
    int curr_index = i - 1;
    const auto& decision = decisions[curr_index];
    const auto& symbol = symbols[curr_index];

    if (!decision.has_boxes) {
      results[curr_index] = CharacterBoundaries{symbol.begin, 0, symbol.end, 0};
      continue;
    }

    // The result is in terms of boxes that are at least partially assigned to
    // characters. Decisions store bounds which need adjustment in case of
    // split boxes.
    auto begin_index = decision.begin.index;
    if (decision.begin.split_count > 0) {
      begin_index--;
    }

    results[curr_index] = CharacterBoundaries{
            static_cast<int>(get_box_pos_begin(decision.begin)),
            begin_index,
            static_cast<int>(get_box_pos_end(decision.end)),
            decision.end.index};
  }

  return results;
}

int BoxBoundariesCalculator::get_best_end_decision(
    const CharacterPlaceDecisions& decisions) {
  assert(!decisions.decisions.empty());

  unsigned best_decision = 0;
  double min_cost = std::numeric_limits<double>::infinity();

  for (unsigned i = 0; i < decisions.decisions.size(); ++i) {
    const auto& decision = decisions.decisions[i];
    if (decision.end.split_index != 0)
      continue;
    if (decision.cost < min_cost) {
      best_decision = i;
      min_cost = decision.cost;
    }
  }

  return best_decision;
}

} // namespace tesseract
