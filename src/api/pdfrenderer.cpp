///////////////////////////////////////////////////////////////////////
// File:        pdfrenderer.cpp
// Description: PDF rendering interface to inject into TessBaseAPI
//
// (C) Copyright 2011, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "pdf_ttf.h"
#include "tprintf.h"
#include "helpers.h" // for Swap, copy_string

#include <allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/publictypes.h> // for PTIsTextType()
#include <tesseract/renderer.h>
#include <cmath>
#include <cstring>
#include <fstream>   // for std::ifstream
#include <locale>    // for std::locale::classic
#include <memory>    // std::unique_ptr
#include <sstream>   // for std::stringstream
#include <string_view>

using namespace std::literals;

#ifndef NDEBUG
#define DEBUG_PDF
#endif
#ifdef DEBUG_PDF
#define NO_PDF_COMPRESSION
#endif

namespace tesseract {

// If the font is 10 pts, nominal character width is 5 pts
static const int kCharWidth = 2;

// Used for memory allocation. A codepoint must take no more than this
// many bytes, when written in the PDF way. e.g. "<0063>" for the
// letter 'c'
static const int kMaxBytesPerCodepoint = 20;

/**********************************************************************
 * PDF Renderer interface implementation
 **********************************************************************/
TessPDFRenderer::TessPDFRenderer(const char *outputbase, const char *datadir, bool textonly)
    : TessResultRenderer(outputbase, "pdf"), datadir_(datadir) {
  obj_ = 0;
  textonly_ = textonly;
  offsets_.push_back(0);
}

void TessPDFRenderer::AppendPDFObjectDIY(size_t objectsize) {
  offsets_.push_back(objectsize + offsets_.back());
  obj_++;
}

void TessPDFRenderer::AppendPDFObject(const char *data) {
  AppendPDFObjectDIY(strlen(data));
  AppendString(data);
}

// Helper function to prevent us from accidentally writing
// scientific notation to an HOCR or PDF file. Besides, three
// decimal points are all you really need.
static double prec(double x) {
  double kPrecision = 1000.0;
  double a = round(x * kPrecision) / kPrecision;
  if (a == -0) {
    return 0;
  }
  return a;
}

static long dist2(int x1, int y1, int x2, int y2) {
  return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

// Viewers like evince can get really confused during copy-paste when
// the baseline wanders around. So I've decided to project every word
// onto the (straight) line baseline. All numbers are in the native
// PDF coordinate system, which has the origin in the bottom left and
// the unit is points, which is 1/72 inch. Tesseract reports baselines
// left-to-right no matter what the reading order is. We need the
// word baseline in reading order, so we do that conversion here. Returns
// the word's baseline origin and length.
static void GetWordBaseline(int writing_direction, int ppi, int height, int word_x1, int word_y1,
                            int word_x2, int word_y2, int line_x1, int line_y1, int line_x2,
                            int line_y2, double *x0, double *y0, double *length) {
  if (writing_direction == WRITING_DIRECTION_RIGHT_TO_LEFT) {
    std::swap(word_x1, word_x2);
    std::swap(word_y1, word_y2);
  }
  double word_length;
  double x, y;
  {
    double l2 = dist2(line_x1, line_y1, line_x2, line_y2);
    if (l2 == 0) {
      x = line_x1;
      y = line_y1;
    } else {
      int px = word_x1;
      int py = word_y1;
      double t = ((px - line_x2) * (line_x2 - line_x1) + (py - line_y2) * (line_y2 - line_y1)) / l2;
      x = line_x2 + t * (line_x2 - line_x1);
      y = line_y2 + t * (line_y2 - line_y1);
    }
    word_length = sqrt(static_cast<double>(dist2(word_x1, word_y1, word_x2, word_y2)));
    word_length = word_length * 72.0 / ppi;
    x = x * 72 / ppi;
    y = height - (y * 72.0 / ppi);
  }
  *x0 = x;
  *y0 = y;
  *length = word_length;
}

// Compute coefficients for an affine matrix describing the rotation
// of the text. If the text is right-to-left such as Arabic or Hebrew,
// we reflect over the Y-axis. This matrix will set the coordinate
// system for placing text in the PDF file.
//
//                           RTL
// [ x' ] = [ a b ][ x ] = [-1 0 ] [ cos sin ][ x ]
// [ y' ]   [ c d ][ y ]   [ 0 1 ] [-sin cos ][ y ]
static void AffineMatrix(int writing_direction, int line_x1, int line_y1, int line_x2, int line_y2,
                         double *a, double *b, double *c, double *d) {
  double theta =
      atan2(static_cast<double>(line_y1 - line_y2), static_cast<double>(line_x2 - line_x1));
  *a = cos(theta);
  *b = sin(theta);
  *c = -sin(theta);
  *d = cos(theta);
  switch (writing_direction) {
    case WRITING_DIRECTION_RIGHT_TO_LEFT:
      *a = -*a;
      *b = -*b;
      break;
    case WRITING_DIRECTION_TOP_TO_BOTTOM:
      // TODO(jbreiden) Consider using the vertical PDF writing mode.
      break;
    default:
      break;
  }
}

// There are some really awkward PDF viewers in the wild, such as
// 'Preview' which ships with the Mac. They do a better job with text
// selection and highlighting when given perfectly flat baseline
// instead of very slightly tilted. We clip small tilts to appease
// these viewers. I chose this threshold large enough to absorb noise,
// but small enough that lines probably won't cross each other if the
// whole page is tilted at almost exactly the clipping threshold.
static void ClipBaseline(int ppi, int x1, int y1, int x2, int y2, int *line_x1, int *line_y1,
                         int *line_x2, int *line_y2) {
  *line_x1 = x1;
  *line_y1 = y1;
  *line_x2 = x2;
  *line_y2 = y2;
  int rise = abs(y2 - y1) * 72;
  int run = abs(x2 - x1) * 72;
  if (rise < 2 * ppi && 2 * ppi < run) {
    *line_y1 = *line_y2 = (y1 + y2) / 2;
  }
}

static bool CodepointToUtf16be(int code, char utf16[kMaxBytesPerCodepoint]) {
  if ((code > 0xD7FF && code < 0xE000) || code > 0x10FFFF) {
    tprintf("Dropping invalid codepoint %d\n", code);
    return false;
  }
  if (code < 0x10000) {
    snprintf(utf16, kMaxBytesPerCodepoint, "%04X", code);
  } else {
    int a = code - 0x010000;
    int high_surrogate = (0x03FF & (a >> 10)) + 0xD800;
    int low_surrogate = (0x03FF & a) + 0xDC00;
    snprintf(utf16, kMaxBytesPerCodepoint, "%04X%04X", high_surrogate, low_surrogate);
  }
  return true;
}

char *TessPDFRenderer::GetPDFTextObjects(TessBaseAPI *api, double width, double height) {
  double ppi = api->GetSourceYResolution();

  // These initial conditions are all arbitrary and will be overwritten
  double old_x = 0.0, old_y = 0.0;
  int old_fontsize = 0;
  tesseract::WritingDirection old_writing_direction = WRITING_DIRECTION_LEFT_TO_RIGHT;
  bool new_block = true;
  int fontsize = 0;
  double a = 1;
  double b = 0;
  double c = 0;
  double d = 1;

  std::stringstream pdf_str;
  // Use "C" locale (needed for double values prec()).
  pdf_str.imbue(std::locale::classic());
  // Use 8 digits for double values.
  pdf_str.precision(8);

  // TODO(jbreiden) This marries the text and image together.
  // Slightly cleaner from an abstraction standpoint if this were to
  // live inside a separate text object.
  pdf_str << "q " << prec(width) << " 0 0 " << prec(height) << " 0 0 cm";
  if (!textonly_) {
    pdf_str << " /Im1 Do";
  }
  pdf_str << " Q\n";

  int line_x1 = 0;
  int line_y1 = 0;
  int line_x2 = 0;
  int line_y2 = 0;

  const std::unique_ptr</*non-const*/ ResultIterator> res_it(api->GetIterator());
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      auto block_type = res_it->BlockType();
      if (!PTIsTextType(block_type)) {
        // ignore non-text blocks
        res_it->Next(RIL_BLOCK);
        continue;
      }
      pdf_str << "BT\n3 Tr"; // Begin text object, use invisible ink
      old_fontsize = 0;      // Every block will declare its fontsize
      new_block = true;      // Every block will declare its affine matrix
    }

    if (res_it->IsAtBeginningOf(RIL_TEXTLINE)) {
      int x1, y1, x2, y2;
      res_it->Baseline(RIL_TEXTLINE, &x1, &y1, &x2, &y2);
      ClipBaseline(ppi, x1, y1, x2, y2, &line_x1, &line_y1, &line_x2, &line_y2);
    }

    if (res_it->Empty(RIL_WORD)) {
      res_it->Next(RIL_WORD);
      continue;
    }

    // Writing direction changes at a per-word granularity
    tesseract::WritingDirection writing_direction;
    {
      tesseract::Orientation orientation;
      tesseract::TextlineOrder textline_order;
      float deskew_angle;
      res_it->Orientation(&orientation, &writing_direction, &textline_order, &deskew_angle);
      if (writing_direction != WRITING_DIRECTION_TOP_TO_BOTTOM) {
        switch (res_it->WordDirection()) {
          case DIR_LEFT_TO_RIGHT:
            writing_direction = WRITING_DIRECTION_LEFT_TO_RIGHT;
            break;
          case DIR_RIGHT_TO_LEFT:
            writing_direction = WRITING_DIRECTION_RIGHT_TO_LEFT;
            break;
          default:
            writing_direction = old_writing_direction;
        }
      }
    }

    // Where is word origin and how long is it?
    double x, y, word_length;
    {
      int word_x1, word_y1, word_x2, word_y2;
      res_it->Baseline(RIL_WORD, &word_x1, &word_y1, &word_x2, &word_y2);
      GetWordBaseline(writing_direction, ppi, height, word_x1, word_y1, word_x2, word_y2, line_x1,
                      line_y1, line_x2, line_y2, &x, &y, &word_length);
    }

    if (writing_direction != old_writing_direction || new_block) {
      AffineMatrix(writing_direction, line_x1, line_y1, line_x2, line_y2, &a, &b, &c, &d);
      pdf_str << " " << prec(a) // . This affine matrix
              << " " << prec(b) // . sets the coordinate
              << " " << prec(c) // . system for all
              << " " << prec(d) // . text that follows.
              << " " << prec(x) // .
              << " " << prec(y) // .
              << (" Tm ");      // Place cursor absolutely
      new_block = false;
    } else {
      double dx = x - old_x;
      double dy = y - old_y;
      pdf_str << " " << prec(dx * a + dy * b) << " " << prec(dx * c + dy * d)
              << (" Td "); // Relative moveto
    }
    old_x = x;
    old_y = y;
    old_writing_direction = writing_direction;

    // Adjust font size on a per word granularity. Pay attention to
    // fontsize, old_fontsize, and pdf_str. We've found that for
    // in Arabic, Tesseract will happily return a fontsize of zero,
    // so we make up a default number to protect ourselves.
    {
      bool bold, italic, underlined, monospace, serif, smallcaps;
      int font_id;
      res_it->WordFontAttributes(&bold, &italic, &underlined, &monospace, &serif, &smallcaps,
                                 &fontsize, &font_id);
      const int kDefaultFontsize = 8;
      if (fontsize <= 0) {
        fontsize = kDefaultFontsize;
      }
      if (fontsize != old_fontsize) {
        pdf_str << "/f-0-0 " << fontsize << " Tf ";
        old_fontsize = fontsize;
#ifdef DEBUG_PDF
        pdf_str << "\n";
#endif
      }
    }

    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);
    std::string pdf_word;
    int pdf_word_len = 0;
    do {
      const std::unique_ptr<const char[]> grapheme(res_it->GetUTF8Text(RIL_SYMBOL));
      if (grapheme && grapheme[0] != '\0') {
        std::vector<char32> unicodes = UNICHAR::UTF8ToUTF32(grapheme.get());
        char utf16[kMaxBytesPerCodepoint];
        for (char32 code : unicodes) {
          if (CodepointToUtf16be(code, utf16)) {
            pdf_word += utf16;
            pdf_word_len++;
          }
        }
      }
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    if (res_it->IsAtBeginningOf(RIL_WORD)) {
      pdf_word += "0020";
    }
    if (word_length > 0 && pdf_word_len > 0) {
      double h_stretch = kCharWidth * prec(100.0 * word_length / (fontsize * pdf_word_len));
      pdf_str << h_stretch << " Tz"; // horizontal stretch
      pdf_str
          << " [ <" << pdf_word // UTF-16BE representation
          << "> ] TJ";          // show the text
#ifdef DEBUG_PDF
      pdf_str << "\n";
#endif
    }
    if (last_word_in_line) {
      pdf_str << " \n";
    }
    if (last_word_in_block) {
      pdf_str << "ET\n"; // end the text object
    }
  }
  return copy_string(pdf_str.str());
}

