///////////////////////////////////////////////////////////////////////
// File:        unicharset_load_test.cc
// Description: Tests that UNICHARSET::load_via_fgets rejects unicharset
//              files whose insertions desynchronize the id loop index
//              from the unichars vector (duplicate or empty
//              representations), which would make the subsequent set_*
//              calls write out of bounds, and non-positive size counts.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "serialis.h" // for TFile
#include "unicharset.h"

#include <cstring>

namespace tesseract {
namespace {

// Loads the given unicharset text via the TFile-based loader.
bool LoadUnicharset(const char *text, UNICHARSET *unicharset) {
  TFile fp;
  if (!fp.Open(text, std::strlen(text))) {
    return false;
  }
  return unicharset->load_from_file(&fp, false);
}

// A duplicate representation makes the second insert a no-op, so on
// unpatched code the set_* calls for the remaining lines write past
// the end of the unichars vector (ASan container-overflow).
TEST(UnicharsetLoadTest, RejectsDuplicateRepresentation) {
  const char *text = "3\nA 0 Latin\nA 0 Latin\nB 0 Latin\n";
  UNICHARSET unicharset;
  EXPECT_FALSE(LoadUnicharset(text, &unicharset));
}

// A non-positive size count must be rejected; on unpatched code a
// zero or negative count loads an empty unicharset successfully.
TEST(UnicharsetLoadTest, RejectsNonPositiveCount) {
  const char *texts[] = {"0\n", "-1\n"};
  for (const char *text : texts) {
    UNICHARSET unicharset;
    EXPECT_FALSE(LoadUnicharset(text, &unicharset));
  }
}

// A valid unicharset must still be accepted.
TEST(UnicharsetLoadTest, AcceptsValidUnicharset) {
  const char *text = "3\nA 0 Latin\nB 0 Latin\nC 0 Latin\n";
  UNICHARSET unicharset;
  ASSERT_TRUE(LoadUnicharset(text, &unicharset));
  EXPECT_EQ(unicharset.size(), 3u);
  EXPECT_STREQ(unicharset.id_to_unichar(1), "B");
}

} // namespace
} // namespace tesseract
