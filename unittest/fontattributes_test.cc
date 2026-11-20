///////////////////////////////////////////////////////////////////////
// File:        fontattributes_test.cc
// Description: Unit tests for the font attributes (bold, italic, small
//              caps) in ALTO and PAGE output.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include "include_gunit.h"

#include "fontinfo.h"
#include "image.h"
#include "pageres.h"

#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/resultiterator.h>

#include <memory>
#include <string>
#include <vector>

namespace tesseract {

// Gives access to the results of the recognition, so the tests can set the
// font of each word. The output then does not depend on the fonts which an
// OCR engine finds.
class FriendlyTessBaseAPI : public TessBaseAPI {
public:
  PAGE_RES *page_res() {
    return page_res_;
  }
};

// Returns the attributes of the first element with the given name in the
// text, without the name of the element, or "none" if there is no such
// element. The name must end with the space before the attributes.
static std::string Attributes(const std::string &text, const std::string &name) {
  const size_t start = text.find("<" + name);
  if (start == std::string::npos) {
    return "none";
  }
  const size_t end = text.find('>', start);
  std::string attributes = text.substr(start + 1 + name.size(), end - start - 1 - name.size());
  if (!attributes.empty() && attributes.back() == '/') {
    attributes.pop_back();
  }
  return attributes;
}

// Returns the parts of the text which start with the given string.
static std::vector<std::string> Split(const std::string &text, const std::string &start) {
  std::vector<std::string> parts;
  size_t pos = text.find(start);
  while (pos != std::string::npos) {
    const size_t next = text.find(start, pos + 1);
    parts.push_back(text.substr(pos, next == std::string::npos ? next : next - pos));
    pos = next;
  }
  return parts;
}

// The tests recognize phototest.tif, which has eight text lines, and give
// the words of the first five lines these fonts:
// line 0: bold
// line 1: italic
// line 2: bold and italic, and the words are small caps
// line 3: bold for the first word, a regular font for the other words
// line 4: bold and bold italic in turns
// The words of the other lines get no font.
class FontAttributesTest : public testing::Test {
protected:
  void SetUp() override {
    REQUIRE_TRAINEDDATA(TESSDATA_DIR, "eng");
    if (api_.Init(TESSDATA_DIR, "eng") == -1) {
      // eng.traineddata not found.
      GTEST_SKIP();
    }
    api_.SetPageSegMode(PSM_AUTO);
    char names[4][12] = {"Regular", "Italic", "Bold", "Bold Italic"};
    for (unsigned i = 0; i < 4; ++i) {
      names_[i] = names[i];
      fonts_[i].name = &names_[i][0];
      // The properties are those of FontInfo: 1 = italic, 2 = bold.
      fonts_[i].properties = i;
      fonts_[i].universal_id = i;
    }
  }

  // Recognizes the image and sets the fonts. Skips the test if the API
  // does not report the font attributes of a word, which is the case for
  // some builds without the legacy engine.
  void Recognize() {
    Image pix = pixRead(TESTING_DIR "/phototest.tif");
    ASSERT_TRUE(pix != nullptr);
    api_.SetImage(pix);
    pix.destroy();
    ASSERT_EQ(api_.Recognize(nullptr), 0);
    ASSERT_NE(api_.page_res(), nullptr);
    int line = -1;
    int word_in_line = 0;
    const ROW_RES *row = nullptr;
    PAGE_RES_IT it(api_.page_res());
    for (it.restart_page(); it.word() != nullptr; it.forward()) {
      if (it.row() != row) {
        row = it.row();
        ++line;
        word_in_line = 0;
        expected_lines_.push_back("");
      }
      WERD_RES *word = it.word();
      bool bold = false;
      bool italic = false;
      bool smallcaps = false;
      bool has_font = true;
      switch (line) {
        case 0:
          bold = true;
          break;
        case 1:
          italic = true;
          break;
        case 2:
          bold = italic = smallcaps = true;
          break;
        case 3:
          bold = word_in_line == 0;
          break;
        case 4:
          bold = true;
          italic = word_in_line % 2 == 1;
          break;
        default:
          has_font = false;
      }
      word->fontinfo = has_font ? &fonts_[(italic ? 1 : 0) | (bold ? 2 : 0)] : nullptr;
      word->small_caps = smallcaps;
      expected_alto_.push_back(std::string(bold ? "bold " : "") + (italic ? "italics " : "") +
                               (smallcaps ? "smallcaps " : ""));
      if (!expected_alto_.back().empty()) {
        expected_alto_.back().pop_back();
      }
      expected_page_.push_back(std::string(bold ? " bold=\"true\"" : "") +
                               (italic ? " italic=\"true\"" : "") +
                               (smallcaps ? " smallCaps=\"true\"" : ""));
      ++word_in_line;
    }
    ASSERT_GE(line, 5);
    // A line has the attributes which all of its words have.
    expected_lines_[0] = " bold=\"true\"";
    expected_lines_[1] = " italic=\"true\"";
    expected_lines_[2] = " bold=\"true\" italic=\"true\" smallCaps=\"true\"";
    expected_lines_[4] = " bold=\"true\"";

    std::unique_ptr<ResultIterator> result(api_.GetIterator());
    ASSERT_NE(result, nullptr);
    bool bold, italic, underlined, monospace, serif, smallcaps;
    int pointsize, font_id;
    result->WordFontAttributes(&bold, &italic, &underlined, &monospace, &serif, &smallcaps,
                               &pointsize, &font_id);
    if (!bold) {
      LOG(INFO) << "Skip test because the API reports no font attributes\n";
      GTEST_SKIP();
    }
  }