bool TessPDFRenderer::BeginDocumentHandler() {
  AppendPDFObject("%PDF-1.5\n%\xDE\xAD\xBE\xEB\n");

  // CATALOG
  AppendPDFObject(
      "1 0 obj\n"
      "<<\n"
      "  /Type /Catalog\n"
      "  /Pages 2 0 R\n"
      ">>\nendobj\n");

  // We are reserving object #2 for the /Pages
  // object, which I am going to create and write
  // at the end of the PDF file.
  AppendPDFObject("");

  // TYPE0 FONT
  AppendPDFObject(
      "3 0 obj\n"
      "<<\n"
      "  /BaseFont /GlyphLessFont\n"
      "  /DescendantFonts [ 4 0 R ]\n" // CIDFontType2 font
      "  /Encoding /Identity-H\n"
      "  /Subtype /Type0\n"
      "  /ToUnicode 6 0 R\n" // ToUnicode
      "  /Type /Font\n"
      ">>\n"
      "endobj\n");

  // CIDFONTTYPE2
  std::stringstream stream;
  // Use "C" locale (needed for int values larger than 999).
  stream.imbue(std::locale::classic());
  stream << "4 0 obj\n"
            "<<\n"
            "  /BaseFont /GlyphLessFont\n"
            "  /CIDToGIDMap 5 0 R\n" // CIDToGIDMap
            "  /CIDSystemInfo\n"
            "  <<\n"
            "     /Ordering (Identity)\n"
            "     /Registry (Adobe)\n"
            "     /Supplement 0\n"
            "  >>\n"
            "  /FontDescriptor 7 0 R\n" // Font descriptor
            "  /Subtype /CIDFontType2\n"
            "  /Type /Font\n"
            "  /DW "
         << (1000 / kCharWidth)
         << "\n"
            ">>\n"
            "endobj\n";
  AppendPDFObject(stream.str().c_str());

  // CIDTOGIDMAP
  const int kCIDToGIDMapSize = 2 * (1 << 16);
  const std::unique_ptr<unsigned char[]> cidtogidmap(new unsigned char[kCIDToGIDMapSize]);
  for (int i = 0; i < kCIDToGIDMapSize; i++) {
    cidtogidmap[i] = (i % 2) ? 1 : 0;
  }
  size_t len = kCIDToGIDMapSize;
#ifndef NO_PDF_COMPRESSION
  auto comp = zlibCompress(cidtogidmap.get(), kCIDToGIDMapSize, &len);
#endif
  stream.str("");
  stream << "5 0 obj\n"
            "<<\n"
            "  /Length "
         << len
         << ""
#ifndef NO_PDF_COMPRESSION
            " /Filter /FlateDecode"
#endif
            "\n"
            ">>\n"
            "stream\n"
            ;
  AppendString(stream.str().c_str());
  long objsize = stream.str().size();
#ifndef NO_PDF_COMPRESSION
  AppendData(reinterpret_cast<char *>(comp), len);
#else
  AppendData(reinterpret_cast<char *>(cidtogidmap.get()), len);
#endif
  objsize += len;
#ifndef NO_PDF_COMPRESSION
  lept_free(comp);
#endif
  objsize += AppendData("endstream\n"sv);
  objsize += AppendData("endobj\n"sv);
  AppendPDFObjectDIY(objsize);

  const char stream2[] =
      "/CIDInit /ProcSet findresource begin\n"
      "12 dict begin\n"
      "begincmap\n"
      "/CIDSystemInfo\n"
      "<<\n"
      "  /Registry (Adobe)\n"
      "  /Ordering (UCS)\n"
      "  /Supplement 0\n"
      ">> def\n"
      "/CMapName /Adobe-Identify-UCS def\n"
      "/CMapType 2 def\n"
      "1 begincodespacerange\n"
      "<0000> <FFFF>\n"
      "endcodespacerange\n"
      "1 beginbfrange\n"
      "<0000> <FFFF> <0000>\n"
      "endbfrange\n"
      "endcmap\n"
      "CMapName currentdict /CMap defineresource pop\n"
      "end\n"
      "end\n";

  // TOUNICODE
  stream.str("");
  stream << "6 0 obj\n"
            "<< /Length "
         << (sizeof(stream2) - 1)
         << " >>\n"
            "stream\n"
         << stream2
         << "endstream\n"
            "endobj\n";
  AppendPDFObject(stream.str().c_str());

  // FONT DESCRIPTOR
  stream.str("");
  stream << "7 0 obj\n"
            "<<\n"
            "  /Ascent 1000\n"
            "  /CapHeight 1000\n"
            "  /Descent -1\n" // Spec says must be negative
            "  /Flags 5\n"    // FixedPitch + Symbolic
            "  /FontBBox  [ 0 0 "
         << (1000 / kCharWidth)
         << " 1000 ]\n"
            "  /FontFile2 8 0 R\n"
            "  /FontName /GlyphLessFont\n"
            "  /ItalicAngle 0\n"
            "  /StemV 80\n"
            "  /Type /FontDescriptor\n"
            ">>\n"
            "endobj\n";
  AppendPDFObject(stream.str().c_str());

  stream.str("");
  stream << datadir_.c_str() << "/pdf.ttf";
  const uint8_t *font;
  std::ifstream input(stream.str().c_str(), std::ios::in | std::ios::binary);
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
  auto size = buffer.size();
  if (size) {
    font = buffer.data();
  } else {
#if !defined(NDEBUG)
    tprintf("Cannot open file \"%s\"!\nUsing internal glyphless font.\n", stream.str().c_str());
#endif
    font = pdf_ttf;
    size = sizeof(pdf_ttf);
  }

  // FONTFILE2
  stream.str("");
  stream << "8 0 obj\n"
            "<<\n"
            "  /Length "
         << size
         << "\n"
            "  /Length1 "
         << size
         << "\n"
            ">>\n"
            "stream\n";
  AppendString(stream.str().c_str());
  objsize = stream.str().size();
  AppendData(reinterpret_cast<const char *>(font), size);
  objsize += size;
  objsize += AppendData("endstream\n"sv);
  objsize += AppendData("endobj\n"sv);
  AppendPDFObjectDIY(objsize);
  return true;
}

