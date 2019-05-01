// (C) Copyright 2017, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>                       // for std::string

#include "absl/strings/str_cat.h"       // for absl::StrCat
#include "absl/strings/str_join.h"      // for absl::StrJoin
#include "absl/strings/str_split.h"     // for absl::StrSplit

#include "include_gunit.h"              // for TEST
#include "log.h"                        // for LOG

#include "genericvector.h"
// ccmain
#include "paragraphs.h"
#include "paragraphs_internal.h"
// ccstruct
#include "ocrpara.h"

namespace {  // anonymous namespace

// Functions for making monospace ASCII trial text for the paragraph detector.
const tesseract::ParagraphJustification kLeft = tesseract::JUSTIFICATION_LEFT;
const tesseract::ParagraphJustification kCenter =
    tesseract::JUSTIFICATION_CENTER;
const tesseract::ParagraphJustification kRight = tesseract::JUSTIFICATION_RIGHT;
const tesseract::ParagraphJustification kUnknown =
    tesseract::JUSTIFICATION_UNKNOWN;

enum TextModelInputType {
  PCONT = 0,   // Continuation line of a paragraph (default).
  PSTART = 1,  // First line of a paragraph.
  PNONE = 2,   // Not a paragraph line.
};

struct TextAndModel {
  const char* ascii;
  TextModelInputType model_type;

