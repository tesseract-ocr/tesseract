//

#include "tesseract/api/baseapi.h"

// Dummy enum in the global namespace that checks for collision with awkward
// names.
// If this test fails to compile, clean up the includes in baseapi.h!
// They are not supposed to drag in definitions of any of the tesseract
// types included in this enum!
enum NameTester {
  ABORT,
  OKAY,
  LOG,
  BLOB,
  ELIST,
  TBOX,
  TPOINT,
  WORD
};

namespace {

// Verifies that the global namespace is clean.
TEST(CleanNamespaceTess, DummyTest) {
  tesseract::TessBaseAPI api;
}

}  // namespace.

