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

#include "helpers.h"
#include "indexmapbidi.h"
#include "serialis.h"

#include "include_gunit.h"

const int kPrimeLimit = 1000;

namespace tesseract {

class IndexMapBiDiTest : public testing::Test {
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
  void ComputePrimes(IndexMapBiDi *map) {
    map->Init(kPrimeLimit + 1, false);
    map->SetMap(2, true);
    // Set all the odds to true.
    for (int i = 3; i <= kPrimeLimit; i += 2) {
      map->SetMap(i, true);
    }
    int factor_limit = static_cast<int>(sqrt(1.0 + kPrimeLimit));
    for (int f = 3; f <= factor_limit; f += 2) {
      if (map->SparseToCompact(f) >= 0) {
        for (int m = 2; m * f <= kPrimeLimit; ++m) {
          map->SetMap(f * m, false);
        }
      }
    }
    map->Setup();
  }

  void TestPrimes(const IndexMap &map) {
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
  FILE *fp = fopen(filename.c_str(), "wb");
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

// Invalid sparse indices are ignored, while valid boundary indices retain
// their mapping behavior.
TEST_F(IndexMapBiDiTest, SetMapBounds) {
  IndexMapBiDi map;
  map.Init(3, false);
  map.SetMap(0, true);
  map.SetMap(1, false);
  map.SetMap(2, true);
  map.SetMap(-1, true);
  map.SetMap(-1, false);
  map.SetMap(3, true);
  map.SetMap(3, false);
  map.SetMap(1324324, true);
  map.SetMap(1324324, false);
  map.Setup();

  EXPECT_EQ(3, map.SparseSize());
  EXPECT_EQ(2, map.CompactSize());
  EXPECT_EQ(0, map.SparseToCompact(0));
  EXPECT_EQ(-1, map.SparseToCompact(1));
  EXPECT_EQ(1, map.SparseToCompact(2));
  EXPECT_EQ(0, map.CompactToSparse(0));
  EXPECT_EQ(2, map.CompactToSparse(1));
}

// SetMap is also safe before a map has been initialized.
TEST_F(IndexMapBiDiTest, SetMapEmpty) {
  IndexMapBiDi map;
  map.SetMap(0, true);
  map.SetMap(-1, false);
  map.SetMap(1324324, true);
  map.Setup();

  EXPECT_EQ(0, map.SparseSize());
  EXPECT_EQ(0, map.CompactSize());
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

// Writes a raw IndexMapBiDi serialization (sparse size, compact map,
// remaining pairs) so crafted/invalid data can be fed to DeSerialize.
static void WriteIndexMapBiDiBlob(const std::string &path, int32_t sparse_size,
                                  const std::vector<int32_t> &compact_map,
                                  const std::vector<int32_t> &remaining_pairs) {
  FILE *fp = fopen(path.c_str(), "wb");
  ASSERT_TRUE(fp != nullptr);
  ASSERT_TRUE(tesseract::Serialize(fp, &sparse_size));
  ASSERT_TRUE(tesseract::Serialize(fp, compact_map));
  ASSERT_TRUE(tesseract::Serialize(fp, remaining_pairs));
  fclose(fp);
}

// Indices read from a serialized map are untrusted. Out-of-range values must
// be rejected instead of writing outside sparse_map_.
TEST_F(IndexMapBiDiTest, DeSerializeRejectsBadIndices) {
  // Positive control: a valid many-to-one map round-trips.
  IndexMapBiDi valid;
  valid.Init(13, false);
  valid.SetMap(2, true);
  valid.SetMap(4, true);
  valid.SetMap(7, true);
  valid.SetMap(9, true);
  valid.SetMap(11, true);
  valid.Setup();
  valid.Merge(valid.SparseToCompact(2), valid.SparseToCompact(9));
  valid.Merge(valid.SparseToCompact(4), valid.SparseToCompact(11));
  valid.CompleteMerges();
  const std::string good = OutputNameToPath("good.indexmap");
  {
    FILE *fp = fopen(good.c_str(), "wb");
    ASSERT_TRUE(fp != nullptr);
    ASSERT_TRUE(valid.Serialize(fp));
    fclose(fp);
  }
  {
    FILE *fp = fopen(good.c_str(), "rb");
    ASSERT_TRUE(fp != nullptr);
    IndexMapBiDi m;
    ASSERT_TRUE(m.DeSerialize(false, fp));
    fclose(fp);
    EXPECT_EQ(13, m.SparseSize());
    EXPECT_EQ(3, m.CompactSize());
    EXPECT_EQ(0, m.SparseToCompact(2));
    EXPECT_EQ(0, m.SparseToCompact(9));
  }

  auto reject = [this](const std::string &name, int32_t sparse_size,
                       const std::vector<int32_t> &compact_map,
                       const std::vector<int32_t> &remaining_pairs) {
    const std::string path = OutputNameToPath(name);
    WriteIndexMapBiDiBlob(path, sparse_size, compact_map, remaining_pairs);
    FILE *fp = fopen(path.c_str(), "rb");
    ASSERT_TRUE(fp != nullptr);
    IndexMapBiDi m;
    EXPECT_FALSE(m.DeSerialize(false, fp));
    fclose(fp);
  };

  // compact_map_ entry outside the sparse space.
  reject("bad1.indexmap", 2, {5}, {});
  // Negative compact_map_ entry.
  reject("bad2.indexmap", 2, {-1}, {});
  // Remaining pair with a sparse index outside the sparse space.
  reject("bad3.indexmap", 2, {0}, {5, 0});
  // Remaining pair with a negative sparse index.
  reject("bad4.indexmap", 2, {0}, {-1, 0});
  // Remaining pair with a compact index outside the compact space.
  reject("bad5.indexmap", 2, {0}, {0, 5});
  // Remaining pair with a negative compact index.
  reject("bad6.indexmap", 2, {0}, {0, -1});
  // Odd number of remaining pairs.
  reject("bad7.indexmap", 2, {0}, {0});
  // Cyclic master mapping (0 <-> 1) that would make MasterCompactIndex
  // loop forever.
  reject("bad8.indexmap", 2, {0, 1}, {0, 1, 1, 0});
  // Two compact representatives claiming the same sparse slot.
  reject("bad9.indexmap", 2, {0, 0}, {});
}

// Public accessors must not read outside their maps when given bad indices.
TEST_F(IndexMapBiDiTest, AccessorsRejectBadIndices) {
  IndexMapBiDi map;
  map.Init(4, false);
  map.SetMap(1, true);
  map.SetMap(3, true);
  map.Setup();
  // Sparse space is [0,4); compact space is [0,2) with sparse 1->0, 3->1.
  EXPECT_EQ(4, map.SparseSize());
  EXPECT_EQ(2, map.CompactSize());

  // Out-of-range sparse index reports unmapped instead of reading OOB.
  EXPECT_EQ(-1, map.SparseToCompact(-1));
  EXPECT_EQ(-1, map.SparseToCompact(4));
  EXPECT_EQ(-1, map.SparseToCompact(1324324));
  // In-range behavior is unchanged.
  EXPECT_EQ(0, map.SparseToCompact(1));
  EXPECT_EQ(-1, map.SparseToCompact(0)); // unmapped.
  EXPECT_EQ(1, map.SparseToCompact(3));

  // Out-of-range compact index reports unmapped instead of reading OOB.
  EXPECT_EQ(-1, map.CompactToSparse(-1));
  EXPECT_EQ(-1, map.CompactToSparse(2));
  EXPECT_EQ(1, map.CompactToSparse(0));
  EXPECT_EQ(3, map.CompactToSparse(1));

  // Out-of-range compact indices are not merged.
  EXPECT_FALSE(map.Merge(2, 0));
  EXPECT_FALSE(map.Merge(0, 2));
  EXPECT_FALSE(map.Merge(0, 1324324));
  EXPECT_FALSE(map.Merge(-5, 0));
  // The map is unchanged by the rejected merges.
  EXPECT_EQ(2, map.CompactSize());
  EXPECT_EQ(0, map.SparseToCompact(1));
  EXPECT_EQ(1, map.SparseToCompact(3));

  // Out-of-range compact index is reported as deleted.
  EXPECT_TRUE(map.IsCompactDeleted(-1));
  EXPECT_TRUE(map.IsCompactDeleted(2));
  EXPECT_TRUE(map.IsCompactDeleted(1324324));
  EXPECT_FALSE(map.IsCompactDeleted(0));

  // Out-of-range features count as missed; valid ones still map.
  std::vector<int> compact;
  const int missed = map.MapFeatures({-1, 0, 1, 3, 4}, &compact);
  EXPECT_EQ(3, missed); // -1 (OOB), 0 (unmapped), 4 (OOB).
  ASSERT_EQ(2, compact.size());
  EXPECT_EQ(0, compact[0]);
  EXPECT_EQ(1, compact[1]);

  // The -1 sentinel (delete a compact index) is still permitted.
  EXPECT_TRUE(map.Merge(-1, 1));
  EXPECT_TRUE(map.IsCompactDeleted(1));

  // Empty maps built via the public API must report unmapped for any
  // index instead of reading past the end of their vectors. The plain
  // IndexMap is the only way to reach the base-class binary-search
  // implementation, since the IndexMapBiDi override handles its own
  // empty check.
  IndexMapBiDi empty_bidi;
  empty_bidi.Init(0, false);
  empty_bidi.Setup();
  EXPECT_EQ(-1, empty_bidi.SparseToCompact(0));
  EXPECT_EQ(-1, empty_bidi.CompactToSparse(0));
  IndexMap empty_base;
  empty_base.CopyFrom(empty_bidi);
  EXPECT_EQ(-1, empty_base.SparseToCompact(0));
  EXPECT_EQ(-1, empty_base.CompactToSparse(0));
}

} // namespace tesseract
