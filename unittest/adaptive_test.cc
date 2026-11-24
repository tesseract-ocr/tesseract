///////////////////////////////////////////////////////////////////////
// File:        adaptive_test.cc
// Description: Tests that ReadAdaptedClass rejects a corrupt
//              ADAPT_CLASS_STRUCT without passing file-controlled
//              pointers to the destructor (which unconditionally deletes
//              the Config[] entries and walks the TempProtos list). Covers
//              both the Temp and Perm Config[] union branches.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "adaptive.h" // for ADAPT_CLASS_STRUCT, ReadAdaptedClass
#include "bitvec.h"   // for WordsInVectorOfSize
#include "classify.h" // for Classify (ReadAdaptedTemplates)
#include "serialis.h" // for TFile

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Builds a single ADAPT_CLASS_STRUCT record for ReadAdaptedClass:
//   [ADAPT_CLASS_STRUCT raw] [PermProtos bitvector] [PermConfigs bitvector]
//   [int32 NumTempProtos] [int32 NumConfigs]
// All bytes of the struct are set to a non-null fill value so that the
// pointer-bearing members (TempProtos, Config[]) hold file-controlled
// pointers. NumConfigs controls how many Config[] entries are filled with
// real (nullptr) allocations, and perm_configs controls which slots the file
// marks permanent.
std::vector<char> MakeAdaptedClassRecord(int32_t num_temp_protos, int32_t num_configs,
                                         uint8_t fill, const std::vector<uint32_t> &perm_configs) {
  std::vector<char> data(sizeof(ADAPT_CLASS_STRUCT), static_cast<char>(fill));
  ADAPT_CLASS_STRUCT layout;
  const auto *base = reinterpret_cast<const char *>(&layout);
  data[reinterpret_cast<const char *>(&layout.NumPermConfigs) - base] = 0;
  data[reinterpret_cast<const char *>(&layout.MaxNumTimesSeen) - base] = 0;
  // PermProtos and PermConfigs are reallocated by ReadAdaptedClass, so their
  // initial (file) values are irrelevant; leave them as the fill value.
  auto append = [&data](const void *p, size_t n) {
    const char *b = static_cast<const char *>(p);
    data.insert(data.end(), b, b + n);
  };
  std::vector<uint32_t> perm_protos(WordsInVectorOfSize(MAX_NUM_PROTOS), 0);
  append(perm_protos.data(), perm_protos.size() * sizeof(uint32_t));
  append(perm_configs.data(), perm_configs.size() * sizeof(uint32_t));
  append(&num_temp_protos, sizeof(int32_t));
  append(&num_configs, sizeof(int32_t));
  return data;
}

class ReadAdaptedClassTest : public testing::Test {
 protected:
  // Runs ReadAdaptedClass on the given record. The point is that every exit
  // path destructs the class cleanly, which ASan verifies; a file-controlled
  // pointer reaching the destructor aborts the process.
  ADAPT_CLASS_STRUCT *Read(const std::vector<char> &record) {
    TFile fp;
    fp.Open(record.data(), record.size());
    if (fp.RemainingBytes() != record.size()) {
      return nullptr;
    }
    return ReadAdaptedClass(&fp);
  }
};

// Success path with no configs: every Config[] entry is a file-controlled
// pointer, and TempProtos is a file-controlled (bogus) list. The destructor
// must not delete them.
TEST_F(ReadAdaptedClassTest, SuccessPathDoesNotDeleteFilePointers) {
  std::vector<uint32_t> perm_configs(WordsInVectorOfSize(MAX_NUM_CONFIGS), 0);
  auto record = MakeAdaptedClassRecord(/*num_temp_protos=*/0, /*num_configs=*/0, /*fill=*/0x42,
                                       perm_configs);
  ADAPT_CLASS_STRUCT *Class = Read(record);
  EXPECT_NE(Class, nullptr);
  // Destroying the class must not delete the file-controlled Config[]
  // entries nor walk the file-controlled TempProtos list.
  delete Class;
}

// First reject path: NumTempProtos is negative, so the class is deleted
// before TempProtos is ever reset. The file-controlled TempProtos list
// must not be walked by the destructor.
TEST_F(ReadAdaptedClassTest, RejectPathDoesNotWalkFileTempProtos) {
  std::vector<uint32_t> perm_configs(WordsInVectorOfSize(MAX_NUM_CONFIGS), 0);
  auto record = MakeAdaptedClassRecord(/*num_temp_protos=*/-1, /*num_configs=*/0, /*fill=*/0x42,
                                       perm_configs);
  ADAPT_CLASS_STRUCT *Class = Read(record);
  EXPECT_EQ(Class, nullptr);
}

// Success path where the file marks every config permanent but loads none
// (NumConfigs = 0). The destructor takes the Perm branch for all 64 slots
// the load loop never filled, so it must read the (aliased, nulled) Perm
// member rather than delete a file-controlled pointer.
TEST_F(ReadAdaptedClassTest, PermBranchDoesNotDeleteFilePointers) {
  std::vector<uint32_t> perm_configs(WordsInVectorOfSize(MAX_NUM_CONFIGS), 0xFFFFFFFF);
  auto record = MakeAdaptedClassRecord(/*num_temp_protos=*/0, /*num_configs=*/0, /*fill=*/0x42,
                                       perm_configs);
  ADAPT_CLASS_STRUCT *Class = Read(record);
  EXPECT_NE(Class, nullptr);
  delete Class;
}

// ReadAdaptedTemplates has the same one-line-too-late reset: a truncated
// ADAPT_TEMPLATES_STRUCT record takes the FRead != 1 reject path with
// Templates->Templates (offset 0) still holding file bytes, and the Class[]
// reset loop sits below that branch. ~ADAPT_TEMPLATES_STRUCT reads
// Templates->NumClasses through the file-sourced pointer, so the record must
// be neutralised before the destructor runs.
class ReadAdaptedTemplatesTest : public testing::Test {
 protected:
  // A truncated record: the whole struct is fill, with the first word set to
  // a non-null pointer so the destructor's Templates != nullptr check is true
  // and it reads (Templates)->NumClasses through it. Shorter than the struct
  // size, so FRead copies fewer than sizeof and returns != 1.
  std::vector<char> MakeTruncatedRecord(uint8_t fill, uint64_t first_word) {
    std::vector<char> data(sizeof(ADAPT_TEMPLATES_STRUCT), static_cast<char>(fill));
    std::memcpy(data.data(), &first_word, sizeof(uint64_t));
    data.resize(64); // smaller than sizeof(ADAPT_TEMPLATES_STRUCT) -> FRead != 1
    return data;
  }

  Classify classifier_;
};

// A short record with a non-null first word must be rejected cleanly, not
// crash the destructor on the file-sourced Templates pointer.
TEST_F(ReadAdaptedTemplatesTest, RejectPathDoesNotReadFilePointer) {
  auto record = MakeTruncatedRecord(/*fill=*/0x45, /*first_word=*/0x4545454545454540ULL);
  TFile fp;
  ASSERT_TRUE(fp.Open(record.data(), record.size()));
  EXPECT_EQ(classifier_.ReadAdaptedTemplates(&fp), nullptr);
}

} // namespace
} // namespace tesseract
