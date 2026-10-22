///////////////////////////////////////////////////////////////////////
// File:        intproto_addclass_test.cc
// Description: Tests that AddIntClass rejects an out-of-range class id
//              (instead of writing out of bounds into Class[]) and a legal
//              id that is not the next sequential id, in both cases leaving
//              the template set unchanged.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "intproto.h" // for AddIntClass, INT_TEMPLATES_STRUCT, INT_CLASS_STRUCT

#include <string>
#include <vector>

namespace tesseract {
namespace {

class AddIntClassTest : public testing::Test {
};

// A valid in-order class id must be accepted.
TEST_F(AddIntClassTest, AcceptsValidId) {
  INT_TEMPLATES_STRUCT Templates;
  INT_CLASS_STRUCT *Class = new INT_CLASS_STRUCT(1);
  ASSERT_TRUE(AddIntClass(&Templates, 0, Class));
  EXPECT_EQ(Templates.NumClasses, 1);
}

// A legal id that is not the next sequential id must be rejected and leave
// the set unchanged; this replaces the former process-terminating behavior.
TEST_F(AddIntClassTest, RejectsNonSequentialId) {
  INT_TEMPLATES_STRUCT Templates;
  INT_CLASS_STRUCT *First = new INT_CLASS_STRUCT(1);
  ASSERT_TRUE(AddIntClass(&Templates, 0, First));
  INT_CLASS_STRUCT *Second = new INT_CLASS_STRUCT(1);
  EXPECT_FALSE(AddIntClass(&Templates, 2, Second));
  EXPECT_EQ(Templates.NumClasses, 1);
  // AddIntClass only takes ownership of Class on success; on a reject the
  // caller keeps it.
  delete Second;
}

// An out-of-range (too large) class id must be rejected and leave the set
// unchanged.
TEST_F(AddIntClassTest, RejectsTooLargeId) {
  INT_TEMPLATES_STRUCT Templates;
  INT_CLASS_STRUCT *Class = new INT_CLASS_STRUCT(1);
  EXPECT_FALSE(AddIntClass(&Templates, MAX_NUM_CLASSES, Class));
  EXPECT_EQ(Templates.NumClasses, 0);
  // AddIntClass only takes ownership of Class on success; on a reject the
  // caller keeps it.
  delete Class;
}

// A negative class id must be rejected and leave the set unchanged.
TEST_F(AddIntClassTest, RejectsNegativeId) {
  INT_TEMPLATES_STRUCT Templates;
  INT_CLASS_STRUCT *Class = new INT_CLASS_STRUCT(1);
  EXPECT_FALSE(AddIntClass(&Templates, -1, Class));
  EXPECT_EQ(Templates.NumClasses, 0);
  delete Class;
}

} // namespace
} // namespace tesseract
