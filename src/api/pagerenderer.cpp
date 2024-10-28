// File:        pagerenderer.cpp
// Description: PAGE XML rendering interface
// Author:      Jan Kamlah

// (C) Copyright 2024
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "errcode.h" // for ASSERT_HOST
#include "helpers.h" // for copy_string
#include "tprintf.h" // for tprintf

#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>

#include <ctime>
#include <iomanip>
#include <memory>
#include <regex>
#include <sstream> // for std::stringstream
#include <unordered_set>

#include <allheaders.h>
#if (LIBLEPT_MAJOR_VERSION == 1 && LIBLEPT_MINOR_VERSION >= 83) || \
    LIBLEPT_MAJOR_VERSION > 1
#  include <array_internal.h>
#  include <pix_internal.h>
#endif

namespace tesseract {

///
/// Slope and offset between two points
///
static void GetSlopeAndOffset(float x0, float y0, float x1, float y1, float *m,
                              float *b) {
  float slope;

  slope = ((y1 - y0) / (x1 - x0));
  *m = slope;
  *b = y0 - slope * x0;
}

///
/// Write coordinates in the form of a points to a stream
///
static void AddPointsToPAGE(Pta *pts, std::stringstream &str) {
  int num_pts;

  str << "<Coords points=\"";
  num_pts = ptaGetCount(pts);
  for (int p = 0; p < num_pts; ++p) {
    int x, y;
    ptaGetIPt(pts, p, &x, &y);
    if (p != 0) {
      str << " ";
    }
    str << std::to_string(x) << "," << std::to_string(y);
  }
  str << "\"/>\n";
}

///
/// Convert bbox information to top and bottom polygon
///
static void AddPointToWordPolygon(
    const ResultIterator *res_it, PageIteratorLevel level, Pta *word_top_pts,
    Pta *word_bottom_pts, tesseract::WritingDirection writing_direction) {
  int left, top, right, bottom;

  res_it->BoundingBox(level, &left, &top, &right, &bottom);

  if (writing_direction != WRITING_DIRECTION_TOP_TO_BOTTOM) {
    ptaAddPt(word_top_pts, left, top);
    ptaAddPt(word_top_pts, right, top);

    ptaAddPt(word_bottom_pts, left, bottom);
    ptaAddPt(word_bottom_pts, right, bottom);

  } else {
    // Transform from ttb to ltr
    ptaAddPt(word_top_pts, top, right);
    ptaAddPt(word_top_pts, bottom, right);

    ptaAddPt(word_bottom_pts, top, left);
    ptaAddPt(word_bottom_pts, bottom, left);
  }
}

///
/// Transpose polygonline, destroy old and return new pts
///
Pta *TransposePolygonline(Pta *pts) {
  Pta *pts_transposed;

  pts_transposed = ptaTranspose(pts);
  ptaDestroy(&pts);
  return pts_transposed;
}

///
/// Reverse polygonline, destroy old and return new pts
///
Pta *ReversePolygonline(Pta *pts, int type) {
  Pta *pts_reversed;

  pts_reversed = ptaReverse(pts, type);
  ptaDestroy(&pts);
  return pts_reversed;
}

///
/// Destroy old and create new pts
///
Pta *DestroyAndCreatePta(Pta *pts) {
  ptaDestroy(&pts);
  return ptaCreate(0);
}

///
/// Recalculate linepolygon
/// Create a hull for overlapping areas
///
Pta *RecalcPolygonline(Pta *pts, bool upper) {
  int num_pts, num_bin, index = 0;
  int y, x0, y0, x1, y1;
  float x_min, y_min, x_max, y_max;
  NUMA *bin_line;
  Pta *pts_recalc;

  ptaGetMinMax(pts, &x_min, &y_min, &x_max, &y_max);
  num_bin = x_max - x_min;
  bin_line = numaCreate(num_bin + 1);

  for (int p = 0; p <= num_bin; ++p) {
    bin_line->array[p] = -1.;
  }

  num_pts = ptaGetCount(pts);

  if (num_pts == 2) {
    pts_recalc = ptaCopy(pts);
    ptaDestroy(&pts);
    return pts_recalc;
  }

  do {
    ptaGetIPt(pts, index, &x0, &y0);
    ptaGetIPt(pts, index + 1, &x1, &y1);
    for (int p = x0 - x_min; p <= x1 - x_min; ++p) {
      if (!upper) {
        if (bin_line->array[p] == -1. || y0 > bin_line->array[p]) {
          bin_line->array[p] = y0;
        }
      } else {
        if (bin_line->array[p] == -1. || y0 < bin_line->array[p]) {
          bin_line->array[p] = y0;
        }
      }
    }
    index += 2;
  } while (index < num_pts - 1);

  pts_recalc = ptaCreate(0);

  for (int p = 0; p <= num_bin; ++p) {
    if (p == 0) {
      y = bin_line->array[p];
      ptaAddPt(pts_recalc, x_min + p, y);
    } else if (p == num_bin) {
      ptaAddPt(pts_recalc, x_min + p, y);
      break;
    } else if (y != bin_line->array[p]) {
      if (y != -1.) {
        ptaAddPt(pts_recalc, x_min + p, y);
      }
      y = bin_line->array[p];
      if (y != -1.) {
        ptaAddPt(pts_recalc, x_min + p, y);
      }
    }
  }

  ptaDestroy(&pts);
  return pts_recalc;
}

///
/// Create a rectangle hull around a single line
///
Pta *PolygonToBoxCoords(Pta *pts) {
  Pta *pts_box;
  float x_min, y_min, x_max, y_max;

  pts_box = ptaCreate(0);
  ptaGetMinMax(pts, &x_min, &y_min, &x_max, &y_max);
  ptaAddPt(pts_box, x_min, y_min);
  ptaAddPt(pts_box, x_max, y_min);
  ptaAddPt(pts_box, x_max, y_max);
  ptaAddPt(pts_box, x_min, y_max);
  ptaDestroy(&pts);
  return pts_box;
}

///
/// Create a rectangle polygon round the existing multiple lines
///
static void UpdateBlockPoints(Pta *block_top_pts, Pta *block_bottom_pts,
                              Pta *line_top_pts, Pta *line_bottom_pts, int lcnt,
                              int last_word_in_cblock) {
  int num_pts;
  int x, y;

  // Create a hull around all lines
  if (lcnt == 0 && last_word_in_cblock) {
    ptaJoin(block_top_pts, line_top_pts, 0, -1);
    ptaJoin(block_bottom_pts, line_bottom_pts, 0, -1);
  } else if (lcnt == 0) {
    ptaJoin(block_top_pts, line_top_pts, 0, -1);
    num_pts = ptaGetCount(line_bottom_pts);
    ptaGetIPt(line_bottom_pts, num_pts - 1, &x, &y);
    ptaAddPt(block_top_pts, x, y);
    ptaGetIPt(line_bottom_pts, 0, &x, &y);
    ptaAddPt(block_bottom_pts, x, y);
  } else if (last_word_in_cblock) {
    ptaGetIPt(line_top_pts, 0, &x, &y);
    ptaAddPt(block_bottom_pts, x, y);
    ptaJoin(block_bottom_pts, line_bottom_pts, 0, -1);
    num_pts = ptaGetCount(line_top_pts);
    ptaGetIPt(line_top_pts, num_pts - 1, &x, &y);
    ptaAddPt(block_top_pts, x, y);
  } else {
    ptaGetIPt(line_top_pts, 0, &x, &y);
    ptaAddPt(block_bottom_pts, x, y);
    ptaGetIPt(line_bottom_pts, 0, &x, &y);
    ptaAddPt(block_bottom_pts, x, y);
    num_pts = ptaGetCount(line_top_pts);
    ptaGetIPt(line_top_pts, num_pts - 1, &x, &y);
    ptaAddPt(block_top_pts, x, y);
    num_pts = ptaGetCount(line_bottom_pts);
    ptaGetIPt(line_bottom_pts, num_pts - 1, &x, &y);
    ptaAddPt(block_top_pts, x, y);
  };
}

///
/// Simplify polygonlines (only expanding not shrinking) (Due to recalculation
/// currently not necessary)
///
static void SimplifyLinePolygon(Pta *polyline, int tolerance, bool upper) {
  int x0, y0, x1, y1, x2, y2, x3, y3, index = 1;
  float m, b, y_min, y_max;

  while (index <= polyline->n - 2) {
    ptaGetIPt(polyline, index - 1, &x0, &y0);
    ptaGetIPt(polyline, index, &x1, &y1);
    ptaGetIPt(polyline, index + 1, &x2, &y2);
    if (index + 2 < polyline->n) {
      // Delete two point indentations
      ptaGetIPt(polyline, index + 2, &x3, &y3);
      if (abs(x3 - x0) <= tolerance * 2) {
        GetSlopeAndOffset(x0, y0, x3, y3, &m, &b);

        if (upper && (m * x1 + b) < y1 && (m * x2 + b) < y2) {
          ptaRemovePt(polyline, index + 1);
          ptaRemovePt(polyline, index);
          continue;
        } else if (!upper && (m * x1 + b) > y1 && (m * x2 + b) > y2) {
          ptaRemovePt(polyline, index + 1);
          ptaRemovePt(polyline, index);
          continue;
        }
      }
    }
    // Delete one point indentations
    if (abs(y0 - y1) <= tolerance && abs(y1 - y2) <= tolerance) {
      GetSlopeAndOffset(x0, y0, x2, y2, &m, &b);
      if (upper && (m * x1 + b) <= y1) {
        ptaRemovePt(polyline, index);
        continue;
      } else if (!upper && (m * x1 + b) >= y1) {
        ptaRemovePt(polyline, index);
        continue;
      }
    }
    // Delete near by points
    if (x1 != x0 && abs(y1 - y0) < 4 && abs(x1 - x0) <= tolerance) {
      if (upper) {
        y_min = std::min(y0, y1);
        GetSlopeAndOffset(x0, y_min, x2, y2, &m, &b);
        if ((m * x1 + b) <= y1) {
          polyline->y[index - 1] = std::min(y0, y1);
          ptaRemovePt(polyline, index);
          continue;
        }
      } else {
        y_max = std::max(y0, y1);
        GetSlopeAndOffset(x0, y_max, x2, y2, &m, &b);
        if ((m * x1 + b) >= y1) {
          polyline->y[index - 1] = y_max;
          ptaRemovePt(polyline, index);
          continue;
        }
      }
    }
    index++;
  }
}

///
/// Directly write bounding box information as coordinates a stream
///
static void AddBoxToPAGE(const ResultIterator *it, PageIteratorLevel level,
                         std::stringstream &page_str) {
  int left, top, right, bottom;

  it->BoundingBox(level, &left, &top, &right, &bottom);
  page_str << "<Coords points=\"" << left << "," << top << " " << right << ","
           << top << " " << right << "," << bottom << " " << left << ","
           << bottom << "\"/>\n";
}

///
/// Join ltr and rtl polygon information
///
static void AppendLinePolygon(Pta *pts_ltr, Pta *pts_rtl, Pta *ptss,
                              tesseract::WritingDirection writing_direction) {
  // If writing direction is NOT right-to-left, handle the left-to-right case.
  if (writing_direction != WRITING_DIRECTION_RIGHT_TO_LEFT) {
    if (ptaGetCount(pts_rtl) != 0) {
      ptaJoin(pts_ltr, pts_rtl, 0, -1);
      DestroyAndCreatePta(pts_rtl);
    }
    ptaJoin(pts_ltr, ptss, 0, -1);
  } else {
    // For right-to-left, work with a copy of ptss initially.
    PTA *ptsd = ptaCopy(ptss);
    if (ptaGetCount(pts_rtl) != 0) {
      ptaJoin(ptsd, pts_rtl, 0, -1);
    }
    ptaDestroy(&pts_rtl);
    ptaCopy(ptsd);
  }
}

///
/// Convert baseline to points and add to polygon
///
static void AddBaselineToPTA(const ResultIterator *it, PageIteratorLevel level,
                             Pta *baseline_pts) {
  int x1, y1, x2, y2;

  it->Baseline(level, &x1, &y1, &x2, &y2);
  ptaAddPt(baseline_pts, x1, y1);
  ptaAddPt(baseline_pts, x2, y2);
}

///
/// Directly write baseline information as baseline points a stream
///
static void AddBaselinePtsToPAGE(Pta *baseline_pts, std::stringstream &str) {
  int x, y, num_pts = baseline_pts->n;

  str << "<Baseline points=\"";
  for (int p = 0; p < num_pts; ++p) {
    ptaGetIPt(baseline_pts, p, &x, &y);
    if (p != 0) {
      str << " ";
    }
    str << std::to_string(x) << "," << std::to_string(y);
  }
  str << "\"/>\n";
}

///
/// Sort baseline points ascending and deleting duplicates
///
Pta *SortBaseline(Pta *baseline_pts,
                  tesseract::WritingDirection writing_direction) {
  int num_pts, index = 0;
  float x0, y0, x1, y1;
  Pta *sorted_baseline_pts;

  sorted_baseline_pts =
      ptaSort(baseline_pts, L_SORT_BY_X, L_SORT_INCREASING, nullptr);

  do {
    ptaGetPt(sorted_baseline_pts, index, &x0, &y0);
    ptaGetPt(sorted_baseline_pts, index + 1, &x1, &y1);
    if (x0 >= x1) {
      sorted_baseline_pts->y[index] = std::min(y0, y1);
      ptaRemovePt(sorted_baseline_pts, index + 1);
    } else {
      index++;
    }
    num_pts = ptaGetCount(sorted_baseline_pts);
  } while (index < num_pts - 1);

  ptaDestroy(&baseline_pts);
  return sorted_baseline_pts;
}

///
/// Clip baseline to range of the exsitings polygon and simplifies the baseline
/// linepolygon
///
Pta *ClipAndSimplifyBaseline(Pta *bottom_pts, Pta *baseline_pts,
                             tesseract::WritingDirection writing_direction) {
  int num_pts;
  float m, b, x0, y0, x1, y1;
  float x_min, y_min, x_max, y_max;
  Pta *baseline_clipped_pts;

  ptaGetMinMax(bottom_pts, &x_min, &y_min, &x_max, &y_max);
  num_pts = ptaGetCount(baseline_pts);
  baseline_clipped_pts = ptaCreate(0);

  // Clip Baseline
  for (int p = 0; p < num_pts; ++p) {
    ptaGetPt(baseline_pts, p, &x0, &y0);
    if (x0 < x_min) {
      if (p + 1 < num_pts) {
        ptaGetPt(baseline_pts, p + 1, &x1, &y1);
        if (x1 < x_min) {
          continue;
        } else {
          GetSlopeAndOffset(x0, y0, x1, y1, &m, &b);
          y0 = int(x_min * m + b);
          x0 = x_min;
        }
      }
    } else if (x0 > x_max) {
      if (ptaGetCount(baseline_clipped_pts) > 0 && p > 0) {
        ptaGetPt(baseline_pts, p - 1, &x1, &y1);
        // See comment above
        GetSlopeAndOffset(x1, y1, x0, y0, &m, &b);
        y0 = int(x_max * m + b);
        x0 = x_max;
        ptaAddPt(baseline_clipped_pts, x0, y0);
        break;
      }
    }
    ptaAddPt(baseline_clipped_pts, x0, y0);
  }
  if (writing_direction == WRITING_DIRECTION_TOP_TO_BOTTOM) {
    SimplifyLinePolygon(baseline_clipped_pts, 3, 0);
  } else {
    SimplifyLinePolygon(baseline_clipped_pts, 3, 1);
  }
  SimplifyLinePolygon(
      baseline_clipped_pts, 3,
      writing_direction == WRITING_DIRECTION_TOP_TO_BOTTOM ? 0 : 1);

  // Check the number of points in baseline_clipped_pts after processing
  int clipped_pts_count = ptaGetCount(baseline_clipped_pts);

  if (clipped_pts_count < 2) {
    // If there's only one point in baseline_clipped_pts, duplicate it
    ptaDestroy(&baseline_clipped_pts); // Clean up the created but unused Pta
    baseline_clipped_pts = ptaCreate(0);
    ptaAddPt(baseline_clipped_pts, x_min, y_min);
    ptaAddPt(baseline_clipped_pts, x_max, y_min);
  }

  return baseline_clipped_pts;
}

///
/// Fit the baseline points into the existings polygon
///
Pta *FitBaselineIntoLinePolygon(Pta *bottom_pts, Pta *baseline_pts,
                                tesseract::WritingDirection writing_direction) {
  int num_pts, num_bin, x0, y0, x1, y1;
  float m, b;
  float x_min, y_min, x_max, y_max;
  float delta_median, delta_median_Q1, delta_median_Q3;
  NUMA *bin_line, *poly_bl_delta;
  Pta *baseline_recalc_pts, *baseline_clipped_pts;

  ptaGetMinMax(bottom_pts, &x_min, &y_min, &x_max, &y_max);
  num_bin = x_max - x_min;
  bin_line = numaCreate(num_bin + 1);

  for (int p = 0; p < num_bin + 1; ++p) {
    bin_line->array[p] = -1.;
  }

  num_pts = ptaGetCount(bottom_pts);
  // Create an interpolated polygon with stepsize 1.
  for (int index = 0; index < num_pts - 1; ++index) {
    ptaGetIPt(bottom_pts, index, &x0, &y0);
    ptaGetIPt(bottom_pts, index + 1, &x1, &y1);
    if (x0 >= x1) {
      continue;
    }
    if (y0 == y1) {
      for (int p = x0 - x_min; p < x1 - x_min + 1; ++p) {
        if (bin_line->array[p] == -1. || y0 > bin_line->array[p]) {
          bin_line->array[p] = y0;
        }
      }
    } else {
      GetSlopeAndOffset(x0, y0, x1, y1, &m, &b);
      for (int p = x0 - x_min; p < x1 - x_min + 1; ++p) {
        if (bin_line->array[p] == -1. ||
            ((p + x_min) * m + b) > bin_line->array[p]) {
          bin_line->array[p] = ((p + x_min) * m + b);
        }
      }
    }
  }

  num_pts = ptaGetCount(baseline_pts);
  baseline_clipped_pts = ptaCreate(0);
  poly_bl_delta = numaCreate(0);

  // Clip Baseline and create a set of deltas between baseline and polygon
  for (int p = 0; p < num_pts; ++p) {
    ptaGetIPt(baseline_pts, p, &x0, &y0);

    if (x0 < x_min) {
      ptaGetIPt(baseline_pts, p + 1, &x1, &y1);
      if (x1 < x_min) {
        continue;
      } else {
        GetSlopeAndOffset(x0, y0, x1, y1, &m, &b);
        y0 = int(x_min * m + b);
        x0 = x_min;
      }
    } else if (x0 > x_max) {
      if (ptaGetCount(baseline_clipped_pts) > 0) {
        ptaGetIPt(baseline_pts, p - 1, &x1, &y1);
        GetSlopeAndOffset(x1, y1, x0, y0, &m, &b);
        y0 = int(x_max * m + b);
        x0 = x_max;
        int x_val = x0 - x_min;
        numaAddNumber(poly_bl_delta, abs(bin_line->array[x_val] - y0));
        ptaAddPt(baseline_clipped_pts, x0, y0);
        break;
      }
    }
    int x_val = x0 - x_min;
    numaAddNumber(poly_bl_delta, abs(bin_line->array[x_val] - y0));
    ptaAddPt(baseline_clipped_pts, x0, y0);
  }

  ptaDestroy(&baseline_pts);

  // Calculate quartiles to find outliers
  numaGetMedian(poly_bl_delta, &delta_median);
  numaGetRankValue(poly_bl_delta, 0.25, nullptr, 0, &delta_median_Q1);
  numaGetRankValue(poly_bl_delta, 0.75, nullptr, 0, &delta_median_Q3);

  // Fit baseline into the polygon
  // Todo: Needs maybe some adjustments to suppress fitting to superscript
  // glyphs
  baseline_recalc_pts = ptaCreate(0);
  num_pts = ptaGetCount(baseline_clipped_pts);
  for (int p = 0; p < num_pts; ++p) {
    ptaGetIPt(baseline_clipped_pts, p, &x0, &y0);
    int x_val = x0 - x_min;
    // Delete outliers with IQR
    if (abs(y0 - bin_line->array[x_val]) >
            1.5 * delta_median_Q3 + delta_median &&
        p != 0 && p != num_pts - 1) {
      continue;
    }
    if (writing_direction == WRITING_DIRECTION_TOP_TO_BOTTOM) {
      if (y0 < bin_line->array[x_val]) {
        ptaAddPt(baseline_recalc_pts, x0, bin_line->array[x_val]);
      } else {
        ptaAddPt(baseline_recalc_pts, x0, y0);
      }
    } else {
      if (y0 > bin_line->array[x_val]) {
        ptaAddPt(baseline_recalc_pts, x0, bin_line->array[x_val]);
      } else {
        ptaAddPt(baseline_recalc_pts, x0, y0);
      }
    }
  }
  // Return recalculated baseline if this fails return the bottom line as
  // baseline
  ptaDestroy(&baseline_clipped_pts);
  if (ptaGetCount(baseline_recalc_pts) < 2) {
    ptaDestroy(&baseline_recalc_pts);
    return ptaCopy(bottom_pts);
  } else {
    return baseline_recalc_pts;
  }
}

/// Convert writing direction to string representation
const char *WritingDirectionToStr(int wd) {
  switch (wd) {
    case 0:
      return "left-to-right";
    case 1:
      return "right-to-left";
    case 2:
      return "top-to-bottom";
    default:
      return "bottom-to-top";
  }
}
///
/// Append the PAGE XML for the beginning of the document
///
bool TessPAGERenderer::BeginDocumentHandler() {
  // Delay the XML output because we need the name of the image file.
  begin_document = true;
  return true;
}

///
/// Append the PAGE XML for the layout of the image
///
bool TessPAGERenderer::AddImageHandler(TessBaseAPI *api) {
  if (begin_document) {
    AppendString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<PcGts "
        "xmlns=\"http://schema.primaresearch.org/PAGE/gts/pagecontent/"
        "2019-07-15\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"http://schema.primaresearch.org/PAGE/gts/"
        "pagecontent/2019-07-15 "
        "http://schema.primaresearch.org/PAGE/gts/pagecontent/2019-07-15/"
        "pagecontent.xsd\">\n"
        "\t<Metadata");

    // If a URL is used to recognize an image add it as <Metadata
    // externalRef="url">
    if (std::regex_search(api->GetInputName(),
                          std::regex("^(https?|ftp|ssh):"))) {
      AppendString(" externalRef=\"");
      AppendString(api->GetInputName());
      AppendString("\" ");
    }

    AppendString(
        ">\n"
        "\t\t<Creator>Tesseract - ");
    AppendString(TESSERACT_VERSION_STR);
    // If gmtime conversion is problematic maybe l_getFormattedDate can be used
    // here
    // char *datestr = l_getFormattedDate();
    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::gmtime(&now);
    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%dT%H:%M:%S", now_tm);
    AppendString(
        "</Creator>\n"
        "\t\t<Created>");
    AppendString(mbstr);
    AppendString("</Created>\n");
    AppendString("\t\t<LastChange>");
    AppendString(mbstr);
    AppendString(
        "</LastChange>\n"
        "\t</Metadata>\n");
    begin_document = false;
  }