bool TessPDFRenderer::imageToPDFObj(Pix *pix, const char *filename, long int objnum,
                                    char **pdf_object, long int *pdf_object_size,
                                    const int jpg_quality) {
  if (!pdf_object_size || !pdf_object) {
    return false;
  }
  *pdf_object = nullptr;
  *pdf_object_size = 0;
  if (!filename && !pix) {
    return false;
  }

  L_Compressed_Data *cid = nullptr;
  auto sad = l_generateCIDataForPdf(filename, pix, jpg_quality, &cid);

  if (sad || !cid) {
    l_CIDataDestroy(&cid);
    return false;
  }

  const char *group4 = "";
  const char *filter;
  switch (cid->type) {
    case L_FLATE_ENCODE:
      filter = "/FlateDecode";
      break;
    case L_JPEG_ENCODE:
      filter = "/DCTDecode";
      break;
    case L_G4_ENCODE:
      filter = "/CCITTFaxDecode";
      group4 = "    /K -1\n";
      break;
    case L_JP2K_ENCODE:
      filter = "/JPXDecode";
      break;
    default:
      l_CIDataDestroy(&cid);
      return false;
  }

  // Maybe someday we will accept RGBA but today is not that day.
  // It requires creating an /SMask for the alpha channel.
  // http://stackoverflow.com/questions/14220221
  std::stringstream colorspace;
  // Use "C" locale (needed for int values larger than 999).
  colorspace.imbue(std::locale::classic());
  if (cid->ncolors > 0) {
    colorspace << "  /ColorSpace [ /Indexed /DeviceRGB " << (cid->ncolors - 1) << " "
               << cid->cmapdatahex << " ]\n";
  } else {
    switch (cid->spp) {
      case 1:
        if (cid->bps == 1 && pixGetInputFormat(pix) == IFF_PNG) {
          colorspace.str(
              "  /ColorSpace /DeviceGray\n"
              "  /Decode [1 0]\n");
        } else {
          colorspace.str("  /ColorSpace /DeviceGray\n");
        }
        break;
      case 3:
        colorspace.str("  /ColorSpace /DeviceRGB\n");
        break;
      default:
        l_CIDataDestroy(&cid);
        return false;
    }
  }

  int predictor = (cid->predictor) ? 14 : 1;

  // IMAGE
  std::stringstream b1;
  // Use "C" locale (needed for int values larger than 999).
  b1.imbue(std::locale::classic());
  b1 << objnum
     << " 0 obj\n"
        "<<\n"
        "  /Length "
     << cid->nbytescomp
     << "\n"
        "  /Subtype /Image\n";

  std::stringstream b2;
  // Use "C" locale (needed for int values larger than 999).
  b2.imbue(std::locale::classic());
  b2 << "  /Width " << cid->w
     << "\n"
        "  /Height "
     << cid->h
     << "\n"
        "  /BitsPerComponent "
     << cid->bps
     << "\n"
        "  /Filter "
     << filter
     << "\n"
        "  /DecodeParms\n"
        "  <<\n"
        "    /Predictor "
     << predictor
     << "\n"
        "    /Colors "
     << cid->spp << "\n"
     << group4 << "    /Columns " << cid->w
     << "\n"
        "    /BitsPerComponent "
     << cid->bps
     << "\n"
        "  >>\n"
        ">>\n"
        "stream\n";

  const char *b3 =
      "endstream\n"
      "endobj\n";

  size_t b1_len = b1.str().size();
  size_t b2_len = b2.str().size();
  size_t b3_len = strlen(b3);
  size_t colorspace_len = colorspace.str().size();

  *pdf_object_size = b1_len + colorspace_len + b2_len + cid->nbytescomp + b3_len;
  *pdf_object = new char[*pdf_object_size];

  char *p = *pdf_object;
  memcpy(p, b1.str().c_str(), b1_len);
  p += b1_len;
  memcpy(p, colorspace.str().c_str(), colorspace_len);
  p += colorspace_len;
  memcpy(p, b2.str().c_str(), b2_len);
  p += b2_len;
  memcpy(p, cid->datacomp, cid->nbytescomp);
  p += cid->nbytescomp;
  memcpy(p, b3, b3_len);
  l_CIDataDestroy(&cid);
  return true;
}

