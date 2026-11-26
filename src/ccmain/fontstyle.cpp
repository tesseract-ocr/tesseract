///////////////////////////////////////////////////////////////////////
// File:        fontstyle.cpp
// Description: Estimation of font style attributes (bold, italic) from
//              the image of a word.
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

#include "fontstyle.h"

#include "boxword.h"
#include "fontinfo.h"
#include "pageres.h"
#include "tesseractclass.h"
#include "tprintf.h"

#include <allheaders.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring> // for strchr
#include <utility> // for std::pair
#include <vector>

namespace tesseract {

// Range of the slants of upright and italic text, as tangents of the slant
// angle. Strokes outside this range are diagonal strokes like those of v, w
// and y.
const float kMinTextSlant = -0.1f;
const float kMaxTextSlant = 0.35f;
// The slants are tested in these steps. The test includes one more slant on
// each side of the range to see whether a maximum at its limit is a real one,
// so kNumSlants is (kMaxTextSlant - kMinTextSlant) / kSlantStep + 3.
const float kSlantStep = 0.05f;
const int kNumSlants = 12;
// Resolution of the shear in fractions of a pixel.
const int kSubPixels = 4;

// A word is italic if its slant is at least this value.
const float kItalicSlant = 0.07f;
// A word is bold if its strokes are at least this much wider than those of
// the typical word of the page, relative to the size of the text.
const float kBoldFactor = 1.25f;
// The stroke width of the regular words of a text line is the median of its
// thin words. Those are the words up to this factor above the stroke width at
// this quantile of the words, so some bold words do not change it.
const float kThinWordsQuantile = 0.15f;
const float kThinWordsRange = 1.15f;
// Text lines have the same font and size if their regular stroke widths and
// the heights of their tall words differ by less than these shares. The
// height of the tall words is the one at this quantile of the words.
const float kSameStrokeTolerance = 0.08f;
const float kSameHeightTolerance = 0.2f;
const float kTallWordsQuantile = 0.75f;
// The x-height of a text line is too large if a word with lowercase letters
// is lower than this share of it. A word cannot be lower than half of it.
const float kMaxLowWord = 0.85f;
const float kMinLowWord = 0.5f;
// Typical x-height of a font relative to the height of its capital letters.
const float kXHeightOfCapitals = 0.7f;
// The italic words of a page have a reference of their own for bold if there
// are at least this many of them.
const size_t kMinItalicWords = 5;
// A word needs this number of letters and digits for a decision of its own.
const int kMinStyleLetters = 3;
// Letters and symbols with diagonal strokes which can have the slant of the
// stems of italic text. They are left out when the slant is measured.
const char kDiagonalLetters[] = "AMVWXYvwxy/";

namespace {

// A horizontal run of text pixels: row y, columns start to end - 1.
struct PixelRun {
  int y;
  int start;
  int end;
};

} // namespace

bool MeasureTextShape(Image pix, TextShape *shape) {
  if (pix == nullptr || pixGetDepth(pix) != 1) {
    return false;
  }
  const int width = pixGetWidth(pix);
  const int height = pixGetHeight(pix);
  if (width <= 0 || height <= 0) {
    return false;
  }

  // Collect the runs of text pixels and count them by their length. A run
  // which touches no text in the row above or below it is a speck of noise.
  // Many of those would hide the strokes of the text, so they are left out.
  std::vector<PixelRun> runs;
  std::vector<PixelRun> specks;
  const l_uint32 *data = pixGetData(pix);
  const int wpl = pixGetWpl(pix);
  for (int y = 0; y < height; ++y) {
    const l_uint32 *line = data + static_cast<size_t>(y) * wpl;
    const l_uint32 *above = y > 0 ? line - wpl : nullptr;
    const l_uint32 *below = y + 1 < height ? line + wpl : nullptr;
    int start = -1;
    bool connected = false;
    for (int x = 0; x <= width; ++x) {
      const bool text = x < width && GET_DATA_BIT(line, x) != 0;
      if (text) {
        if (start < 0) {
          start = x;
          connected = false;
        }
        connected = connected || (above != nullptr && GET_DATA_BIT(above, x) != 0) ||
                    (below != nullptr && GET_DATA_BIT(below, x) != 0);
      } else if (start >= 0) {
        (connected ? runs : specks).push_back({y, start, x});
        start = -1;
      }
    }
  }
  if (runs.empty()) {
    // The text has no stroke which is higher than a pixel.
    runs.swap(specks);
  }
  if (runs.empty()) {
    return false;
  }
  std::vector<int> num_runs(width + 1);
  for (const auto &run : runs) {
    ++num_runs[run.end - run.start];
  }

  // Most runs cross a single stroke, so the median of their lengths is the
  // stroke width. The runs of the length l count as spread evenly from
  // l - 0.5 to l + 0.5, which gives a median between the integer lengths.
  const double half = 0.5 * runs.size();
  double shorter = 0.0;
  int length = 1;
  while (shorter + num_runs[length] < half) {
    shorter += num_runs[length++];
  }
  shape->stroke_width = length - 0.5f + static_cast<float>((half - shorter) / num_runs[length]);

  // Shear the text by a slant and add up the pixels in each column. The sum
  // of the squares of the column sums has a maximum where strokes are vertical.
  // The rows are shifted in steps of 1 / kSubPixels pixel, because whole
  // pixels cannot show a slant of less than one pixel over the height of a
  // stroke.
  const float y_center = 0.5f * (height - 1);
  const float max_slant = std::max(kSlantStep - kMinTextSlant, kMaxTextSlant + kSlantStep);
  const int pad = static_cast<int>(std::ceil(max_slant * y_center)) + 1;
  // The columns hold the differences between neighbouring column sums.
  std::vector<int> columns(kSubPixels * (width + 2 * pad) + 1);
  auto score_of = [&](float slant) {
    std::fill(columns.begin(), columns.end(), 0);
    int y = -1;
    int offset = 0;
    for (const auto &run : runs) {
      if (run.y != y) {
        y = run.y;
        offset = static_cast<int>(std::lround(kSubPixels * (pad + slant * (y - y_center))));
      }
      ++columns[kSubPixels * run.start + offset];
      --columns[kSubPixels * run.end + offset];
    }
    double score = 0.0;
    int sum = 0;
    for (int delta : columns) {
      sum += delta;
      score += static_cast<double>(sum) * sum;
    }
    return score;
  };
  const float first_slant = kMinTextSlant - kSlantStep;
  std::array<double, kNumSlants> scores;
  for (int i = 0; i < kNumSlants; ++i) {
    scores[i] = score_of(first_slant + i * kSlantStep);
  }
  // The text has a slant if the best score is in the range of upright and
  // italic text. If it is at one of the two slants outside the range, the
  // strongest strokes are diagonal ones.
  const int best =
      static_cast<int>(std::max_element(scores.begin(), scores.end()) - scores.begin());
  shape->has_slant = best > 0 && best + 1 < kNumSlants;
  shape->slant = 0.0f;
  if (shape->has_slant) {
    // Refine the maximum: test the slants half a step away from it, and fit a
    // parabola to the best of the three and its two neighbours.
    const float half_step = 0.5f * kSlantStep;
    float slant = first_slant + best * kSlantStep;
    double below = score_of(slant - half_step);
    double peak = scores[best];
    double above = score_of(slant + half_step);
    if (below > peak && below >= above) {
      slant -= half_step;
      above = peak;
      peak = below;
      below = scores[best - 1];
    } else if (above > peak) {
      slant += half_step;
      below = peak;
      peak = above;
      above = scores[best + 1];
    }
    const double curvature = below - 2.0 * peak + above;
    if (curvature < 0.0) {
      slant += static_cast<float>(half_step * 0.5 * (below - above) / curvature);
    }
    shape->slant = slant;
  }
  return true;
}

const FontInfo *NamelessFont(bool bold, bool italic) {
  static const std::array<FontInfo, 4> fonts = [] {
    std::array<FontInfo, 4> table;
    for (unsigned i = 0; i < table.size(); ++i) {
      // The properties are those of FontInfo: 1 = italic, 2 = bold.
      table[i].properties = i;
      table[i].universal_id = -1;
    }
    return table;
  }();
  return &fonts[(italic ? 1 : 0) | (bold ? 2 : 0)];
}

namespace {

// Style measurements and decisions for one word.
struct WordStyle {
  WERD_RES *word = nullptr;
  const ROW *row = nullptr;
  // Slant of the text relative to the perpendicular of the baseline.
  float slant = 0.0f;
  // Stroke width relative to the size of the text.
  float weight = 0.0f;
  // The word has enough text for a decision of its own on bold.
  bool bold_reliable = false;
  // The same for italic, which also needs strokes with a slant.
  bool italic_reliable = false;
  bool bold = false;
  bool italic = false;
};

// Text size and stroke width of the words of one text line, which are the
// elements from begin to end - 1 of the words.
struct RowStyle {
  size_t begin = 0;
  size_t end = 0;
  // The x-height from the layout analysis, corrected with the recognized
  // words.
  float x_height = 0.0f;
  // Stroke width of the regular words in pixels.
  float stroke = 0.0f;
  // Number of words which were used for the stroke width.
  int num_words = 0;
  // Height of the tall words above the baseline in pixels.
  float height = 0.0f;
};

} // namespace

// Returns the median of the values, or the upper one of the two medians.
// Changes the order of the values, which must not be empty.
static float Median(std::vector<float> *values) {
  auto median = values->begin() + values->size() / 2;
  std::nth_element(values->begin(), median, values->end());
  return *median;
}

// Returns the text size and the regular stroke width of the text line with
// the given words. The strokes are the stroke widths of all words.
//
// The x-height of the layout analysis is sometimes the height of the capital
// letters, mainly in lines with capital letters which have accents. The
// recognized text shows that: a word with lowercase letters is at least as
// high as the x-height, and a word of capital letters is higher. If words are
// too low for the x-height of the line, their height replaces it.
static RowStyle MeasureRow(const std::vector<WordStrokes> &words,
                           const std::vector<float> &strokes, size_t begin, size_t end) {
  RowStyle row;
  row.begin = begin;
  row.end = end;
  row.x_height = words[begin].x_height;
  std::vector<float> x_heights;
  std::vector<float> row_strokes;
  // Heights of the words with letters of a known height, of the words with
  // enough text, and of all words.
  std::vector<float> letter_heights;
  std::vector<float> reliable_heights;
  std::vector<float> heights;
  for (size_t i = begin; i < end; ++i) {
    const WordStrokes &word = words[i];
    heights.push_back(word.height);
    if (!word.reliable) {
      continue;
    }
    reliable_heights.push_back(word.height);
    if (word.has_lower || word.capitals_only) {
      letter_heights.push_back(word.height);
    }
    row_strokes.push_back(strokes[i]);
    float x_height = 0.0f;
    if (word.has_lower) {
      x_height = word.height;
    } else if (word.capitals_only) {
      x_height = kXHeightOfCapitals * word.height;
    }
    if (x_height >= kMinLowWord * row.x_height && x_height <= kMaxLowWord * row.x_height) {
      x_heights.push_back(x_height);
    }
  }
  if (!x_heights.empty()) {
    row.x_height = Median(&x_heights);
  }
  if (row_strokes.empty()) {
    row_strokes.assign(strokes.begin() + begin, strokes.begin() + end);
  }
  std::sort(row_strokes.begin(), row_strokes.end());
  const float thin = row_strokes[static_cast<size_t>(kThinWordsQuantile * row_strokes.size())];
  const size_t num_thin =
      std::upper_bound(row_strokes.begin(), row_strokes.end(), kThinWordsRange * thin) -
      row_strokes.begin();
  row.stroke = row_strokes[num_thin / 2];
  row.num_words = static_cast<int>(row_strokes.size());
  // Capital letters with accents are higher than other tall letters, so
  // prefer the words which cannot have them.
  std::vector<float> &tall = !letter_heights.empty()
                                 ? letter_heights
                                 : !reliable_heights.empty() ? reliable_heights : heights;
  auto quantile = tall.begin() + std::min(tall.size() - 1, static_cast<size_t>(
                                                             kTallWordsQuantile * tall.size()));
  std::nth_element(tall.begin(), quantile, tall.end());
  row.height = *quantile;
  return row;
}

void SetWordWeights(std::vector<WordStrokes> *words) {
  // The italic of some fonts has thinner strokes than the upright font, so
  // its bold is not much heavier than regular upright text. Compare italic
  // words with the typical italic word then, by scaling their strokes.
  std::vector<float> upright;
  std::vector<float> italic;
  for (const auto &word : *words) {
    if (word.reliable && word.x_height > 0.0f) {
      (word.italic ? italic : upright).push_back(word.stroke / word.x_height);
    }
  }
  float italic_stroke = 1.0f;
  if (!upright.empty() && italic.size() >= kMinItalicWords) {
    italic_stroke = std::min(1.0f, Median(&italic) / Median(&upright));
  }
  std::vector<float> strokes;
  for (const auto &word : *words) {
    strokes.push_back(word.italic && italic_stroke > 0.0f ? word.stroke / italic_stroke
                                                           : word.stroke);
  }

  std::vector<RowStyle> rows;
  for (size_t begin = 0; begin < words->size();) {
    size_t end = begin + 1;
    while (end < words->size() && (*words)[end].row == (*words)[begin].row) {
      ++end;
    }
    rows.push_back(MeasureRow(*words, strokes, begin, end));
    begin = end;
  }
  // The x-height of a single text line is not exact enough, even after its
  // correction. Lines with the same regular stroke width and the same height
  // of their words have the same font and size, so they share the median of
  // their x-heights. That leaves lines which are bold throughout as they are,
  // because their strokes differ from those of the other lines. Larger text
  // can have the strokes of smaller bold text, but its words are higher.
  std::vector<std::pair<float, int>> x_heights;
  for (const auto &row : rows) {
    x_heights.clear();
    int num_words = 0;
    for (const auto &other : rows) {
      if (row.stroke > 0.0f && row.height > 0.0f &&
          std::fabs(other.stroke / row.stroke - 1.0f) <= kSameStrokeTolerance &&
          std::fabs(other.height / row.height - 1.0f) <= kSameHeightTolerance) {
        x_heights.emplace_back(other.x_height, other.num_words);
        num_words += other.num_words;
      }
    }
    std::sort(x_heights.begin(), x_heights.end());
    float x_height = row.x_height;
    int num_smaller = 0;
    for (const auto &entry : x_heights) {
      num_smaller += entry.second;
      if (num_smaller > num_words / 2) {
        x_height = entry.first;
        break;
      }
    }
    for (size_t i = row.begin; i < row.end; ++i) {
      (*words)[i].weight = x_height > 0.0f ? strokes[i] / x_height : 0.0f;
    }
  }
}

// Returns the value which a word with too little text gets for a style
// attribute: the value of its reliable neighbours in the same row if they
// agree or if there is only one, else the value from its own measurement.
static bool StyleFromNeighbours(const std::vector<WordStyle> &styles, size_t index,
                                bool WordStyle::*attribute, bool WordStyle::*reliable) {
  const WordStyle &style = styles[index];
  const WordStyle *prev = nullptr;
  const WordStyle *next = nullptr;
  for (size_t i = index; i > 0 && styles[i - 1].row == style.row && prev == nullptr; --i) {
    if (styles[i - 1].*reliable) {
      prev = &styles[i - 1];
    }
  }
  for (size_t i = index + 1; i < styles.size() && styles[i].row == style.row && next == nullptr;
       ++i) {
    if (styles[i].*reliable) {
      next = &styles[i];
    }
  }
  if (prev != nullptr && (next == nullptr || prev->*attribute == next->*attribute)) {
    return prev->*attribute;
  }
  if (next != nullptr && prev == nullptr) {
    return next->*attribute;
  }
  return style.*attribute;
}

// Returns the number of clockwise quarter turns of an image which the given
// rotation of a block stands for. Blocks are only rotated by quarter turns,
// and the vector (0, 1) turns anticlockwise.
static int QuarterTurns(const FCOORD &rotation) {
  if (rotation.y() < 0.0f) {
    return 1;
  }
  if (rotation.y() > 0.0f) {
    return 3;
  }
  return rotation.x() < 0.0f ? 2 : 0;
}

// Returns true if the word has more characters of the scripts which keep
// their characters upright in vertical text lines (Chinese, Japanese, Korean)
// than of other scripts. Characters which many scripts share do not count.
static bool HasUprightScript(const WERD_RES &word) {
  const UNICHARSET &unicharset = *word.uch_set;
  const WERD_CHOICE &choice = *word.best_choice;
  int num_upright = 0;
  int num_others = 0;
  for (unsigned i = 0; i < choice.length(); ++i) {
    const int script = unicharset.get_script(choice.unichar_id(i));
    if (script == unicharset.null_sid() || script == unicharset.common_sid()) {
      continue;
    }
    if (script == unicharset.han_sid() || script == unicharset.hiragana_sid() ||
        script == unicharset.katakana_sid() || script == unicharset.hangul_sid()) {
      ++num_upright;
    } else {
      ++num_others;
    }
  }
  return num_upright > num_others;
}

// Measures the shape of the text in an image which needs the given number of
// clockwise quarter turns to get upright characters.
static bool MeasureTurnedText(Image pix, int turns, TextShape *shape) {
  if (turns == 0) {
    return MeasureTextShape(pix, shape);
  }
  Image upright = pixRotateOrth(pix, turns);
  const bool measured = MeasureTextShape(upright, shape);
  upright.destroy();
  return measured;
}

/**
 * font_style_pass
 *
 * Estimate bold and italic for the words which have no font, which are those
 * recognized by LSTM, from the binary image.
 */
void Tesseract::font_style_pass(PAGE_RES *page_res) {
  Image pix = pix_binary();
  if (pix == nullptr) {
    return;
  }
  const TBOX image_box(0, 0, pixGetWidth(pix), pixGetHeight(pix));
  std::vector<WordStyle> styles;
  // The measurements for bold of the same words.
  std::vector<WordStrokes> strokes;
  PAGE_RES_IT page_res_it(page_res);
  for (page_res_it.restart_page(); page_res_it.word() != nullptr; page_res_it.forward()) {
    WERD_RES *word = page_res_it.word();
    if (word->fontinfo != nullptr || word->best_choice == nullptr || word->word == nullptr) {
      continue;
    }
    const BLOCK *block = page_res_it.block()->block;
    const ROW *row = page_res_it.row()->row;
    if (block == nullptr || row == nullptr || row->x_height() <= 0.0f) {
      continue;
    }
    // The boxes refer to the block, which is rotated against the image if the
    // page is not upright or if the text lines are vertical. Get the image of
    // the word as it is in the block, with a horizontal text line.
    const FCOORD re_rotation = block->re_rotation();
    const int block_turns = (4 - QuarterTurns(re_rotation)) % 4;
    // Vertical lines of a rotated page have upright characters in the block.
    // Those of Chinese, Japanese and Korean text are turned in the block, and
    // the rotation for the classifier makes them upright. The block is the
    // same for both, so the script of the word decides.
    const int upright_turns =
        HasUprightScript(*word) ? QuarterTurns(block->classify_rotation()) : 0;
    TBOX box = word->word->bounding_box();
    box.rotate(re_rotation);
    box = box.intersection(image_box);
    if (box.null_box()) {
      continue;
    }
    Box *clip_box =
        boxCreate(box.left(), image_box.top() - box.top(), box.width(), box.height());
    Image word_pix = pixClipRectangle(pix, clip_box, nullptr);
    boxDestroy(&clip_box);
    if (word_pix != nullptr && block_turns != 0) {
      Image turned = pixRotateOrth(word_pix, block_turns);
      word_pix.destroy();
      word_pix = turned;
    }
    if (word_pix == nullptr) {
      continue;
    }
    // From here on the box is the part of the word in the image, in the
    // coordinates of the block.
    box.rotate(FCOORD(re_rotation.x(), -re_rotation.y()));
    // Invert white text on black. Such text has more set pixels than others,
    // but so can heavy bold text. The upper and lower edge of the box are
    // above and below the text, so they must show the background, too.
    l_int32 num_set = 0;
    l_int32 num_top = 0;
    l_int32 num_bottom = 0;
    pixCountPixels(word_pix, &num_set, nullptr);
    pixCountPixelsInRow(word_pix, 0, &num_top, nullptr);
    pixCountPixelsInRow(word_pix, box.height() - 1, &num_bottom, nullptr);
    if (2 * static_cast<int64_t>(num_set) > static_cast<int64_t>(box.width()) * box.height() &&
        num_top + num_bottom > box.width()) {
      pixInvert(word_pix, word_pix);
    }
    TextShape shape;
    if (!MeasureTurnedText(word_pix, upright_turns, &shape)) {
      word_pix.destroy();
      continue;
    }
    // Measure the slant again without the letters which have diagonal strokes.
    const WERD_CHOICE &choice = *word->best_choice;
    if (word->box_word != nullptr && word->box_word->length() == choice.length()) {
      bool erased = false;
      for (unsigned i = 0; i < choice.length(); ++i) {
        const char *unichar = word->uch_set->id_to_unichar(choice.unichar_id(i));
        if (unichar[0] == '\0' || strchr(kDiagonalLetters, unichar[0]) == nullptr) {
          continue;
        }
        const TBOX letter_box = word->box_word->BlobBox(i).intersection(box);
        if (letter_box.null_box()) {
          continue;
        }
        Box *letter = boxCreate(letter_box.left() - box.left(), 0, letter_box.width(),
                                box.height());
        pixClearInRect(word_pix, letter);
        boxDestroy(&letter);
        erased = true;
      }
      if (erased) {
        TextShape stem_shape;
        shape.has_slant =
            MeasureTurnedText(word_pix, upright_turns, &stem_shape) && stem_shape.has_slant;
        shape.slant = stem_shape.slant;
      }
    }
    word_pix.destroy();
    WordStyle style;
    style.word = word;
    style.row = row;
    // Upright text on a baseline with the gradient g has the slant -g.
    const float gradient = box.width() > 0
                               ? (row->base_line(box.right()) - row->base_line(box.left())) /
                                     box.width()
                               : 0.0f;
    style.slant = shape.slant + gradient;
    WordStrokes word_strokes;
    word_strokes.x_height = row->x_height();
    word_strokes.stroke = shape.stroke_width;
    word_strokes.height = box.top() - row->base_line(0.5f * (box.left() + box.right()));
    int num_letters = 0;
    bool has_upper = false;
    bool ascii = true;
    for (unsigned i = 0; i < choice.length(); ++i) {
      const UNICHAR_ID id = choice.unichar_id(i);
      if (word->uch_set->get_isalpha(id) || word->uch_set->get_isdigit(id)) {
        ++num_letters;
      }
      word_strokes.has_lower = word_strokes.has_lower || word->uch_set->get_islower(id);
      has_upper = has_upper || word->uch_set->get_isupper(id);
      for (const char *byte = word->uch_set->id_to_unichar(id); *byte != '\0'; ++byte) {
        ascii = ascii && static_cast<unsigned char>(*byte) < 0x80;
      }
    }
    // Such a word has no accents, so it is as high as a capital letter.
    word_strokes.capitals_only = has_upper && !word_strokes.has_lower && ascii;
    style.bold_reliable = num_letters >= kMinStyleLetters;
    style.italic_reliable = style.bold_reliable && shape.has_slant;
    style.italic = shape.has_slant && style.slant >= kItalicSlant;
    word_strokes.reliable = style.bold_reliable;
    word_strokes.italic = style.italic;
    // The words of a text line follow each other.
    word_strokes.row = styles.empty() ? 0 : strokes.back().row + (styles.back().row != row);
    styles.push_back(style);
    strokes.push_back(word_strokes);
  }
  if (styles.empty()) {
    return;
  }

  // The weight of the typical word of the page is the reference for bold.
  SetWordWeights(&strokes);
  for (size_t i = 0; i < styles.size(); ++i) {
    styles[i].weight = strokes[i].weight;
  }
  std::vector<float> weights;
  for (const auto &style : styles) {
    if (style.bold_reliable) {
      weights.push_back(style.weight);
    }
  }
  if (weights.empty()) {
    for (const auto &style : styles) {
      weights.push_back(style.weight);
    }
  }
  auto median = weights.begin() + weights.size() / 2;
  std::nth_element(weights.begin(), median, weights.end());
  const float bold_weight = kBoldFactor * *median;

  for (auto &style : styles) {
    style.bold = style.weight >= bold_weight;
  }
  for (size_t i = 0; i < styles.size(); ++i) {
    const WordStyle &style = styles[i];
    bool bold = style.bold;
    bool italic = style.italic;
    if (!style.bold_reliable) {
      bold = StyleFromNeighbours(styles, i, &WordStyle::bold, &WordStyle::bold_reliable);
    }
    if (!style.italic_reliable) {
      italic = StyleFromNeighbours(styles, i, &WordStyle::italic, &WordStyle::italic_reliable);
    }
    style.word->fontinfo = NamelessFont(bold, italic);
    style.word->fontinfo_id_count = 1;
    if (tessedit_debug_fonts) {
      // reliable: 1 = for bold, 2 = for italic, 3 = for both.
      tprintf("Word style: slant=%.3f weight=%.3f (page %.3f) reliable=%d bold=%d italic=%d %s\n",
              style.slant, style.weight, *median, style.bold_reliable + 2 * style.italic_reliable,
              bold, italic, style.word->best_choice->unichar_string().c_str());
    }
  }
}

} // namespace tesseract
