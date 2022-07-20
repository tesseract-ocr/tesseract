/**********************************************************************
 * File:        text2image.cpp
 * Description: Program to generate OCR training pages. Given a text file it
 *              outputs an image with a given font and degradation.
 *
 *              Note that since the results depend on the fonts available on
 *              your system, running the code on a different machine, or
 *              different OS, or even at a different time on the same machine,
 *              may produce different fonts even if --font is given explicitly.
 *              To see names of available fonts, use --list_available_fonts with
 *              the appropriate --fonts_dir path.
 *              Specifying --use_only_legacy_fonts will restrict the available
 *              fonts to those listed in legacy_fonts.h
 * Authors:     Ranjith Unnikrishnan, Ray Smith
 *
 * (C) Copyright 2013, Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **********************************************************************/

#include "boxchar.h"
#include "commandlineflags.h"
#include "commontraining.h" // CheckSharedLibraryVersion
#include "degradeimage.h"
#include "errcode.h"
#include "fileio.h"
#include "helpers.h"
#include "normstrngs.h"
#include "stringrenderer.h"
#include "tlog.h"
#include "unicharset.h"

#include <allheaders.h> // from leptonica

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#  define putenv(s) _putenv(s)
#endif

using namespace tesseract;

// A number with which to initialize the random number generator.
const int kRandomSeed = 0x18273645;

// The text input file.
static STRING_PARAM_FLAG(text, "", "File name of text input to process");

// The text output file.
static STRING_PARAM_FLAG(outputbase, "", "Basename for output image/box file");

// Degrade the rendered image to mimic scanner quality.
static BOOL_PARAM_FLAG(degrade_image, true,
                       "Degrade rendered image with speckle noise, dilation/erosion "
                       "and rotation");

// Rotate the rendered image to have more realistic glyph borders
static BOOL_PARAM_FLAG(rotate_image, true, "Rotate the image in a random way.");

// Degradation to apply to the image.
static INT_PARAM_FLAG(exposure, 0, "Exposure level in photocopier");

// Distort the rendered image by various means according to the bool flags.
static BOOL_PARAM_FLAG(distort_image, false, "Degrade rendered image with noise, blur, invert.");

// Distortion to apply to the image.
static BOOL_PARAM_FLAG(invert, true, "Invert the image");

// Distortion to apply to the image.
static BOOL_PARAM_FLAG(white_noise, true, "Add  Gaussian Noise");

// Distortion to apply to the image.
static BOOL_PARAM_FLAG(smooth_noise, true, "Smoothen Noise");

// Distortion to apply to the image.
static BOOL_PARAM_FLAG(blur, true, "Blur the image");

#if 0

// Distortion to apply to the image.
static BOOL_PARAM_FLAG(perspective, false, "Generate Perspective Distortion");

// Distortion to apply to the image.
static INT_PARAM_FLAG(box_reduction, 0, "Integer reduction factor box_scale");

#endif

// Output image resolution.
static INT_PARAM_FLAG(resolution, 300, "Pixels per inch");

// Width of output image (in pixels).
static INT_PARAM_FLAG(xsize, 3600, "Width of output image");

// Max height of output image (in pixels).
static INT_PARAM_FLAG(ysize, 4800, "Height of output image");

// Max number of pages to produce.
static INT_PARAM_FLAG(max_pages, 0, "Maximum number of pages to output (0=unlimited)");

// Margin around text (in pixels).
static INT_PARAM_FLAG(margin, 100, "Margin round edges of image");

// Size of text (in points).
static INT_PARAM_FLAG(ptsize, 12, "Size of printed text");

// Inter-character space (in ems).
static DOUBLE_PARAM_FLAG(char_spacing, 0, "Inter-character space in ems");

// Sets the probability (value in [0, 1]) of starting to render a word with an
// underline. Words are assumed to be space-delimited.
static DOUBLE_PARAM_FLAG(underline_start_prob, 0,
                         "Fraction of words to underline (value in [0,1])");
