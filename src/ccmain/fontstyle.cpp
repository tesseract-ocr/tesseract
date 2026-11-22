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
// the typical word of the page, relative to the x-height.
const float kBoldFactor = 1.25f;
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
  // Stroke width relative to the x-height of the row.
  float weight = 0.0f;
  // The word has enough text for a decision of its own on bold.
  bool bold_reliable = false;
  // The same for italic, which also needs strokes with a slant.
  bool italic_reliable = false;
  bool bold = false;
  bool italic = false;
};

} // namespace

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
    if (block->re_rotation().x() <= 0.0f || block->re_rotation().y() != 0.0f) {
      // The block is rotated, so its boxes do not refer to the image.
      continue;
    }
    const TBOX box = word->word->bounding_box().intersection(image_box);
    if (box.null_box()) {
      continue;
    }
    Box *clip_box =
        boxCreate(box.left(), image_box.top() - box.top(), box.width(), box.height());
    Image word_pix = pixClipRectangle(pix, clip_box, nullptr);
    boxDestroy(&clip_box);
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
    if (!MeasureTextShape(word_pix, &shape)) {
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
        shape.has_slant = MeasureTextShape(word_pix, &stem_shape) && stem_shape.has_slant;
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
    style.weight = shape.stroke_width / row->x_height();
    int num_letters = 0;
    for (unsigned i = 0; i < choice.length(); ++i) {
      if (word->uch_set->get_isalpha(choice.unichar_id(i)) ||
          word->uch_set->get_isdigit(choice.unichar_id(i))) {
        ++num_letters;
      }
    }
    style.bold_reliable = num_letters >= kMinStyleLetters;
    style.italic_reliable = style.bold_reliable && shape.has_slant;
    style.italic = shape.has_slant && style.slant >= kItalicSlant;
    styles.push_back(style);
  }
  if (styles.empty()) {
    return;
  }

  // The weight of the typical word of the page is the reference for bold.
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
