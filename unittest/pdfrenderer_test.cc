// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <tesseract/renderer.h>

#include "include_gunit.h"

#include <allheaders.h> // for zlibUncompress

#include <cstdlib> // for strtoul
#include <filesystem>
#include <fstream>
#include <iterator> // for std::istreambuf_iterator
#include <sstream>
#include <string>
#include <vector>

namespace tesseract {

namespace {

// Writes a PDF file without pages and returns its contents.
std::string MakeEmptyPdf() {
  std::error_code ec;
  std::filesystem::create_directories(FLAGS_test_tmpdir, ec);
  const std::string outputbase = std::string(FLAGS_test_tmpdir) + "/pdfrenderer_test";
  {
    // The data directory has no pdf.ttf, so the built-in font is used.
    TessPDFRenderer renderer(outputbase.c_str(), FLAGS_test_tmpdir, true);
    EXPECT_TRUE(renderer.BeginDocument("pdfrenderer_test"));
    EXPECT_TRUE(renderer.EndDocument());
  }
  std::ifstream input(outputbase + ".pdf", std::ios::in | std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(input), {});
}

// Returns the number at the start of the given text, or -1 if there is none.
long ParseNumber(const std::string &text, size_t pos = 0) {
  if (pos >= text.size() || text[pos] < '0' || text[pos] > '9') {
    return -1;
  }
  return strtol(text.c_str() + pos, nullptr, 10);
}

// Gets the value of a 2 byte code written like <01FF>.
bool ParseCode(const std::string &token, unsigned *code) {
  if (token.size() != 6 || token.front() != '<' || token.back() != '>') {
    return false;
  }
  char *end;
  *code = strtoul(token.c_str() + 1, &end, 16);
  return end == token.c_str() + 5;
}

// Returns the decoded stream of the object with the given number.
// Fails if the stream does not have the size given by /Length.
std::string GetStream(const std::string &pdf, int objnum) {
  const std::string header = "\n" + std::to_string(objnum) + " 0 obj\n";
  const std::string length_key = "/Length ";
  const std::string stream_key = "stream\n";
  const std::string end_key = "endstream\n";
  auto pos = pdf.find(header);
  if (pos == std::string::npos) {
    ADD_FAILURE() << "No object " << objnum;
    return "";
  }
  const auto dictionary = pos;
  pos = pdf.find(length_key, pos);
  if (pos == std::string::npos) {
    ADD_FAILURE() << "No /Length in object " << objnum;
    return "";
  }
  const long length = ParseNumber(pdf, pos + length_key.size());
  pos = pdf.find(stream_key, pos);
  if (length < 0 || pos == std::string::npos) {
    ADD_FAILURE() << "No stream in object " << objnum;
    return "";
  }
  const bool compressed =
      pdf.substr(dictionary, pos - dictionary).find("/FlateDecode") != std::string::npos;
  pos += stream_key.size();
  EXPECT_EQ(pdf.compare(pos + length, end_key.size(), end_key), 0)
      << "Wrong /Length " << length << " in object " << objnum;
  if (!compressed) {
    return pdf.substr(pos, length);
  }
  size_t size = 0;
  l_uint8 *data =
      zlibUncompress(reinterpret_cast<const l_uint8 *>(pdf.data()) + pos, length, &size);
  if (data == nullptr) {
    ADD_FAILURE() << "Cannot decode the stream of object " << objnum;
    return "";
  }
  const std::string stream(reinterpret_cast<const char *>(data), size);
  lept_free(data);
  return stream;
}

} // namespace

// Tests that the ToUnicode CMap of the glyphless font maps each 2 byte code
// to the same UTF-16 code unit with ranges which are allowed by the PDF
// standard (https://github.com/tesseract-ocr/tesseract/issues/4718).
TEST(PDFRendererTest, ToUnicodeRanges) {
  const std::string pdf = MakeEmptyPdf();
  ASSERT_NE(pdf.find("/ToUnicode 6 0 R"), std::string::npos);
  std::istringstream cmap(GetStream(pdf, 6));

  const unsigned kNumCodes = 0x10000;
  // A block must not have more than 100 ranges (Adobe Technical Note #5014).
  const int kMaxRangesPerBlock = 100;
  std::vector<long> unicode(kNumCodes, -1);
  int num_blocks = 0;
  std::string token;
  std::string previous;
  while (cmap >> token) {
    if (token == "beginbfrange") {
      num_blocks++;
      int num_ranges = 0;
      std::string first;
      std::string last;
      std::string destination;
      while (cmap >> first && first != "endbfrange") {
        ASSERT_TRUE(cmap >> last >> destination);
        num_ranges++;
        unsigned code1;
        unsigned code2;
        unsigned unicode1;
        ASSERT_TRUE(ParseCode(first, &code1)) << first;
        ASSERT_TRUE(ParseCode(last, &code2)) << last;
        ASSERT_TRUE(ParseCode(destination, &unicode1)) << destination;
        ASSERT_LE(code1, code2);
        const std::string range = first + " " + last + " " + destination;
        // Only the last byte of the code changes in a range.
        EXPECT_EQ(code1 >> 8, code2 >> 8) << range;
        // Only the last byte of the destination is incremented, so it must
        // not exceed 255 for the last code (ISO 32000-1, 9.10.3).
        EXPECT_LE((unicode1 & 0xFF) + (code2 - code1), 255U) << range;
        for (unsigned code = code1; code <= code2; code++) {
          EXPECT_EQ(unicode[code], -1) << "Code in more than one range: " << range;
          unicode[code] = unicode1 + (code - code1);
        }
      }
      EXPECT_EQ(first, "endbfrange");
      EXPECT_EQ(num_ranges, ParseNumber(previous));
      EXPECT_LE(num_ranges, kMaxRangesPerBlock);
    }
    previous = token;
  }
  EXPECT_GT(num_blocks, 0);
  unsigned num_wrong_codes = 0;
  for (unsigned code = 0; code < kNumCodes; code++) {
    if (unicode[code] != static_cast<long>(code)) {
      num_wrong_codes++;
    }
  }
  EXPECT_EQ(num_wrong_codes, 0U);
}

// Tests that each entry of the cross reference table gives the position of
// its object.
TEST(PDFRendererTest, CrossReferenceTable) {
  const std::string pdf = MakeEmptyPdf();
  const std::string startxref_key = "startxref\n";
  const std::string xref_key = "xref\n0 ";
  const size_t kEntrySize = 20;
  auto pos = pdf.rfind(startxref_key);
  ASSERT_NE(pos, std::string::npos);
  const long xref = ParseNumber(pdf, pos + startxref_key.size());
  ASSERT_GE(xref, 0);
  ASSERT_EQ(pdf.compare(xref, xref_key.size(), xref_key), 0);
  const long num_objects = ParseNumber(pdf, xref + xref_key.size());
  // The file has at least the 8 objects of the catalog, the pages and the font.
  ASSERT_GT(num_objects, 8);
  pos = pdf.find('\n', xref + xref_key.size());
  ASSERT_NE(pos, std::string::npos);
  pos++;
  ASSERT_LE(pos + num_objects * kEntrySize, pdf.size());
  EXPECT_EQ(pdf.substr(pos, kEntrySize), "0000000000 65535 f \n");
  for (long objnum = 1; objnum < num_objects; objnum++) {
    const std::string entry = pdf.substr(pos + objnum * kEntrySize, kEntrySize);
    EXPECT_EQ(entry.substr(10), " 00000 n \n") << "Object " << objnum;
    const long offset = ParseNumber(entry);
    ASSERT_GE(offset, 0) << "Object " << objnum;
    const std::string header = std::to_string(objnum) + " 0 obj\n";
    EXPECT_EQ(pdf.compare(offset, header.size(), header), 0) << "Object " << objnum;
  }
}

} // namespace tesseract