// Set the probability (value in [0, 1]) of continuing a started underline to
// the next word.
static DOUBLE_PARAM_FLAG(underline_continuation_prob, 0,
                         "Fraction of words to underline (value in [0,1])");

// Inter-line space (in pixels).
static INT_PARAM_FLAG(leading, 12, "Inter-line space (in pixels)");

// Layout and glyph orientation on rendering.
static STRING_PARAM_FLAG(writing_mode, "horizontal",
                         "Specify one of the following writing"
                         " modes.\n"
                         "'horizontal' : Render regular horizontal text. (default)\n"
                         "'vertical' : Render vertical text. Glyph orientation is"
                         " selected by Pango.\n"
                         "'vertical-upright' : Render vertical text. Glyph "
                         " orientation is set to be upright.");

static INT_PARAM_FLAG(box_padding, 0, "Padding around produced bounding boxes");

static BOOL_PARAM_FLAG(strip_unrenderable_words, true,
                       "Remove unrenderable words from source text");

// Font name.
static STRING_PARAM_FLAG(font, "Arial", "Font description name to use");

static BOOL_PARAM_FLAG(ligatures, false, "Rebuild and render ligatures");

static BOOL_PARAM_FLAG(find_fonts, false, "Search for all fonts that can render the text");
static BOOL_PARAM_FLAG(render_per_font, true,
                       "If find_fonts==true, render each font to its own image. "
                       "Image filenames are of the form output_name.font_name.tif");
static DOUBLE_PARAM_FLAG(min_coverage, 1.0,
                         "If find_fonts==true, the minimum coverage the font has of "
                         "the characters in the text file to include it, between "
                         "0 and 1.");

static BOOL_PARAM_FLAG(list_available_fonts, false, "List available fonts and quit.");

static BOOL_PARAM_FLAG(render_ngrams, false,
                       "Put each space-separated entity from the"
                       " input file into one bounding box. The ngrams in the input"
                       " file will be randomly permuted before rendering (so that"
                       " there is sufficient variety of characters on each line).");

static BOOL_PARAM_FLAG(output_word_boxes, false,
                       "Output word bounding boxes instead of character boxes. "
                       "This is used for Cube training, and implied by "
                       "--render_ngrams.");

static STRING_PARAM_FLAG(unicharset_file, "",
                         "File with characters in the unicharset. If --render_ngrams"
                         " is true and --unicharset_file is specified, ngrams with"
                         " characters that are not in unicharset will be omitted");

static BOOL_PARAM_FLAG(bidirectional_rotation, false, "Rotate the generated characters both ways.");

static BOOL_PARAM_FLAG(only_extract_font_properties, false,
                       "Assumes that the input file contains a list of ngrams. Renders"
                       " each ngram, extracts spacing properties and records them in"
                       " output_base/[font_name].fontinfo file.");

// Use these flags to output zero-padded, square individual character images
static BOOL_PARAM_FLAG(output_individual_glyph_images, false,
                       "If true also outputs individual character images");
static INT_PARAM_FLAG(glyph_resized_size, 0,
                      "Each glyph is square with this side length in pixels");
static INT_PARAM_FLAG(glyph_num_border_pixels_to_pad, 0,
                      "Final_size=glyph_resized_size+2*glyph_num_border_pixels_to_pad");

