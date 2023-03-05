///////////////////////////////////////////////////////////////////////
// File:        fontinfo.cpp
// Description: Font information classes abstracted from intproto.h/cpp.
// Author:      rays@google.com (Ray Smith)
//
// (C) Copyright 2011, Google Inc.
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

#include "fontinfo.h"
#include "bitvector.h"
#include "unicity_table.h"

namespace tesseract {

// Writes to the given file. Returns false in case of error.
bool FontInfo::Serialize(FILE *fp) const {
  if (!write_info(fp, *this)) {
    return false;
  }
  if (!write_spacing_info(fp, *this)) {
    return false;
  }
  return true;
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool FontInfo::DeSerialize(TFile *fp) {
  if (!read_info(fp, this)) {
    return false;
  }
  if (!read_spacing_info(fp, this)) {
    return false;
  }
  return true;
}

FontInfoTable::FontInfoTable() {
  using namespace std::placeholders; // for _1, _2
  set_clear_callback(std::bind(FontInfoDeleteCallback, _1));
}

FontInfoTable::~FontInfoTable() = default;

// Writes to the given file. Returns false in case of error.
bool FontInfoTable::Serialize(FILE *fp) const {
  return this->SerializeClasses(fp);
}
// Reads from the given file. Returns false in case of error.
// If swap is true, assumes a big/little-endian swap is needed.
bool FontInfoTable::DeSerialize(TFile *fp) {
  truncate(0);
  return this->DeSerializeClasses(fp);
}

// Returns true if the given set of fonts includes one with the same
// properties as font_id.
bool FontInfoTable::SetContainsFontProperties(int font_id,
                                              const std::vector<ScoredFont> &font_set) const {
  uint32_t properties = at(font_id).properties;
  for (auto &&f : font_set) {
    if (at(f.fontinfo_id).properties == properties) {
      return true;
    }
  }
  return false;
}

// Returns true if the given set of fonts includes multiple properties.
bool FontInfoTable::SetContainsMultipleFontProperties(
    const std::vector<ScoredFont> &font_set) const {
  if (font_set.empty()) {
    return false;
  }
  int first_font = font_set[0].fontinfo_id;
  uint32_t properties = at(first_font).properties;
  for (unsigned f = 1; f < font_set.size(); ++f) {
    if (at(font_set[f].fontinfo_id).properties != properties) {
      return true;
    }
  }
  return false;
}

// Moves any non-empty FontSpacingInfo entries from other to this.
void FontInfoTable::MoveSpacingInfoFrom(FontInfoTable *other) {
  using namespace std::placeholders; // for _1, _2
  set_clear_callback(std::bind(FontInfoDeleteCallback, _1));
  for (unsigned i = 0; i < other->size(); ++i) {
    std::vector<FontSpacingInfo *> *spacing_vec = other->at(i).spacing_vec;
    if (spacing_vec != nullptr) {
      int target_index = get_index(other->at(i));
      if (target_index < 0) {
        // Bit copy the FontInfo and steal all the pointers.
        push_back(other->at(i));
        other->at(i).name = nullptr;
      } else {
        delete at(target_index).spacing_vec;
        at(target_index).spacing_vec = other->at(i).spacing_vec;
      }
      other->at(i).spacing_vec = nullptr;
    }
  }
}

// Moves this to the target unicity table.
void FontInfoTable::MoveTo(UnicityTable<FontInfo> *target) {
  target->clear();
  using namespace std::placeholders; // for _1, _2
  target->set_clear_callback(std::bind(FontInfoDeleteCallback, _1));
  for (unsigned i = 0; i < size(); ++i) {
    // Bit copy the FontInfo and steal all the pointers.
    target->push_back(at(i));
    at(i).name = nullptr;
    at(i).spacing_vec = nullptr;
  }
}

// Callbacks for GenericVector.
void FontInfoDeleteCallback(FontInfo f) {
  if (f.spacing_vec != nullptr) {
    for (auto data : *f.spacing_vec) {
      delete data;
    }
    delete f.spacing_vec;
    f.spacing_vec = nullptr;
  }
  delete[] f.name;
  f.name = nullptr;
}

/*---------------------------------------------------------------------------*/
// Callbacks used by UnicityTable to read/write FontInfo/FontSet structures.
bool read_info(TFile *f, FontInfo *fi) {
  uint32_t size;
  if (!f->DeSerialize(&size)) {
    return false;
  }
  char *font_name = new char[size + 1];
  fi->name = font_name;
  if (!f->DeSerialize(font_name, size)) {
    return false;
  }
  font_name[size] = '\0';
  return f->DeSerialize(&fi->properties);
}

bool write_info(FILE *f, const FontInfo &fi) {
  int32_t size = strlen(fi.name);
  return tesseract::Serialize(f, &size) && tesseract::Serialize(f, &fi.name[0], size) &&
         tesseract::Serialize(f, &fi.properties);
}

bool read_spacing_info(TFile *f, FontInfo *fi) {
  int32_t vec_size, kern_size;
  if (!f->DeSerialize(&vec_size)) {
    return false;
  }
  ASSERT_HOST(vec_size >= 0);
  if (vec_size == 0) {
    return true;
  }
  fi->init_spacing(vec_size);
  for (int i = 0; i < vec_size; ++i) {
    auto *fs = new FontSpacingInfo();
    if (!f->DeSerialize(&fs->x_gap_before) || !f->DeSerialize(&fs->x_gap_after) ||
        !f->DeSerialize(&kern_size)) {
      delete fs;
      return false;
    }
    if (kern_size < 0) { // indication of a nullptr entry in fi->spacing_vec
      delete fs;
      continue;
    }
    if (kern_size > 0 &&
        (!f->DeSerialize(fs->kerned_unichar_ids) || !f->DeSerialize(fs->kerned_x_gaps))) {
      delete fs;
      return false;
    }
    fi->add_spacing(i, fs);
  }
  return true;
}

bool write_spacing_info(FILE *f, const FontInfo &fi) {
  int32_t vec_size = (fi.spacing_vec == nullptr) ? 0 : fi.spacing_vec->size();
  if (!tesseract::Serialize(f, &vec_size)) {
    return false;
  }
  int16_t x_gap_invalid = -1;
  for (int i = 0; i < vec_size; ++i) {
    FontSpacingInfo *fs = fi.spacing_vec->at(i);
    int32_t kern_size = (fs == nullptr) ? -1 : fs->kerned_x_gaps.size();
    if (fs == nullptr) {
      // Writing two invalid x-gaps.
      if (!tesseract::Serialize(f, &x_gap_invalid, 2) || !tesseract::Serialize(f, &kern_size)) {
        return false;
      }
    } else {
      if (!tesseract::Serialize(f, &fs->x_gap_before) ||
          !tesseract::Serialize(f, &fs->x_gap_after) || !tesseract::Serialize(f, &kern_size)) {
        return false;
      }
    }
    if (kern_size > 0 &&
        (!Serialize(f, fs->kerned_unichar_ids) || !Serialize(f, fs->kerned_x_gaps))) {
      return false;
    }
  }
  return true;
}

bool write_set(FILE *f, const FontSet &fs) {
  int size = fs.size();
  return tesseract::Serialize(f, &size) &&
         (size > 0 ? tesseract::Serialize(f, &fs[0], size) : true);
}

} // namespace tesseract.
