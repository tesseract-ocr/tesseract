///////////////////////////////////////////////////////////////////////
// File:        fontstyle.cpp
// Description: Estimation of font style attributes (bold, italic,
//              underlined) from the image of a word.
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
// An underline is searched from this share of the x-height above the
// baseline to this share of it below the baseline. The search starts above
// the baseline, because the baseline of the layout analysis is too low where
// descenders join the text and its underline.
const float kMaxUnderlineRise = 0.3f;
const float kMaxUnderlineOffset = 0.8f;
// A line which was found there is followed to the neighbours of the word in
// rows which are up to this share of the x-height further away, because the
// baseline is not exact.
const float kUnderlineSlack = 0.5f;
// A line consists of horizontal runs of pixels which are at least this share
// of the x-height long. The serifs and the bars of the letters are shorter.
const float kMinUnderlinePiece = 0.75f;
// One of its runs is at least this share of the x-height long, which is
// longer than the bar of any letter, for example that of E, L or Z.
const float kMinUnderlineRun = 1.5f;
// A line is thicker than a line of the background which is this share of the
// x-height.
const float kMaxUnderlineThickness = 0.3f;
// An underline covers at least this share of the columns of a word.
const float kMinUnderlineCover = 0.8f;
// A line can have gaps of this share of the x-height, for example next to a
// descender.
const float kMaxUnderlineGap = 0.15f;
// A line which goes on for more than this share of the x-height beyond the
// text is a ruling.
const float kMaxUnderlineOverhang = 0.5f;
// Space on both sides of a text line which is needed to see that, as a share
// of the x-height.
const float kUnderlineMargin = 1.0f;

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

