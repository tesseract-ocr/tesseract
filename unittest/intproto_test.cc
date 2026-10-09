///////////////////////////////////////////////////////////////////////
// File:        intproto_test.cc
// Description: Tests that a corrupt TESSDATA_INTTEMP component in a
//              .traineddata file is rejected without memory corruption.
//              The count fields (NumClassPruners, NumClasses,
//              NumProtoSets) are read from the untrusted file and used
//              as loop bounds that write into fixed-size arrays in
//              Classify::ReadIntTemplates, so they must be validated
//              before use.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include <tesseract/baseapi.h>

#include "intproto.h"     // for MAX_NUM_CLASS_PRUNERS, MAX_NUM_CLASSES
#include "tessdatamanager.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace tesseract {
namespace {

// Minimal legacy unicharset component (two characters: space and 'a').
const char kMinUnicharset[] =
    "2\n"
    "NULL 1 0,255,0,255,0,0,0,0,0,0 Latin 2 0 2\n"
    "a 1 0,255,0,255,0,0,0,0,0,0 Latin 2 0 2\n";

// Builds an inttemp component in the current (version -5) on-disk layout,
// as written by Classify::WriteIntTemplates:
//   uint32 unicharset_size, int32 version_id, uint32 NumClassPruners,
//   uint32 NumClasses, then per class: uint16 NumProtos, uint8 NumProtoSets,
//   uint8 NumConfigs.
std::vector<char> MakeInttemp(int32_t version_id, uint32_t num_class_pruners,
                              uint32_t num_classes, uint32_t unicharset_size,
                              uint8_t num_proto_sets = 0) {
  std::vector<char> data;
  auto append = [&data](const void *p, size_t n) {
    const char *b = static_cast<const char *>(p);
    data.insert(data.end(), b, b + n);
  };
  append(&unicharset_size, sizeof(unicharset_size));
  append(&version_id, sizeof(version_id));
  append(&num_class_pruners, sizeof(num_class_pruners));
  append(&num_classes, sizeof(num_classes));
  for (uint32_t c = 0; c < num_classes && c < MAX_NUM_CLASSES; ++c) {
    uint16_t num_protos = 0;
    uint8_t num_configs = 0;
    append(&num_protos, sizeof(num_protos));
    append(&num_proto_sets, sizeof(num_proto_sets));
    append(&num_configs, sizeof(num_configs));
  }
  return data;
}

// Builds an inttemp component in the old (version 1) on-disk layout. Besides
// the common header this carries the IndexFor[] table (unicharset_size x
// int16), the ClassIdFor[] table (num_classes x int32) and one class pruner
// (NUM_CP_BUCKETS^3 x WERDS_PER_CP_VECTOR uint32 words), as read by the version < 2 path of
// Classify::ReadIntTemplates.
std::vector<char> MakeLegacyInttemp(int32_t version_id, uint32_t num_classes,
                                    uint32_t unicharset_size,
                                    const std::vector<int32_t> &class_ids) {
  std::vector<char> data;
  auto append = [&data](const void *p, size_t n) {
    const char *b = static_cast<const char *>(p);
    data.insert(data.end(), b, b + n);
  };
  uint32_t num_class_pruners = 1;
  append(&unicharset_size, sizeof(unicharset_size));
  append(&version_id, sizeof(version_id));
  append(&num_class_pruners, sizeof(num_class_pruners));
  append(&num_classes, sizeof(num_classes));
  for (uint32_t i = 0; i < unicharset_size; ++i) {
    int16_t index = 0;
    append(&index, sizeof(index));
  }
  for (uint32_t i = 0; i < num_classes; ++i) {
    int32_t class_id =
        i < class_ids.size() ? class_ids[i] : static_cast<int32_t>(i);
    append(&class_id, sizeof(class_id));
  }
  // One class pruner is NUM_CP_BUCKETS^3 vectors of WERDS_PER_CP_VECTOR words
  // each, i.e. WERDS_PER_CP uint32 words total.
  for (uint32_t w = 0; w < WERDS_PER_CP; ++w) {
    uint32_t zero = 0;
    append(&zero, sizeof(zero));
  }
  uint16_t num_protos = 0;
  uint8_t num_proto_sets = 0;
  uint8_t num_configs = 0;
  for (uint32_t c = 0; c < num_classes && c < MAX_NUM_CLASSES; ++c) {
    append(&num_protos, sizeof(num_protos));
    append(&num_proto_sets, sizeof(num_proto_sets));
    append(&num_configs, sizeof(num_configs));
  }
  return data;
}

// Writes a traineddata file with a minimal unicharset and the given
// (corrupt) inttemp component to dir/eng.traineddata.
bool WriteCorruptTraineddata(const std::string &dir, const std::vector<char> &inttemp) {
  TessdataManager mgr;
  mgr.OverwriteEntry(TESSDATA_UNICHARSET, kMinUnicharset, sizeof(kMinUnicharset) - 1);
  mgr.OverwriteEntry(TESSDATA_INTTEMP, inttemp.data(), static_cast<int>(inttemp.size()));
  return mgr.SaveFile((dir + "/eng.traineddata").c_str(), nullptr);
}

class IntprotoTest : public testing::Test {
protected:
  void SetUp() override {
    tmpl_ = "/tmp/tess_intproto_test_XXXXXX";
    char *dir = mkdtemp(tmpl_.data());
    ASSERT_NE(dir, nullptr);
    dir_ = dir;
  }
  void TearDown() override {
    std::remove((dir_ + "/eng.traineddata").c_str());
    rmdir(dir_.c_str());
  }
  // Expects the legacy engine to reject the corrupted traineddata
  // gracefully (init failure) instead of corrupting memory.
  void ExpectInitFails(const std::vector<char> &inttemp) {
    ASSERT_TRUE(WriteCorruptTraineddata(dir_, inttemp));
    tesseract::TessBaseAPI api;
    EXPECT_EQ(api.Init(dir_.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY), -1);
  }
  std::string dir_;
  std::string tmpl_;
};

// NumClassPruners exceeds MAX_NUM_CLASS_PRUNERS: the pruner-read loop would
// write past the end of INT_TEMPLATES_STRUCT::ClassPruners[].
TEST_F(IntprotoTest, RejectsTooManyClassPruners) {
  ExpectInitFails(MakeInttemp(-5, MAX_NUM_CLASS_PRUNERS + 1, 0, 1));
}

// NumClasses exceeds MAX_NUM_CLASSES: the class-read loop would write past
// the end of INT_TEMPLATES_STRUCT::Class[].
TEST_F(IntprotoTest, RejectsTooManyClasses) {
  ExpectInitFails(MakeInttemp(-5, 0, MAX_NUM_CLASSES + 1, 1));
}

// A class with NumProtoSets > MAX_NUM_PROTO_SETS: the proto-set loop would
// write past the end of INT_CLASS_STRUCT::ProtoSets[].
TEST_F(IntprotoTest, RejectsTooManyProtoSets) {
  ExpectInitFails(MakeInttemp(-5, 0, 1, 1, MAX_NUM_PROTO_SETS + 1));
}

// unicharset_size exceeds MAX_NUM_CLASSES: the version < 2 class-id-index
// read would write past the end of IndexFor[].
TEST_F(IntprotoTest, RejectsTooLargeUnicharsetSize) {
  ExpectInitFails(MakeInttemp(-1, 0, 0, MAX_NUM_CLASSES + 1));
}

// A version 1 inttemp whose ClassIdFor[] carries a negative class id. The
// max_class_id upper-bound check can never fire for a negative id, so it
// must be rejected by the individual LegalClassId check before it indexes
// Class[].
TEST_F(IntprotoTest, RejectsNegativeClassId) {
  ExpectInitFails(MakeLegacyInttemp(-1, 1, 2, {static_cast<int32_t>(-30000)}));
}

// Same, with a class id at exactly MAX_NUM_CLASSES: above the legal range,
// but still a positive value.
TEST_F(IntprotoTest, RejectsTooLargeClassId) {
  ExpectInitFails(MakeLegacyInttemp(-1, 1, 2, {MAX_NUM_CLASSES}));
}

} // namespace
} // namespace tesseract
