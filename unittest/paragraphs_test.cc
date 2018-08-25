
#include <string>

#include "tesseract/ccmain/paragraphs.h"
#include "tesseract/ccmain/paragraphs_internal.h"
#include "tesseract/ccstruct/ocrpara.h"

namespace {  // anonymous namespace

// Functions for making monospace ASCII trial text for the paragraph detector.
const tesseract::ParagraphJustification kLeft = tesseract::JUSTIFICATION_LEFT;
const tesseract::ParagraphJustification kCenter
    = tesseract::JUSTIFICATION_CENTER;
const tesseract::ParagraphJustification kRight
    = tesseract::JUSTIFICATION_RIGHT;
const tesseract::ParagraphJustification kUnknown
    = tesseract::JUSTIFICATION_UNKNOWN;

enum TextModelInputType {
  PCONT = 0,   // Continuation line of a paragraph (default).
  PSTART = 1,  // First line of a paragraph.
  PNONE = 2,   // Not a paragraph line.
};

struct TextAndModel {
  const char *ascii;
  TextModelInputType model_type;

  // fields corresponding to PARA (see ccstruct/ocrpara.h)
  ParagraphModel model;
  bool is_very_first_or_continuation;
  bool is_list_item;
};

// Imagine that the given text is typewriter ASCII with each character ten
// pixels wide and twenty pixels high and return an appropriate row_info.
void AsciiToRowInfo(const char *text, int row_number,
                    tesseract::RowInfo *info) {
  const int kCharWidth = 10;
  const int kLineSpace = 30;
  info->text = text;
  info->has_leaders = strstr(text, "...") != NULL ||
      strstr(text, ". . .") != NULL;
  info->has_drop_cap = false;
  info->pix_ldistance = info->pix_rdistance = 0;
  info->average_interword_space = kCharWidth;
  info->pix_xheight = kCharWidth;
  info->lword_text = info->rword_text = "";
  info->ltr = true;

  std::vector<string> words = absl::StrSplit(text, ' ', absl::SkipEmpty());
  info->num_words = words.size();
  if (info->num_words < 1)
    return;

  info->lword_text = words[0].c_str();
  info->rword_text = words[words.size() - 1].c_str();
  int lspace = 0;
  while (lspace < info->text.size() && text[lspace] == ' ') { lspace++; }
  int rspace = 0;
  while (rspace < info->text.size() &&
         text[info->text.size() - rspace - 1] == ' ') {
    rspace++;
  }

  int top = - kLineSpace * row_number;
  int bottom = top - kLineSpace;
  int row_right = kCharWidth * info->text.size();
  int lword_width = kCharWidth * info->lword_text.size();
  int rword_width = kCharWidth * info->rword_text.size();
  info->pix_ldistance = lspace * kCharWidth;
  info->pix_rdistance = rspace * kCharWidth;
  info->lword_box =
      TBOX(info->pix_ldistance, bottom,
           info->pix_ldistance + lword_width, top);
  info->rword_box =
      TBOX(row_right - info->pix_rdistance - rword_width, bottom,
           row_right - info->pix_rdistance, top);
  tesseract::LeftWordAttributes(
      NULL, NULL, info->lword_text,
      &info->lword_indicates_list_item,
      &info->lword_likely_starts_idea,
      &info->lword_likely_ends_idea);
  tesseract::RightWordAttributes(
      NULL, NULL, info->rword_text,
      &info->rword_indicates_list_item,
      &info->rword_likely_starts_idea,
      &info->rword_likely_ends_idea);
}

void MakeAsciiRowInfos(const TextAndModel *row_infos, int n,
                       GenericVector<tesseract::RowInfo> *output) {
  output->clear();
  tesseract::RowInfo info;
  for (int i = 0; i < n; i++) {
    AsciiToRowInfo(row_infos[i].ascii, i, &info);
    output->push_back(info);
  }
}

// Given n rows of reference ground truth, evaluate whether the n rows
// of PARA * pointers yield the same paragraph breakpoints.
void EvaluateParagraphDetection(const TextAndModel *correct, int n,
                                const GenericVector<PARA *> &detector_output) {
  int incorrect_breaks = 0;
  int missed_breaks = 0;
  int poorly_matched_models = 0;
  int bad_crowns = 0;
  int bad_list_items = 0;
  ASSERT_EQ(detector_output.size(), n);
  for (int i = 1; i < n; i++) {
    bool has_break = correct[i].model_type != PCONT;
    bool detected_break = (detector_output[i - 1] != detector_output[i]);
    if (has_break && !detected_break)
      missed_breaks++;
    if (detected_break && !has_break)
      incorrect_breaks++;
    if (has_break) {
      if (correct[i].model_type == PNONE) {
        if (detector_output[i]->model != NULL) {
          poorly_matched_models++;
        }
      } else {
        if (correct[i].model.justification() != kUnknown &&
            (detector_output[i]->model == NULL ||
             !correct[i].model.Comparable(*detector_output[i]->model))) {
          poorly_matched_models++;
        }
      }
      if (correct[i].is_very_first_or_continuation ^
          detector_output[i]->is_very_first_or_continuation) {
        bad_crowns++;
      }
      if (correct[i].is_list_item ^
          detector_output[i]->is_list_item) {
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
    std::vector<string> dbg_lines;
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
      string annotation;
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

void TestParagraphDetection(const TextAndModel *correct, int num_rows) {
  GenericVector<tesseract::RowInfo> row_infos;
  GenericVector<PARA *> row_owners;
  PARA_LIST paragraphs;
  GenericVector<ParagraphModel *> models;

  MakeAsciiRowInfos(correct, num_rows, &row_infos);
  int debug_level(3);
  tesseract::DetectParagraphs(debug_level, &row_infos, &row_owners,
                              &paragraphs, &models);
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
  {"  Look here, I have a paragraph.", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"This paragraph starts at the top"},
  {"of the page and takes 3 lines.  "},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"which indicates that the first  "},
  {"paragraph is not a continuation "},
  {"from a previous page, as it is  "},
  {"indented just like this second  "},
  {"paragraph.                      "},
};

TEST(ParagraphsTest, TestSimpleParagraphDetection) {
  TestParagraphDetection(kTwoSimpleParagraphs,
                         ABSL_ARRAYSIZE(kTwoSimpleParagraphs));
}

const TextAndModel kFewCluesWithCrown[] = {
  {"This paragraph starts at the top", PSTART, PModel(kLeft, 0, 20, 0, 0), true},
  {"of the page and takes two lines."},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"which indicates that the first  "},
  {"paragraph is a continuation from"},
  {"a previous page, as it is       "},
  {"indented just like this second  "},
  {"paragraph.                      "},
};


TEST(ParagraphsTest, TestFewCluesWithCrown) {
  TestParagraphDetection(kFewCluesWithCrown,
                         ABSL_ARRAYSIZE(kFewCluesWithCrown));
}

const TextAndModel kCrownedParagraph[] = {
  {"The first paragraph on a page is", PSTART, PModel(kLeft, 0, 20, 0, 0), true},
  {"often not indented as the rest  "},
  {"of the paragraphs are.  Nonethe-"},
  {"less it should be counted as the"},
  {"same type of paragraph.         "},
  {"  The second and third para-    ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"graphs are both indented two    "},
  {"spaces.                         "},
  {"  The first paragraph has what  ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"fmt refers to as a 'crown.'     "},
};

TEST(ParagraphsTest, TestCrownParagraphDetection) {
  TestParagraphDetection(kCrownedParagraph, ABSL_ARRAYSIZE(kCrownedParagraph));
}

const TextAndModel kFlushLeftParagraphs[] = {
  {"It  is sometimes  the case  that", PSTART, PModel(kLeft, 0, 0, 0, 0)},
  {"flush  left   paragraphs  (those"},
  {"with  no  body  indent)  are not"},
  {"actually crowns.                "},
  {"Instead,  further paragraphs are", PSTART, PModel(kLeft, 0, 0, 0, 0)},
  {"also flush left aligned.  Usual-"},
  {"ly,  these  paragraphs  are  set"},
  {"apart vertically  by some white-"},
  {"space,  but you can also  detect"},
  {"them by observing  the big empty"},
  {"space at the  ends  of the para-"},
  {"graphs.                         "},
};

TEST(ParagraphsText, TestRealFlushLeftParagraphs) {
  TestParagraphDetection(kFlushLeftParagraphs,
                         ABSL_ARRAYSIZE(kFlushLeftParagraphs));
};

const TextAndModel kSingleFullPageContinuation[] = {
  {"sometimes a page is one giant", PSTART, PModel(kLeft, 0, 20, 0, 0), true},
  {"continuation.  It flows  from"},
  {"line to  line, using the full"},
  {"column  width  with  no clear"},
  {"paragraph  break,  because it"},
  {"actually doesn't have one. It"},
  {"is the  middle of one monster"},
  {"paragraph continued  from the"},
  {"previous page and  continuing"},
  {"onto the  next  page.  There-"},
  {"fore,  it  ends  up   getting"},
  {"marked  as a  crown  and then"},
  {"getting re-marked as any  ex-"},
  {"isting model.  Not great, but"},
};

TEST(ParagraphsTest, TestSingleFullPageContinuation) {
  const TextAndModel *correct = kSingleFullPageContinuation;
  int num_rows = ABSL_ARRAYSIZE(kSingleFullPageContinuation);
  GenericVector<tesseract::RowInfo> row_infos;
  GenericVector<PARA *> row_owners;
  PARA_LIST paragraphs;
  GenericVector<ParagraphModel *> models;
  models.push_back(new ParagraphModel(kLeft, 0, 20, 0, 10));
  MakeAsciiRowInfos(correct, num_rows, &row_infos);
  tesseract::DetectParagraphs(3, &row_infos, &row_owners, &paragraphs,
                              &models);
  EvaluateParagraphDetection(correct, num_rows, row_owners);
  models.delete_data_pointers();
}

const TextAndModel kRightAligned[] = {
  {"Right-aligned paragraphs are", PSTART, PModel(kRight, 0, 0, 0, 0)},
  {"   uncommon in Left-to-Right"},
  {"      languages, but they do"},
  {"                      exist."},
  {"    Mostly, however, they're", PSTART, PModel(kRight, 0, 0, 0, 0)},
  {" horribly tiny paragraphs in"},
  {"  tables on which we have no"},
  {"             chance anyways."},
};

TEST(ParagraphsTest, TestRightAlignedParagraph) {
  TestParagraphDetection(kRightAligned, ABSL_ARRAYSIZE(kRightAligned));
}

const TextAndModel kTinyParagraphs[] = {
  {"  Occasionally, interspersed with", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"obvious paragraph text, you might"},
  {"find short exchanges of dialogue "},
  {"between characters.              "},
  {"  'Oh?'                          ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"  'Don't be confused!'           ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"  'Not me!'                      ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"  One naive approach would be to ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"mark a new paragraph whenever one"},
  {"of the statistics (left, right or"},
  {"center)  changes  from  one text-"},
  {"line  to  the  next.    Such   an"},
  {"approach  would  misclassify  the"},
  {"tiny paragraphs above as a single"},
  {"paragraph.                       "},
};

TEST(ParagraphsTest, TestTinyParagraphs) {
  TestParagraphDetection(kTinyParagraphs, ABSL_ARRAYSIZE(kTinyParagraphs));
}


const TextAndModel kComplexPage1[] = {
  {"       Awesome                  ", PSTART, PModel(kCenter, 0, 0, 0, 0)},
  {"   Centered Title               "},
  {" Paragraph Detection            "},
  {"      OCR TEAM                  "},
  {"  10 November 2010              "},
  {"                                ", PNONE},
  {"  Look here, I have a paragraph.", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"This paragraph starts at the top"},
  {"of the page and takes 3 lines.  "},
  {"  Here I have a second paragraph", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"which indicates that the first  "},
  {"paragraph is not a continuation "},
  {"from a previous page, as it is  "},
  {"indented just like this second  "},
  {"paragraph.                      "},
  {"   Here is a block quote. It    ", PSTART, PModel(kLeft, 30, 0, 0, 0), true},
  {"   looks like the prior text    "},
  {"   but it  is indented  more    "},
  {"   and is fully justified.      "},
  {"  So how does one deal with     ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"centered text, block quotes,    "},
  {"normal paragraphs, and lists    "},
  {"like what follows?              "},
  {"1. Make a plan.                 ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"2. Use a heuristic, for example,", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   looking for lines where the  "},
  {"   first word of the next line  "},
  {"   would fit on the previous    "},
  {"   line.                        "},
  {"8. Try to implement the plan in ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   Python and try it out.       "},
  {"4. Determine how to fix the     ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   mistakes.                    "},
  {"5. Repeat.                      ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"  For extra painful penalty work", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"you can try to identify source  "},
  {"code.  Ouch!                    "},
};

TEST(ParagraphsTest, TestComplexPage1) {
  TestParagraphDetection(kComplexPage1, ABSL_ARRAYSIZE(kComplexPage1));
}

// The same as above, but wider.
const TextAndModel kComplexPage2[] = {
  {"       Awesome                     ", PSTART, PModel(kCenter, 0, 0, 0, 0)},
  {"   Centered Title                  "},
  {" Paragraph Detection               "},
  {"      OCR TEAM                     "},
  {"  10 November 2010                 "},
  {"                                   ", PNONE},
  {"  Look here, I have a paragraph.   ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"This paragraph starts at the top of"},
  {"the page and takes 3 lines.        "},
  {"  Here I have a second paragraph   ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"which indicates that the first     "},
  {"paragraph is not a continuation    "},
  {"from a previous page, as it is in- "},
  {"dented just like this second para- "},
  {"graph.                             "},
  {"   Here is a block quote. It       ", PSTART, PModel(kLeft, 30, 0, 0, 0), true},
  {"   looks like the prior text       "},
  {"   but it  is indented  more       "},
  {"   and is fully justified.         "},
  {"  So how does one deal with center-", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"ed text, block quotes, normal para-"},
  {"graphs, and lists like what follow?"},
  {"1. Make a plan.                    "},  // BUG!!
  {"2. Use a heuristic, for example,   ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   looking for lines where the     "},
  {"   first word of the next line     "},
  {"   would fit on the previous line. "},
  {"8. Try to implement the plan in    ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   Python and try it out.          "},
  {"4. Determine how to fix the        ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"   mistakes.                       "},
  {"5. Repeat.                         ", PSTART, PModel(kLeft, 0, 0, 30, 0), false, true},
  {"  For extra painful penalty work   ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"you can try to identify source     "},
  {"code.  Ouch!                       "},
};

TEST(ParagraphsTest, TestComplexPage2) {
  TestParagraphDetection(kComplexPage2, ABSL_ARRAYSIZE(kComplexPage2));
}

const TextAndModel kSubtleCrown[] = {
  {"The first paragraph on a page is", PSTART, PModel(kLeft, 0, 20, 0, 0), true},
  {"often not indented as the rest  "},
  {"of the paragraphs are.  Nonethe-"},
  {"less it should be counted as the"},
  {"same type of paragraph.         "},
  {"  Even a short second paragraph ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"should suffice.                 "},
  {"             1235               ", PNONE},
};

TEST(ParagraphsTest, TestSubtleCrown) {
  TestParagraphDetection(kSubtleCrown, ABSL_ARRAYSIZE(kSubtleCrown) - 1);
}

TEST(ParagraphsTest, TestStrayLineInBlock) {
  TestParagraphDetection(kSubtleCrown, ABSL_ARRAYSIZE(kSubtleCrown));
}

const TextAndModel kUnlvRep3AO[] = {
  {"    Defined contribution plans cover employees in Australia, New", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"Zealand, Spain, the United Kingdom and some U.S. subsidiaries.  "},
  {"In addition, employees in the U.S. are eligible to participate in    "},
  {"deﬁned contribution plans (Employee Savings Plans) by contribut-"},
  {"ing a portion of their compensation. The Company matches com- "},
  {"pensation, depending on Company proﬁt levels. Contributions    "},
  {"charged to income for deﬁned contribution plans were $92 in    "},
  {"1993, $98 in 1992 and $89 in 1991.                             "},
  {"     In addition to providing pension beneﬁts, the Company pro- ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"vides certain health care and life insurance beneﬁts to retired     "},
  {"employees. As discussed in Note A, the Company adopted FASB   "},
  {"Statement No. 106 effective January 1, 1992. Previously, the     "},
  {"Company recognized the cost of providing these beneﬁts as the     "},
  {"beneﬁts were paid. These pretax costs amounted to $53 in 1991.   "},
  {"The Company continues to fund most of the cost of these medical "},
  {"and life insurance beneﬁts in the year incurred.                "},
  {"     The U.S. plan covering the parent company is the largest plan.", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"It provides medical and life insurance beneﬁts including hospital,  "},
  {"physicians’ services and major medical expense beneﬁts and life   "},
  {"insurance beneﬁts. The plan provides beneﬁts supplemental to    "},
  {"Medicare after retirees are eligible for these beneﬁts. The cost of  "},
  {"these beneﬁts are shared by the Company and the retiree, with the  "},
  {"Company portion increasing as the retiree has increased years of   "},
  {"credited service. The Company has the ability to change these    "},
  {"beneﬁts at any time.                                            "},
  {"     Effective October 1993, the Company amended its health   ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"beneﬁts plan in the U.S. to cap the cost absorbed by the Company "},
  {"at approximately twice the 1993 cost per person for employees who"},
  {"retire after December 31, 1993. The effect of this amendment was "},
  {"to reduce the December 31, 1993 accumulated postretirement   "},
  {"beneﬁt obligation by $327. It also reduced the net periodic postre- "},
  {"tirement cost by $21 for 1993 and is estimated to reduce this cost  "},
  {"for 1994 by approximately $83.                                     "},
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
  {"1 Hmong People ........... 1", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"   Hmong Origins . . . . . 1", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"    Language . . . . . . . 1", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"     Proverbs . . . . . .  2", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"        Discussion . . . . 2", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"     Riddles . . . . . . . 2", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"        Discussion . . . . 3", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"     Appearance . . . . .  3", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"   Hmong History . . . . . 4", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"    Hmong in SE Asia . . . 4", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"    Hmong in the West . . .5", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"    Hmong in the USA . . . 5", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
  {"        Discussion . . . . 6", PSTART, PModel(kUnknown, 0, 0, 0, 0)},
};

TEST(ParagraphsTest, TestSplitsOutLeaderLines) {
  TestParagraphDetection(kTableOfContents, ABSL_ARRAYSIZE(kTableOfContents));
}

const TextAndModel kTextWithSourceCode[] = {
  {"  A typical page of a programming book may contain", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"examples of source code to exemplify an algorithm "},
  {"being described in prose.  Such examples should be"},
  {"rendered as lineated text, meaning text with      "},
  {"explicit line breaks but without extra inter-line "},
  {"spacing.  Accidentally finding stray paragraphs in"},
  {"source code would lead to a bad reading experience"},
  {"when the text is re-flowed.                       "},
  {"  Let's show this by describing the function fact-", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"orial.  Factorial is a simple recursive function  "},
  {"which grows very quickly.  So quickly, in fact,   "},
  {"that the typical C implementation will only work  "},
  {"for values less than about 12:                    "},
  {"                                                  ", PNONE},
  {"  # Naive implementation in C                     "},
  {"  int factorial(int n) {                          "},
  {"    if (n < 2)                                    "},
  {"      return 1;                                   "},
  {"    return  n * factorial(n - 1);                 "},
  {"  }                                               "},
  {"                                                  "},
  {"  The C programming language does not have built- ", PSTART, PModel(kLeft, 0, 20, 0, 0)},
  {"in support for detecting integer overflow, so this"},
  {"naive implementation simply returns random values "},
  {"if even a moderate sized n is provided.           "},
};

TEST(ParagraphsTest, NotDistractedBySourceCode) {
  TestParagraphDetection(kTextWithSourceCode,
                         ABSL_ARRAYSIZE(kTextWithSourceCode));
}

const TextAndModel kOldManAndSea[] = {
  {"royal  palm  which  are called  guano  and  in it  there was a bed,  a", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"table, one chair, and a place on the dirt floor to cook with charcoal."},
  {"On  the  brown  walls  of  the ﬂattened,  overlapping  leaves  of  the"},
  {"sturdy  fibered guano  there  was  a  picture in  color of  the Sacred"},
  {"Heart  of  Jesus  and  another  of  the  Virgin  of Cobre.  These were"},
  {"relics of  his wife.   Once there had been  a tinted photograph of his"},
  {"wife on  the wall  but he  had taken  it  down because it made him too"},
  {"lonely to see it and it was on the shelf in the corner under his clean"},
  {"shirt.                                                                "},
  {"     \"What  do  you  have  to  eat?\"     the  boy   asked.          ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"A pot of yellow rice with fish. Do you want some?\"            ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"No. I will eat at home. Do you want me to make the fire?\"   ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"No. I will make it later on. Or I may eat the rice cold.\"     ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"May I take the cast net?\"                                     ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"Of course.\"                                                   ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     There was  no  cast net  and  the boy  remembered  when  they had", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"sold it.   But they went through  this fiction every day. There was no"},
  {"pot of yellow rice and fish and the boy knew this too.                  "},
  {"     \"Eighty-five  is a lucky number,\"  the  old  man  said.   \"How", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"would  you  like to see  me  bring one  in that dressed out over a thou-"},
  {"sand pounds?                                                            "},
  {"     \"I'll get the cast net and go for sardines.  Will you sit in the sun", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"in the doorway?\"                                                         "},
  {"     \"Yes.  I have yesterday's paper and I will read the baseball.\"   ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     The boy  did not  know  whether  yesterday's paper  was a fiction", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"too.  But the old man brought it out from under the bed.              "},
  {"     \"Pedrico gave it to me at the bodega,\" he explained.              ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"I'll be back when I have the sardines.  I'll keep yours and mine", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"together  on ice  and  we  can  share  them  in the  morning.   When I"},
  {"come back you can tell me about the baseball.\"                       "},
  {"     \"The Yankees cannot lose.\"                                     ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"But I fear the Indians of Cleveland.\"                         ", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"     \"Have faith  in  the Yankees  my son.   Think of  the great  Di-", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"Maggio.\"                                                             "},
  {"     \"I  fear both  the Tigers of Detroit  and the  Indians of Cleve-", PSTART, PModel(kLeft, 0, 50, 0, 0)},
  {"land.\"                                                               "}
};

TEST(ParagraphsTest, NotOverlyAggressiveWithBlockQuotes) {
  TestParagraphDetection(kOldManAndSea, ABSL_ARRAYSIZE(kOldManAndSea));
}

const TextAndModel kNewZealandIndex[] = {
  {"Oats, 51                      ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"O'Brien, Gregory, 175         ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Occupational composition, 110,", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"   138                        "},
  {"OECD rankings, 155, 172       ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Okiato (original capital), 47 ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Oil shock: 1974, xxx, 143; 1979,", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"   145                        "},
  {"Old Age Pensions, xxii, 89-90 ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Old World evils, 77           ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Oliver, W. H., 39, 77, 89     ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Olssen, Erik, 45, 64, 84      ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Olympic Games, 1924, 111, 144 ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Once on Chunuk Bair, 149      ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Once Were Warriors, xxxiii, 170", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"On—shore whaling, xvi         ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Opotiki, xix                  ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Orakau battle of, xviii, 57   ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"O’Regan, Tipene, 170, 198-99  ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Organic agriculture, 177      ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Orwell, George, 151           ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago, xvii, 45, 49-50, 70    ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago block, xvii             ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago Daily Times, 67         ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago Girls’ High School, xix, 61,", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"   85                         "},
  {"Otago gold rushes, 61-63      ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago Peninsula, xx           ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otago Provincial Council, 68  ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Otaki, 33                     ", PSTART, PModel(kLeft, 0, 0, 30, 0)},
  {"Owls Do Cry, 139              ", PSTART, PModel(kLeft, 0, 0, 30, 0)}
};

TEST(ParagraphsTest, IndexPageTest) {
  TestParagraphDetection(kNewZealandIndex, ABSL_ARRAYSIZE(kNewZealandIndex));
}

// TOOO(eger): Add some right-to-left examples, and fix the algorithm as needed.

}  // namespace
