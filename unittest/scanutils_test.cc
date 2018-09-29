#include <stdio.h>

#include "tesseract/ccutil/scanutils.h"

namespace {

class ScanutilsTest : public ::testing::Test {
 protected:
  string TestDataNameToPath(const string& name) {
    return file::JoinPath(FLAGS_test_srcdir, "testdata/" + name);
  }
};

TEST_F(ScanutilsTest, DoesScanf) {
  // This test verifies that tfscanf does Scanf the same as stdio fscanf.
  // There are probably a gazillion more test cases that could be added, but
  // these brought the tesseract and unittest test results in line.
  string filename = TestDataNameToPath("scanftest.txt");
  FILE* fp1 = fopen(filename.c_str(), "r");
  FILE* fp2 = fopen(filename.c_str(), "r");
  // The file contains this:
  // 42.5 17 0.001000 -0.001000
  // 0 1 123 -123 0x100
  // abcdefghijklmnopqrstuvwxyz
  // abcdefghijklmnopqrstuvwxyz
  // MF 25 6.25e-2 0.5e5 -1e+4
  // 42 MF 25 6.25e-2 0.5
  // 24
  const int kNumFloats = 4;
  float f1[kNumFloats], f2[kNumFloats];
  int r1 = fscanf(fp1, "%f %f %f %f", &f1[0], &f1[1], &f1[2], &f1[3]);
  int r2 = tfscanf(fp2, "%f %f %f %f", &f2[0], &f2[1], &f2[2], &f2[3]);
  EXPECT_EQ(r1, r2);
  for (int i = 0; i < kNumFloats; ++i) EXPECT_FLOAT_EQ(f1[i], f2[i]);
  const int kNumInts = 5;
  int i1[kNumInts], i2[kNumInts];
  r1 = fscanf(fp1, "%d %d %d %d %i", &i1[0], &i1[1], &i1[2], &i1[3], &i1[4]);
  r2 = tfscanf(fp2, "%d %d %d %d %i", &i2[0], &i2[1], &i2[2], &i2[3], &i2[4]);
  EXPECT_EQ(r1, r2);
  for (int i = 0; i < kNumInts; ++i) EXPECT_EQ(i1[i], i2[i]);
  const int kStrLen = 1024;
  char s1[kStrLen];
  char s2[kStrLen];
  r1 = fscanf(fp1, "%1023s", s1);
  r2 = tfscanf(fp2, "%1023s", s2);
  EXPECT_EQ(r1, r2);
  EXPECT_STREQ(s1, s2);
  EXPECT_EQ(26, strlen(s2));
  r1 = fscanf(fp1, "%20s", s1);
  r2 = tfscanf(fp2, "%20s", s2);
  EXPECT_EQ(r1, r2);
  EXPECT_STREQ(s1, s2);
  EXPECT_EQ(20, strlen(s2));
  // Now read the rest of the alphabet.
  r1 = fscanf(fp1, "%s", s1);
  r2 = tfscanf(fp2, "%s", s2);
  EXPECT_EQ(r1, r2);
  EXPECT_STREQ(s1, s2);
  EXPECT_EQ(6, strlen(s2));
  r1 = fscanf(fp1, "%s", s1);
  r2 = tfscanf(fp2, "%s", s2);
  EXPECT_EQ(r1, r2);
  EXPECT_STREQ(s1, s2);
  EXPECT_EQ(2, strlen(s2));
  r1 = fscanf(fp1, "%f %f %f %f", &f1[0], &f1[1], &f1[2], &f1[3]);
  r2 = tfscanf(fp2, "%f %f %f %f", &f2[0], &f2[1], &f2[2], &f2[3]);
  EXPECT_EQ(r1, r2);
  for (int i = 0; i < kNumFloats; ++i) EXPECT_FLOAT_EQ(f1[i], f2[i]);
  // Test the * for field suppression.
  r1 = fscanf(fp1, "%d %*s %*d %*f %*f", &i1[0]);
  r2 = tfscanf(fp2, "%d %*s %*d %*f %*f", &i2[0]);
  EXPECT_EQ(r1, r2);
  EXPECT_EQ(i1[0], i2[0]);
  // We should still see the next value and no phantoms.
  r1 = fscanf(fp1, "%d %s", &i1[0], s1);
  r2 = tfscanf(fp2, "%d %s", &i2[0], s2);
  EXPECT_EQ(r1, r2);
  EXPECT_EQ(1, r2);
  EXPECT_EQ(i1[0], i2[0]);
}

}  // namespace
