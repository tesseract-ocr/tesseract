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

#include "indexmapbidi.h"

#include "include_gunit.h"

using tesseract::IndexMap;
using tesseract::IndexMapBiDi;

const int kPrimeLimit = 1000;

namespace {

class IndexMapBiDiTest : public testing::Test {
 protected:
  void SetUp() {
    std::locale::global(std::locale(""));
  }

 public:
  std::string OutputNameToPath(const std::string& name) {
    return file::JoinPath(FLAGS_test_tmpdir, name);
  }
  // Computes primes up to kPrimeLimit, using the sieve of Eratosthenes.
  void ComputePrimes(IndexMapBiDi* map) {
    map->Init(kPrimeLimit + 1, false);
    map->SetMap(2, true);
    // Set all the odds to true.
    for (int i = 3; i <= kPrimeLimit; i += 2) map->SetMap(i, true);
    int factor_limit = static_cast<int>(sqrt(1.0 + kPrimeLimit));
    for (int f = 3; f <= factor_limit; f += 2) {
      if (map->SparseToCompact(f) >= 0) {
        for (int m = 2; m * f <= kPrimeLimit; ++m) map->SetMap(f * m, false);
      }
    }
    map->Setup();
  }

  void TestPrimes(const IndexMap& map) {
    // Now all primes are mapped in the sparse map to their index.
    // According to Wikipedia, the 168th prime is 997, and it has compact
    // index 167 because we are indexing from 0.
    EXPECT_EQ(167, map.SparseToCompact(997));
    EXPECT_EQ(997, map.CompactToSparse(167));
    // 995, 996, 998, 999 are not prime.
    EXPECT_EQ(-1, map.SparseToCompact(995));
    EXPECT_EQ(-1, map.SparseToCompact(996));
    EXPECT_EQ(-1, map.SparseToCompact(998));
    EXPECT_EQ(-1, map.SparseToCompact(999));
    // The 167th prime is 991.
    EXPECT_EQ(991, map.CompactToSparse(166));
    // There are 168 primes in 0..1000.
    EXPECT_EQ(168, map.CompactSize());
    EXPECT_EQ(kPrimeLimit + 1, map.SparseSize());
  }
};

// Tests the sieve of Eratosthenes as a way of testing setup.
TEST_F(IndexMapBiDiTest, Primes) {
  IndexMapBiDi map;
  ComputePrimes(&map);
  TestPrimes(map);
  // It still works if we assign it to another.
  IndexMapBiDi map2;
  map2.CopyFrom(map);
  TestPrimes(map2);
  // Or if we assign it to a base class.
  IndexMap base_map;
  base_map.CopyFrom(map);
  TestPrimes(base_map);
  // Test file i/o too.
  std::string filename = OutputNameToPath("primesmap");
  FILE* fp = fopen(filename.c_str(), "wb");
  CHECK(fp != nullptr);
  EXPECT_TRUE(map.Serialize(fp));
  fclose(fp);
  fp = fopen(filename.c_str(), "rb");
  CHECK(fp != nullptr);
  IndexMapBiDi read_map;
  EXPECT_TRUE(read_map.DeSerialize(false, fp));
  fclose(fp);
  TestPrimes(read_map);
}

// Tests the many-to-one setup feature.
TEST_F(IndexMapBiDiTest, ManyToOne) {
  // Test the example in the comment on CompleteMerges.
  IndexMapBiDi map;
  map.Init(13, false);
  map.SetMap(2, true);
  map.SetMap(4, true);
  map.SetMap(7, true);
  map.SetMap(9, true);
  map.SetMap(11, true);
  map.Setup();
  map.Merge(map.SparseToCompact(2), map.SparseToCompact(9));
  map.Merge(map.SparseToCompact(4), map.SparseToCompact(11));
  map.CompleteMerges();
  EXPECT_EQ(3, map.CompactSize());
  EXPECT_EQ(13, map.SparseSize());
  EXPECT_EQ(1, map.SparseToCompact(4));
  EXPECT_EQ(4, map.CompactToSparse(1));
  EXPECT_EQ(1, map.SparseToCompact(11));
}

}  // namespace.
