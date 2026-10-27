// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pageres.h"

#include "boxword.h"
#include "ocrblock.h"
#include "ocrrow.h"
#include "rect.h"
#include "stepblob.h"
#include "unicharset.h"
#include "werd.h"

#include "include_gunit.h"

#include <set>
#include <vector>

namespace tesseract {

namespace {

// Makes a word of rectangular blobs, one per box.
WERD *MakeWord(const std::vector<TBOX> &boxes) {
  C_BLOB_LIST blobs;
  C_BLOB_IT b_it(&blobs);
  for (const auto &box : boxes) {
    b_it.add_after_then_move(C_BLOB::FakeBlob(box));
  }
  return new WERD(&blobs, 1, nullptr);
}

// Adds a block to blocks, with one row holding the given words.
void AddRow(BLOCK_LIST *blocks, const std::vector<WERD *> &words) {
  int32_t xstarts[] = {0, 200};
  double coeffs[] = {0.0, 0.0, 10.0};
  auto *row = new ROW(1, xstarts, coeffs, 20.0f, 10.0f, -5.0f, 0, 10);
  WERD_IT w_it(row->word_list());
  for (auto *word : words) {
    w_it.add_to_end(word);
  }
  row->recalc_bounding_box();
  auto *block = new BLOCK("", true, 0, 0, 0, 0, 200, 50);
  ROW_IT row_it(block->row_list());
  row_it.add_to_end(row);
  BLOCK_IT block_it(blocks);
  block_it.add_to_end(block);
}

// Makes a word as the LSTM recognizer gives it: a combination that owns a
// word with one fake blob per character.
WERD_RES *MakeLSTMWord(const std::vector<TBOX> &boxes,
                       const UNICHARSET &unicharset) {
  auto *word_res = new WERD_RES(MakeWord(boxes));
  word_res->combination = true;
  word_res->SetupFake(unicharset);
  return word_res;
}

// Makes the word a language gives back when its LSTM recognizer finds no
// words: its copy of the input word (WordData::lang_words), with the fake
// result of WERD_RES::SetupFake.
WERD_RES *MakeWholeWord(const WERD_RES &input, const UNICHARSET &unicharset) {
  auto *word_res = new WERD_RES;
  word_res->InitForRetryRecognition(input);
  word_res->SetupFake(unicharset);
  return word_res;
}

// Checks that each word of the row has one owner: a combination owns its
// word, any other word is on the row's word list once, and no two words of
// the row share a WERD.
void ExpectEachWordOwnedOnce(ROW_RES *row_res) {
  WERD_LIST *row_words = row_res->row->word_list();
  std::set<const WERD *> seen;
  int non_combinations = 0;
  WERD_RES_IT wr_it(&row_res->word_res_list);
  for (wr_it.mark_cycle_pt(); !wr_it.cycled_list(); wr_it.forward()) {
    const WERD_RES *word_res = wr_it.data();
    EXPECT_TRUE(seen.insert(word_res->word).second);
    int on_row = 0;
    WERD_IT w_it(row_words);
    for (w_it.mark_cycle_pt(); !w_it.cycled_list(); w_it.forward()) {
      if (w_it.data() == word_res->word) {
        ++on_row;
      }
    }
    if (word_res->combination) {
      EXPECT_EQ(0, on_row);
    } else {
      EXPECT_EQ(1, on_row);
      ++non_combinations;
    }
  }
  EXPECT_EQ(non_combinations, row_words->length());
}

const std::vector<TBOX> kInputBlobs = {
    TBOX(10, 10, 20, 30), TBOX(25, 10, 35, 30), TBOX(40, 10, 50, 30)};

// With several languages, classify_word_and_language can replace a word
// with LSTM words of one language mixed with the whole input word of
// another, which shares the input WERD. ReplaceCurrentWord must not link
// that WERD into the row a second time and then delete it.
TEST(PageResTest, ReplaceCurrentWordCopiesSharedInputWord) {
  UNICHARSET unicharset;
  BLOCK_LIST blocks;
  WERD *input_werd = MakeWord(kInputBlobs);
  input_werd->set_flag(W_BOL, true);
  input_werd->set_flag(W_EOL, true);
  AddRow(&blocks, {input_werd});
  PAGE_RES page_res(false, &blocks, nullptr);
  PAGE_RES_IT it(&page_res);
  ASSERT_EQ(input_werd, it.word()->word);

  WERD_RES *whole = MakeWholeWord(*it.word(), unicharset);
  WERD_RES *lstm = MakeLSTMWord({TBOX(60, 10, 70, 30)}, unicharset);
  PointerVector<WERD_RES> words;
  words.push_back(whole);
  words.push_back(lstm);
  it.ReplaceCurrentWord(&words);

  // The input WERD is deleted, so no replacement may still use it. Only the
  // pointer values are compared, so this fails cleanly without the fix.
  ASSERT_NE(input_werd, whole->word);
  ExpectEachWordOwnedOnce(it.row());
  EXPECT_FALSE(whole->combination);
  EXPECT_FALSE(lstm->combination);
  // The whole word gets the input blobs back, one per character.
  EXPECT_EQ(3, whole->word->cblob_list()->length());
  EXPECT_EQ(3u, whole->box_word->length());
  EXPECT_EQ(TBOX(10, 10, 50, 30), whole->word->bounding_box());
  EXPECT_TRUE(whole->word->flag(W_BOL));
  EXPECT_FALSE(whole->word->flag(W_EOL));
  EXPECT_FALSE(lstm->word->flag(W_BOL));
  EXPECT_TRUE(lstm->word->flag(W_EOL));

  it.restart_page();
  EXPECT_EQ(whole, it.word());
  EXPECT_EQ(lstm, it.forward());
  EXPECT_EQ(nullptr, it.forward());
}

// The same, with an input word that is a combination of fuzzy-spaced words
// (made by ROW_RES), which owns its WERD and deletes it with itself.
TEST(PageResTest, ReplaceCurrentWordCopiesSharedCombinationWord) {
  UNICHARSET unicharset;
  BLOCK_LIST blocks;
  WERD *first = MakeWord({kInputBlobs[0], kInputBlobs[1]});
  WERD *second = MakeWord({kInputBlobs[2]});
  second->set_flag(W_FUZZY_NON, true);
  AddRow(&blocks, {first, second});
  PAGE_RES page_res(false, &blocks, nullptr);
  PAGE_RES_IT it(&page_res);
  ASSERT_TRUE(it.word()->combination);
  const WERD *input_werd = it.word()->word;

  WERD_RES *whole = MakeWholeWord(*it.word(), unicharset);
  WERD_RES *lstm = MakeLSTMWord({TBOX(60, 10, 70, 30)}, unicharset);
  PointerVector<WERD_RES> words;
  words.push_back(whole);
  words.push_back(lstm);
  it.ReplaceCurrentWord(&words);

  // The input WERD is deleted, so no replacement may still use it.
  ASSERT_NE(input_werd, whole->word);
  ExpectEachWordOwnedOnce(it.row());
  EXPECT_TRUE(whole->combination);
  EXPECT_TRUE(lstm->combination);
  EXPECT_EQ(3, whole->word->cblob_list()->length());
  EXPECT_EQ(TBOX(10, 10, 50, 30), whole->word->bounding_box());
  // The words of the fuzzy-spaced combination stay on the row.
  EXPECT_EQ(2, it.row()->row->word_list()->length());

  it.restart_page();
  EXPECT_EQ(whole, it.word());
  EXPECT_EQ(lstm, it.forward());
  EXPECT_EQ(nullptr, it.forward());
}

} // namespace

} // namespace tesseract