  const std::unique_ptr<const char[]> text(api->GetPAGEText(imagenum()));
  if (text == nullptr) {
    return false;
  }

  AppendString(text.get());

  return true;
}

///
/// Append the PAGE XML for the end of the document
///
bool TessPAGERenderer::EndDocumentHandler() {
  AppendString("\t\t</Page>\n</PcGts>\n");
  return true;
}

TessPAGERenderer::TessPAGERenderer(const char *outputbase)
    : TessResultRenderer(outputbase, "page.xml"), begin_document(false) {}

///
/// Make an XML-formatted string with PAGE markup from the internal
/// data structures.
///
char *TessBaseAPI::GetPAGEText(int page_number) {
  return GetPAGEText(nullptr, page_number);
}

///
/// Make an XML-formatted string with PAGE markup from the internal
/// data structures.
///
char *TessBaseAPI::GetPAGEText(ETEXT_DESC *monitor, int page_number) {
  if (tesseract_ == nullptr ||
      (page_res_ == nullptr && Recognize(monitor) < 0)) {
    return nullptr;
  }

  int rcnt = 0, lcnt = 0, wcnt = 0;

  if (input_file_.empty()) {
    SetInputName(nullptr);
  }

  // Used variables

  std::stringstream reading_order_str;
  std::stringstream region_content;
  std::stringstream line_content;
  std::stringstream word_content;
  std::stringstream line_str;
  std::stringstream line_inter_str;
  std::stringstream word_str;
  std::stringstream page_str;

  float x1, y1, x2, y2;

  tesseract::Orientation orientation_block = ORIENTATION_PAGE_UP;
  tesseract::WritingDirection writing_direction_block =
      WRITING_DIRECTION_LEFT_TO_RIGHT;
  tesseract::TextlineOrder textline_order_block;

  Pta *block_top_pts = ptaCreate(0);
  Pta *block_bottom_pts = ptaCreate(0);
  Pta *line_top_ltr_pts = ptaCreate(0);
  Pta *line_bottom_ltr_pts = ptaCreate(0);
  Pta *line_top_rtl_pts = ptaCreate(0);
  Pta *line_bottom_rtl_pts = ptaCreate(0);
  Pta *word_top_pts = ptaCreate(0);
  Pta *word_bottom_pts = ptaCreate(0);
  Pta *word_baseline_pts = ptaCreate(0);
  Pta *line_baseline_rtl_pts = ptaCreate(0);
  Pta *line_baseline_ltr_pts = ptaCreate(0);
  Pta *line_baseline_pts = ptaCreate(0);

  bool POLYGONFLAG;
  GetBoolVariable("page_xml_polygon", &POLYGONFLAG);
  int LEVELFLAG;
  GetIntVariable("page_xml_level", &LEVELFLAG);

  if (LEVELFLAG != 0 && LEVELFLAG != 1) {
    tprintf(
        "For now, only line level and word level are available, and the level "
        "is reset to line level.\n");
    LEVELFLAG = 0;
  }

  // Use "C" locale (needed for int values larger than 999).
  page_str.imbue(std::locale::classic());
  reading_order_str << "\t<Page " << "imageFilename=\"" << GetInputName();
  // AppendString(api->GetInputName());
  reading_order_str << "\" " << "imageWidth=\"" << rect_width_ << "\" "
                    << "imageHeight=\"" << rect_height_ << "\">\n";
  std::size_t ro_id = std::hash<std::string>{}(GetInputName());
  reading_order_str << "\t\t<ReadingOrder>\n"
                    << "\t\t\t<OrderedGroup id=\"ro" << ro_id
                    << "\" caption=\"Regions reading order\">\n";

  std::unique_ptr<ResultIterator> res_it(GetIterator());

  float block_conf = 0;
  float line_conf = 0;

  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    auto block_type = res_it->BlockType();

    switch (block_type) {
      case PT_FLOWING_IMAGE:
      case PT_HEADING_IMAGE:
      case PT_PULLOUT_IMAGE: {
        // Handle all kinds of images.
        page_str << "\t\t<GraphicRegion id=\"r" << rcnt++ << "\">\n";
        page_str << "\t\t\t";
        AddBoxToPAGE(res_it.get(), RIL_BLOCK, page_str);
        page_str << "\t\t</GraphicRegion>\n";
        res_it->Next(RIL_BLOCK);
        continue;
      }
      case PT_HORZ_LINE:
      case PT_VERT_LINE:
        // Handle horizontal and vertical lines.
        page_str << "\t\t<SeparatorRegion id=\"r" << rcnt++ << "\">\n";
        page_str << "\t\t\t";
        AddBoxToPAGE(res_it.get(), RIL_BLOCK, page_str);
        page_str << "\t\t</SeparatorRegion>\n";
        res_it->Next(RIL_BLOCK);
        continue;
      case PT_NOISE:
        tprintf("TODO: Please report image which triggers the noise case.\n");
        ASSERT_HOST(false);
      default:
        break;
    }

    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      // Add Block to reading order
      reading_order_str << "\t\t\t\t<RegionRefIndexed " << "index=\"" << rcnt
                        << "\" " << "regionRef=\"r" << rcnt << "\"/>\n";

      float deskew_angle;
      res_it->Orientation(&orientation_block, &writing_direction_block,
                          &textline_order_block, &deskew_angle);
      block_conf = ((res_it->Confidence(RIL_BLOCK)) / 100.);
      page_str << "\t\t<TextRegion id=\"r" << rcnt << "\" " << "custom=\""
               << "readingOrder {index:" << rcnt << ";} ";
      if (writing_direction_block != WRITING_DIRECTION_LEFT_TO_RIGHT) {
        page_str << "readingDirection {"
                 << WritingDirectionToStr(writing_direction_block) << ";} ";
      }
      page_str << "orientation {" << orientation_block << ";}\">\n";
      page_str << "\t\t\t";
      if ((!POLYGONFLAG || (orientation_block != ORIENTATION_PAGE_UP &&
                            orientation_block != ORIENTATION_PAGE_DOWN)) &&
          LEVELFLAG == 0) {
        AddBoxToPAGE(res_it.get(), RIL_BLOCK, page_str);
      }
    }

    // Writing direction changes at a per-word granularity
    // tesseract::WritingDirection writing_direction_before;
    auto writing_direction = writing_direction_block;
    if (writing_direction_block != WRITING_DIRECTION_TOP_TO_BOTTOM) {
      switch (res_it->WordDirection()) {
        case DIR_LEFT_TO_RIGHT:
          writing_direction = WRITING_DIRECTION_LEFT_TO_RIGHT;
          break;
        case DIR_RIGHT_TO_LEFT:
          writing_direction = WRITING_DIRECTION_RIGHT_TO_LEFT;
          break;
        default:
          break;
      }
    }

    bool ttb_flag = (writing_direction == WRITING_DIRECTION_TOP_TO_BOTTOM);
    // TODO: Rework polygon handling if line is skewed (90 or 180 degress),
    // for now using LinePts
    bool skewed_flag = (orientation_block != ORIENTATION_PAGE_UP &&
                        orientation_block != ORIENTATION_PAGE_DOWN);

    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      // writing_direction_before = writing_direction;
      line_conf = ((res_it->Confidence(RIL_TEXTLINE)) / 100.);
      std::string textline = res_it->GetUTF8Text(RIL_TEXTLINE);
      if (textline.back() == '\n') {
        textline.erase(textline.length() - 1);
      }
      line_content << HOcrEscape(textline.c_str());
      line_str << "\t\t\t<TextLine id=\"r" << rcnt << "l" << lcnt << "\" ";
      if (writing_direction != WRITING_DIRECTION_LEFT_TO_RIGHT &&
          writing_direction != writing_direction_block) {
        line_str << "readingDirection=\""
                 << WritingDirectionToStr(writing_direction) << "\" ";
      }
      line_str << "custom=\"" << "readingOrder {index:" << lcnt << ";}\">\n";
      // If level is linebased, get the line polygon and baseline
      if (LEVELFLAG == 0 && (!POLYGONFLAG || skewed_flag)) {
        AddPointToWordPolygon(res_it.get(), RIL_TEXTLINE, line_top_ltr_pts,
                              line_bottom_ltr_pts, writing_direction);
        AddBaselineToPTA(res_it.get(), RIL_TEXTLINE, line_baseline_pts);
        if (ttb_flag) {
          line_baseline_pts = TransposePolygonline(line_baseline_pts);
        }
      }
    }

    // Get information if word is last in line and if its last in the region
    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_cblock = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);

    float word_conf = ((res_it->Confidence(RIL_WORD)) / 100.);

    // Create word stream if word level output is active
    if (LEVELFLAG > 0) {
      word_str << "\t\t\t\t<Word id=\"r" << rcnt << "l" << lcnt << "w" << wcnt
               << "\" readingDirection=\""
               << WritingDirectionToStr(writing_direction) << "\" "
               << "custom=\"" << "readingOrder {index:" << wcnt << ";}\">\n";
      if ((!POLYGONFLAG || skewed_flag) || ttb_flag) {
        AddPointToWordPolygon(res_it.get(), RIL_WORD, word_top_pts, word_bottom_pts,
                              writing_direction);
      }
    }

    if (POLYGONFLAG && !skewed_flag && ttb_flag && LEVELFLAG == 0) {
      AddPointToWordPolygon(res_it.get(), RIL_WORD, word_top_pts, word_bottom_pts,
                            writing_direction);
    }

    // Get the word baseline information
    AddBaselineToPTA(res_it.get(), RIL_WORD, word_baseline_pts);

    // Get the word text content and polygon
    do {
      const std::unique_ptr<const char[]> grapheme(
          res_it->GetUTF8Text(RIL_SYMBOL));
      if (grapheme && grapheme[0] != 0) {
        word_content << HOcrEscape(grapheme.get()).c_str();
        if (POLYGONFLAG && !skewed_flag && !ttb_flag) {
          AddPointToWordPolygon(res_it.get(), RIL_SYMBOL, word_top_pts,
                                word_bottom_pts, writing_direction);
        }
      }
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));

    if (LEVELFLAG > 0 || (POLYGONFLAG && !skewed_flag)) {
      // Sort wordpolygons
      word_top_pts = RecalcPolygonline(word_top_pts, 1 - ttb_flag);
      word_bottom_pts = RecalcPolygonline(word_bottom_pts, 0 + ttb_flag);

      // AppendLinePolygon
      AppendLinePolygon(line_top_ltr_pts, line_top_rtl_pts, word_top_pts,
                        writing_direction);
      AppendLinePolygon(line_bottom_ltr_pts, line_bottom_rtl_pts,
                        word_bottom_pts, writing_direction);

      // Word level polygon
      word_bottom_pts = ReversePolygonline(word_bottom_pts, 1);
      ptaJoin(word_top_pts, word_bottom_pts, 0, -1);
    }

    // Reverse the word baseline direction for rtl
    if (writing_direction == WRITING_DIRECTION_RIGHT_TO_LEFT) {
      word_baseline_pts = ReversePolygonline(word_baseline_pts, 1);
    }

    // Write word information to the output
    if (LEVELFLAG > 0) {
      word_str << "\t\t\t\t\t";
      if (ttb_flag) {
        word_top_pts = TransposePolygonline(word_top_pts);
      }
      AddPointsToPAGE(word_top_pts, word_str);
      word_str << "\t\t\t\t\t";
      AddBaselinePtsToPAGE(word_baseline_pts, word_str);
      word_str << "\t\t\t\t\t<TextEquiv index=\"1\" conf=\""
               << std::setprecision(4) << word_conf << "\">\n"
               << "\t\t\t\t\t\t<Unicode>" << word_content.str()
               << "</Unicode>\n"
               << "\t\t\t\t\t</TextEquiv>\n"
               << "\t\t\t\t</Word>\n";
    }
    if (LEVELFLAG > 0 || (POLYGONFLAG && !skewed_flag)) {
      // Add wordbaseline to linebaseline
      if (ttb_flag) {
        word_baseline_pts = TransposePolygonline(word_baseline_pts);
      }
      ptaJoin(line_baseline_pts, word_baseline_pts, 0, -1);
    }
    word_baseline_pts = DestroyAndCreatePta(word_baseline_pts);

    // Reset word pts arrays
    word_top_pts = DestroyAndCreatePta(word_top_pts);
    word_bottom_pts = DestroyAndCreatePta(word_bottom_pts);

    // Check why this combination of words is not working as expected!
    // Write the word contents to the line