bool TessPDFRenderer::AddImageHandler(TessBaseAPI *api) {
  Pix *pix = api->GetInputImage();
  const char *filename = api->GetInputName();
  int ppi = api->GetSourceYResolution();
  if (!pix || ppi <= 0) {
    return false;
  }
  double width = pixGetWidth(pix) * 72.0 / ppi;
  double height = pixGetHeight(pix) * 72.0 / ppi;

  std::stringstream xobject;
  // Use "C" locale (needed for int values larger than 999).
  xobject.imbue(std::locale::classic());
  if (!textonly_) {
    xobject << "/XObject << /Im1 " << (obj_ + 2) << " 0 R >>\n";
  }

  // PAGE
  std::stringstream stream;
  // Use "C" locale (needed for double values width and height).
  stream.imbue(std::locale::classic());
  stream.precision(2);
  stream << std::fixed << obj_
         << " 0 obj\n"
            "<<\n"
            "  /Type /Page\n"
            "  /Parent 2 0 R\n" // Pages object
            "  /MediaBox [0 0 "
         << width << " " << height
         << "]\n"
            "  /Contents "
         << (obj_ + 1)
         << " 0 R\n" // Contents object
            "  /Resources\n"
            "  <<\n"
            "    "
         << xobject.str() << // Image object
      "    /ProcSet [ /PDF /Text /ImageB /ImageI /ImageC ]\n"
      "    /Font << /f-0-0 3 0 R >>\n" // Type0 Font
      "  >>\n"
      ">>\n"
      "endobj\n";
  pages_.push_back(obj_);
  AppendPDFObject(stream.str().c_str());

  // CONTENTS
  const std::unique_ptr<char[]> pdftext(GetPDFTextObjects(api, width, height));
  const size_t pdftext_len = strlen(pdftext.get());
  size_t len = pdftext_len;
#ifndef NO_PDF_COMPRESSION
  auto comp_pdftext = zlibCompress(reinterpret_cast<unsigned char *>(pdftext.get()), pdftext_len, &len);
#endif
  stream.str("");
  stream << obj_
         << " 0 obj\n"
            "<<\n"
            "  /Length "
         << len
         << ""
#ifndef NO_PDF_COMPRESSION
            " /Filter /FlateDecode"
#endif
            "\n"
            ">>\n"
            "stream\n"
            ;
  AppendString(stream.str().c_str());
  long objsize = stream.str().size();
#ifndef NO_PDF_COMPRESSION
  AppendData(reinterpret_cast<char *>(comp_pdftext), len);
#else
  AppendData(reinterpret_cast<char *>(pdftext.get()), len);
#endif
  objsize += len;
#ifndef NO_PDF_COMPRESSION
  lept_free(comp_pdftext);
#endif
  objsize += AppendData("endstream\n"sv);
  objsize += AppendData("endobj\n"sv);
  AppendPDFObjectDIY(objsize);

  if (!textonly_) {
    char *pdf_object = nullptr;
    int jpg_quality;
    api->GetIntVariable("jpg_quality", &jpg_quality);
    if (!imageToPDFObj(pix, filename, obj_, &pdf_object, &objsize, jpg_quality)) {
      return false;
    }
    AppendData(pdf_object, objsize);
    AppendPDFObjectDIY(objsize);
    delete[] pdf_object;
  }
  return true;
}

