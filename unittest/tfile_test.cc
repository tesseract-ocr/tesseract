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

#include "serialis.h"

#include "include_gunit.h"

namespace tesseract {

// Tests TFile and std::vector serialization by serializing and
// writing/reading.

class TfileTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
  }

  TfileTest() = default;

  // Some data to serialize.
  class MathData {
  public:
    MathData() : num_squares_(0), num_triangles_(0) {}
    void Setup() {
      // Setup some data.
      for (int s = 0; s < 42; ++s) {
        squares_.push_back(s * s);
      }
      num_squares_ = squares_.size();
      for (int t = 0; t < 52; ++t) {
        triangles_.push_back(t * (t + 1) / 2);
      }
      num_triangles_ = triangles_.size();
    }
    void ExpectEq(const MathData &other) {
      // Check the data.
      EXPECT_EQ(num_squares_, other.num_squares_);
      for (unsigned s = 0; s < squares_.size(); ++s) {
        EXPECT_EQ(squares_[s], other.squares_[s]);
      }
      EXPECT_EQ(num_triangles_, other.num_triangles_);
      for (unsigned s = 0; s < triangles_.size(); ++s) {
        EXPECT_EQ(triangles_[s], other.triangles_[s]);
      }
    }
    bool Serialize(TFile *fp) {
      if (fp->FWrite(&num_squares_, sizeof(num_squares_), 1) != 1) {
        return false;
      }
      if (!fp->Serialize(squares_)) {
        return false;
      }
      if (fp->FWrite(&num_triangles_, sizeof(num_triangles_), 1) != 1) {
        return false;
      }
      if (!fp->Serialize(triangles_)) {
        return false;
      }
      return true;
    }
    bool DeSerialize(TFile *fp) {
      if (fp->FReadEndian(&num_squares_, sizeof(num_squares_), 1) != 1) {
        return false;
      }
      if (!fp->DeSerialize(squares_)) {
        return false;
      }
      if (fp->FReadEndian(&num_triangles_, sizeof(num_triangles_), 1) != 1) {
        return false;
      }
      if (!fp->DeSerialize(triangles_)) {
        return false;
      }
      return true;
    }
    bool SerializeBigEndian(TFile *fp) {
      ReverseN(&num_squares_, sizeof(num_squares_));
      if (fp->FWrite(&num_squares_, sizeof(num_squares_), 1) != 1) {
        return false;
      }
      // Write an additional reversed size before the vector, which will get
      // used as its size on reading.
      if (fp->FWrite(&num_squares_, sizeof(num_squares_), 1) != 1) {
        return false;
      }
      for (int &square : squares_) {
        ReverseN(&square, sizeof(square));
      }
      if (!fp->Serialize(squares_)) {
        return false;
      }
      ReverseN(&num_triangles_, sizeof(num_triangles_));
      if (fp->FWrite(&num_triangles_, sizeof(num_triangles_), 1) != 1) {
        return false;
      }
      if (fp->FWrite(&num_triangles_, sizeof(num_triangles_), 1) != 1) {
        return false;
      }
      for (auto &triangle : triangles_) {
        ReverseN(&triangle, sizeof(triangles_[0]));
      }
      return fp->Serialize(triangles_);
    }
    bool DeSerializeBigEndian(TFile *fp) {
      if (fp->FReadEndian(&num_squares_, sizeof(num_squares_), 1) != 1) {
        return false;
      }
      if (!fp->DeSerialize(squares_)) {
        return false;
      }
      // The first element is the size that was written, so we will delete it
      // and read the last element separately.
      int last_element;
      if (fp->FReadEndian(&last_element, sizeof(last_element), 1) != 1) {
        return false;
      }
      squares_.erase(squares_.begin());
      squares_.push_back(last_element);
      if (fp->FReadEndian(&num_triangles_, sizeof(num_triangles_), 1) != 1) {
        return false;
      }
      if (!fp->DeSerialize(triangles_)) {
        return false;
      }
      if (fp->FReadEndian(&last_element, sizeof(last_element), 1) != 1) {
        return false;
      }
      triangles_.erase(triangles_.begin());
      triangles_.push_back(last_element);
      return true;
    }

  private:
    std::vector<int> squares_;
    int num_squares_;
    std::vector<int> triangles_;
    int num_triangles_;
  };
};

TEST_F(TfileTest, Serialize) {
  // This test verifies that Tfile can serialize a class.
  MathData m1;
  m1.Setup();
  std::vector<char> data;
  TFile fpw;
  fpw.OpenWrite(&data);
  EXPECT_TRUE(m1.Serialize(&fpw));
  TFile fpr;
  EXPECT_TRUE(fpr.Open(&data[0], data.size()));
  MathData m2;
  EXPECT_TRUE(m2.DeSerialize(&fpr));
  m1.ExpectEq(m2);
  MathData m3;
  EXPECT_FALSE(m3.DeSerialize(&fpr));
  fpr.Rewind();
  EXPECT_TRUE(m3.DeSerialize(&fpr));
  m1.ExpectEq(m3);
}

TEST_F(TfileTest, FGets) {
  // This test verifies that Tfile can interleave FGets with binary data.
  MathData m1;
  std::string line_str = "This is a textline with a newline\n";
  m1.Setup();
  std::vector<char> data;
  TFile fpw;
  fpw.OpenWrite(&data);
  EXPECT_TRUE(m1.Serialize(&fpw));
  EXPECT_EQ(1, fpw.FWrite(line_str.data(), line_str.size(), 1));
  EXPECT_TRUE(m1.Serialize(&fpw));
  // Now get back the 2 copies of m1 with the line in between.
  TFile fpr;
  EXPECT_TRUE(fpr.Open(&data[0], data.size()));
  MathData m2;
  EXPECT_TRUE(m2.DeSerialize(&fpr));
  m1.ExpectEq(m2);
  const int kBufsize = 1024;
  char buffer[kBufsize + 1];
  EXPECT_EQ(buffer, fpr.FGets(buffer, kBufsize));
  EXPECT_STREQ(line_str.c_str(), buffer);
  MathData m3;
  EXPECT_TRUE(m3.DeSerialize(&fpr));
  m1.ExpectEq(m3);
}

TEST_F(TfileTest, BigEndian) {
  // This test verifies that Tfile can auto-reverse big-endian data.
  MathData m1;
  m1.Setup();
  std::vector<char> data;
  TFile fpw;
  fpw.OpenWrite(&data);
  EXPECT_TRUE(m1.SerializeBigEndian(&fpw));
  TFile fpr;
  EXPECT_TRUE(fpr.Open(&data[0], data.size()));
  fpr.set_swap(true);
  MathData m2;
  EXPECT_TRUE(m2.DeSerializeBigEndian(&fpr));
  // That serialize was destructive, so test against a fresh MathData.
  MathData m3;
  m3.Setup();
  m3.ExpectEq(m2);
}

} // namespace tesseract
