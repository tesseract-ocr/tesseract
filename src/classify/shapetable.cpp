// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapetable.cpp
// Description: Class to map a classifier shape index to unicharset
//              indices and font indices.
// Author:      Ray Smith
//
// (C) Copyright 2010, Google Inc.
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

#include "shapetable.h"

#include "bitvector.h"
#include "fontinfo.h"
#include "intfeaturespace.h"
#include "unicharset.h"
#include "unicity_table.h"

#include <algorithm>

namespace tesseract {

// Helper function to get the index of the first result with the required
// unichar_id. If the results are sorted by rating, this will also be the
// best result with the required unichar_id.
// Returns -1 if the unichar_id is not found
int ShapeRating::FirstResultWithUnichar(const std::vector<ShapeRating> &results,
                                        const ShapeTable &shape_table, UNICHAR_ID unichar_id) {
  for (unsigned r = 0; r < results.size(); ++r) {
    const auto shape_id = results[r].shape_id;
    const Shape &shape = shape_table.GetShape(shape_id);
    if (shape.ContainsUnichar(unichar_id)) {
      return r;
    }
  }
  return -1;
}

// Helper function to get the index of the first result with the required
// unichar_id. If the results are sorted by rating, this will also be the
// best result with the required unichar_id.
// Returns -1 if the unichar_id is not found
int UnicharRating::FirstResultWithUnichar(const std::vector<UnicharRating> &results,
                                          UNICHAR_ID unichar_id) {
  for (unsigned r = 0; r < results.size(); ++r) {
    if (results[r].unichar_id == unichar_id) {
      return r;
    }
  }
  return -1;
}

// Writes to the given file. Returns false in case of error.
bool UnicharAndFonts::Serialize(FILE *fp) const {
  return tesseract::Serialize(fp, &unichar_id) && tesseract::Serialize(fp, font_ids);
}

// Reads from the given file. Returns false in case of error.
bool UnicharAndFonts::DeSerialize(TFile *fp) {
  return fp->DeSerialize(&unichar_id) && fp->DeSerialize(font_ids);
}

// Sort function to sort a pair of UnicharAndFonts by unichar_id.
int UnicharAndFonts::SortByUnicharId(const void *v1, const void *v2) {
  const auto *p1 = static_cast<const UnicharAndFonts *>(v1);
  const auto *p2 = static_cast<const UnicharAndFonts *>(v2);
  return p1->unichar_id - p2->unichar_id;
}

bool UnicharAndFonts::StdSortByUnicharId(const UnicharAndFonts &v1, const UnicharAndFonts &v2) {
  return v1.unichar_id < v2.unichar_id;
}

// Writes to the given file. Returns false in case of error.
bool Shape::Serialize(FILE *fp) const {
  uint8_t sorted = unichars_sorted_;
  return tesseract::Serialize(fp, &sorted) && tesseract::Serialize(fp, unichars_);
}
// Reads from the given file. Returns false in case of error.

bool Shape::DeSerialize(TFile *fp) {
  uint8_t sorted;
  if (!fp->DeSerialize(&sorted)) {
    return false;
  }
  unichars_sorted_ = sorted != 0;
  return fp->DeSerialize(unichars_);
}

// Adds a font_id for the given unichar_id. If the unichar_id is not
// in the shape, it is added.
void Shape::AddToShape(int unichar_id, int font_id) {
  for (auto &unichar : unichars_) {
    if (unichar.unichar_id == unichar_id) {
      // Found the unichar in the shape table.
      std::vector<int> &font_list = unichar.font_ids;
      for (int f : font_list) {
        if (f == font_id) {
          return; // Font is already there.
        }
      }
      font_list.push_back(font_id);
      return;
    }
  }
  // Unichar_id is not in shape, so add it to shape.
  unichars_.emplace_back(unichar_id, font_id);
  unichars_sorted_ = unichars_.size() <= 1;
}

// Adds everything in other to this.
void Shape::AddShape(const Shape &other) {
  for (const auto &unichar : other.unichars_) {
    for (unsigned f = 0; f < unichar.font_ids.size(); ++f) {
      AddToShape(unichar.unichar_id, unichar.font_ids[f]);
    }
  }
  unichars_sorted_ = unichars_.size() <= 1;
}

// Returns true if the shape contains the given unichar_id, font_id pair.
bool Shape::ContainsUnicharAndFont(int unichar_id, int font_id) const {
  for (const auto &unichar : unichars_) {
    if (unichar.unichar_id == unichar_id) {
      // Found the unichar, so look for the font.
      auto &font_list = unichar.font_ids;
      for (int f : font_list) {
        if (f == font_id) {
          return true;
        }
      }
      return false;
    }
  }
  return false;
}

// Returns true if the shape contains the given unichar_id, ignoring font.
bool Shape::ContainsUnichar(int unichar_id) const {
  for (const auto &unichar : unichars_) {
    if (unichar.unichar_id == unichar_id) {
      return true;
    }
  }
  return false;
}

// Returns true if the shape contains the given font, ignoring unichar_id.
bool Shape::ContainsFont(int font_id) const {
  for (const auto &unichar : unichars_) {
    auto &font_list = unichar.font_ids;
    for (int f : font_list) {
      if (f == font_id) {
        return true;
      }
    }
  }
  return false;
}
// Returns true if the shape contains the given font properties, ignoring
// unichar_id.
bool Shape::ContainsFontProperties(const FontInfoTable &font_table, uint32_t properties) const {
  for (const auto &unichar : unichars_) {
    auto &font_list = unichar.font_ids;
    for (int f : font_list) {
      if (font_table.at(f).properties == properties) {
        return true;
      }
    }
  }
  return false;
}
// Returns true if the shape contains multiple different font properties,
// ignoring unichar_id.
bool Shape::ContainsMultipleFontProperties(const FontInfoTable &font_table) const {
  uint32_t properties = font_table.at(unichars_[0].font_ids[0]).properties;
  for (const auto &unichar : unichars_) {
    auto &font_list = unichar.font_ids;
    for (int f : font_list) {
      if (font_table.at(f).properties != properties) {
        return true;
      }
    }
  }
  return false;
}

// Returns true if this shape is equal to other (ignoring order of unichars
// and fonts).
bool Shape::operator==(const Shape &other) const {
  return IsSubsetOf(other) && other.IsSubsetOf(*this);
}

// Returns true if this is a subset (including equal) of other.
bool Shape::IsSubsetOf(const Shape &other) const {
  for (const auto &unichar : unichars_) {
    int unichar_id = unichar.unichar_id;
    const std::vector<int> &font_list = unichar.font_ids;
    for (int f : font_list) {
      if (!other.ContainsUnicharAndFont(unichar_id, f)) {
        return false;
      }
    }
  }
  return true;
}

// Returns true if the lists of unichar ids are the same in this and other,
// ignoring fonts.
// NOT const, as it will sort the unichars on demand.
bool Shape::IsEqualUnichars(Shape *other) {
  if (unichars_.size() != other->unichars_.size()) {
    return false;
  }
  if (!unichars_sorted_) {
    SortUnichars();
  }
  if (!other->unichars_sorted_) {
    other->SortUnichars();
  }
  for (unsigned c = 0; c < unichars_.size(); ++c) {
    if (unichars_[c].unichar_id != other->unichars_[c].unichar_id) {
      return false;
    }
  }
  return true;
}

// Sorts the unichars_ vector by unichar.
void Shape::SortUnichars() {
  std::sort(unichars_.begin(), unichars_.end(), UnicharAndFonts::StdSortByUnicharId);
  unichars_sorted_ = true;
}

ShapeTable::ShapeTable() : unicharset_(nullptr), num_fonts_(0) {}
ShapeTable::ShapeTable(const UNICHARSET &unicharset) : unicharset_(&unicharset), num_fonts_(0) {}

// Writes to the given file. Returns false in case of error.
bool ShapeTable::Serialize(FILE *fp) const {
  return tesseract::Serialize(fp, shape_table_);
}
// Reads from the given file. Returns false in case of error.

bool ShapeTable::DeSerialize(TFile *fp) {
  if (!fp->DeSerialize(shape_table_)) {
    return false;
  }
  num_fonts_ = 0;
  return true;
}

// Returns the number of fonts used in this ShapeTable, computing it if
// necessary.
int ShapeTable::NumFonts() const {
  if (num_fonts_ <= 0) {
    for (auto shape_id : shape_table_) {
      const Shape &shape = *shape_id;
      for (int c = 0; c < shape.size(); ++c) {
        for (int font_id : shape[c].font_ids) {
          if (font_id >= num_fonts_) {
            num_fonts_ = font_id + 1;
          }
        }
      }
    }
  }
  return num_fonts_;
}

// Re-indexes the class_ids in the shapetable according to the given map.
// Useful in conjunction with set_unicharset.
void ShapeTable::ReMapClassIds(const std::vector<int> &unicharset_map) {
  for (auto shape : shape_table_) {
    for (int c = 0; c < shape->size(); ++c) {
      shape->SetUnicharId(c, unicharset_map[(*shape)[c].unichar_id]);
    }
  }
}

// Returns a string listing the classes/fonts in a shape.
std::string ShapeTable::DebugStr(unsigned shape_id) const {
  if (shape_id >= shape_table_.size()) {
    return "INVALID_UNICHAR_ID";
  }
  const Shape &shape = GetShape(shape_id);
  std::string result;
  result += "Shape" + std::to_string(shape_id);
  if (shape.size() > 100) {
    result += " Num unichars=" + std::to_string(shape.size());
    return result;
  }
  for (int c = 0; c < shape.size(); ++c) {
    result += " c_id=" + std::to_string(shape[c].unichar_id);
    result += "=";
    result += unicharset_->id_to_unichar(shape[c].unichar_id);
    if (shape.size() < 10) {
      result += ", " + std::to_string(shape[c].font_ids.size());
      result += " fonts =";
      int num_fonts = shape[c].font_ids.size();
      if (num_fonts > 10) {
        result += " " + std::to_string(shape[c].font_ids[0]);
        result += " ... " + std::to_string(shape[c].font_ids[num_fonts - 1]);
      } else {
        for (int f = 0; f < num_fonts; ++f) {
          result += " " + std::to_string(shape[c].font_ids[f]);
        }
      }
    }
  }
  return result;
}

// Returns a debug string summarizing the table.
std::string ShapeTable::SummaryStr() const {
  int max_unichars = 0;
  int num_multi_shapes = 0;
  int num_master_shapes = 0;
  for (unsigned s = 0; s < shape_table_.size(); ++s) {
    if (MasterDestinationIndex(s) != s) {
      continue;
    }
    ++num_master_shapes;
    int shape_size = GetShape(s).size();
    if (shape_size > 1) {
      ++num_multi_shapes;
    }
    if (shape_size > max_unichars) {
      max_unichars = shape_size;
    }
  }
  std::string result;
  result += "Number of shapes = " + std::to_string(num_master_shapes);
  result += " max unichars = " + std::to_string(max_unichars);
  result += " number with multiple unichars = " + std::to_string(num_multi_shapes);
  return result;
}

// Adds a new shape starting with the given unichar_id and font_id.
// Returns the assigned index.
unsigned ShapeTable::AddShape(int unichar_id, int font_id) {
  auto index = shape_table_.size();
  auto *shape = new Shape;
  shape->AddToShape(unichar_id, font_id);
  shape_table_.push_back(shape);
  num_fonts_ = std::max(num_fonts_, font_id + 1);
  return index;
}

// Adds a copy of the given shape unless it is already present.
// Returns the assigned index or index of existing shape if already present.
unsigned ShapeTable::AddShape(const Shape &other) {
  unsigned index;
  for (index = 0; index < shape_table_.size() && !(other == *shape_table_[index]); ++index) {
    continue;
  }
  if (index == shape_table_.size()) {
    auto *shape = new Shape(other);
    shape_table_.push_back(shape);
  }
  num_fonts_ = 0;
  return index;
}

// Removes the shape given by the shape index.
void ShapeTable::DeleteShape(unsigned shape_id) {
  delete shape_table_[shape_id];
  shape_table_.erase(shape_table_.begin() + shape_id);
}

// Adds a font_id to the given existing shape index for the given
// unichar_id. If the unichar_id is not in the shape, it is added.
void ShapeTable::AddToShape(unsigned shape_id, int unichar_id, int font_id) {
  Shape &shape = *shape_table_[shape_id];
  shape.AddToShape(unichar_id, font_id);
  num_fonts_ = std::max(num_fonts_, font_id + 1);
}

// Adds the given shape to the existing shape with the given index.
void ShapeTable::AddShapeToShape(unsigned shape_id, const Shape &other) {
  Shape &shape = *shape_table_[shape_id];
  shape.AddShape(other);
  num_fonts_ = 0;
}

// Returns the id of the shape that contains the given unichar and font.
// If not found, returns -1.
// If font_id < 0, the font_id is ignored and the first shape that matches
// the unichar_id is returned.
int ShapeTable::FindShape(int unichar_id, int font_id) const {
  for (unsigned s = 0; s < shape_table_.size(); ++s) {
    const Shape &shape = GetShape(s);
    for (int c = 0; c < shape.size(); ++c) {
      if (shape[c].unichar_id == unichar_id) {
        if (font_id < 0) {
          return s; // We don't care about the font.
        }
        for (int f : shape[c].font_ids) {
          if (f == font_id) {
            return s;
          }
        }
      }
    }
  }
  return -1;
}

// Returns the first unichar_id and font_id in the given shape.
void ShapeTable::GetFirstUnicharAndFont(unsigned shape_id, int *unichar_id, int *font_id) const {
  const UnicharAndFonts &unichar_and_fonts = (*shape_table_[shape_id])[0];
  *unichar_id = unichar_and_fonts.unichar_id;
  *font_id = unichar_and_fonts.font_ids[0];
}

// Expands all the classes/fonts in the shape individually to build
// a ShapeTable.
int ShapeTable::BuildFromShape(const Shape &shape, const ShapeTable &master_shapes) {
  BitVector shape_map(master_shapes.NumShapes());
  for (int u_ind = 0; u_ind < shape.size(); ++u_ind) {
    for (unsigned f_ind = 0; f_ind < shape[u_ind].font_ids.size(); ++f_ind) {
      int c = shape[u_ind].unichar_id;
      int f = shape[u_ind].font_ids[f_ind];
      int master_id = master_shapes.FindShape(c, f);
      if (master_id >= 0) {
        shape_map.SetBit(master_id);
      } else if (FindShape(c, f) < 0) {
        AddShape(c, f);
      }
    }
  }
  int num_masters = 0;
  for (unsigned s = 0; s < master_shapes.NumShapes(); ++s) {
    if (shape_map[s]) {
      AddShape(master_shapes.GetShape(s));
      ++num_masters;
    }
  }
  return num_masters;
}

// Returns true if the shapes are already merged.
bool ShapeTable::AlreadyMerged(unsigned shape_id1, unsigned shape_id2) const {
  return MasterDestinationIndex(shape_id1) == MasterDestinationIndex(shape_id2);
}

// Returns true if any shape contains multiple unichars.
bool ShapeTable::AnyMultipleUnichars() const {
  auto num_shapes = NumShapes();
  for (unsigned s1 = 0; s1 < num_shapes; ++s1) {
    if (MasterDestinationIndex(s1) != s1) {
      continue;
    }
    if (GetShape(s1).size() > 1) {
      return true;
    }
  }
  return false;
}

// Returns the maximum number of unichars over all shapes.
int ShapeTable::MaxNumUnichars() const {
  int max_num_unichars = 0;
  int num_shapes = NumShapes();
  for (int s = 0; s < num_shapes; ++s) {
    if (GetShape(s).size() > max_num_unichars) {
      max_num_unichars = GetShape(s).size();
    }
  }
  return max_num_unichars;
}

// Merges shapes with a common unichar over the [start, end) interval.
// Assumes single unichar per shape.
void ShapeTable::ForceFontMerges(unsigned start, unsigned end) {
  for (unsigned s1 = start; s1 < end; ++s1) {
    if (MasterDestinationIndex(s1) == s1 && GetShape(s1).size() == 1) {
      int unichar_id = GetShape(s1)[0].unichar_id;
      for (auto s2 = s1 + 1; s2 < end; ++s2) {
        if (MasterDestinationIndex(s2) == s2 && GetShape(s2).size() == 1 &&
            unichar_id == GetShape(s2)[0].unichar_id) {
          MergeShapes(s1, s2);
        }
      }
    }
  }
  ShapeTable compacted(*unicharset_);
  compacted.AppendMasterShapes(*this, nullptr);
  *this = compacted;
}

// Returns the number of unichars in the master shape.
unsigned ShapeTable::MasterUnicharCount(unsigned shape_id) const {
  int master_id = MasterDestinationIndex(shape_id);
  return GetShape(master_id).size();
}

// Returns the sum of the font counts in the master shape.
int ShapeTable::MasterFontCount(unsigned shape_id) const {
  int master_id = MasterDestinationIndex(shape_id);
  const Shape &shape = GetShape(master_id);
  int font_count = 0;
  for (int c = 0; c < shape.size(); ++c) {
    font_count += shape[c].font_ids.size();
  }
  return font_count;
}

// Returns the number of unichars that would result from merging the shapes.
int ShapeTable::MergedUnicharCount(unsigned shape_id1, unsigned shape_id2) const {
  // Do it the easy way for now.
  int master_id1 = MasterDestinationIndex(shape_id1);
  int master_id2 = MasterDestinationIndex(shape_id2);
  Shape combined_shape(*shape_table_[master_id1]);
  combined_shape.AddShape(*shape_table_[master_id2]);
  return combined_shape.size();
}

// Merges two shape_ids, leaving shape_id2 marked as merged.
void ShapeTable::MergeShapes(unsigned shape_id1, unsigned shape_id2) {
  auto master_id1 = MasterDestinationIndex(shape_id1);
  auto master_id2 = MasterDestinationIndex(shape_id2);
  // Point master_id2 (and all merged shapes) to master_id1.
  shape_table_[master_id2]->set_destination_index(master_id1);
  // Add all the shapes of master_id2 to master_id1.
  shape_table_[master_id1]->AddShape(*shape_table_[master_id2]);
}

// Swaps two shape_ids.
void ShapeTable::SwapShapes(unsigned shape_id1, unsigned shape_id2) {
  Shape *tmp = shape_table_[shape_id1];
  shape_table_[shape_id1] = shape_table_[shape_id2];
  shape_table_[shape_id2] = tmp;
}

// Returns the destination of this shape, (if merged), taking into account
// the fact that the destination may itself have been merged.
unsigned ShapeTable::MasterDestinationIndex(unsigned shape_id) const {
  auto dest_id = shape_table_[shape_id]->destination_index();
  if (static_cast<unsigned>(dest_id) == shape_id || dest_id < 0) {
    return shape_id; // Is master already.
  }
  auto master_id = shape_table_[dest_id]->destination_index();
  if (master_id == dest_id || master_id < 0) {
    return dest_id; // Dest is the master and shape_id points to it.
  }
  master_id = MasterDestinationIndex(master_id);
  return master_id;
}

// Returns false if the unichars in neither shape is a subset of the other.
bool ShapeTable::SubsetUnichar(unsigned shape_id1, unsigned shape_id2) const {
  const Shape &shape1 = GetShape(shape_id1);
  const Shape &shape2 = GetShape(shape_id2);
  int c1, c2;
  for (c1 = 0; c1 < shape1.size(); ++c1) {
    int unichar_id1 = shape1[c1].unichar_id;
    if (!shape2.ContainsUnichar(unichar_id1)) {
      break;
    }
  }
  for (c2 = 0; c2 < shape2.size(); ++c2) {
    int unichar_id2 = shape2[c2].unichar_id;
    if (!shape1.ContainsUnichar(unichar_id2)) {
      break;
    }
  }
  return c1 == shape1.size() || c2 == shape2.size();
}

// Returns false if the unichars in neither shape is a subset of the other.
bool ShapeTable::MergeSubsetUnichar(int merge_id1, int merge_id2, unsigned shape_id) const {
  const Shape &merge1 = GetShape(merge_id1);
  const Shape &merge2 = GetShape(merge_id2);
  const Shape &shape = GetShape(shape_id);
  int cm1, cm2, cs;
  for (cs = 0; cs < shape.size(); ++cs) {
    int unichar_id = shape[cs].unichar_id;
    if (!merge1.ContainsUnichar(unichar_id) && !merge2.ContainsUnichar(unichar_id)) {
      break; // Shape is not a subset of the merge.
    }
  }
  for (cm1 = 0; cm1 < merge1.size(); ++cm1) {
    int unichar_id1 = merge1[cm1].unichar_id;
    if (!shape.ContainsUnichar(unichar_id1)) {
      break; // Merge is not a subset of shape
    }
  }
  for (cm2 = 0; cm2 < merge2.size(); ++cm2) {
    int unichar_id2 = merge2[cm2].unichar_id;
    if (!shape.ContainsUnichar(unichar_id2)) {
      break; // Merge is not a subset of shape
    }
  }
  return cs == shape.size() || (cm1 == merge1.size() && cm2 == merge2.size());
}

// Returns true if the unichar sets are equal between the shapes.
bool ShapeTable::EqualUnichars(unsigned shape_id1, unsigned shape_id2) const {
  const Shape &shape1 = GetShape(shape_id1);
  const Shape &shape2 = GetShape(shape_id2);
  for (int c1 = 0; c1 < shape1.size(); ++c1) {
    int unichar_id1 = shape1[c1].unichar_id;
    if (!shape2.ContainsUnichar(unichar_id1)) {
      return false;
    }
  }
  for (int c2 = 0; c2 < shape2.size(); ++c2) {
    int unichar_id2 = shape2[c2].unichar_id;
    if (!shape1.ContainsUnichar(unichar_id2)) {
      return false;
    }
  }
  return true;
}

// Returns true if the unichar sets are equal between the shapes.
bool ShapeTable::MergeEqualUnichars(int merge_id1, int merge_id2, unsigned shape_id) const {
  const Shape &merge1 = GetShape(merge_id1);
  const Shape &merge2 = GetShape(merge_id2);
  const Shape &shape = GetShape(shape_id);
  for (int cs = 0; cs < shape.size(); ++cs) {
    int unichar_id = shape[cs].unichar_id;
    if (!merge1.ContainsUnichar(unichar_id) && !merge2.ContainsUnichar(unichar_id)) {
      return false; // Shape has a unichar that appears in neither merge.
    }
  }
  for (int cm1 = 0; cm1 < merge1.size(); ++cm1) {
    int unichar_id1 = merge1[cm1].unichar_id;
    if (!shape.ContainsUnichar(unichar_id1)) {
      return false; // Merge has a unichar that is not in shape.
    }
  }
  for (int cm2 = 0; cm2 < merge2.size(); ++cm2) {
    int unichar_id2 = merge2[cm2].unichar_id;
    if (!shape.ContainsUnichar(unichar_id2)) {
      return false; // Merge has a unichar that is not in shape.
    }
  }
  return true;
}

// Returns true if there is a common unichar between the shapes.
bool ShapeTable::CommonUnichars(unsigned shape_id1, unsigned shape_id2) const {
  const Shape &shape1 = GetShape(shape_id1);
  const Shape &shape2 = GetShape(shape_id2);
  for (int c1 = 0; c1 < shape1.size(); ++c1) {
    int unichar_id1 = shape1[c1].unichar_id;
    if (shape2.ContainsUnichar(unichar_id1)) {
      return true;
    }
  }
  return false;
}

// Returns true if there is a common font id between the shapes.
bool ShapeTable::CommonFont(unsigned shape_id1, unsigned shape_id2) const {
  const Shape &shape1 = GetShape(shape_id1);
  const Shape &shape2 = GetShape(shape_id2);
  for (int c1 = 0; c1 < shape1.size(); ++c1) {
    const std::vector<int> &font_list1 = shape1[c1].font_ids;
    for (int f : font_list1) {
      if (shape2.ContainsFont(f)) {
        return true;
      }
    }
  }
  return false;
}

// Appends the master shapes from other to this.
// If not nullptr, shape_map is set to map other shape_ids to this's shape_ids.
void ShapeTable::AppendMasterShapes(const ShapeTable &other, std::vector<int> *shape_map) {
  if (shape_map != nullptr) {
    shape_map->clear();
    shape_map->resize(other.NumShapes(), -1);
  }
  for (unsigned s = 0; s < other.shape_table_.size(); ++s) {
    if (other.shape_table_[s]->destination_index() < 0) {
      int index = AddShape(*other.shape_table_[s]);
      if (shape_map != nullptr) {
        (*shape_map)[s] = index;
      }
    }
  }
}

// Returns the number of master shapes remaining after merging.
int ShapeTable::NumMasterShapes() const {
  int num_shapes = 0;
  for (auto s : shape_table_) {
    if (s->destination_index() < 0) {
      ++num_shapes;
    }
  }
  return num_shapes;
}

// Adds the unichars of the given shape_id to the vector of results. Any
// unichar_id that is already present just has the fonts added to the
// font set for that result without adding a new entry in the vector.
// NOTE: it is assumed that the results are given to this function in order
// of decreasing rating.
// The unichar_map vector indicates the index of the results entry containing
// each unichar, or -1 if the unichar is not yet included in results.
void ShapeTable::AddShapeToResults(const ShapeRating &shape_rating, std::vector<int> *unichar_map,
                                   std::vector<UnicharRating> *results) const {
  if (shape_rating.joined) {
    AddUnicharToResults(UNICHAR_JOINED, shape_rating.rating, unichar_map, results);
  }
  if (shape_rating.broken) {
    AddUnicharToResults(UNICHAR_BROKEN, shape_rating.rating, unichar_map, results);
  }
  const Shape &shape = GetShape(shape_rating.shape_id);
  for (int u = 0; u < shape.size(); ++u) {
    int result_index =
        AddUnicharToResults(shape[u].unichar_id, shape_rating.rating, unichar_map, results);
    for (int font_id : shape[u].font_ids) {
      (*results)[result_index].fonts.emplace_back(font_id,
                                                  IntCastRounded(shape_rating.rating * INT16_MAX));
    }
  }
}

// Adds the given unichar_id to the results if needed, updating unichar_map
// and returning the index of unichar in results.
int ShapeTable::AddUnicharToResults(int unichar_id, float rating, std::vector<int> *unichar_map,
                                    std::vector<UnicharRating> *results) const {
  int result_index = unichar_map->at(unichar_id);
  if (result_index < 0) {
    UnicharRating result(unichar_id, rating);
    result_index = results->size();
    results->push_back(result);
    (*unichar_map)[unichar_id] = result_index;
  }
  return result_index;
}

} // namespace tesseract