bool TessPDFRenderer::EndDocumentHandler() {
  // We reserved the /Pages object number early, so that the /Page
  // objects could refer to their parent. We finally have enough
  // information to go fill it in. Using lower level calls to manipulate
  // the offset record in two spots, because we are placing objects
  // out of order in the file.

  // PAGES
  const long int kPagesObjectNumber = 2;
  offsets_[kPagesObjectNumber] = offsets_.back(); // manipulation #1
  std::stringstream stream;
  // Use "C" locale (needed for int values larger than 999).
  stream.imbue(std::locale::classic());
  stream << kPagesObjectNumber << " 0 obj\n<<\n  /Type /Pages\n  /Kids [ ";
  AppendString(stream.str().c_str());
  size_t pages_objsize = stream.str().size();
  for (const auto &page : pages_) {
    stream.str("");
    stream << page << " 0 R ";
    AppendString(stream.str().c_str());
    pages_objsize += stream.str().size();
  }
  stream.str("");
  stream << "]\n  /Count " << pages_.size() << "\n>>\nendobj\n";
  AppendString(stream.str().c_str());
  pages_objsize += stream.str().size();
  offsets_.back() += pages_objsize; // manipulation #2

  // INFO
  std::string utf16_title = "FEFF"; // byte_order_marker
  std::vector<char32> unicodes = UNICHAR::UTF8ToUTF32(title());
  char utf16[kMaxBytesPerCodepoint];
  for (char32 code : unicodes) {
    if (CodepointToUtf16be(code, utf16)) {
      utf16_title += utf16;
    }
  }

  char *datestr = l_getFormattedDate();
  stream.str("");
  stream << obj_
         << " 0 obj\n"
            "<<\n"
            "  /Producer (Tesseract "
         << tesseract::TessBaseAPI::Version()
         << ")\n"
            "  /CreationDate (D:"
         << datestr
         << ")\n"
            "  /Title <"
         << utf16_title.c_str()
         << ">\n"
            ">>\n"
            "endobj\n";
  lept_free(datestr);
  AppendPDFObject(stream.str().c_str());
  stream.str("");
  stream << "xref\n0 " << obj_ << "\n0000000000 65535 f \n";
  AppendString(stream.str().c_str());
  for (int i = 1; i < obj_; i++) {
    stream.str("");
    stream.width(10);
    stream.fill('0');
    stream << offsets_[i] << " 00000 n \n";
    AppendString(stream.str().c_str());
  }
  stream.str("");
  stream << "trailer\n<<\n  /Size " << obj_
         << "\n"
            "  /Root 1 0 R\n" // catalog
            "  /Info "
         << (obj_ - 1)
         << " 0 R\n" // info
            ">>\nstartxref\n"
         << offsets_.back() << "\n%%EOF\n";
  AppendString(stream.str().c_str());
  return true;
}
} // namespace tesseract