std::vector<bool> FindUnderlinedWords(Image pix, const std::vector<float> &baseline,
                                      float x_height,
                                      const std::vector<std::pair<int, int>> &words) {
  std::vector<bool> underlined(words.size(), false);
  if (pix == nullptr || pixGetDepth(pix) != 1 || x_height <= 0.0f || words.empty()) {
    return underlined;
  }
  const int width = pixGetWidth(pix);
  const int height = pixGetHeight(pix);
  if (width <= 0 || height <= 0 || baseline.size() != static_cast<size_t>(width)) {
    return underlined;
  }
  // The rows of each column in which a line is searched.
  const int rise = static_cast<int>(std::ceil(kMaxUnderlineRise * x_height));
  const int depth = static_cast<int>(std::ceil(kMaxUnderlineOffset * x_height));
  const int slack = static_cast<int>(std::ceil(kUnderlineSlack * x_height));
  std::vector<int> first_rows(width);
  int first_row = height;
  int last_row = -1;
  for (int x = 0; x < width; ++x) {
    first_rows[x] = static_cast<int>(std::floor(baseline[x] + 0.5f)) - rise;
    first_row = std::min(first_row, first_rows[x] - slack);
    last_row = std::max(last_row, first_rows[x] + rise + depth + slack);
  }
  first_row = std::max(first_row, 0);
  last_row = std::min(last_row, height - 1);

  // Collect the long runs of pixels in these rows: for each column the number
  // of rows in which it is part of such a run, and the length of the longest
  // of these runs. A line is followed in more rows than those in which it is
  // searched.
  const int min_piece =
      std::max(2, static_cast<int>(std::ceil(kMinUnderlinePiece * x_height)));
  std::vector<int> thickness(width, 0);
  std::vector<int> longest(width, 0);
  std::vector<bool> has_line(width, false);
  const l_uint32 *data = pixGetData(pix);
  const int wpl = pixGetWpl(pix);
  for (int y = first_row; y <= last_row; ++y) {
    const l_uint32 *line = data + static_cast<size_t>(y) * wpl;
    int start = -1;
    for (int x = 0; x <= width; ++x) {
      if (x < width && GET_DATA_BIT(line, x) != 0) {
        if (start < 0) {
          start = x;
        }
        continue;
      }
      if (start >= 0 && x - start >= min_piece) {
        for (int run_x = start; run_x < x; ++run_x) {
          const int offset = y - first_rows[run_x];
          if (offset >= 0 && offset <= rise + depth) {
            ++thickness[run_x];
          }
          if (offset >= -slack && offset <= rise + depth + slack) {
            has_line[run_x] = true;
            longest[run_x] = std::max(longest[run_x], x - start);
          }
        }
      }
      start = -1;
    }
  }

  // The text ends with its first and its last column which have a pixel that
  // is not part of a line, from the top of the lowercase letters to the
  // descenders. The words alone do not tell that, because the layout analysis
  // can add the end of a ruling to a word.
  int text_left = width;
  int text_right = -1;
  for (const auto &word : words) {
    text_left = std::min(text_left, word.first);
    text_right = std::max(text_right, word.second);
  }
  text_left = std::max(text_left, 0);
  text_right = std::min(text_right, width - 1);
  const int text_height = static_cast<int>(std::ceil(x_height)) - rise;
  auto has_text = [&](int x) {
    for (int y = std::max(first_rows[x] - text_height, 0);
         y < first_rows[x] + 2 * rise && y < height; ++y) {
      const l_uint32 *line = data + static_cast<size_t>(y) * wpl;
      if (GET_DATA_BIT(line, x) == 0) {
        continue;
      }
      int start = x;
      while (start > 0 && x - start < min_piece && GET_DATA_BIT(line, start - 1) != 0) {
        --start;
      }
      int end = x + 1;
      while (end < width && end - start < min_piece && GET_DATA_BIT(line, end) != 0) {
        ++end;
      }
      if (end - start < min_piece) {
        return true;
      }
    }
    return false;
  };
  while (text_left < text_right && !has_text(text_left)) {
    ++text_left;
  }
  while (text_right > text_left && !has_text(text_right)) {
    --text_right;
  }
  const int max_thickness = std::max(2, static_cast<int>(kMaxUnderlineThickness * x_height));
  const int max_gap = static_cast<int>(kMaxUnderlineGap * x_height) + 1;
  const float max_overhang = kMaxUnderlineOverhang * x_height;
  for (size_t w = 0; w < words.size(); ++w) {
    const int first = std::max(0, words[w].first);
    const int last = std::min(width - 1, words[w].second);
    if (first > last) {
      continue;
    }
    int num_covered = 0;
    int num_thick = 0;
    for (int x = first; x <= last; ++x) {
      num_covered += thickness[x] > 0;
      num_thick += thickness[x] > max_thickness;
    }
    // Text on a dark background has a line which is too thick.
    if (num_covered < kMinUnderlineCover * (last - first + 1) || 2 * num_thick > num_covered) {
      continue;
    }
    // Follow the line to both sides of the word.
    int line_left = first;
    for (int x = first - 1, gap = 0; x >= 0 && gap <= max_gap; --x) {
      if (has_line[x]) {
        line_left = x;
        gap = 0;
      } else {
        ++gap;
      }
    }
    int line_right = last;
    for (int x = last + 1, gap = 0; x < width && gap <= max_gap; ++x) {
      if (has_line[x]) {
        line_right = x;
        gap = 0;
      } else {
        ++gap;
      }
    }
    if (line_left < text_left - max_overhang || line_right > text_right + max_overhang) {
      continue;
    }
    const int longest_run =
        *std::max_element(longest.begin() + line_left, longest.begin() + line_right + 1);
    underlined[w] = longest_run >= kMinUnderlineRun * x_height;
  }
  return underlined;
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

// Returns true if the word has more characters of the scripts which have
// long horizontal strokes next to the baseline than of other scripts. Those
// are Arabic, which joins its letters there, and Chinese, Japanese and Korean.
// Such a stroke cannot be told from an underline.
static bool HasLineLikeStrokes(const WERD_RES &word) {
  if (HasUprightScript(word)) {
    return true;
  }
  const UNICHARSET &unicharset = *word.uch_set;
  const WERD_CHOICE &choice = *word.best_choice;
  const int arabic_sid = unicharset.get_script_id_from_name("Arabic");
  if (arabic_sid == unicharset.null_sid()) {
    return false;
  }
  int num_arabic = 0;
  int num_others = 0;
  for (unsigned i = 0; i < choice.length(); ++i) {
    const int script = unicharset.get_script(choice.unichar_id(i));
    if (script == arabic_sid) {
      ++num_arabic;
    } else if (script != unicharset.null_sid() && script != unicharset.common_sid()) {
      ++num_others;
    }
  }
  return num_arabic > num_others;
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

// Returns the part of the image which the given box of a block covers, turned
// like the block, so that the text lines are horizontal. The boxes of a block
// refer to the block, which is rotated against the image if the page is not
// upright or if the text lines are vertical. The box is reduced to its part
// in the image. Returns nullptr if there is none.
static Image ClipBlockImage(Image pix, const FCOORD &re_rotation, TBOX *box) {
  const TBOX image_box(0, 0, pixGetWidth(pix), pixGetHeight(pix));
  TBOX image_part = *box;
  image_part.rotate(re_rotation);
  image_part = image_part.intersection(image_box);
  if (image_part.null_box()) {
    return nullptr;
  }
  Box *clip_box = boxCreate(image_part.left(), image_box.top() - image_part.top(),
                            image_part.width(), image_part.height());
  Image result = pixClipRectangle(pix, clip_box, nullptr);
  boxDestroy(&clip_box);
  const int turns = (4 - QuarterTurns(re_rotation)) % 4;
  if (result != nullptr && turns != 0) {
    Image turned = pixRotateOrth(result, turns);
    result.destroy();
    result = turned;
  }
  image_part.rotate(FCOORD(re_rotation.x(), -re_rotation.y()));
  *box = image_part;
  return result;
}

// Marks the underlined words of the page which have no font, which are those
// recognized by LSTM. removed_lines has the pixels of the lines which the
// layout analysis removed from the image, or is nullptr.
static void MarkUnderlinedWords(Image pix, Image removed_lines, PAGE_RES *page_res,
                                bool debug) {
  PAGE_RES_IT page_res_it(page_res);
  page_res_it.restart_page();
  while (page_res_it.word() != nullptr) {
    // The words of the next text line.
    const BLOCK *block = page_res_it.block()->block;
    const ROW *row = page_res_it.row()->row;
    std::vector<WERD_RES *> row_words;
    do {
      row_words.push_back(page_res_it.word());
      page_res_it.forward();
    } while (page_res_it.word() != nullptr && page_res_it.row()->row == row);
    if (block == nullptr || row == nullptr || row->x_height() <= 0.0f) {
      continue;
    }
    const float x_height = row->x_height();
    TBOX text_box;
    bool has_candidate = false;
    for (const WERD_RES *word : row_words) {
      if (word->word != nullptr) {
        text_box += word->word->bounding_box();
        has_candidate =
            has_candidate || (word->fontinfo == nullptr && word->best_choice != nullptr);
      }
    }
    if (!has_candidate || text_box.null_box()) {
      continue;
    }
    // The image of the text line from the top of its lowercase letters to
    // the space below the baseline, with a margin on both sides.
    const int margin = static_cast<int>(std::ceil(kUnderlineMargin * x_height));
    const int left = text_box.left() - margin;
    const int right = text_box.right() + margin;
    float lowest = row->base_line(left);
    float highest = lowest;
    for (const int x : {(left + right) / 2, right}) {
      lowest = std::min(lowest, row->base_line(x));
      highest = std::max(highest, row->base_line(x));
    }
    const int depth =
        static_cast<int>(std::ceil((kMaxUnderlineOffset + kUnderlineSlack) * x_height)) + 2;
    // The lowercase letters show where the text ends.
    const int rise = static_cast<int>(std::ceil(x_height)) + 2;
    TBOX box(left, static_cast<int>(std::floor(lowest)) - depth, right,
             static_cast<int>(std::ceil(highest)) + rise);
    TBOX lines_box = box;
    Image row_pix = ClipBlockImage(pix, block->re_rotation(), &box);
    if (row_pix == nullptr) {
      continue;
    }
    if (removed_lines != nullptr) {
      Image row_lines = ClipBlockImage(removed_lines, block->re_rotation(), &lines_box);
      if (row_lines != nullptr) {
        pixOr(row_pix, row_pix, row_lines);
        row_lines.destroy();
      }
    }
    std::vector<float> baseline(box.width());
    for (int x = 0; x < box.width(); ++x) {
      baseline[x] = box.top() - row->base_line(box.left() + x + 0.5f);
    }
    std::vector<std::pair<int, int>> columns;
    for (const WERD_RES *word : row_words) {
      if (word->word != nullptr) {
        const TBOX word_box = word->word->bounding_box();
        columns.emplace_back(word_box.left() - box.left(), word_box.right() - 1 - box.left());
      }
    }
    const std::vector<bool> underlined = FindUnderlinedWords(row_pix, baseline, x_height, columns);
    row_pix.destroy();
    size_t index = 0;
    for (WERD_RES *word : row_words) {
      if (word->word == nullptr) {
        continue;
      }
      const bool has_line = underlined[index++];
      if (word->fontinfo != nullptr || word->best_choice == nullptr) {
        continue;
      }
      // A word needs a letter or a digit. A line below a dash or below nothing
      // is a line of its own. And the line must not be a stroke of the word.
      bool has_text = false;
      const WERD_CHOICE &choice = *word->best_choice;
      for (unsigned i = 0; i < choice.length(); ++i) {
        const UNICHAR_ID id = choice.unichar_id(i);
        has_text = has_text || word->uch_set->get_isalpha(id) || word->uch_set->get_isdigit(id);
      }
      word->underlined = has_line && has_text && !HasLineLikeStrokes(*word);
      if (debug && has_line) {
        tprintf("Word underline: text=%d %s\n", has_text, choice.unichar_string().c_str());
      }
    }
  }
}

/**
 * font_style_pass
 *
 * Estimate bold, italic and underlined for the words which have no font,
 * which are those recognized by LSTM, from the binary image.
 */
void Tesseract::font_style_pass(PAGE_RES *page_res) {
  Image pix = pix_binary();
  if (pix == nullptr) {
    return;
  }
  // This needs the words without the fonts which they get below.
  MarkUnderlinedWords(pix, pix_removed_lines_, page_res, tessedit_debug_fonts);
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
    // Vertical lines of a rotated page have upright characters in the block.
    // Those of Chinese, Japanese and Korean text are turned in the block, and
    // the rotation for the classifier makes them upright. The block is the
    // same for both, so the script of the word decides.
    const int upright_turns =
        HasUprightScript(*word) ? QuarterTurns(block->classify_rotation()) : 0;
    // The image of the word as it is in the block, with a horizontal text
    // line. From here on the box is the part of the word in the image.
    TBOX box = word->word->bounding_box();
    Image word_pix = ClipBlockImage(pix, block->re_rotation(), &box);
    if (word_pix == nullptr) {
      continue;
    }
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
