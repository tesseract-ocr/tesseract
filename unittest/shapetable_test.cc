// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <utility>

#include "absl/strings/str_format.h"	// for absl::StrFormat

#include "include_gunit.h"

#include <tesseract/serialis.h>
#include "shapetable.h"
#include "unicharset.h"

namespace {

#ifndef DISABLED_LEGACY_ENGINE

  using tesseract::Shape;
using tesseract::ShapeTable;
using tesseract::TFile;
using tesseract::UnicharAndFonts;

static std::string TmpNameToPath(const std::string& name) {
  return file::JoinPath(FLAGS_test_tmpdir, name);
}

// Sets up a simple shape with some unichars.
static void Setup352(int font_id, Shape* shape) {
  shape->AddToShape(3, font_id);
  shape->AddToShape(5, font_id);
  shape->AddToShape(2, font_id);
}

// Verifies some properties of the 352 shape.
static void Expect352(int font_id, const Shape& shape) {
  EXPECT_EQ(3, shape.size());
  EXPECT_TRUE(shape.ContainsUnichar(2));
  EXPECT_TRUE(shape.ContainsUnichar(3));
  EXPECT_TRUE(shape.ContainsUnichar(5));
  EXPECT_FALSE(shape.ContainsUnichar(1));
  EXPECT_TRUE(shape.ContainsUnicharAndFont(2, font_id));
  EXPECT_FALSE(shape.ContainsUnicharAndFont(2, font_id - 1));
  EXPECT_FALSE(shape.ContainsUnicharAndFont(font_id, 2));
  // It should be a subset of itself.
  EXPECT_TRUE(shape.IsSubsetOf(shape));
}

#endif

// The fixture for testing Shape.
class ShapeTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }
};

// Tests that a Shape works as expected for all the basic functions.
TEST_F(ShapeTest, BasicTest) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because Shape is missing.
  GTEST_SKIP();
#else
  Shape shape1;
  EXPECT_EQ(0, shape1.size());
  Setup352(101, &shape1);
  Expect352(101, shape1);
  // It should still work after file I/O.
  std::string filename = TmpNameToPath("shapefile");
  FILE* fp = fopen(filename.c_str(), "wb");
  EXPECT_TRUE(fp != nullptr);
  EXPECT_TRUE(shape1.Serialize(fp));
  fclose(fp);
  TFile tfp;
  EXPECT_TRUE(tfp.Open(filename.c_str(), nullptr));
  Shape shape2;
  EXPECT_TRUE(shape2.DeSerialize(&tfp));
  Expect352(101, shape2);
  // They should be subsets of each other.
  EXPECT_TRUE(shape1.IsSubsetOf(shape2));
  EXPECT_TRUE(shape2.IsSubsetOf(shape1));
  // They should be equal unichars.
  EXPECT_TRUE(shape1.IsEqualUnichars(&shape2));
  // and still pass afterwards.
  Expect352(101, shape1);
  Expect352(101, shape2);
#endif
}

// Tests AddShape separately, as it takes quite a bit of work.
TEST_F(ShapeTest, AddShapeTest) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because Shape is missing.
  GTEST_SKIP();
#else
  Shape shape1;
  Setup352(101, &shape1);
  Expect352(101, shape1);
  // Now setup a different shape with different content.
  Shape shape2;
  shape2.AddToShape(3, 101);  // Duplicates shape1.
  shape2.AddToShape(5, 110);  // Different font to shape1.
  shape2.AddToShape(7, 101);  // Different unichar to shape1.
  // They should NOT be subsets of each other.
  EXPECT_FALSE(shape1.IsSubsetOf(shape2));
  EXPECT_FALSE(shape2.IsSubsetOf(shape1));
  // Now add shape2 to shape1.
  shape1.AddShape(shape2);
  // Test subsets again.
  EXPECT_FALSE(shape1.IsSubsetOf(shape2));
  EXPECT_TRUE(shape2.IsSubsetOf(shape1));
  EXPECT_EQ(4, shape1.size());
  EXPECT_FALSE(shape1.ContainsUnichar(1));
  EXPECT_TRUE(shape1.ContainsUnicharAndFont(5, 101));
  EXPECT_TRUE(shape1.ContainsUnicharAndFont(5, 110));
  EXPECT_FALSE(shape1.ContainsUnicharAndFont(3, 110));
  EXPECT_FALSE(shape1.ContainsUnicharAndFont(7, 110));
  EXPECT_FALSE(shape1.IsEqualUnichars(&shape2));
#endif
}

// The fixture for testing Shape.
class ShapeTableTest : public testing::Test {};

// Tests that a Shape works as expected for all the basic functions.
TEST_F(ShapeTableTest, FullTest) {
#ifdef DISABLED_LEGACY_ENGINE
  // Skip test because Shape is missing.
  GTEST_SKIP();
#else
  Shape shape1;
  Setup352(101, &shape1);
  // Build a shape table with the same data, but in separate shapes.
  UNICHARSET unicharset;
  unicharset.unichar_insert(" ");
  for (int i = 1; i <= 10; ++i) {
    std::string class_str = absl::StrFormat("class%d", i);
    unicharset.unichar_insert(class_str.c_str());
  }
  ShapeTable st(unicharset);
  EXPECT_EQ(0, st.AddShape(3, 101));
  EXPECT_EQ(1, st.AddShape(5, 101));
  EXPECT_EQ(2, st.AddShape(2, 101));
  EXPECT_EQ(3, st.NumShapes());
  Expect352(101, shape1);
  EXPECT_EQ(3, st.AddShape(shape1));
  for (int i = 0; i < 3; ++i) {
    EXPECT_FALSE(st.MutableShape(i)->IsEqualUnichars(&shape1));
  }
  EXPECT_TRUE(st.MutableShape(3)->IsEqualUnichars(&shape1));
  EXPECT_TRUE(st.AnyMultipleUnichars());
  st.DeleteShape(3);
  EXPECT_FALSE(st.AnyMultipleUnichars());

  // Now merge to make a single shape like shape1.
  EXPECT_EQ(1, st.MasterUnicharCount(0));
  st.MergeShapes(0, 1);
  EXPECT_EQ(3, st.MergedUnicharCount(1, 2));
  st.MergeShapes(1, 2);
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(3, st.MasterUnicharCount(i));
    // Master font count is the sum of all the font counts in the shape, not
    // the actual number of different fonts in the shape.
    EXPECT_EQ(3, st.MasterFontCount(i));
  }
  EXPECT_EQ(0, st.MasterDestinationIndex(1));
  EXPECT_EQ(0, st.MasterDestinationIndex(2));
  ShapeTable st2;
  st2.AppendMasterShapes(st, nullptr);
  EXPECT_EQ(1, st.NumMasterShapes());
  EXPECT_EQ(1, st2.NumShapes());
  EXPECT_TRUE(st2.MutableShape(0)->IsEqualUnichars(&shape1));
  EXPECT_TRUE(st2.AnyMultipleUnichars());
#endif
}

}  // namespace
