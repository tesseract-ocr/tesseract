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
#include "unicharcompress.h" // for RecodedCharID

#include "include_gunit.h"

#include <cstdint> // for uint32_t

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

// Builds a little-endian uint32 followed by the given number of zero
// trailing bytes, mirroring the count-prefixed layout the vector, string
// and char-vector deserializers expect.
std::vector<char> MakeCountBuffer(uint32_t count, int trailing_bytes) {
  std::vector<char> buf;
  for (int i = 0; i < 4; ++i) {
    buf.push_back(static_cast<char>((count >> (8 * i)) & 0xFF));
  }
  buf.resize(buf.size() + trailing_bytes, 0);
  return buf;
}

// A declared element count that is not backed by the bytes remaining in
// the stream must be rejected before the vector is resized, so a tiny
// crafted file cannot force a multi-gigabyte allocation (denial of
// service). This is the case for a recoder encoder_ of 50,000,000
// RecodedCharID: 50,000,000 * sizeof(RecodedCharID) is ~2.2 GB.
TEST_F(TfileTest, RejectsUnbackedVectorCount) {
  std::vector<char> data = MakeCountBuffer(50000000, 8);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<int> vec;
  EXPECT_FALSE(fp.DeSerialize(vec));
  // The vector must not have been resized before the rejection.
  EXPECT_TRUE(vec.empty());
}

TEST_F(TfileTest, RejectsUnbackedStringVectorCount) {
  std::vector<char> data = MakeCountBuffer(50000000, 8);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<std::string> vec;
  EXPECT_FALSE(fp.DeSerialize(vec));
  EXPECT_TRUE(vec.empty());
}

TEST_F(TfileTest, RejectsUnbackedCharVectorSize) {
  std::vector<char> data = MakeCountBuffer(50000000, 8);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<char> vec;
  EXPECT_FALSE(fp.DeSerialize(vec));
  EXPECT_TRUE(vec.empty());
}

TEST_F(TfileTest, RejectsUnbackedStringSize) {
  std::vector<char> data = MakeCountBuffer(50000000, 8);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::string str;
  EXPECT_FALSE(fp.DeSerialize(str));
  EXPECT_TRUE(str.empty());
}

// A declared element count of zero must be accepted (it clears the vector).
// This is the only path that falls out of the branch chain without an
// explicit read, so it must return a value (UBSan: "execution reached the
// end of a value-returning function without returning a value").
TEST_F(TfileTest, AcceptsZeroVectorCount) {
  std::vector<char> data = MakeCountBuffer(0, 0);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<int> vec = {1, 2, 3};
  EXPECT_TRUE(fp.DeSerialize(vec));
  EXPECT_TRUE(vec.empty());
}

// A count that is fully backed by the remaining bytes must still be
// accepted, so the stream-backing check does not reject valid data.
TEST_F(TfileTest, AcceptsFullyBackedVectorCount) {
  TFile fpw;
  std::vector<char> data;
  fpw.OpenWrite(&data);
  std::vector<int> values = {7, -12, 0};
  EXPECT_TRUE(fpw.Serialize(values));
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<int> vec;
  EXPECT_TRUE(fp.DeSerialize(vec));
  ASSERT_EQ(3, vec.size());
  EXPECT_EQ(7, vec[0]);
  EXPECT_EQ(-12, vec[1]);
  EXPECT_EQ(0, vec[2]);
}

// A string vector that is fully backed (each string serialized as a uint32
// length plus its bytes) must still be accepted.
TEST_F(TfileTest, AcceptsFullyBackedStringVectorCount) {
  TFile fpw;
  std::vector<char> data;
  fpw.OpenWrite(&data);
  std::vector<std::string> strings = {"ab", ""};
  EXPECT_TRUE(fpw.Serialize(strings));
  TFile fpr;
  EXPECT_TRUE(fpr.Open(&data[0], data.size()));
  std::vector<std::string> result;
  EXPECT_TRUE(fpr.DeSerialize(result));
  ASSERT_EQ(2, result.size());
  EXPECT_EQ("ab", result[0]);
  EXPECT_EQ("", result[1]);
}

// The exact type from GHSA-fxpc-5x59-r9px: the recoder encoder_.
// Declaring 50,000,000 RecodedCharID (~2.2 GB) from a tiny stream must be
// rejected in the class branch.
TEST_F(TfileTest, RejectsUnbackedRecodedCharIDVectorCount) {
  std::vector<char> data = MakeCountBuffer(50000000, 8);
  TFile fp;
  EXPECT_TRUE(fp.Open(&data[0], data.size()));
  std::vector<RecodedCharID> vec;
  EXPECT_FALSE(fp.DeSerialize(vec));
  EXPECT_TRUE(vec.empty());
}

// A fully backed vector of RecodedCharID must still be accepted, even
// though sizeof(RecodedCharID) is 44 while it serializes to only 5 to 41
// bytes per element (a self-normalized flag, a uint32 length, and that
// many int32 codes): a sizeof-based backing check would wrongly reject
// this.
TEST_F(TfileTest, AcceptsFullyBackedRecodedCharIDVectorCount) {
  std::vector<RecodedCharID> codes(3);
  codes[0].Set(0, 1);
  codes[1].Set3(2, 3, 4);
  TFile fpw;
  std::vector<char> data;
  fpw.OpenWrite(&data);
  EXPECT_TRUE(fpw.Serialize(codes));
  TFile fpr;
  EXPECT_TRUE(fpr.Open(&data[0], data.size()));
  std::vector<RecodedCharID> result;
  EXPECT_TRUE(fpr.DeSerialize(result));
  ASSERT_EQ(3, result.size());
  EXPECT_EQ(1u, result[0].length());
  EXPECT_EQ(1, result[0](0));
  EXPECT_EQ(3u, result[1].length());
  EXPECT_EQ(4, result[1](2));
}

} // namespace tesseract
