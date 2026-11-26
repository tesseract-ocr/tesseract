///////////////////////////////////////////////////////////////////////
// File:        adaptive_templates_test.cc
// Description: Tests that the adapted templates are not built from a
//              unicharset that is too large for the fixed-size class
//              arrays, and that such a failure is detectable by the
//              caller: a unichar id indexes fixed-size arrays of
//              MAX_NUM_CLASSES entries, so an oversized unicharset cannot
//              be represented.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "adaptive.h" // for ADAPT_TEMPLATES_STRUCT
#include "classify.h" // for Classify (NewAdaptedTemplates)
#include "unicharset.h"

#include <cstdio>

namespace tesseract {
namespace {

// Fills the given unicharset with the requested number of entries. The
// unicharset parser rejects an oversized set, so a set of this size can
// only be built in memory.
void FillUnicharset(UNICHARSET *unicharset, size_t size) {
  for (size_t i = 0; i < size; i++) {
    char label[16];
    snprintf(label, sizeof(label), "u%05zu", i);
    unicharset->unichar_insert(label);
  }
}

class AdaptiveTemplatesTest : public testing::Test {
};

// A unicharset that fits into the class arrays must give a usable set of
// adapted templates, with one class per unichar.
TEST_F(AdaptiveTemplatesTest, BuildsTemplatesForValidUnicharset) {
  Classify classifier;
  for (const char *unichar : {"a", "b", "c"}) {
    classifier.unicharset.unichar_insert(unichar);
  }
  ADAPT_TEMPLATES_STRUCT *templates = classifier.NewAdaptedTemplates();
  ASSERT_NE(templates, nullptr);
  EXPECT_TRUE(templates->valid());
  EXPECT_EQ(templates->Templates->NumClasses, classifier.unicharset.size());
  delete templates;
}

// An oversized unicharset cannot be represented: no templates may be built
// and the caller has to be able to detect that.
TEST_F(AdaptiveTemplatesTest, RejectsOversizedUnicharset) {
  Classify classifier;
  FillUnicharset(&classifier.unicharset, MAX_NUM_CLASSES + 1);
  EXPECT_EQ(classifier.NewAdaptedTemplates(), nullptr);
}

// The constructor cannot report a failure, so the object it leaves behind
// has to be recognisable as unusable. The classes added before the failure
// have to be freed, which the sanitizer checks.
TEST_F(AdaptiveTemplatesTest, OversizedUnicharsetLeavesUnusableTemplates) {
  UNICHARSET unicharset;
  FillUnicharset(&unicharset, MAX_NUM_CLASSES + 1);
  ADAPT_TEMPLATES_STRUCT *templates = new ADAPT_TEMPLATES_STRUCT(unicharset);
  EXPECT_FALSE(templates->valid());
  delete templates;
}

} // namespace
} // namespace tesseract
