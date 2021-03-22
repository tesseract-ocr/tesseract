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

#include <cmath>
#include <cstdio>
#include <string>

#include "bitvector.h"

#include "include_gunit.h"

const int kPrimeLimit = 1000;

namespace tesseract {

class BitVectorTest : public testing::Test {
protected:
  void SetUp() override {
    std::locale::global(std::locale(""));
    file::MakeTmpdir();
  }

public:
  std::string OutputNameToPath(const std::string &name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  // Computes primes up to kPrimeLimit, using the sieve of Eratosthenes.
  void ComputePrimes(BitVector *map) {
    map->Init(kPrimeLimit + 1);
    TestAll(*map, false);
    map->SetBit(2);
    // Set all the odds to true.
    for (int i = 3; i <= kPrimeLimit; i += 2) {
      map->SetValue(i, true);
    }
    int factor_limit = static_cast<int>(sqrt(1.0 + kPrimeLimit));
    for (int f = 3; f <= factor_limit; f += 2) {
      if (map->At(f)) {
        for (int m = 2; m * f <= kPrimeLimit; ++m) {
          map->ResetBit(f * m);
        }
      }
    }
  }

  void TestPrimes(const BitVector &map) {
    // Now all primes in the vector are true, and all others false.
    // According to Wikipedia, there are 168 primes under 1000, the last
    // of which is 997.
    int total_primes = 0;
    for (int i = 0; i <= kPrimeLimit; ++i) {
      if (map[i]) {
        ++total_primes;
      }
    }
    EXPECT_EQ(168, total_primes);
    EXPECT_TRUE(map[997]);
    EXPECT_FALSE(map[998]);
    EXPECT_FALSE(map[999]);
  }
  // Test that all bits in the vector have the given value.
  void TestAll(const BitVector &map, bool value) {
    for (int i = 0; i < map.size(); ++i) {
      EXPECT_EQ(value, map[i]);
    }
  }

  // Sets up a BitVector with bit patterns for byte values in
  // [start_byte, end_byte) positioned every spacing bytes (for spacing >= 1)
  // with spacing-1  zero bytes in between the pattern bytes.
  void SetBitPattern(int start_byte, int end_byte, int spacing, BitVector *bv) {
    bv->Init((end_byte - start_byte) * 8 * spacing);
    for (int byte_value = start_byte; byte_value < end_byte; ++byte_value) {
      for (int bit = 0; bit < 8; ++bit) {
        if (byte_value & (1 << bit)) {
          bv->SetBit((byte_value - start_byte) * 8 * spacing + bit);
        }
      }
    }
  }

  // Expects that every return from NextSetBit is really set and that all others
  // are really not set. Checks the return from NumSetBits also.
  void ExpectCorrectBits(const BitVector &bv) {
    int bit_index = -1;
    int prev_bit_index = -1;
    int num_bits_tested = 0;
    while ((bit_index = bv.NextSetBit(bit_index)) >= 0) {
      EXPECT_LT(bit_index, bv.size());
      // All bits in between must be 0.
      for (int i = prev_bit_index + 1; i < bit_index; ++i) {
        EXPECT_EQ(0, bv[i]) << "i = " << i << " prev = " << prev_bit_index;
      }
      // This bit must be 1.
      EXPECT_EQ(1, bv[bit_index]) << "Bit index = " << bit_index;
      ++num_bits_tested;
      prev_bit_index = bit_index;
    }
    // Check the bits between the last and the end.
    for (int i = prev_bit_index + 1; i < bv.size(); ++i) {
      EXPECT_EQ(0, bv[i]);
    }
    EXPECT_EQ(num_bits_tested, bv.NumSetBits());
  }
};

// Tests the sieve of Eratosthenes as a way of testing set/reset and I/O.
TEST_F(BitVectorTest, Primes) {
  BitVector map;
  ComputePrimes(&map);
  TestPrimes(map);
  // It still works if we use the copy constructor.
  BitVector map2(map);
  TestPrimes(map2);
  // Or if we assign it.
  BitVector map3;
  map3 = map;
  TestPrimes(map3);
  // Test file i/o too.
  std::string filename = OutputNameToPath("primesbitvector");
  FILE *fp = fopen(filename.c_str(), "wb");
  ASSERT_TRUE(fp != nullptr);
  EXPECT_TRUE(map.Serialize(fp));
  fclose(fp);
  fp = fopen(filename.c_str(), "rb");
  ASSERT_TRUE(fp != nullptr);
  BitVector read_map;
  EXPECT_TRUE(read_map.DeSerialize(false, fp));
  fclose(fp);
  TestPrimes(read_map);
}

// Tests the many-to-one setup feature.
TEST_F(BitVectorTest, SetAll) {
  // Test the default constructor and set/resetall.
  BitVector map(42);
  TestAll(map, false);
  map.SetAllTrue();
  TestAll(map, true);
  map.SetAllFalse();
  TestAll(map, false);
}

// Tests the values in the tables offset_table_, next_table_, hamming_table_
// by setting all possible byte patterns and verifying that the NextSetBit and
// NumSetBits functions return the correct values.
TEST_F(BitVectorTest, TestNextSetBit) {
  BitVector bv;
  for (int spacing = 1; spacing <= 5; ++spacing) {
    SetBitPattern(0, 256, spacing, &bv);
    ExpectCorrectBits(bv);
  }
}

// Tests the values in hamming_table_ more thoroughly by setting single byte
// patterns for each byte individually.
TEST_F(BitVectorTest, TestNumSetBits) {
  BitVector bv;
  for (int byte = 0; byte < 256; ++byte) {
    SetBitPattern(byte, byte + 1, 1, &bv);
    ExpectCorrectBits(bv);
  }
}

} // namespace tesseract