#if 0
    if (!last_word_in_line && writing_direction_before != writing_direction &&
        writing_direction < 2 && writing_direction_before < 2 &&
        res_it->WordDirection()) {
      if (writing_direction_before == WRITING_DIRECTION_LEFT_TO_RIGHT) {
        // line_content << "‏" << word_content.str();
      } else {
        // line_content << "‎" << word_content.str();
      }
    } else {
      // line_content << word_content.str();
    }
    // Check if WordIsNeutral
    if (res_it->WordDirection()) {
      writing_direction_before = writing_direction;
    }
#endif
    word_content.str("");
    wcnt++;

    // Write line information to the output
    if (last_word_in_line) {
      // Combine ltr and rtl lines
      if (ptaGetCount(line_top_rtl_pts) != 0) {
        ptaJoin(line_top_ltr_pts, line_top_rtl_pts, 0, -1);
        line_top_rtl_pts = DestroyAndCreatePta(line_top_rtl_pts);
      }
      if (ptaGetCount(line_bottom_rtl_pts) != 0) {
        ptaJoin(line_bottom_ltr_pts, line_bottom_rtl_pts, 0, -1);
        line_bottom_rtl_pts = DestroyAndCreatePta(line_bottom_rtl_pts);
      }
      if ((POLYGONFLAG && !skewed_flag) || LEVELFLAG > 0) {
        // Recalc Polygonlines
        line_top_ltr_pts = RecalcPolygonline(line_top_ltr_pts, 1 - ttb_flag);
        line_bottom_ltr_pts =
            RecalcPolygonline(line_bottom_ltr_pts, 0 + ttb_flag);

        // Smooth the polygonline
        SimplifyLinePolygon(line_top_ltr_pts, 5, 1 - ttb_flag);
        SimplifyLinePolygon(line_bottom_ltr_pts, 5, 0 + ttb_flag);

        // Fit linepolygon matching the baselinepoints
        line_baseline_pts = SortBaseline(line_baseline_pts, writing_direction);
        // Fitting baseline into polygon is currently deactivated
        // it tends to push the baseline directly under superscritpts
        // but the baseline is always inside the polygon maybe it will be useful
        // for something line_baseline_pts =
        // FitBaselineIntoLinePolygon(line_bottom_ltr_pts, line_baseline_pts,
        // writing_direction); and it only cut it to the length and simplifies
        // the linepolyon
        line_baseline_pts = ClipAndSimplifyBaseline(
            line_bottom_ltr_pts, line_baseline_pts, writing_direction);

        // Update polygon of the block
        UpdateBlockPoints(block_top_pts, block_bottom_pts, line_top_ltr_pts,
                          line_bottom_ltr_pts, lcnt, last_word_in_cblock);
      }
      // Line level polygon
      line_bottom_ltr_pts = ReversePolygonline(line_bottom_ltr_pts, 1);
      ptaJoin(line_top_ltr_pts, line_bottom_ltr_pts, 0, -1);
      line_bottom_ltr_pts = DestroyAndCreatePta(line_bottom_ltr_pts);

      if (LEVELFLAG > 0 && !(POLYGONFLAG && !skewed_flag)) {
        line_top_ltr_pts = PolygonToBoxCoords(line_top_ltr_pts);
      }

      // Write level points
      line_str << "\t\t\t\t";
      if (ttb_flag) {
        line_top_ltr_pts = TransposePolygonline(line_top_ltr_pts);
      }
      AddPointsToPAGE(line_top_ltr_pts, line_str);
      line_top_ltr_pts = DestroyAndCreatePta(line_top_ltr_pts);

      // Write Baseline
      line_str << "\t\t\t\t";
      if (ttb_flag) {
        line_baseline_pts = TransposePolygonline(line_baseline_pts);
      }
      AddBaselinePtsToPAGE(line_baseline_pts, line_str);
      line_baseline_pts = DestroyAndCreatePta(line_baseline_pts);

      // Add word information if word level output is active
      line_str << word_str.str();
      word_str.str("");
      // Write Line TextEquiv
      line_str << "\t\t\t\t<TextEquiv index=\"1\" conf=\""
               << std::setprecision(4) << line_conf << "\">\n"
               << "\t\t\t\t\t<Unicode>" << line_content.str() << "</Unicode>\n"
               << "\t\t\t\t</TextEquiv>\n";
      line_str << "\t\t\t</TextLine>\n";
      region_content << line_content.str();
      line_content.str("");
      if (!last_word_in_cblock) {
        region_content << '\n';
      }
      lcnt++;
      wcnt = 0;
    }

    // Write region information to the output
    if (last_word_in_cblock) {
      if ((POLYGONFLAG && !skewed_flag) || LEVELFLAG > 0) {
        page_str << "<Coords points=\"";
        block_bottom_pts = ReversePolygonline(block_bottom_pts, 1);
        ptaJoin(block_top_pts, block_bottom_pts, 0, -1);
        if (ttb_flag) {
          block_top_pts = TransposePolygonline(block_top_pts);
        }
        ptaGetMinMax(block_top_pts, &x1, &y1, &x2, &y2);
        page_str << (l_uint32)x1 << "," << (l_uint32)y1;
        page_str << " " << (l_uint32)x2 << "," << (l_uint32)y1;
        page_str << " " << (l_uint32)x2 << "," << (l_uint32)y2;
        page_str << " " << (l_uint32)x1 << "," << (l_uint32)y2;
        page_str << "\"/>\n";
        block_top_pts = DestroyAndCreatePta(block_top_pts);
        block_bottom_pts = DestroyAndCreatePta(block_bottom_pts);
      }
      page_str << line_str.str();
      line_str.str("");
      page_str << "\t\t\t<TextEquiv index=\"1\" conf=\"" << std::setprecision(4)
               << block_conf << "\">\n"
               << "\t\t\t\t<Unicode>" << region_content.str() << "</Unicode>\n"
               << "\t\t\t</TextEquiv>\n";
      page_str << "\t\t</TextRegion>\n";
      region_content.str("");
      rcnt++;
      lcnt = 0;
    }
  }

  // Destroy all point information
  ptaDestroy(&block_top_pts);
  ptaDestroy(&block_bottom_pts);
  ptaDestroy(&line_top_ltr_pts);
  ptaDestroy(&line_bottom_ltr_pts);
  ptaDestroy(&line_top_rtl_pts);
  ptaDestroy(&line_bottom_rtl_pts);
  ptaDestroy(&word_top_pts);
  ptaDestroy(&word_bottom_pts);
  ptaDestroy(&word_baseline_pts);
  ptaDestroy(&line_baseline_rtl_pts);
  ptaDestroy(&line_baseline_ltr_pts);
  ptaDestroy(&line_baseline_pts);

  reading_order_str << "\t\t\t</OrderedGroup>\n"
                    << "\t\t</ReadingOrder>\n";

  reading_order_str << page_str.str();
  page_str.str("");
  const std::string &text = reading_order_str.str();
  reading_order_str.str("");

  return copy_string(text);
}

} // namespace tesseract