namespace tesseract {

struct SpacingProperties {
  SpacingProperties() : x_gap_before(0), x_gap_after(0) {}
  SpacingProperties(int b, int a) : x_gap_before(b), x_gap_after(a) {}
  // These values are obtained from FT_Glyph_Metrics struct
  // used by the FreeType font engine.
  int x_gap_before; // horizontal x bearing
  int x_gap_after;  // horizontal advance - x_gap_before - width
  std::map<std::string, int> kerned_x_gaps;
};

static bool IsWhitespaceBox(const BoxChar *boxchar) {
  return (boxchar->box() == nullptr || SpanUTF8Whitespace(boxchar->ch().c_str()));
}

static std::string StringReplace(const std::string &in, const std::string &oldsub,
                                 const std::string &newsub) {
  std::string out;
  size_t start_pos = 0, pos;
  while ((pos = in.find(oldsub, start_pos)) != std::string::npos) {
    out.append(in.data() + start_pos, pos - start_pos);
    out.append(newsub.data(), newsub.length());
    start_pos = pos + oldsub.length();
  }
  out.append(in.data() + start_pos, in.length() - start_pos);
  return out;
}

// Assumes that each word (whitespace-separated entity) in text is a bigram.
// Renders the bigrams and calls FontInfo::GetSpacingProperties() to
// obtain spacing information. Produces the output .fontinfo file with a line
// per unichar of the form:
// unichar space_before space_after kerned1 kerned_space1 kerned2 ...
// Fox example, if unichar "A" has spacing of 0 pixels before and -1 pixels
// after, is kerned with "V" resulting in spacing of "AV" to be -7 and kerned
// with "T", such that "AT" has spacing of -5, the entry/line for unichar "A"
// in .fontinfo file will be:
// A 0 -1 T -5 V -7
static void ExtractFontProperties(const std::string &utf8_text, StringRenderer *render,
                                  const std::string &output_base) {
  std::map<std::string, SpacingProperties> spacing_map;
  std::map<std::string, SpacingProperties>::iterator spacing_map_it0;
  std::map<std::string, SpacingProperties>::iterator spacing_map_it1;
  int x_bearing, x_advance;
  int len = utf8_text.length();
  int offset = 0;
  const char *text = utf8_text.c_str();
  while (offset < len) {
    offset += render->RenderToImage(text + offset, strlen(text + offset), nullptr);
    const std::vector<BoxChar *> &boxes = render->GetBoxes();

    // If the page break split a bigram, correct the offset so we try the bigram
    // on the next iteration.
    if (boxes.size() > 2 && !IsWhitespaceBox(boxes[boxes.size() - 1]) &&
        IsWhitespaceBox(boxes[boxes.size() - 2])) {
      if (boxes.size() > 3) {
        tprintf("WARNING: Adjusting to bad page break after '%s%s'\n",
                boxes[boxes.size() - 4]->ch().c_str(), boxes[boxes.size() - 3]->ch().c_str());
      }
      offset -= boxes[boxes.size() - 1]->ch().size();
    }

    for (size_t b = 0; b < boxes.size(); b += 2) {
      while (b < boxes.size() && IsWhitespaceBox(boxes[b])) {
        ++b;
      }
      if (b + 1 >= boxes.size()) {
        break;
      }
      const std::string &ch0 = boxes[b]->ch();
      // We encountered a ligature. This happens in at least two scenarios:
      // One is when the rendered bigram forms a grapheme cluster (eg. the
      // second character in the bigram is a combining vowel), in which case we
      // correctly output only one bounding box.
      // A second far less frequent case is when caused some fonts like 'DejaVu
      // Sans Ultra-Light' force Pango to render a ligatured character even if
      // the input consists of the separated characters.  NOTE(ranjith): As per
      // behdad@ this is not currently controllable at the level of the Pango
      // API.
      // The most frequent of all is a single character "word" made by the CJK
      // segmenter.
      // Safeguard against these cases here by just skipping the bigram.
      if (IsWhitespaceBox(boxes[b + 1])) {
        continue;
      }
      int xgap = (boxes[b + 1]->box()->x - (boxes[b]->box()->x + boxes[b]->box()->w));
      spacing_map_it0 = spacing_map.find(ch0);
      int ok_count = 0;
      if (spacing_map_it0 == spacing_map.end() &&
          render->font().GetSpacingProperties(ch0, &x_bearing, &x_advance)) {
        spacing_map[ch0] = SpacingProperties(x_bearing, x_advance - x_bearing - boxes[b]->box()->w);
        spacing_map_it0 = spacing_map.find(ch0);
        ++ok_count;
      }
      const std::string &ch1 = boxes[b + 1]->ch();
      tlog(3, "%s%s\n", ch0.c_str(), ch1.c_str());
      spacing_map_it1 = spacing_map.find(ch1);
      if (spacing_map_it1 == spacing_map.end() &&
          render->font().GetSpacingProperties(ch1, &x_bearing, &x_advance)) {
        spacing_map[ch1] =
            SpacingProperties(x_bearing, x_advance - x_bearing - boxes[b + 1]->box()->w);
        spacing_map_it1 = spacing_map.find(ch1);
        ++ok_count;
      }
      if (ok_count == 2 &&
          xgap != (spacing_map_it0->second.x_gap_after + spacing_map_it1->second.x_gap_before)) {
        spacing_map_it0->second.kerned_x_gaps[ch1] = xgap;
      }
    }
    render->ClearBoxes();
  }
  std::string output_string;
  const int kBufSize = 1024;
  char buf[kBufSize];
  snprintf(buf, kBufSize, "%d\n", static_cast<int>(spacing_map.size()));
  output_string.append(buf);
  std::map<std::string, SpacingProperties>::const_iterator spacing_map_it;
  for (spacing_map_it = spacing_map.begin(); spacing_map_it != spacing_map.end();
       ++spacing_map_it) {
    snprintf(buf, kBufSize, "%s %d %d %d", spacing_map_it->first.c_str(),
             spacing_map_it->second.x_gap_before, spacing_map_it->second.x_gap_after,
             static_cast<int>(spacing_map_it->second.kerned_x_gaps.size()));
    output_string.append(buf);
    std::map<std::string, int>::const_iterator kern_it;
    for (kern_it = spacing_map_it->second.kerned_x_gaps.begin();
         kern_it != spacing_map_it->second.kerned_x_gaps.end(); ++kern_it) {
      snprintf(buf, kBufSize, " %s %d", kern_it->first.c_str(), kern_it->second);
      output_string.append(buf);
    }
    output_string.append("\n");
  }
  File::WriteStringToFileOrDie(output_string, output_base + ".fontinfo");
}

static bool MakeIndividualGlyphs(Image pix, const std::vector<BoxChar *> &vbox,
                                 const int input_tiff_page) {
  // If checks fail, return false without exiting text2image
  if (!pix) {
    tprintf("ERROR: MakeIndividualGlyphs(): Input Pix* is nullptr\n");
    return false;
  } else if (FLAGS_glyph_resized_size <= 0) {
    tprintf("ERROR: --glyph_resized_size must be positive\n");
    return false;
  } else if (FLAGS_glyph_num_border_pixels_to_pad < 0) {
    tprintf("ERROR: --glyph_num_border_pixels_to_pad must be 0 or positive\n");
    return false;
  }

  const int n_boxes = vbox.size();
  int n_boxes_saved = 0;
  int current_tiff_page = 0;
  int y_previous = 0;
  static int glyph_count = 0;
  for (int i = 0; i < n_boxes; i++) {
    // Get one bounding box
    Box *b = vbox[i]->mutable_box();
    if (!b) {
      continue;
    }
    const int x = b->x;
    const int y = b->y;
    const int w = b->w;
    const int h = b->h;
    // Check present tiff page (for multipage tiff)
    if (y < y_previous - pixGetHeight(pix) / 10) {
      tprintf("ERROR: Wrap-around encountered, at i=%d\n", i);
      current_tiff_page++;
    }
    if (current_tiff_page < input_tiff_page) {
      continue;
    } else if (current_tiff_page > input_tiff_page) {
      break;
    }
    // Check box validity
    if (x < 0 || y < 0 || (x + w - 1) >= pixGetWidth(pix) || (y + h - 1) >= pixGetHeight(pix)) {
      tprintf(
          "ERROR: MakeIndividualGlyphs(): Index out of range, at i=%d"
          " (x=%d, y=%d, w=%d, h=%d\n)",
          i, x, y, w, h);
      continue;
    } else if (w < FLAGS_glyph_num_border_pixels_to_pad &&
               h < FLAGS_glyph_num_border_pixels_to_pad) {
      tprintf("ERROR: Input image too small to be a character, at i=%d\n", i);
      continue;
    }
    // Crop the boxed character
    Image pix_glyph = pixClipRectangle(pix, b, nullptr);
    if (!pix_glyph) {
      tprintf("ERROR: MakeIndividualGlyphs(): Failed to clip, at i=%d\n", i);
      continue;
    }
    // Resize to square
    Image pix_glyph_sq =
        pixScaleToSize(pix_glyph, FLAGS_glyph_resized_size, FLAGS_glyph_resized_size);
    if (!pix_glyph_sq) {
      tprintf("ERROR: MakeIndividualGlyphs(): Failed to resize, at i=%d\n", i);
      continue;
    }
    // Zero-pad
    Image pix_glyph_sq_pad = pixAddBorder(pix_glyph_sq, FLAGS_glyph_num_border_pixels_to_pad, 0);
    if (!pix_glyph_sq_pad) {
      tprintf("ERROR: MakeIndividualGlyphs(): Failed to zero-pad, at i=%d\n", i);
      continue;
    }
    // Write out
    Image pix_glyph_sq_pad_8 = pixConvertTo8(pix_glyph_sq_pad, false);
    char filename[1024];
    snprintf(filename, 1024, "%s_%d.jpg", FLAGS_outputbase.c_str(), glyph_count++);
    if (pixWriteJpeg(filename, pix_glyph_sq_pad_8, 100, 0)) {
      tprintf(
          "ERROR: MakeIndividualGlyphs(): Failed to write JPEG to %s,"
          " at i=%d\n",
          filename, i);
      continue;
    }

    pix_glyph.destroy();
    pix_glyph_sq.destroy();
    pix_glyph_sq_pad.destroy();
    pix_glyph_sq_pad_8.destroy();
    n_boxes_saved++;
    y_previous = y;
  }
  if (n_boxes_saved == 0) {
    return false;
  } else {
    tprintf("Total number of characters saved = %d\n", n_boxes_saved);
    return true;
  }
}
} // namespace tesseract