  // Returns the text of the element TextStyle in the given part of a PAGE
  // file as it is expected by the tests: the attributes or an empty string.
  static std::string Style(const std::string &text) {
    const std::string style = Attributes(text, "TextStyle");
    return style == "none" ? "" : style;
  }

  // The fonts must live longer than the results which refer to them.
  std::string names_[4];
  FontInfo fonts_[4];
  FriendlyTessBaseAPI api_;
  // Expected value of the attribute STYLE for each word in ALTO.
  std::vector<std::string> expected_alto_;
  // Expected attributes of the element TextStyle for each word in PAGE.
  std::vector<std::string> expected_page_;
  // The same for each text line.
  std::vector<std::string> expected_lines_;
};

TEST_F(FontAttributesTest, Alto) {
  ASSERT_NO_FATAL_FAILURE(Recognize());
  if (IsSkipped()) {
    return;
  }
  std::unique_ptr<char[]> alto(api_.GetAltoText(0));
  ASSERT_NE(alto, nullptr);
  const std::vector<std::string> words = Split(alto.get(), "<String ");
  ASSERT_EQ(words.size(), expected_alto_.size());
  for (size_t i = 0; i < words.size(); ++i) {
    const std::string attributes = Attributes(words[i], "String ");
    const size_t pos = attributes.find(" STYLE=\"");
    std::string style;
    if (pos != std::string::npos) {
      style = attributes.substr(pos + 8, attributes.find('"', pos + 8) - pos - 8);
      // The attribute is only written with a value.
      EXPECT_FALSE(style.empty()) << words[i];
    }
    EXPECT_EQ(style, expected_alto_[i]) << "word " << i << ": " << words[i];
  }
}

TEST_F(FontAttributesTest, PageWordLevel) {
  ASSERT_TRUE(api_.SetVariable("page_xml_level", "1"));
  ASSERT_NO_FATAL_FAILURE(Recognize());
  if (IsSkipped()) {
    return;
  }
  std::unique_ptr<char[]> page(api_.GetPAGEText(0));
  ASSERT_NE(page, nullptr);
  const std::vector<std::string> lines = Split(page.get(), "<TextLine ");
  ASSERT_EQ(lines.size(), expected_lines_.size());
  size_t num_words = 0;
  for (size_t l = 0; l < lines.size(); ++l) {
    const std::vector<std::string> words = Split(lines[l], "<Word ");
    ASSERT_FALSE(words.empty()) << "line " << l;
    for (const auto &word : words) {
      ASSERT_LT(num_words, expected_page_.size());
      // The style follows the text of the word.
      const size_t text_end = word.find("</TextEquiv>");
      const size_t word_end = word.find("</Word>");
      ASSERT_NE(text_end, std::string::npos);
      ASSERT_NE(word_end, std::string::npos);
      EXPECT_EQ(Style(word.substr(0, text_end)), "") << word;
      EXPECT_EQ(Style(word.substr(text_end, word_end - text_end)), expected_page_[num_words])
          << "word " << num_words << ": " << word;
      ++num_words;
    }
    // The style of the line follows its words and its text.
    const size_t line_end = lines[l].find("</TextLine>");
    ASSERT_NE(line_end, std::string::npos);
    const std::string line = lines[l].substr(0, line_end);
    const size_t last_word_end = line.rfind("</Word>");
    const size_t text_end = line.rfind("</TextEquiv>");
    ASSERT_LT(last_word_end, text_end);
    EXPECT_EQ(Style(line.substr(text_end)), expected_lines_[l]) << "line " << l;
  }
  EXPECT_EQ(num_words, expected_page_.size());
}

TEST_F(FontAttributesTest, PageLineLevel) {
  ASSERT_NO_FATAL_FAILURE(Recognize());
  if (IsSkipped()) {
    return;
  }
  std::unique_ptr<char[]> page(api_.GetPAGEText(0));
  ASSERT_NE(page, nullptr);
  const std::string text(page.get());
  EXPECT_EQ(text.find("<Word "), std::string::npos);
  const std::vector<std::string> lines = Split(text, "<TextLine ");
  ASSERT_EQ(lines.size(), expected_lines_.size());
  for (size_t l = 0; l < lines.size(); ++l) {
    const size_t text_end = lines[l].find("</TextEquiv>");
    const size_t line_end = lines[l].find("</TextLine>");
    ASSERT_LT(text_end, line_end);
    EXPECT_EQ(Style(lines[l].substr(0, text_end)), "") << "line " << l;
    EXPECT_EQ(Style(lines[l].substr(text_end, line_end - text_end)), expected_lines_[l])
        << "line " << l;
  }
}

// Words without font attributes must not change the output.
TEST_F(FontAttributesTest, NoAttributes) {
  Image pix = pixRead(TESTING_DIR "/phototest.tif");
  ASSERT_TRUE(pix != nullptr);
  api_.SetImage(pix);
  pix.destroy();
  ASSERT_EQ(api_.Recognize(nullptr), 0);
  PAGE_RES_IT it(api_.page_res());
  for (it.restart_page(); it.word() != nullptr; it.forward()) {
    it.word()->fontinfo = nullptr;
    it.word()->small_caps = false;
  }
  std::unique_ptr<char[]> alto(api_.GetAltoText(0));
  ASSERT_NE(alto, nullptr);
  EXPECT_EQ(std::string(alto.get()).find("STYLE"), std::string::npos);
  for (const char *level : {"0", "1"}) {
    ASSERT_TRUE(api_.SetVariable("page_xml_level", level));
    std::unique_ptr<char[]> page(api_.GetPAGEText(0));
    ASSERT_NE(page, nullptr);
    EXPECT_EQ(std::string(page.get()).find("TextStyle"), std::string::npos);
  }
}

} // namespace tesseract