  // fields corresponding to PARA (see ccstruct/ocrpara.h)
  ParagraphModel model;
  bool is_very_first_or_continuation;
  bool is_list_item;
};

// Imagine that the given text is typewriter ASCII with each character ten
// pixels wide and twenty pixels high and return an appropriate row_info.
void AsciiToRowInfo(const char* text, int row_number,
                    tesseract::RowInfo* info) {
  const int kCharWidth = 10;
  const int kLineSpace = 30;
  info->text = text;
  info->has_leaders =
      strstr(text, "...") != nullptr || strstr(text, ". . .") != nullptr;
  info->has_drop_cap = false;
  info->pix_ldistance = info->pix_rdistance = 0;
  info->average_interword_space = kCharWidth;
  info->pix_xheight = kCharWidth;
  info->lword_text = info->rword_text = "";
  info->ltr = true;

  std::vector<std::string> words = absl::StrSplit(text, ' ', absl::SkipEmpty());
  info->num_words = words.size();
  if (info->num_words < 1) return;

  info->lword_text = words[0].c_str();
  info->rword_text = words[words.size() - 1].c_str();
  int lspace = 0;
  while (lspace < info->text.size() && text[lspace] == ' ') {
    lspace++;
  }
  int rspace = 0;
  while (rspace < info->text.size() &&
         text[info->text.size() - rspace - 1] == ' ') {
    rspace++;
  }

  int top = -kLineSpace * row_number;
  int bottom = top - kLineSpace;
  int row_right = kCharWidth * info->text.size();
  int lword_width = kCharWidth * info->lword_text.size();
  int rword_width = kCharWidth * info->rword_text.size();
  info->pix_ldistance = lspace * kCharWidth;
  info->pix_rdistance = rspace * kCharWidth;
  info->lword_box =
      TBOX(info->pix_ldistance, bottom, info->pix_ldistance + lword_width, top);
  info->rword_box = TBOX(row_right - info->pix_rdistance - rword_width, bottom,
                         row_right - info->pix_rdistance, top);
  tesseract::LeftWordAttributes(
      nullptr, nullptr, info->lword_text, &info->lword_indicates_list_item,
      &info->lword_likely_starts_idea, &info->lword_likely_ends_idea);
  tesseract::RightWordAttributes(
      nullptr, nullptr, info->rword_text, &info->rword_indicates_list_item,
      &info->rword_likely_starts_idea, &info->rword_likely_ends_idea);
}

void MakeAsciiRowInfos(const TextAndModel* row_infos, int n,
                       GenericVector<tesseract::RowInfo>* output) {
  output->clear();
  tesseract::RowInfo info;
  for (int i = 0; i < n; i++) {
    AsciiToRowInfo(row_infos[i].ascii, i, &info);
    output->push_back(info);
  }
}

// Given n rows of reference ground truth, evaluate whether the n rows
// of PARA * pointers yield the same paragraph breakpoints.
void EvaluateParagraphDetection(const TextAndModel* correct, int n,
                                const GenericVector<PARA*>& detector_output) {
  int incorrect_breaks = 0;
  int missed_breaks = 0;
  int poorly_matched_models = 0;
  int bad_crowns = 0;
  int bad_list_items = 0;
  ASSERT_EQ(detector_output.size(), n);
  for (int i = 1; i < n; i++) {
    bool has_break = correct[i].model_type != PCONT;
    bool detected_break = (detector_output[i - 1] != detector_output[i]);
    if (has_break && !detected_break) missed_breaks++;
    if (detected_break && !has_break) incorrect_breaks++;
    if (has_break) {
      if (correct[i].model_type == PNONE) {
        if (detector_output[i]->model != nullptr) {
          poorly_matched_models++;
        }
      } else {
        if (correct[i].model.justification() != kUnknown &&
            (detector_output[i]->model == nullptr ||
             !correct[i].model.Comparable(*detector_output[i]->model))) {
          poorly_matched_models++;
        }
      }
      if (correct[i].is_very_first_or_continuation ^
          detector_output[i]->is_very_first_or_continuation) {
        bad_crowns++;
      }
      if (correct[i].is_list_item ^ detector_output[i]->is_list_item) {
        bad_list_items++;
      }
    }
  }
  EXPECT_EQ(incorrect_breaks, 0);
  EXPECT_EQ(missed_breaks, 0);
  EXPECT_EQ(poorly_matched_models, 0);
  EXPECT_EQ(bad_list_items, 0);
  EXPECT_EQ(bad_crowns, 0);
  if (incorrect_breaks || missed_breaks || poorly_matched_models ||
      bad_list_items || bad_crowns) {
    std::vector<std::string> dbg_lines;
    dbg_lines.push_back("# ==========================");
    dbg_lines.push_back("# Correct paragraph breaks:");
    dbg_lines.push_back("# ==========================");
    for (int i = 0; i < n; i++) {
      if (correct[i].model_type != PCONT) {
        dbg_lines.push_back(absl::StrCat(
            correct[i].ascii, "  #  ", correct[i].model.ToString().string(),
            correct[i].is_very_first_or_continuation ? " crown" : "",
            correct[i].is_list_item ? " li" : ""));
      } else {
        dbg_lines.push_back(correct[i].ascii);
      }
    }
    dbg_lines.push_back("");
    dbg_lines.push_back("# ==========================");
    dbg_lines.push_back("# Paragraph detector output:");
    dbg_lines.push_back("# ==========================");
    for (int i = 0; i < n; i++) {
      std::string annotation;
      if (i == 0 || (detector_output[i - 1] != detector_output[i])) {
        if (detector_output[i] && detector_output[i]->model) {
          annotation += absl::StrCat(
              "  #  ", detector_output[i]->model->ToString().string(),
              detector_output[i]->is_very_first_or_continuation ? " crown" : "",
              detector_output[i]->is_list_item ? " li" : "");
        } else {
          annotation = "  #  Unmodeled paragraph.";
        }
      }
      dbg_lines.push_back(absl::StrCat(correct[i].ascii, annotation));
    }
    LOG(INFO) << "Discrepency!\n" << absl::StrJoin(dbg_lines, "\n");
  }
}

void TestParagraphDetection(const TextAndModel* correct, int num_rows) {
  GenericVector<tesseract::RowInfo> row_infos;
  GenericVector<PARA*> row_owners;
  PARA_LIST paragraphs;
  GenericVector<ParagraphModel*> models;

  MakeAsciiRowInfos(correct, num_rows, &row_infos);
  int debug_level(3);
  tesseract::DetectParagraphs(debug_level, &row_infos, &row_owners, &paragraphs,
                              &models);
  EvaluateParagraphDetection(correct, num_rows, row_owners);
  models.delete_data_pointers();
}

TEST(ParagraphsTest, ListItemsIdentified) {
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("iii"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("A."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("B."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("C."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("1."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("2."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("3."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("1"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("2"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("3"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("[[1]]"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("A-1."));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("A-2"));
  EXPECT_TRUE(tesseract::AsciiLikelyListItem("(A)(i)"));

  EXPECT_FALSE(tesseract::AsciiLikelyListItem("The"));
  EXPECT_FALSE(tesseract::AsciiLikelyListItem("first"));
  EXPECT_FALSE(tesseract::AsciiLikelyListItem("house"));
  EXPECT_FALSE(tesseract::AsciiLikelyListItem("Oregonian."));
  EXPECT_FALSE(tesseract::AsciiLikelyListItem("on."));
}

typedef ParagraphModel PModel;

const TextAndModel kTwoSimpleParagraphs[] = {
  {"  Look here, I have a paragraph.", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"This paragraph starts at the top", PCONT, PModel(), false, false},
  {"of the page and takes 3 lines.  ", PCONT, PModel(), false, false},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"which indicates that the first  ", PCONT, PModel(), false, false},
  {"paragraph is not a continuation ", PCONT, PModel(), false, false},
  {"from a previous page, as it is  ", PCONT, PModel(), false, false},
  {"indented just like this second  ", PCONT, PModel(), false, false},
  {"paragraph.                      ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestSimpleParagraphDetection) {
  TestParagraphDetection(kTwoSimpleParagraphs,
                         ABSL_ARRAYSIZE(kTwoSimpleParagraphs));
}

const TextAndModel kFewCluesWithCrown[] = {
  {"This paragraph starts at the top", PSTART, PModel(kLeft, 0, 20, 0, 0),
   true, false},
  {"of the page and takes two lines.", PCONT, PModel(), false, false},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"which indicates that the first  ", PCONT, PModel(), false, false},
  {"paragraph is a continuation from", PCONT, PModel(), false, false},
  {"a previous page, as it is       ", PCONT, PModel(), false, false},
  {"indented just like this second  ", PCONT, PModel(), false, false},
  {"paragraph.                      ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestFewCluesWithCrown) {
  TestParagraphDetection(kFewCluesWithCrown,
                         ABSL_ARRAYSIZE(kFewCluesWithCrown));
}

const TextAndModel kCrownedParagraph[] = {
  {"The first paragraph on a page is", PSTART, PModel(kLeft, 0, 20, 0, 0),
   true, false},
  {"often not indented as the rest  ", PCONT, PModel(), false, false},
  {"of the paragraphs are.  Nonethe-", PCONT, PModel(), false, false},
  {"less it should be counted as the", PCONT, PModel(), false, false},
  {"same type of paragraph.         ", PCONT, PModel(), false, false},
  {"  The second and third para-    ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"graphs are both indented two    ", PCONT, PModel(), false, false},
  {"spaces.                         ", PCONT, PModel(), false, false},
  {"  The first paragraph has what  ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"fmt refers to as a 'crown.'     ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestCrownParagraphDetection) {
  TestParagraphDetection(kCrownedParagraph, ABSL_ARRAYSIZE(kCrownedParagraph));
}

const TextAndModel kFlushLeftParagraphs[] = {
  {"It  is sometimes  the case  that", PSTART, PModel(kLeft, 0, 0, 0, 0), false, false},
  {"flush  left   paragraphs  (those", PCONT, PModel(), false, false},
  {"with  no  body  indent)  are not", PCONT, PModel(), false, false},
  {"actually crowns.                ", PCONT, PModel(), false, false},
  {"Instead,  further paragraphs are", PSTART, PModel(kLeft, 0, 0, 0, 0), false, false},
  {"also flush left aligned.  Usual-", PCONT, PModel(), false, false},
  {"ly,  these  paragraphs  are  set", PCONT, PModel(), false, false},
  {"apart vertically  by some white-", PCONT, PModel(), false, false},
  {"space,  but you can also  detect", PCONT, PModel(), false, false},
  {"them by observing  the big empty", PCONT, PModel(), false, false},
  {"space at the  ends  of the para-", PCONT, PModel(), false, false},
  {"graphs.                         ", PCONT, PModel(), false, false},
};

TEST(ParagraphsText, TestRealFlushLeftParagraphs) {
  TestParagraphDetection(kFlushLeftParagraphs,
                         ABSL_ARRAYSIZE(kFlushLeftParagraphs));
}

const TextAndModel kSingleFullPageContinuation[] = {
  {"sometimes a page is one giant", PSTART, PModel(kLeft, 0, 20, 0, 0), true, false},
  {"continuation.  It flows  from", PCONT, PModel(), false, false},
  {"line to  line, using the full", PCONT, PModel(), false, false},
  {"column  width  with  no clear", PCONT, PModel(), false, false},
  {"paragraph  break,  because it", PCONT, PModel(), false, false},
  {"actually doesn't have one. It", PCONT, PModel(), false, false},
  {"is the  middle of one monster", PCONT, PModel(), false, false},
  {"paragraph continued  from the", PCONT, PModel(), false, false},
  {"previous page and  continuing", PCONT, PModel(), false, false},
  {"onto the  next  page.  There-", PCONT, PModel(), false, false},
  {"fore,  it  ends  up   getting", PCONT, PModel(), false, false},
  {"marked  as a  crown  and then", PCONT, PModel(), false, false},
  {"getting re-marked as any  ex-", PCONT, PModel(), false, false},
  {"isting model.  Not great, but", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestSingleFullPageContinuation) {
  const TextAndModel* correct = kSingleFullPageContinuation;
  int num_rows = ABSL_ARRAYSIZE(kSingleFullPageContinuation);
  GenericVector<tesseract::RowInfo> row_infos;
  GenericVector<PARA*> row_owners;
  PARA_LIST paragraphs;
  GenericVector<ParagraphModel*> models;
  models.push_back(new ParagraphModel(kLeft, 0, 20, 0, 10));
  MakeAsciiRowInfos(correct, num_rows, &row_infos);
  tesseract::DetectParagraphs(3, &row_infos, &row_owners, &paragraphs, &models);
  EvaluateParagraphDetection(correct, num_rows, row_owners);
  models.delete_data_pointers();
}

const TextAndModel kRightAligned[] = {
  {"Right-aligned paragraphs are", PSTART, PModel(kRight, 0, 0, 0, 0), false, false},
  {"   uncommon in Left-to-Right", PCONT, PModel(), false, false},
  {"      languages, but they do", PCONT, PModel(), false, false},
  {"                      exist.", PCONT, PModel(), false, false},
  {"    Mostly, however, they're", PSTART, PModel(kRight, 0, 0, 0, 0), false, false},
  {" horribly tiny paragraphs in", PCONT, PModel(), false, false},
  {"  tables on which we have no", PCONT, PModel(), false, false},
  {"             chance anyways.", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestRightAlignedParagraph) {
  TestParagraphDetection(kRightAligned, ABSL_ARRAYSIZE(kRightAligned));
}

const TextAndModel kTinyParagraphs[] = {
  {"  Occasionally, interspersed with", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"obvious paragraph text, you might", PCONT, PModel(), false, false},
  {"find short exchanges of dialogue ", PCONT, PModel(), false, false},
  {"between characters.              ", PCONT, PModel(), false, false},
  {"  'Oh?'                          ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"  'Don't be confused!'           ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"  'Not me!'                      ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"  One naive approach would be to ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"mark a new paragraph whenever one", PCONT, PModel(), false, false},
  {"of the statistics (left, right or", PCONT, PModel(), false, false},
  {"center)  changes  from  one text-", PCONT, PModel(), false, false},
  {"line  to  the  next.    Such   an", PCONT, PModel(), false, false},
  {"approach  would  misclassify  the", PCONT, PModel(), false, false},
  {"tiny paragraphs above as a single", PCONT, PModel(), false, false},
  {"paragraph.                       ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestTinyParagraphs) {
  TestParagraphDetection(kTinyParagraphs, ABSL_ARRAYSIZE(kTinyParagraphs));
}

const TextAndModel kComplexPage1[] = {
  {"       Awesome                  ", PSTART, PModel(kCenter, 0, 0, 0, 0), false, false},
  {"   Centered Title               ", PCONT, PModel(), false, false},
  {" Paragraph Detection            ", PCONT, PModel(), false, false},
  {"      OCR TEAM                  ", PCONT, PModel(), false, false},
  {"  10 November 2010              ", PCONT, PModel(), false, false},
  {"                                ", PNONE, PModel(), false, false},
  {"  Look here, I have a paragraph.", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"This paragraph starts at the top", PCONT, PModel(), false, false},
  {"of the page and takes 3 lines.  ", PCONT, PModel(), false, false},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"which indicates that the first  ", PCONT, PModel(), false, false},
  {"paragraph is not a continuation ", PCONT, PModel(), false, false},
  {"from a previous page, as it is  ", PCONT, PModel(), false, false},
  {"indented just like this second  ", PCONT, PModel(), false, false},
  {"paragraph.                      ", PCONT, PModel(), false, false},
  {"   Here is a block quote. It    ", PSTART, PModel(kLeft, 30, 0, 0, 0),
   true, false},
  {"   looks like the prior text    ", PCONT, PModel(), false, false},
  {"   but it  is indented  more    ", PCONT, PModel(), false, false},
  {"   and is fully justified.      ", PCONT, PModel(), false, false},
  {"  So how does one deal with     ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"centered text, block quotes,    ", PCONT, PModel(), false, false},
  {"normal paragraphs, and lists    ", PCONT, PModel(), false, false},
  {"like what follows?              ", PCONT, PModel(), false, false},
  {"1. Make a plan.                 ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"2. Use a heuristic, for example,", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   looking for lines where the  ", PCONT, PModel(), false, false},
  {"   first word of the next line  ", PCONT, PModel(), false, false},
  {"   would fit on the previous    ", PCONT, PModel(), false, false},
  {"   line.                        ", PCONT, PModel(), false, false},
  {"8. Try to implement the plan in ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   Python and try it out.       ", PCONT, PModel(), false, false},
  {"4. Determine how to fix the     ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   mistakes.                    ", PCONT, PModel(), false, false},
  {"5. Repeat.                      ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"  For extra painful penalty work", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"you can try to identify source  ", PCONT, PModel(), false, false},
  {"code.  Ouch!                    ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestComplexPage1) {
  TestParagraphDetection(kComplexPage1, ABSL_ARRAYSIZE(kComplexPage1));
}

// The same as above, but wider.
const TextAndModel kComplexPage2[] = {
  {"       Awesome                     ", PSTART,
   PModel(kCenter, 0, 0, 0, 0), false, false},
  {"   Centered Title                  ", PCONT, PModel(), false, false},
  {" Paragraph Detection               ", PCONT, PModel(), false, false},
  {"      OCR TEAM                     ", PCONT, PModel(), false, false},
  {"  10 November 2010                 ", PCONT, PModel(), false, false},
  {"                                   ", PNONE, PModel(), false, false},
  {"  Look here, I have a paragraph.   ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"This paragraph starts at the top of", PCONT, PModel(), false, false},
  {"the page and takes 3 lines.        ", PCONT, PModel(), false, false},
  {"  Here I have a second paragraph   ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"which indicates that the first     ", PCONT, PModel(), false, false},
  {"paragraph is not a continuation    ", PCONT, PModel(), false, false},
  {"from a previous page, as it is in- ", PCONT, PModel(), false, false},
  {"dented just like this second para- ", PCONT, PModel(), false, false},
  {"graph.                             ", PCONT, PModel(), false, false},
  {"   Here is a block quote. It       ", PSTART, PModel(kLeft, 30, 0, 0, 0),
   true, false},
  {"   looks like the prior text       ", PCONT, PModel(), false, false},
  {"   but it  is indented  more       ", PCONT, PModel(), false, false},
  {"   and is fully justified.         ", PCONT, PModel(), false, false},
  {"  So how does one deal with center-", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"ed text, block quotes, normal para-", PCONT, PModel(), false, false},
  {"graphs, and lists like what follow?", PCONT, PModel(), false, false},
  {"1. Make a plan.                    ", PCONT, PModel(), false, false},  // BUG!!
  {"2. Use a heuristic, for example,   ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   looking for lines where the     ", PCONT, PModel(), false, false},
  {"   first word of the next line     ", PCONT, PModel(), false, false},
  {"   would fit on the previous line. ", PCONT, PModel(), false, false},
  {"8. Try to implement the plan in    ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   Python and try it out.          ", PCONT, PModel(), false, false},
  {"4. Determine how to fix the        ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"   mistakes.                       ", PCONT, PModel(), false, false},
  {"5. Repeat.                         ", PSTART, PModel(kLeft, 0, 0, 30, 0),
   false, true},
  {"  For extra painful penalty work   ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"you can try to identify source     ", PCONT, PModel(), false, false},
  {"code.  Ouch!                       ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestComplexPage2) {
  TestParagraphDetection(kComplexPage2, ABSL_ARRAYSIZE(kComplexPage2));
}

const TextAndModel kSubtleCrown[] = {
  {"The first paragraph on a page is", PSTART, PModel(kLeft, 0, 20, 0, 0),
   true, false},
  {"often not indented as the rest  ", PCONT, PModel(), false, false},
  {"of the paragraphs are.  Nonethe-", PCONT, PModel(), false, false},
  {"less it should be counted as the", PCONT, PModel(), false, false},
  {"same type of paragraph.         ", PCONT, PModel(), false, false},
  {"  Even a short second paragraph ", PSTART, PModel(kLeft, 0, 20, 0, 0), false, false},
  {"should suffice.                 ", PCONT, PModel(), false, false},
  {"             1235               ", PNONE, PModel(), false, false},
};

TEST(ParagraphsTest, TestSubtleCrown) {
  TestParagraphDetection(kSubtleCrown, ABSL_ARRAYSIZE(kSubtleCrown) - 1);
}

TEST(ParagraphsTest, TestStrayLineInBlock) {
  TestParagraphDetection(kSubtleCrown, ABSL_ARRAYSIZE(kSubtleCrown));
}

const TextAndModel kUnlvRep3AO[] = {
  {"    Defined contribution plans cover employees in Australia, New", PSTART,
   PModel(kLeft, 0, 50, 0, 0), false, false},
  {"Zealand, Spain, the United Kingdom and some U.S. subsidiaries.  ", PCONT, PModel(), false, false},
  {"In addition, employees in the U.S. are eligible to participate in    ", PCONT, PModel(), false, false},
  {"deﬁned contribution plans (Employee Savings Plans) by contribut-", PCONT, PModel(), false, false},
  {"ing a portion of their compensation. The Company matches com- ", PCONT, PModel(), false, false},
  {"pensation, depending on Company proﬁt levels. Contributions    ", PCONT, PModel(), false, false},
  {"charged to income for deﬁned contribution plans were $92 in    ", PCONT, PModel(), false, false},
  {"1993, $98 in 1992 and $89 in 1991.                             ", PCONT, PModel(), false, false},
  {"     In addition to providing pension beneﬁts, the Company pro- ", PSTART,
   PModel(kLeft, 0, 50, 0, 0), false, false},
  {"vides certain health care and life insurance beneﬁts to retired     ", PCONT, PModel(), false, false},
  {"employees. As discussed in Note A, the Company adopted FASB   ", PCONT, PModel(), false, false},
  {"Statement No. 106 effective January 1, 1992. Previously, the     ", PCONT, PModel(), false, false},
  {"Company recognized the cost of providing these beneﬁts as the     ", PCONT, PModel(), false, false},
  {"beneﬁts were paid. These pretax costs amounted to $53 in 1991.   ", PCONT, PModel(), false, false},
  {"The Company continues to fund most of the cost of these medical ", PCONT, PModel(), false, false},
  {"and life insurance beneﬁts in the year incurred.                ", PCONT, PModel(), false, false},
  {"     The U.S. plan covering the parent company is the largest plan.",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"It provides medical and life insurance beneﬁts including hospital,  ", PCONT, PModel(), false, false},
  {"physicians’ services and major medical expense beneﬁts and life   ", PCONT, PModel(), false, false},
  {"insurance beneﬁts. The plan provides beneﬁts supplemental to    ", PCONT, PModel(), false, false},
  {"Medicare after retirees are eligible for these beneﬁts. The cost of  ", PCONT, PModel(), false, false},
  {"these beneﬁts are shared by the Company and the retiree, with the  ", PCONT, PModel(), false, false},
  {"Company portion increasing as the retiree has increased years of   ", PCONT, PModel(), false, false},
  {"credited service. The Company has the ability to change these    ", PCONT, PModel(), false, false},
  {"beneﬁts at any time.                                            ", PCONT, PModel(), false, false},
  {"     Effective October 1993, the Company amended its health   ", PSTART,
   PModel(kLeft, 0, 50, 0, 0), false, false},
  {"beneﬁts plan in the U.S. to cap the cost absorbed by the Company ", PCONT, PModel(), false, false},
  {"at approximately twice the 1993 cost per person for employees who", PCONT, PModel(), false, false},
  {"retire after December 31, 1993. The effect of this amendment was ", PCONT, PModel(), false, false},
  {"to reduce the December 31, 1993 accumulated postretirement   ", PCONT, PModel(), false, false},
  {"beneﬁt obligation by $327. It also reduced the net periodic postre- ", PCONT, PModel(), false, false},
  {"tirement cost by $21 for 1993 and is estimated to reduce this cost  ", PCONT, PModel(), false, false},
  {"for 1994 by approximately $83.                                     ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, TestUnlvInsurance) {
  TestParagraphDetection(kUnlvRep3AO, ABSL_ARRAYSIZE(kUnlvRep3AO));
}

// The basic outcome we want for something with a bunch of leader dots is that
// we group each logical entry as a separate item.  Without knowledge of
// leaders, we would most likely mark the text below as a simple right aligned
// paragraph or two.
// This example comes from Volume 9886293, Page 5
const TextAndModel kTableOfContents[] = {
  {"1 Hmong People ........... 1", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"   Hmong Origins . . . . . 1", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"    Language . . . . . . . 1", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"     Proverbs . . . . . .  2", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"        Discussion . . . . 2", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"     Riddles . . . . . . . 2", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"        Discussion . . . . 3", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"     Appearance . . . . .  3", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"   Hmong History . . . . . 4", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"    Hmong in SE Asia . . . 4", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"    Hmong in the West . . .5", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"    Hmong in the USA . . . 5", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
  {"        Discussion . . . . 6", PSTART, PModel(kUnknown, 0, 0, 0, 0), false, false},
};

TEST(ParagraphsTest, TestSplitsOutLeaderLines) {
  TestParagraphDetection(kTableOfContents, ABSL_ARRAYSIZE(kTableOfContents));
}

const TextAndModel kTextWithSourceCode[] = {
  {"  A typical page of a programming book may contain", PSTART,
   PModel(kLeft, 0, 20, 0, 0), false, false},
  {"examples of source code to exemplify an algorithm ", PCONT, PModel(), false, false},
  {"being described in prose.  Such examples should be", PCONT, PModel(), false, false},
  {"rendered as lineated text, meaning text with      ", PCONT, PModel(), false, false},
  {"explicit line breaks but without extra inter-line ", PCONT, PModel(), false, false},
  {"spacing.  Accidentally finding stray paragraphs in", PCONT, PModel(), false, false},
  {"source code would lead to a bad reading experience", PCONT, PModel(), false, false},
  {"when the text is re-flowed.                       ", PCONT, PModel(), false, false},
  {"  Let's show this by describing the function fact-", PSTART,
   PModel(kLeft, 0, 20, 0, 0), false, false},
  {"orial.  Factorial is a simple recursive function  ", PCONT, PModel(), false, false},
  {"which grows very quickly.  So quickly, in fact,   ", PCONT, PModel(), false, false},
  {"that the typical C implementation will only work  ", PCONT, PModel(), false, false},
  {"for values less than about 12:                    ", PCONT, PModel(), false, false},
  {"                                                  ", PNONE, PModel(), false, false},
  {"  # Naive implementation in C                     ", PCONT, PModel(), false, false},
  {"  int factorial(int n) {                          ", PCONT, PModel(), false, false},
  {"    if (n < 2)                                    ", PCONT, PModel(), false, false},
  {"      return 1;                                   ", PCONT, PModel(), false, false},
  {"    return  n * factorial(n - 1);                 ", PCONT, PModel(), false, false},
  {"  }                                               ", PCONT, PModel(), false, false},
  {"                                                  ", PCONT, PModel(), false, false},
  {"  The C programming language does not have built- ", PSTART,
   PModel(kLeft, 0, 20, 0, 0), false, false},
  {"in support for detecting integer overflow, so this", PCONT, PModel(), false, false},
  {"naive implementation simply returns random values ", PCONT, PModel(), false, false},
  {"if even a moderate sized n is provided.           ", PCONT, PModel(), false, false},
};

TEST(ParagraphsTest, NotDistractedBySourceCode) {
  TestParagraphDetection(kTextWithSourceCode,
                         ABSL_ARRAYSIZE(kTextWithSourceCode));
}

const TextAndModel kOldManAndSea[] = {
  {"royal  palm  which  are called  guano  and  in it  there was a bed,  a",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"table, one chair, and a place on the dirt floor to cook with charcoal.", PCONT, PModel(), false, false},
  {"On  the  brown  walls  of  the ﬂattened,  overlapping  leaves  of  the", PCONT, PModel(), false, false},
  {"sturdy  fibered guano  there  was  a  picture in  color of  the Sacred", PCONT, PModel(), false, false},
  {"Heart  of  Jesus  and  another  of  the  Virgin  of Cobre.  These were", PCONT, PModel(), false, false},
  {"relics of  his wife.   Once there had been  a tinted photograph of his", PCONT, PModel(), false, false},
  {"wife on  the wall  but he  had taken  it  down because it made him too", PCONT, PModel(), false, false},
  {"lonely to see it and it was on the shelf in the corner under his clean", PCONT, PModel(), false, false},
  {"shirt.                                                                ", PCONT, PModel(), false, false},
  {"     \"What  do  you  have  to  eat?\"     the  boy   asked.          ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"A pot of yellow rice with fish. Do you want some?\"            ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"No. I will eat at home. Do you want me to make the fire?\"   ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"No. I will make it later on. Or I may eat the rice cold.\"     ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"May I take the cast net?\"                                     ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"Of course.\"                                                   ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     There was  no  cast net  and  the boy  remembered  when  they had",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"sold it.   But they went through  this fiction every day. There was no", PCONT, PModel(), false, false},
  {"pot of yellow rice and fish and the boy knew this too.                 "
   " ", PCONT, PModel(), false, false},
  {"     \"Eighty-five  is a lucky number,\"  the  old  man  said.   \"How",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"would  you  like to see  me  bring one  in that dressed out over a "
   "thou-", PCONT, PModel(), false, false},
  {"sand pounds?                                                           "
   " ", PCONT, PModel(), false, false},
  {"     \"I'll get the cast net and go for sardines.  Will you sit in the "
   "sun",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"in the doorway?\"                                                        "
   " ", PCONT, PModel(), false, false},
  {"     \"Yes.  I have yesterday's paper and I will read the baseball.\"   ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     The boy  did not  know  whether  yesterday's paper  was a fiction",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"too.  But the old man brought it out from under the bed.              ", PCONT, PModel(), false, false},
  {"     \"Pedrico gave it to me at the bodega,\" he explained.             "
   " ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"I'll be back when I have the sardines.  I'll keep yours and mine",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"together  on ice  and  we  can  share  them  in the  morning.   When I", PCONT, PModel(), false, false},
  {"come back you can tell me about the baseball.\"                       ", PCONT, PModel(), false, false},
  {"     \"The Yankees cannot lose.\"                                     ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"But I fear the Indians of Cleveland.\"                         ",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"     \"Have faith  in  the Yankees  my son.   Think of  the great  Di-",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"Maggio.\"                                                             ", PCONT, PModel(), false, false},
  {"     \"I  fear both  the Tigers of Detroit  and the  Indians of Cleve-",
   PSTART, PModel(kLeft, 0, 50, 0, 0), false, false},
  {"land.\"                                                               ", PCONT, PModel(), false, false}
};

TEST(ParagraphsTest, NotOverlyAggressiveWithBlockQuotes) {
  TestParagraphDetection(kOldManAndSea, ABSL_ARRAYSIZE(kOldManAndSea));
}

const TextAndModel kNewZealandIndex[] = {
  {"Oats, 51                      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"O'Brien, Gregory, 175         ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Occupational composition, 110,", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"   138                        ", PCONT, PModel(), false, false},
  {"OECD rankings, 155, 172       ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Okiato (original capital), 47 ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Oil shock: 1974, xxx, 143; 1979,", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"   145                        ", PCONT, PModel(), false, false},
  {"Old Age Pensions, xxii, 89-90 ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Old World evils, 77           ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Oliver, W. H., 39, 77, 89     ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Olssen, Erik, 45, 64, 84      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Olympic Games, 1924, 111, 144 ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Once on Chunuk Bair, 149      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Once Were Warriors, xxxiii, 170", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"On—shore whaling, xvi         ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Opotiki, xix                  ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Orakau battle of, xviii, 57   ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"O’Regan, Tipene, 170, 198-99  ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Organic agriculture, 177      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Orwell, George, 151           ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago, xvii, 45, 49-50, 70    ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago block, xvii             ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago Daily Times, 67         ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago Girls’ High School, xix, 61,", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"   85                         ", PCONT, PModel(), false, false},
  {"Otago gold rushes, 61-63      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago Peninsula, xx           ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otago Provincial Council, 68  ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Otaki, 33                     ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false},
  {"Owls Do Cry, 139              ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, false}
};

TEST(ParagraphsTest, IndexPageTest) {
  TestParagraphDetection(kNewZealandIndex, ABSL_ARRAYSIZE(kNewZealandIndex));
}

// TODO(eger): Add some right-to-left examples, and fix the algorithm as needed.

}  // namespace