using tesseract::DegradeImage;
using tesseract::ExtractFontProperties;
using tesseract::File;
using tesseract::FontUtils;
using tesseract::SpanUTF8NotWhitespace;
using tesseract::SpanUTF8Whitespace;
using tesseract::StringRenderer;

static int Main() {
  if (FLAGS_list_available_fonts) {
    const std::vector<std::string> &all_fonts = FontUtils::ListAvailableFonts();
    for (unsigned int i = 0; i < all_fonts.size(); ++i) {
      // Remove trailing comma: pango-font-description-to-string adds a comma
      // to some fonts.
      // See https://github.com/tesseract-ocr/tesseract/issues/408
      std::string font_name(all_fonts[i].c_str());
      if (font_name.back() == ',') {
        font_name.pop_back();
      }
      printf("%3u: %s\n", i, font_name.c_str());
      ASSERT_HOST_MSG(FontUtils::IsAvailableFont(all_fonts[i].c_str()),
                      "Font %s is unrecognized.\n", all_fonts[i].c_str());
    }
    return EXIT_SUCCESS;
  }

  // Check validity of input flags.
  if (FLAGS_text.empty()) {
    tprintf("'--text' option is missing!\n");
    return EXIT_FAILURE;
  }
  if (FLAGS_outputbase.empty()) {
    tprintf("'--outputbase' option is missing!\n");
    return EXIT_FAILURE;
  }
  if (!FLAGS_unicharset_file.empty() && FLAGS_render_ngrams) {
    tprintf("Use '--unicharset_file' only if '--render_ngrams' is set.\n");
    return EXIT_FAILURE;
  }

  std::string font_name = FLAGS_font.c_str();
  if (!FLAGS_find_fonts && !FontUtils::IsAvailableFont(font_name.c_str())) {
    font_name += ',';
    std::string pango_name;
    if (!FontUtils::IsAvailableFont(font_name.c_str(), &pango_name)) {
      tprintf("Could not find font named '%s'.\n", FLAGS_font.c_str());
      if (!pango_name.empty()) {
        tprintf("Pango suggested font '%s'.\n", pango_name.c_str());
      }
      tprintf("Please correct --font arg.\n");
      return EXIT_FAILURE;
    }
  }

  if (FLAGS_render_ngrams) {
    FLAGS_output_word_boxes = true;
  }

  char font_desc_name[1024];
  snprintf(font_desc_name, 1024, "%s %d", font_name.c_str(), static_cast<int>(FLAGS_ptsize));

  StringRenderer render(font_desc_name, FLAGS_xsize, FLAGS_ysize);
  render.set_add_ligatures(FLAGS_ligatures);
  render.set_leading(FLAGS_leading);
  render.set_resolution(FLAGS_resolution);
  render.set_char_spacing(FLAGS_char_spacing * FLAGS_ptsize);
  render.set_h_margin(FLAGS_margin);
  render.set_v_margin(FLAGS_margin);
  render.set_output_word_boxes(FLAGS_output_word_boxes);
  render.set_box_padding(FLAGS_box_padding);
  render.set_strip_unrenderable_words(FLAGS_strip_unrenderable_words);
  render.set_underline_start_prob(FLAGS_underline_start_prob);
  render.set_underline_continuation_prob(FLAGS_underline_continuation_prob);

  // Set text rendering orientation and their forms.
  if (FLAGS_writing_mode == "horizontal") {
    // Render regular horizontal text (default).
    render.set_vertical_text(false);
    render.set_gravity_hint_strong(false);
    render.set_render_fullwidth_latin(false);
  } else if (FLAGS_writing_mode == "vertical") {
    // Render vertical text. Glyph orientation is selected by Pango.
    render.set_vertical_text(true);
    render.set_gravity_hint_strong(false);
    render.set_render_fullwidth_latin(false);
  } else if (FLAGS_writing_mode == "vertical-upright") {
    // Render vertical text. Glyph orientation is set to be upright.
    // Also Basic Latin characters are converted to their fullwidth forms
    // on rendering, since fullwidth Latin characters are well designed to fit
    // vertical text lines, while .box files store halfwidth Basic Latin
    // unichars.
    render.set_vertical_text(true);
    render.set_gravity_hint_strong(true);
    render.set_render_fullwidth_latin(true);
  } else {
    tprintf("Invalid writing mode: %s\n", FLAGS_writing_mode.c_str());
    return EXIT_FAILURE;
  }

  std::string src_utf8;
  // This c_str is NOT redundant!
  if (!File::ReadFileToString(FLAGS_text.c_str(), &src_utf8)) {
    tprintf("Failed to read file: %s\n", FLAGS_text.c_str());
    return EXIT_FAILURE;
  }

  // Remove the unicode mark if present.
  if (strncmp(src_utf8.c_str(), "\xef\xbb\xbf", 3) == 0) {
    src_utf8.erase(0, 3);
  }
  tlog(1, "Render string of size %zu\n", src_utf8.length());

  if (FLAGS_render_ngrams || FLAGS_only_extract_font_properties) {
    // Try to preserve behavior of old text2image by expanding inter-word
    // spaces by a factor of 4.
    const std::string kSeparator = FLAGS_render_ngrams ? "    " : " ";
    // Also restrict the number of characters per line to try and avoid
    // line-breaking in the middle of words like "-A", "R$" etc. which are
    // otherwise allowed by the standard unicode line-breaking rules.
    const unsigned int kCharsPerLine = (FLAGS_ptsize > 20) ? 50 : 100;
    std::string rand_utf8;
    UNICHARSET unicharset;
    if (FLAGS_render_ngrams && !FLAGS_unicharset_file.empty() &&
        !unicharset.load_from_file(FLAGS_unicharset_file.c_str())) {
      tprintf("Failed to load unicharset from file %s\n", FLAGS_unicharset_file.c_str());
      return EXIT_FAILURE;
    }

    // If we are rendering ngrams that will be OCRed later, shuffle them so that
    // tesseract does not have difficulties finding correct baseline, word
    // spaces, etc.
    const char *str8 = src_utf8.c_str();
    int len = src_utf8.length();
    int step;
    std::vector<std::pair<int, int>> offsets;
    int offset = SpanUTF8Whitespace(str8);
    while (offset < len) {
      step = SpanUTF8NotWhitespace(str8 + offset);
      offsets.emplace_back(offset, step);
      offset += step;
      offset += SpanUTF8Whitespace(str8 + offset);
    }
    if (FLAGS_render_ngrams) {
      std::seed_seq seed{kRandomSeed};
      std::mt19937 random_gen(seed);
      std::shuffle(offsets.begin(), offsets.end(), random_gen);
    }

    for (size_t i = 0, line = 1; i < offsets.size(); ++i) {
      const char *curr_pos = str8 + offsets[i].first;
      int ngram_len = offsets[i].second;
      // Skip words that contain characters not in found in unicharset.
      std::string cleaned = UNICHARSET::CleanupString(curr_pos, ngram_len);
      if (!FLAGS_unicharset_file.empty() &&
          !unicharset.encodable_string(cleaned.c_str(), nullptr)) {
        continue;
      }
      rand_utf8.append(curr_pos, ngram_len);
      if (rand_utf8.length() > line * kCharsPerLine) {
        rand_utf8.append(" \n");
        ++line;
        if (line & 0x1) {
          rand_utf8.append(kSeparator);
        }
      } else {
        rand_utf8.append(kSeparator);
      }
    }
    tlog(1, "Rendered ngram string of size %zu\n", rand_utf8.length());
    src_utf8.swap(rand_utf8);
  }
  if (FLAGS_only_extract_font_properties) {
    tprintf("Extracting font properties only\n");
    ExtractFontProperties(src_utf8, &render, FLAGS_outputbase.c_str());
    tprintf("Done!\n");
    return EXIT_SUCCESS;
  }

  int im = 0;
  std::vector<float> page_rotation;
  const char *to_render_utf8 = src_utf8.c_str();

  tesseract::TRand randomizer;
  randomizer.set_seed(kRandomSeed);
  std::vector<std::string> font_names;
  // We use a two pass mechanism to rotate images in both direction.
  // The first pass(0) will rotate the images in random directions and
  // the second pass(1) will mirror those rotations.
  int num_pass = FLAGS_bidirectional_rotation ? 2 : 1;
  for (int pass = 0; pass < num_pass; ++pass) {
    int page_num = 0;
    std::string font_used;
    for (size_t offset = 0;
         offset < strlen(to_render_utf8) && (FLAGS_max_pages == 0 || page_num < FLAGS_max_pages);
         ++im, ++page_num) {
      tlog(1, "Starting page %d\n", im);
      Image pix = nullptr;
      if (FLAGS_find_fonts) {
        offset += render.RenderAllFontsToImage(FLAGS_min_coverage, to_render_utf8 + offset,
                                               strlen(to_render_utf8 + offset), &font_used, &pix);
      } else {
        offset +=
            render.RenderToImage(to_render_utf8 + offset, strlen(to_render_utf8 + offset), &pix);
      }
      if (pix != nullptr) {
        float rotation = 0;
        if (pass == 1) {
          // Pass 2, do mirror rotation.
          rotation = -1 * page_rotation[page_num];
        }
        if (FLAGS_degrade_image) {
          pix = DegradeImage(pix, FLAGS_exposure, &randomizer,
                             FLAGS_rotate_image ? &rotation : nullptr);
        }
        if (FLAGS_distort_image) {
          // TODO: perspective is set to false and box_reduction to 1.
          pix = PrepareDistortedPix(pix, false, FLAGS_invert, FLAGS_white_noise, FLAGS_smooth_noise,
                                    FLAGS_blur, 1, &randomizer, nullptr);
        }
        render.RotatePageBoxes(rotation);

        if (pass == 0) {
          // Pass 1, rotate randomly and store the rotation..
          page_rotation.push_back(rotation);
        }

        Image gray_pix = pixConvertTo8(pix, false);
        pix.destroy();
        Image binary = pixThresholdToBinary(gray_pix, 128);
        gray_pix.destroy();
        char tiff_name[1024];
        if (FLAGS_find_fonts) {
          if (FLAGS_render_per_font) {
            std::string fontname_for_file = tesseract::StringReplace(font_used, " ", "_");
            snprintf(tiff_name, 1024, "%s.%s.tif", FLAGS_outputbase.c_str(),
                     fontname_for_file.c_str());
            pixWriteTiff(tiff_name, binary, IFF_TIFF_G4, "w");
            tprintf("Rendered page %d to file %s\n", im, tiff_name);
          } else {
            font_names.push_back(font_used);
          }
        } else {
          snprintf(tiff_name, 1024, "%s.tif", FLAGS_outputbase.c_str());
          pixWriteTiff(tiff_name, binary, IFF_TIFF_G4, im == 0 ? "w" : "a");
          tprintf("Rendered page %d to file %s\n", im, tiff_name);
        }
        // Make individual glyphs
        if (FLAGS_output_individual_glyph_images) {
          if (!MakeIndividualGlyphs(binary, render.GetBoxes(), im)) {
            tprintf("ERROR: Individual glyphs not saved\n");
          }
        }
        binary.destroy();
      }
      if (FLAGS_find_fonts && offset != 0) {
        // We just want a list of names, or some sample images so we don't need
        // to render more than the first page of the text.
        break;
      }
    }
  }
  if (!FLAGS_find_fonts) {
    std::string box_name = FLAGS_outputbase.c_str();
    box_name += ".box";
    render.WriteAllBoxes(box_name);
  } else if (!FLAGS_render_per_font && !font_names.empty()) {
    std::string filename = FLAGS_outputbase.c_str();
    filename += ".fontlist.txt";
    FILE *fp = fopen(filename.c_str(), "wb");
    if (fp == nullptr) {
      tprintf("Failed to create output font list %s\n", filename.c_str());
    } else {
      for (auto &font_name : font_names) {
        fprintf(fp, "%s\n", font_name.c_str());
      }
      fclose(fp);
    }
  }

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  // Respect environment variable. could be:
  // fc (fontconfig), win32, and coretext
  // If not set force fontconfig for Mac OS.
  // See https://github.com/tesseract-ocr/tesseract/issues/736
  char *backend;
  backend = getenv("PANGOCAIRO_BACKEND");
  if (backend == nullptr) {
    static char envstring[] = "PANGOCAIRO_BACKEND=fc";
    putenv(envstring);
  } else {
    printf(
        "Using '%s' as pango cairo backend based on environment "
        "variable.\n",
        backend);
  }
  tesseract::CheckSharedLibraryVersion();
  if (argc > 1) {
    if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
      FontUtils::PangoFontTypeInfo();
      printf("Pango version: %s\n", pango_version_string());
    }
  }
  tesseract::ParseCommandLineFlags(argv[0], &argc, &argv, true);
  return Main();
}
