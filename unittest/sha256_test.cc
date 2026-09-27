// SPDX-License-Identifier: Apache-2.0
// Unit tests for tesseract::SHA256 and tesseract::SHA256File.
//
// All expected digests were independently verified with Python hashlib.
// FIPS 180-2 test vectors are used for the core algorithm.

#include "sha256.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "include_gunit.h"

namespace tesseract {

// =========================================================================
// FIPS 180-2 test vectors
// =========================================================================

// Vector 1: empty string
TEST(SHA256Test, EmptyString) {
  SHA256 h;
  EXPECT_EQ(h.FinalHex(),
            "e3b0c44298fc1c149afbf4c8996fb924"
            "27ae41e4649b934ca495991b7852b855");
}

// Vector 2: "abc"
TEST(SHA256Test, Abc) {
  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>("abc"), 3);
  EXPECT_EQ(h.FinalHex(),
            "ba7816bf8f01cfea414140de5dae2223"
            "b00361a396177a9cb410ff61f20015ad");
}

// Vector 3: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
TEST(SHA256Test, TwoBlockMessage) {
  const std::string msg =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>(msg.data()), msg.size());
  EXPECT_EQ(h.FinalHex(),
            "248d6a61d20638b8e5c026930c3e6039"
            "a33ce45964ff2167f6ecedd419db06c1");
}

// Vector 4: one million 'a' characters (streamed in 1000-byte chunks)
TEST(SHA256Test, MillionA) {
  SHA256 h;
  const std::vector<uint8_t> chunk(1000, static_cast<uint8_t>('a'));
  for (int i = 0; i < 1000; ++i) {
    h.Update(chunk.data(), chunk.size());
  }
  EXPECT_EQ(h.FinalHex(),
            "cdc76e5c9914fb9281a1c7e284d73e67"
            "f1809a48a497200e046d39ccc7112cd0");
}

// =========================================================================
// Padding boundary tests
// =========================================================================

// 55 bytes: padding fits entirely in the same block.
TEST(SHA256Test, FiftyFiveBytes) {
  std::string input(55, 'x');
  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>(input.data()), input.size());
  // Expected: hashlib.sha256(b"x"*55).hexdigest()
  EXPECT_EQ(h.FinalHex(),
            "d5e285683cd4efc02d021a5c62014694"
            "958901005d6f71e89e0989fac77e4072");
}

// 56 bytes: exact boundary — padding spills into a new block.
TEST(SHA256Test, FiftySixBytes) {
  std::string input(56, 'x');
  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>(input.data()), input.size());
  // Expected: hashlib.sha256(b"x"*56).hexdigest()
  EXPECT_EQ(h.FinalHex(),
            "04c26261370ee7541549d16dee320c72"
            "3e3fd14671e66a099afe0a377c16888e");
}

// 64 bytes: exactly one full block before padding.
TEST(SHA256Test, SixtyFourBytes) {
  std::string input(64, 'x');
  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>(input.data()), input.size());
  // Expected: hashlib.sha256(b"x"*64).hexdigest()
  EXPECT_EQ(h.FinalHex(),
            "7ce100971f64e7001e8fe5a51973ecdf"
            "e1ced42befe7ee8d5fd6219506b5393c");
}

// =========================================================================
// Streaming equivalence
// =========================================================================

// Byte-at-a-time streaming must equal a single Update().
TEST(SHA256Test, ByteAtATime) {
  const std::string msg = "The quick brown fox jumps over the lazy dog";

  SHA256 h_bulk;
  h_bulk.Update(reinterpret_cast<const uint8_t *>(msg.data()), msg.size());

  SHA256 h_byte;
  for (size_t i = 0; i < msg.size(); ++i) {
    h_byte.Update(reinterpret_cast<const uint8_t *>(&msg[i]), 1);
  }

  EXPECT_EQ(h_bulk.FinalHex(), h_byte.FinalHex());
}

// =========================================================================
// Embedded NUL bytes
// =========================================================================

TEST(SHA256Test, EmbeddedNul) {
  uint8_t data[] = {'a', 0, 'b', 0, 'c'};
  SHA256 h;
  h.Update(data, sizeof(data));
  // Expected: hashlib.sha256(b"a\x00b\x00c").hexdigest()
  EXPECT_EQ(h.FinalHex(),
            "8badde10c760e9b702defb4b5e225de7"
            "9c515b1d2a5cfb000e140f3c6fbb5629");
}

// =========================================================================
// SHA256File tests
// =========================================================================

// Missing file returns empty string.
TEST(SHA256Test, MissingFile) {
  EXPECT_EQ(SHA256File("/no/such/file/anywhere.xyz"), "");
}

// Null path returns empty string.
TEST(SHA256Test, NullPath) {
  EXPECT_EQ(SHA256File(nullptr), "");
}

// Empty file hashes to the empty-string digest (not "").
TEST(SHA256Test, EmptyFile) {
  file::MakeTmpdir();
  std::string path = std::string(FLAGS_test_tmpdir) + "/empty_file_for_sha256";
  {
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out.good());
  }

  std::string digest = SHA256File(path.c_str());
  EXPECT_EQ(digest,
            "e3b0c44298fc1c149afbf4c8996fb924"
            "27ae41e4649b934ca495991b7852b855");
}

// SHA256File agrees with in-memory hash on a small file.
// A path that opens but cannot be read (a directory: fopen succeeds, fread
// fails with EISDIR) must return "" rather than the digest of zero bytes,
// which would be indistinguishable from a valid hash of an empty file.
TEST(SHA256Test, DirectoryReturnsEmptyString) {
  EXPECT_EQ(SHA256File("/tmp"), std::string());
}

// Guards the distinction the above test depends on: an empty *file* is
// readable and must produce the real digest of zero bytes.
TEST(SHA256Test, DirectoryIsNotConfusedWithEmptyFile) {
  const char *kEmptyDigest =
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  const std::string path = "sha256_test_empty_guard.bin";
  {
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out.good());
  }
  EXPECT_EQ(SHA256File(path.c_str()), kEmptyDigest);   // empty file -> digest
  EXPECT_NE(SHA256File("/tmp"), kEmptyDigest);         // directory  -> not
  remove(path.c_str());
}

TEST(SHA256Test, FileMatchesMemory) {
  file::MakeTmpdir();
  std::string path =
      std::string(FLAGS_test_tmpdir) + "/sha256_file_test.bin";
  const std::string content = "Hello, Tesseract SHA-256!";

  {
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out.good());
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
  }

  SHA256 h;
  h.Update(reinterpret_cast<const uint8_t *>(content.data()), content.size());
  std::string expected = h.FinalHex();

  EXPECT_EQ(SHA256File(path.c_str()), expected);
}

} // namespace tesseract
