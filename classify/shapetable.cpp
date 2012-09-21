// Copyright 2010 Google Inc. All Rights Reserved.
// Author: rays@google.com (Ray Smith)
///////////////////////////////////////////////////////////////////////
// File:        shapetable.cpp
// Description: Class to map a classifier shape index to unicharset
//              indices and font indices.
// Author:      Ray Smith
// Created:     Tue Nov 02 15:31:32 PDT 2010
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

#include "intfeaturespace.h"
#include "strngs.h"
#include "unicharset.h"

namespace tesseract {

// Writes to the given file. Returns false in case of error.
bool UnicharAndFonts::Serialize(FILE* fp) const {
  if (fwrite(&unichar_id, sizeof(unichar_id), 1, fp) != 1) return false;
  if (!font_ids.Serialize(fp)) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool UnicharAndFonts::DeSerialize(bool swap, FILE* fp) {
  if (fread(&unichar_id, sizeof(unichar_id), 1, fp) != 1) return false;
  if (swap)
    ReverseN(&unichar_id, sizeof(unichar_id));
  if (!font_ids.DeSerialize(swap, fp)) return false;
  return true;
}

// Sort function to sort a pair of UnicharAndFonts by unichar_id.
int UnicharAndFonts::SortByUnicharId(const void* v1, const void* v2) {
  const UnicharAndFonts* p1 = reinterpret_cast<const UnicharAndFonts*>(v1);
  const UnicharAndFonts* p2 = reinterpret_cast<const UnicharAndFonts*>(v2);
  return p1->unichar_id - p2->unichar_id;
}

// Writes to the given file. Returns false in case of error.
bool Shape::Serialize(FILE* fp) const {
  uinT8 sorted = unichars_sorted_;
  if (fwrite(&sorted, sizeof(sorted), 1, fp) != 1)
    return false;
  if (!unichars_.SerializeClasses(fp)) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool Shape::DeSerialize(bool swap, FILE* fp) {
  uinT8 sorted;
  if (fread(&sorted, sizeof(sorted), 1, fp) != 1)
    return false;
  unichars_sorted_ = sorted != 0;
  if (!unichars_.DeSerializeClasses(swap, fp)) return false;
  return true;
}

// Adds a font_id for the given unichar_id. If the unichar_id is not
// in the shape, it is added.
void Shape::AddToShape(int unichar_id, int font_id) {
  for (int c = 0; c < unichars_.size(); ++c) {
    if (unichars_[c].unichar_id == unichar_id) {
      // Found the unichar in the shape table.
      GenericVector<int>& font_list = unichars_[c].font_ids;
      for (int f = 0; f < font_list.size(); ++f) {
        if (font_list[f] == font_id)
          return;  // Font is already there.
      }
      font_list.push_back(font_id);
      return;
    }
  }
  // Unichar_id is not in shape, so add it to shape.
  unichars_.push_back(UnicharAndFonts(unichar_id, font_id));
  unichars_sorted_ =  unichars_.size() <= 1;
}

// Adds everything in other to this.
void Shape::AddShape(const Shape& other) {
  for (int c = 0; c < other.unichars_.size(); ++c) {
    for (int f = 0; f < other.unichars_[c].font_ids.size(); ++f) {
      AddToShape(other.unichars_[c].unichar_id,
                 other.unichars_[c].font_ids[f]);
    }
  }
  unichars_sorted_ =  unichars_.size() <= 1;
}

// Returns true if the shape contains the given unichar_id, font_id pair.
bool Shape::ContainsUnicharAndFont(int unichar_id, int font_id) const {
  for (int c = 0; c < unichars_.size(); ++c) {
    if (unichars_[c].unichar_id == unichar_id) {
      // Found the unichar, so look for the font.
      GenericVector<int>& font_list = unichars_[c].font_ids;
      for (int f = 0; f < font_list.size(); ++f) {
        if (font_list[f] == font_id)
          return true;
      }
      return false;
    }
  }
  return false;
}

// Returns true if the shape contains the given unichar_id, ignoring font.
bool Shape::ContainsUnichar(int unichar_id) const {
  for (int c = 0; c < unichars_.size(); ++c) {
    if (unichars_[c].unichar_id == unichar_id) {
      return true;
    }
  }
  return false;
}

// Returns true if the shape contains the given font, ignoring unichar_id.
bool Shape::ContainsFont(int font_id) const {
  for (int c = 0; c < unichars_.size(); ++c) {
    GenericVector<int>& font_list = unichars_[c].font_ids;
    for (int f = 0; f < font_list.size(); ++f) {
      if (font_list[f] == font_id)
        return true;
    }
  }
  return false;
}

// Returns true if this is a subset (including equal) of other.
bool Shape::IsSubsetOf(const Shape& other) const {
  for (int c = 0; c < unichars_.size(); ++c) {
    int unichar_id = unichars_[c].unichar_id;
    const GenericVector<int>& font_list = unichars_[c].font_ids;
    for (int f = 0; f < font_list.size(); ++f) {
      if (!other.ContainsUnicharAndFont(unichar_id, font_list[f]))
        return false;
    }
  }
  return true;
}

// Returns true if the lists of unichar ids are the same in this and other,
// ignoring fonts.
// NOT const, as it will sort the unichars on demand.
bool Shape::IsEqualUnichars(Shape* other) {
  if (unichars_.size() != other->unichars_.size()) return false;
  if (!unichars_sorted_) SortUnichars();
  if (!other->unichars_sorted_) other->SortUnichars();
  for (int c = 0; c < unichars_.size(); ++c) {
    if (unichars_[c].unichar_id != other->unichars_[c].unichar_id)
      return false;
  }
  return true;
}

// Sorts the unichars_ vector by unichar.
void Shape::SortUnichars() {
  unichars_.sort(UnicharAndFonts::SortByUnicharId);
  unichars_sorted_ = true;
}

ShapeTable::ShapeTable() : unicharset_(NULL) {
}
ShapeTable::ShapeTable(const UNICHARSET& unicharset)
  : unicharset_(&unicharset) {
}

// Writes to the given file. Returns false in case of error.
bool ShapeTable::Serialize(FILE* fp) const {
  if (!shape_table_.Serialize(fp)) return false;
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool ShapeTable::DeSerialize(bool swap, FILE* fp) {
  if (!shape_table_.DeSerialize(swap, fp)) return false;
  return true;
}

// Returns a string listing the classes/fonts in a shape.
STRING ShapeTable::DebugStr(int shape_id) const {
  if (shape_id < 0 || shape_id >= shape_table_.size())
    return STRING("INVALID_UNICHAR_ID");
  const Shape& shape = GetShape(shape_id);
  STRING result;
  result.add_str_int("Shape", shape_id);
  if (shape.size() > 100) {
    result.add_str_int(" Num unichars=", shape.size());
    return result;
  }
  for (int c = 0; c < shape.size(); ++c) {
    result.add_str_int(" c_id=", shape[c].unichar_id);
    result += "=";
    result += unicharset_->id_to_unichar(shape[c].unichar_id);
    if (shape.size() < 10) {
      result.add_str_int(", ", shape[c].font_ids.size());
      result += " fonts =";
      int num_fonts = shape[c].font_ids.size();
      if (num_fonts > 10) {
        result.add_str_int(" ", shape[c].font_ids[0]);
        result.add_str_int(" ... ", shape[c].font_ids[num_fonts - 1]);
      } else {
        for (int f = 0; f < num_fonts; ++f) {
          result.add_str_int(" ", shape[c].font_ids[f]);
        }
      }
    }
  }
  return result;
}

// Returns a debug string summarizing the table.
STRING ShapeTable::SummaryStr() const {
  int max_unichars = 0;
  int num_multi_shapes = 0;
  int num_master_shapes = 0;
  for (int s = 0; s < shape_table_.size(); ++s) {
    if (MasterDestinationIndex(s) != s) continue;
    ++num_master_shapes;
    int shape_size = GetShape(s).size();
    if (shape_size > 1)
      ++num_multi_shapes;
    if (shape_size > max_unichars)
      max_unichars = shape_size;
  }
  STRING result;
  result.add_str_int("Number of shapes = ", num_master_shapes);
  result.add_str_int(" max unichars = ", max_unichars);
  result.add_str_int(" number with multiple unichars = ", num_multi_shapes);
  return result;
}


// Adds a new shape starting with the given unichar_id and font_id.
// Returns the assigned index.
int ShapeTable::AddShape(int unichar_id, int font_id) {
  int index = shape_table_.size();
  Shape* shape = new Shape;
  shape->AddToShape(unichar_id, font_id);
  shape_table_.push_back(shape);
  return index;
}

// Adds a copy of the given shape.
// Returns the assigned index.
int ShapeTable::AddShape(const Shape& other) {
  int index = shape_table_.size();
  Shape* shape = new Shape(other);
  shape_table_.push_back(shape);
  return index;
}

// Removes the shape given by the shape index.
void ShapeTable::DeleteShape(int shape_id) {
  delete shape_table_[shape_id];
  shape_table_[shape_id] = NULL;
  shape_table_.remove(shape_id);
}

// Adds a font_id to the given existing shape index for the given
// unichar_id. If the unichar_id is not in the shape, it is added.
void ShapeTable::AddToShape(int shape_id, int unichar_id, int font_id) {
  Shape& shape = *shape_table_[shape_id];
  shape.AddToShape(unichar_id, font_id);
}

// Adds the given shape to the existing shape with the given index.
void ShapeTable::AddShapeToShape(int shape_id, const Shape& other) {
  Shape& shape = *shape_table_[shape_id];
  shape.AddShape(other);
}

// Returns the id of the shape that contains the given unichar and font.
// If not found, returns -1.
// If font_id < 0, the font_id is ignored and the first shape that matches
// the unichar_id is returned.
int ShapeTable::FindShape(int unichar_id, int font_id) const {
  for (int s = 0; s < shape_table_.size(); ++s) {
    const Shape& shape = GetShape(s);
    for (int c = 0; c < shape.size(); ++c) {
      if (shape[c].unichar_id == unichar_id) {
        if (font_id < 0)
          return s;  // We don't care about the font.
        for (int f = 0; f < shape[c].font_ids.size(); ++f) {
          if (shape[c].font_ids[f] == font_id)
            return s;
        }
      }
    }
  }
  return -1;
}

// Returns the first unichar_id and font_id in the given shape.
void ShapeTable::GetFirstUnicharAndFont(int shape_id,
                                        int* unichar_id, int* font_id) const {
  const UnicharAndFonts& unichar_and_fonts = (*shape_table_[shape_id])[0];
  *unichar_id = unichar_and_fonts.unichar_id;
  *font_id = unichar_and_fonts.font_ids[0];
}

// Expands all the classes/fonts in the shape individually to build
// a ShapeTable.
int ShapeTable::BuildFromShape(const Shape& shape,
                               const ShapeTable& master_shapes) {
  int num_masters = 0;
  for (int u_ind = 0; u_ind < shape.size(); ++u_ind) {
    for (int f_ind = 0; f_ind < shape[u_ind].font_ids.size(); ++f_ind) {
      int c = shape[u_ind].unichar_id;
      int f = shape[u_ind].font_ids[f_ind];
      if (FindShape(c, f) < 0) {
        int shape_id = AddShape(c, f);
        int master_id = master_shapes.FindShape(c, f);
        if (master_id >= 0 && shape.size() > 1) {
          const Shape& master = master_shapes.GetShape(master_id);
          if (master.IsSubsetOf(shape) && !shape.IsSubsetOf(master)) {
            // Add everything else from the master shape.
            shape_table_[shape_id]->AddShape(master);
            ++num_masters;
          }
        }
      }
    }
  }
  return num_masters;
}

// Returns true if the shapes are already merged.
bool ShapeTable::AlreadyMerged(int shape_id1, int shape_id2) const {
  return MasterDestinationIndex(shape_id1) == MasterDestinationIndex(shape_id2);
}

// Returns true if any shape contains multiple unichars.
bool ShapeTable::AnyMultipleUnichars() const {
  int num_shapes = NumShapes();
  for (int s1 = 0; s1 < num_shapes; ++s1) {
    if (MasterDestinationIndex(s1) != s1) continue;
    if (GetShape(s1).size() > 1)
      return true;
  }
  return false;
}

// Returns the maximum number of unichars over all shapes.
int ShapeTable::MaxNumUnichars() const {
  int max_num_unichars = 0;
  int num_shapes = NumShapes();
  for (int s = 0; s < num_shapes; ++s) {
    if (GetShape(s).size() > max_num_unichars)
      max_num_unichars = GetShape(s).size();
  }
  return max_num_unichars;
}


// Merges shapes with a common unichar over the [start, end) interval.
// Assumes single unichar per shape.
void ShapeTable::ForceFontMerges(int start, int end) {
  for (int s1 = start; s1 < end; ++s1) {
    if (MasterDestinationIndex(s1) == s1 && GetShape(s1).size() == 1) {
      int unichar_id = GetShape(s1)[0].unichar_id;
      for (int s2 = s1 + 1; s2 < end; ++s2) {
        if (MasterDestinationIndex(s2) == s2 && GetShape(s2).size() == 1 &&
            unichar_id == GetShape(s2)[0].unichar_id) {
          MergeShapes(s1, s2);
        }
      }
    }
  }
  ShapeTable compacted(*unicharset_);
  compacted.AppendMasterShapes(*this);
  *this = compacted;
}

// Returns the number of unichars in the master shape.
int ShapeTable::MasterUnicharCount(int shape_id) const {
  int master_id = MasterDestinationIndex(shape_id);
  return GetShape(master_id).size();
}

// Returns the sum of the font counts in the master shape.
int ShapeTable::MasterFontCount(int shape_id) const {
  int master_id = MasterDestinationIndex(shape_id);
  const Shape& shape = GetShape(master_id);
  int font_count = 0;
  for (int c = 0; c < shape.size(); ++c) {
    font_count += shape[c].font_ids.size();
  }
  return font_count;
}

// Returns the number of unichars that would result from merging the shapes.
int ShapeTable::MergedUnicharCount(int shape_id1, int shape_id2) const {
  // Do it the easy way for now.
  int master_id1 = MasterDestinationIndex(shape_id1);
  int master_id2 = MasterDestinationIndex(shape_id2);
  Shape combined_shape(*shape_table_[master_id1]);
  combined_shape.AddShape(*shape_table_[master_id2]);
  return combined_shape.size();
}

// Merges two shape_ids, leaving shape_id2 marked as merged.
void ShapeTable::MergeShapes(int shape_id1, int shape_id2) {
  int master_id1 = MasterDestinationIndex(shape_id1);
  int master_id2 = MasterDestinationIndex(shape_id2);
  // Point master_id2 (and all merged shapes) to master_id1.
  shape_table_[master_id2]->set_destination_index(master_id1);
  // Add all the shapes of master_id2 to master_id1.
  shape_table_[master_id1]->AddShape(*shape_table_[master_id2]);
}

// Returns the destination of this shape, (if merged), taking into account
// the fact that the destination may itself have been merged.
int ShapeTable::MasterDestinationIndex(int shape_id) const {
  int dest_id = shape_table_[shape_id]->destination_index();
  if (dest_id == shape_id || dest_id < 0)
    return shape_id;  // Is master already.
  int master_id = shape_table_[dest_id]->destination_index();
  if (master_id == dest_id || master_id < 0)
    return dest_id;  // Dest is the master and shape_id points to it.
  master_id = MasterDestinationIndex(master_id);
  return master_id;
}

// Appends the master shapes from other to this.
void ShapeTable::AppendMasterShapes(const ShapeTable& other) {
  for (int s = 0; s < other.shape_table_.size(); ++s) {
    if (other.shape_table_[s]->destination_index() < 0) {
      AddShape(*other.shape_table_[s]);
    }
  }
}

// Returns the number of master shapes remaining after merging.
int ShapeTable::NumMasterShapes() const {
  int num_shapes = 0;
  for (int s = 0; s < shape_table_.size(); ++s) {
    if (shape_table_[s]->destination_index() < 0)
      ++num_shapes;
  }
  return num_shapes;
}


}  // namespace tesseract


