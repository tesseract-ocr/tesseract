// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

#include "baseapi.h"
#include "renderer.h"
#include "math.h"
#include "strngs.h"
#include "cube_utils.h"
#include "allheaders.h"

#ifdef _MSC_VER
#include "mathfix.h"
#endif

namespace tesseract {

// Use for PDF object fragments. Must be large enough
// to hold a colormap with 256 colors in the verbose
// PDF representation.
const int kBasicBufSize = 2048;

// If the font is 10 pts, nominal character width is 5 pts
const int kCharWidth = 2;

/**********************************************************************
 * PDF Renderer interface implementation
 **********************************************************************/

TessPDFRenderer::TessPDFRenderer(const char* outputbase, const char *datadir)
    : TessResultRenderer(outputbase, "pdf") {
  obj_  = 0;
  datadir_ = datadir;
  offsets_.push_back(0);
}

void TessPDFRenderer::AppendPDFObjectDIY(size_t objectsize) {
  offsets_.push_back(objectsize + offsets_.back());
  obj_++;
}

void TessPDFRenderer::AppendPDFObject(const char *data) {
  AppendPDFObjectDIY(strlen(data));
  AppendString((const char *)data);
}

// Helper function to prevent us from accidentaly writing
// scientific notation to an HOCR or PDF file. Besides, three
// decimal points are all you really need.
double prec(double x) {
  double kPrecision = 1000.0;
  double a = round(x * kPrecision) / kPrecision;
  if (a == -0)
    return 0;
  return a;
}

long dist2(int x1, int y1, int x2, int y2) {
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
void GetWordBaseline(int writing_direction, int ppi, int height,
                     int word_x1, int word_y1, int word_x2, int word_y2,
                     int line_x1, int line_y1, int line_x2, int line_y2,
                     double *x0, double *y0, double *length) {
  if (writing_direction == WRITING_DIRECTION_RIGHT_TO_LEFT) {
    Swap(&word_x1, &word_x2);
    Swap(&word_y1, &word_y2);
  }
  double word_length;
  double x, y;
  {
    int px = word_x1;
    int py = word_y1;
    double l2 = dist2(line_x1, line_y1, line_x2, line_y2);
    if (l2 == 0) {
      x = line_x1;
      y = line_y1;
    } else {
      double t = ((px - line_x2) * (line_x2 - line_x1) +
                  (py - line_y2) * (line_y2 - line_y1)) / l2;
      x = line_x2 + t * (line_x2 - line_x1);
      y = line_y2 + t * (line_y2 - line_y1);
    }
    word_length = sqrt(static_cast<double>(dist2(word_x1, word_y1,
                                                 word_x2, word_y2)));
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
void AffineMatrix(int writing_direction,
                  int line_x1, int line_y1, int line_x2, int line_y2,
                  double *a, double *b, double *c, double *d) {
  double theta = atan2(static_cast<double>(line_y1 - line_y2),
                       static_cast<double>(line_x2 - line_x1));
  *a = cos(theta);
  *b = sin(theta);
  *c = -sin(theta);
  *d = cos(theta);
  switch(writing_direction) {
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

// There are some really stupid PDF viewers in the wild, such as
// 'Preview' which ships with the Mac. They do a better job with text
// selection and highlighting when given perfectly flat baseline
// instead of very slightly tilted. We clip small tilts to appease
// these viewers. I chose this threshold large enough to absorb noise,
// but small enough that lines probably won't cross each other if the
// whole page is tilted at almost exactly the clipping threshold.
void ClipBaseline(int ppi, int x1, int y1, int x2, int y2,
                  int *line_x1, int *line_y1,
                  int *line_x2, int *line_y2) {
  *line_x1 = x1;
  *line_y1 = y1;
  *line_x2 = x2;
  *line_y2 = y2;
  double rise = abs(y2 - y1) * 72 / ppi;
  double run = abs(x2 - x1) * 72 / ppi;
  if (rise < 2.0 && 2.0 < run)
    *line_y1 = *line_y2 = (y1 + y2) / 2;
}

char* TessPDFRenderer::GetPDFTextObjects(TessBaseAPI* api,
                                         double width, double height) {
  STRING pdf_str("");
  double ppi = api->GetSourceYResolution();

  // These initial conditions are all arbitrary and will be overwritten
  double old_x = 0.0, old_y = 0.0;
  int old_fontsize = 0;
  tesseract::WritingDirection old_writing_direction =
      WRITING_DIRECTION_LEFT_TO_RIGHT;
  bool new_block = true;
  int fontsize = 0;
  double a = 1;
  double b = 0;
  double c = 0;
  double d = 1;

  // TODO(jbreiden) This marries the text and image together.
  // Slightly cleaner from an abstraction standpoint if this were to
  // live inside a separate text object.
  pdf_str += "q ";
  pdf_str.add_str_double("", prec(width));
  pdf_str += " 0 0 ";
  pdf_str.add_str_double("", prec(height));
  pdf_str += " 0 0 cm /Im1 Do Q\n";

  ResultIterator *res_it = api->GetIterator();
  while (!res_it->Empty(RIL_BLOCK)) {
    if (res_it->IsAtBeginningOf(RIL_BLOCK)) {
      pdf_str += "BT\n3 Tr";     // Begin text object, use invisible ink
      old_fontsize = 0;          // Every block will declare its fontsize
      new_block = true;          // Every block will declare its affine matrix
    }

    int line_x1, line_y1, line_x2, line_y2;
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
      res_it->Orientation(&orientation, &writing_direction,
                          &textline_order, &deskew_angle);
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
      GetWordBaseline(writing_direction, ppi, height,
                      word_x1, word_y1, word_x2, word_y2,
                      line_x1, line_y1, line_x2, line_y2,
                      &x, &y, &word_length);
    }

    if (writing_direction != old_writing_direction || new_block) {
      AffineMatrix(writing_direction,
                   line_x1, line_y1, line_x2, line_y2, &a, &b, &c, &d);
      pdf_str.add_str_double(" ", prec(a));  // . This affine matrix
      pdf_str.add_str_double(" ", prec(b));  // . sets the coordinate
      pdf_str.add_str_double(" ", prec(c));  // . system for all
      pdf_str.add_str_double(" ", prec(d));  // . text that follows.
      pdf_str.add_str_double(" ", prec(x));  // .
      pdf_str.add_str_double(" ", prec(y));  // .
      pdf_str += (" Tm ");                   // Place cursor absolutely
      new_block = false;
    } else {
      double dx = x - old_x;
      double dy = y - old_y;
      pdf_str.add_str_double(" ", prec(dx * a + dy * b));
      pdf_str.add_str_double(" ", prec(dx * c + dy * d));
      pdf_str += (" Td ");                   // Relative moveto
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
      res_it->WordFontAttributes(&bold, &italic, &underlined, &monospace,
                                 &serif, &smallcaps, &fontsize, &font_id);
      const int kDefaultFontsize = 8;
      if (fontsize <= 0)
        fontsize = kDefaultFontsize;
      if (fontsize != old_fontsize) {
        char textfont[20];
        snprintf(textfont, sizeof(textfont), "/f-0-0 %d Tf ", fontsize);
        pdf_str += textfont;
        old_fontsize = fontsize;
      }
    }

    bool last_word_in_line = res_it->IsAtFinalElement(RIL_TEXTLINE, RIL_WORD);
    bool last_word_in_block = res_it->IsAtFinalElement(RIL_BLOCK, RIL_WORD);
    STRING pdf_word("");
    int pdf_word_len = 0;
    do {
      const char *grapheme = res_it->GetUTF8Text(RIL_SYMBOL);
      if (grapheme && grapheme[0] != '\0') {
        // TODO(jbreiden) Do a real UTF-16BE conversion
        // http://en.wikipedia.org/wiki/UTF-16#Example_UTF-16_encoding_procedure
        string_32 utf32;
        CubeUtils::UTF8ToUTF32(grapheme, &utf32);
        char utf16[20];
        for (int i = 0; i < static_cast<int>(utf32.length()); i++) {
          snprintf(utf16, sizeof(utf16), "<%04X>", utf32[i]);
          pdf_word += utf16;
          pdf_word_len++;
        }
      }
      delete []grapheme;
      res_it->Next(RIL_SYMBOL);
    } while (!res_it->Empty(RIL_BLOCK) && !res_it->IsAtBeginningOf(RIL_WORD));
    if (word_length > 0 && pdf_word_len > 0 && fontsize > 0) {
      double h_stretch =
          kCharWidth * prec(100.0 * word_length / (fontsize * pdf_word_len));
      pdf_str.add_str_double("", h_stretch);
      pdf_str += " Tz";          // horizontal stretch
      pdf_str += " [ ";
      pdf_str += pdf_word;       // UTF-16BE representation
      pdf_str += " ] TJ";        // show the text
    }
    if (last_word_in_line) {
      pdf_str += " \n";
    }
    if (last_word_in_block) {
      pdf_str += "ET\n";         // end the text object
    }
  }
  char *ret = new char[pdf_str.length() + 1];
  strcpy(ret, pdf_str.string());
  delete res_it;
  return ret;
}

bool TessPDFRenderer::BeginDocumentHandler() {
  char buf[kBasicBufSize];
  size_t n;

  n = snprintf(buf, sizeof(buf),
               "%%PDF-1.5\n"
               "%%%c%c%c%c\n",
               0xDE, 0xAD, 0xBE, 0xEB);
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  // CATALOG
  n = snprintf(buf, sizeof(buf),
               "1 0 obj\n"
               "<<\n"
               "  /Type /Catalog\n"
               "  /Pages %ld 0 R\n"
               ">>\n"
               "endobj\n", 2L);
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  // We are reserving object #2 for the /Pages
  // object, which I am going to create and write
  // at the end of the PDF file.
  AppendPDFObject("");

  // TYPE0 FONT
  n = snprintf(buf, sizeof(buf),
               "3 0 obj\n"
               "<<\n"
               "  /BaseFont /GlyphLessFont\n"
               "  /DescendantFonts [ %ld 0 R ]\n"
               "  /Encoding /Identity-H\n"
               "  /Subtype /Type0\n"
               "  /ToUnicode %ld 0 R\n"
               "  /Type /Font\n"
               ">>\n"
               "endobj\n",
               4L,          // CIDFontType2 font
               5L           // ToUnicode
               );
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  // CIDFONTTYPE2
  n = snprintf(buf, sizeof(buf),
               "4 0 obj\n"
               "<<\n"
               "  /BaseFont /GlyphLessFont\n"
               "  /CIDToGIDMap /Identity\n"
               "  /CIDSystemInfo\n"
               "  <<\n"
               "     /Ordering (Identity)\n"
               "     /Registry (Adobe)\n"
               "     /Supplement 0\n"
               "  >>\n"
               "  /FontDescriptor %ld 0 R\n"
               "  /Subtype /CIDFontType2\n"
               "  /Type /Font\n"
               "  /DW %d\n"
               ">>\n"
               "endobj\n",
               6L,         // Font descriptor
               1000 / kCharWidth);
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  const char *stream =
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
  n = snprintf(buf, sizeof(buf),
               "5 0 obj\n"
               "<< /Length %lu >>\n"
               "stream\n"
               "%s"
               "endstream\n"
               "endobj\n", (unsigned long) strlen(stream), stream);
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  // FONT DESCRIPTOR
  const int kCharHeight = 2;  // Effect: highlights are half height
  n = snprintf(buf, sizeof(buf),
               "6 0 obj\n"
               "<<\n"
               "  /Ascent %d\n"
               "  /CapHeight %d\n"
               "  /Descent -1\n"       // Spec says must be negative
               "  /Flags 5\n"          // FixedPitch + Symbolic
               "  /FontBBox  [ 0 0 %d %d ]\n"
               "  /FontFile2 %ld 0 R\n"
               "  /FontName /GlyphLessFont\n"
               "  /ItalicAngle 0\n"
               "  /StemV 80\n"
               "  /Type /FontDescriptor\n"
               ">>\n"
               "endobj\n",
               1000 / kCharHeight,
               1000 / kCharHeight,
               1000 / kCharWidth,
               1000 / kCharHeight,
               7L      // Font data
               );
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);

  n = snprintf(buf, sizeof(buf), "%s/pdf.ttf", datadir_);
  if (n >= sizeof(buf)) return false;
  FILE *fp = fopen(buf, "rb");
  if (!fp)
    return false;
  fseek(fp, 0, SEEK_END);
  long int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *buffer = new char[size];
  if (fread(buffer, 1, size, fp) != size) {
    fclose(fp);
    delete[] buffer;
    return false;
  }
  fclose(fp);
  // FONTFILE2
  n = snprintf(buf, sizeof(buf),
               "7 0 obj\n"
               "<<\n"
               "  /Length %ld\n"
               "  /Length1 %ld\n"
               ">>\n"
               "stream\n", size, size);
  if (n >= sizeof(buf)) return false;
  AppendString(buf);
  size_t objsize  = strlen(buf);
  AppendData(buffer, size);
  delete[] buffer;
  objsize += size;
  const char *b2 =
      "endstream\n"
      "endobj\n";
  AppendString(b2);
  objsize += strlen(b2);
  AppendPDFObjectDIY(objsize);
  return true;
}

bool TessPDFRenderer::imageToPDFObj(Pix *pix,
                                    char *filename,
                                    long int objnum,
                                    char **pdf_object,
                                    long int *pdf_object_size) {
  size_t n;
  char b0[kBasicBufSize];
  char b1[kBasicBufSize];
  char b2[kBasicBufSize];
  if (!pdf_object_size || !pdf_object)
    return false;
  *pdf_object = NULL;
  *pdf_object_size = 0;
  if (!filename)
    return false;

  L_COMP_DATA *cid = NULL;
  const int kJpegQuality = 85;

  // TODO(jbreiden) Leptonica 1.71 doesn't correctly handle certain
  // types of PNG files, especially if there are 2 samples per pixel.
  // We can get rid of this logic after Leptonica 1.72 is released and
  // has propagated everywhere. Bug discussion as follows.
  // https://code.google.com/p/tesseract-ocr/issues/detail?id=1300
  int format, sad;
  findFileFormat(filename, &format);
  if (pixGetSpp(pix) == 4 && format == IFF_PNG) {
    pixSetSpp(pix, 3);
    sad = pixGenerateCIData(pix, L_FLATE_ENCODE, 0, 0, &cid);
  } else {
    sad = l_generateCIDataForPdf(filename, pix, kJpegQuality, &cid);
  }

  if (sad || !cid) {
    l_CIDataDestroy(&cid);
    return false;
  }

  const char *group4 = "";
  const char *filter;
  switch(cid->type) {
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
  const char *colorspace;
  if (cid->ncolors > 0) {
    n = snprintf(b0, sizeof(b0),
                 "  /ColorSpace [ /Indexed /DeviceRGB %d %s ]\n",
                 cid->ncolors - 1, cid->cmapdatahex);
    if (n >= sizeof(b0)) {
      l_CIDataDestroy(&cid);
      return false;
    }
    colorspace = b0;
  } else {
    switch (cid->spp) {
      case 1:
        colorspace = "  /ColorSpace /DeviceGray\n";
        break;
      case 3:
        colorspace = "  /ColorSpace /DeviceRGB\n";
        break;
      default:
        l_CIDataDestroy(&cid);
        return false;
    }
  }

  int predictor = (cid->predictor) ? 14 : 1;

  // IMAGE
  n = snprintf(b1, sizeof(b1),
               "%ld 0 obj\n"
               "<<\n"
               "  /Length %ld\n"
               "  /Subtype /Image\n",
               objnum, (unsigned long) cid->nbytescomp);
  if (n >= sizeof(b1)) {
    l_CIDataDestroy(&cid);
    return false;
  }

  n = snprintf(b2, sizeof(b2),
               "  /Width %d\n"
               "  /Height %d\n"
               "  /BitsPerComponent %d\n"
               "  /Filter %s\n"
               "  /DecodeParms\n"
               "  <<\n"
               "    /Predictor %d\n"
               "    /Colors %d\n"
               "%s"
               "    /Columns %d\n"
               "    /BitsPerComponent %d\n"
               "  >>\n"
               ">>\n"
               "stream\n",
               cid->w, cid->h, cid->bps, filter, predictor, cid->spp,
               group4, cid->w, cid->bps);
  if (n >= sizeof(b2)) {
    l_CIDataDestroy(&cid);
    return false;
  }

  const char *b3 =
      "endstream\n"
      "endobj\n";

  size_t b1_len = strlen(b1);
  size_t b2_len = strlen(b2);
  size_t b3_len = strlen(b3);
  size_t colorspace_len = strlen(colorspace);

  *pdf_object_size =
      b1_len + colorspace_len + b2_len + cid->nbytescomp + b3_len;
  *pdf_object = new char[*pdf_object_size];
  if (!pdf_object) {
    l_CIDataDestroy(&cid);
    return false;
  }

  char *p = *pdf_object;
  memcpy(p, b1, b1_len);
  p += b1_len;
  memcpy(p, colorspace, colorspace_len);
  p += colorspace_len;
  memcpy(p, b2, b2_len);
  p += b2_len;
  memcpy(p, cid->datacomp, cid->nbytescomp);
  p += cid->nbytescomp;
  memcpy(p, b3, b3_len);
  l_CIDataDestroy(&cid);
  return true;
}

bool TessPDFRenderer::AddImageHandler(TessBaseAPI* api) {
  size_t n;
  char buf[kBasicBufSize];
  Pix *pix = api->GetInputImage();
  char *filename = (char *)api->GetInputName();
  int ppi = api->GetSourceYResolution();
  if (!pix || ppi <= 0)
    return false;
  double width = pixGetWidth(pix) * 72.0 / ppi;
  double height = pixGetHeight(pix) * 72.0 / ppi;

  // PAGE
  n = snprintf(buf, sizeof(buf),
               "%ld 0 obj\n"
               "<<\n"
               "  /Type /Page\n"
               "  /Parent %ld 0 R\n"
               "  /MediaBox [0 0 %.2f %.2f]\n"
               "  /Contents %ld 0 R\n"
               "  /Resources\n"
               "  <<\n"
               "    /XObject << /Im1 %ld 0 R >>\n"
               "    /ProcSet [ /PDF /Text /ImageB /ImageI /ImageC ]\n"
               "    /Font << /f-0-0 %ld 0 R >>\n"
               "  >>\n"
               ">>\n"
               "endobj\n",
               obj_,
               2L,            // Pages object
               width,
               height,
               obj_ + 1,      // Contents object
               obj_ + 2,      // Image object
               3L);           // Type0 Font
  if (n >= sizeof(buf)) return false;
  pages_.push_back(obj_);
  AppendPDFObject(buf);

  // CONTENTS
  char* pdftext = GetPDFTextObjects(api, width, height);
  long pdftext_len = strlen(pdftext);
  unsigned char *pdftext_casted = reinterpret_cast<unsigned char *>(pdftext);
  size_t len;
  unsigned char *comp_pdftext =
      zlibCompress(pdftext_casted,
                   pdftext_len,
                   &len);
  long comp_pdftext_len = len;
  n = snprintf(buf, sizeof(buf),
               "%ld 0 obj\n"
               "<<\n"
               "  /Length %ld /Filter /FlateDecode\n"
               ">>\n"
               "stream\n", obj_, comp_pdftext_len);
  if (n >= sizeof(buf)) {
    delete[] pdftext;
    lept_free(comp_pdftext);
    return false;
  }
  AppendString(buf);
  long objsize = strlen(buf);
  AppendData(reinterpret_cast<char *>(comp_pdftext), comp_pdftext_len);
  objsize += comp_pdftext_len;
  lept_free(comp_pdftext);
  delete[] pdftext;
  const char *b2 =
      "endstream\n"
      "endobj\n";
  AppendString(b2);
  objsize += strlen(b2);
  AppendPDFObjectDIY(objsize);

  char *pdf_object;
  if (!imageToPDFObj(pix, filename, obj_, &pdf_object, &objsize)) {
    return false;
  }
  AppendData(pdf_object, objsize);
  AppendPDFObjectDIY(objsize);
  delete[] pdf_object;
  return true;
}


bool TessPDFRenderer::EndDocumentHandler() {
  size_t n;
  char buf[kBasicBufSize];

  // We reserved the /Pages object number early, so that the /Page
  // objects could refer to their parent. We finally have enough
  // information to go fill it in. Using lower level calls to manipulate
  // the offset record in two spots, because we are placing objects
  // out of order in the file.

  // PAGES
  const long int kPagesObjectNumber = 2;
  offsets_[kPagesObjectNumber] = offsets_.back();  // manipulation #1
  n = snprintf(buf, sizeof(buf),
               "%ld 0 obj\n"
               "<<\n"
               "  /Type /Pages\n"
               "  /Kids [ ", kPagesObjectNumber);
  if (n >= sizeof(buf)) return false;
  AppendString(buf);
  size_t pages_objsize  = strlen(buf);
  for (size_t i = 0; i < pages_.size(); i++) {
    n = snprintf(buf, sizeof(buf),
                 "%ld 0 R ", pages_[i]);
    if (n >= sizeof(buf)) return false;
    AppendString(buf);
    pages_objsize += strlen(buf);
  }
  n = snprintf(buf, sizeof(buf),
               "]\n"
               "  /Count %d\n"
               ">>\n"
               "endobj\n", pages_.size());
  if (n >= sizeof(buf)) return false;
  AppendString(buf);
  pages_objsize += strlen(buf);
  offsets_.back() += pages_objsize;    // manipulation #2

  // INFO
  char* datestr = l_getFormattedDate();
  n = snprintf(buf, sizeof(buf),
               "%ld 0 obj\n"
               "<<\n"
               "  /Producer (Tesseract %s)\n"
               "  /CreationDate (D:%s)\n"
               "  /Title (%s)"
               ">>\n"
               "endobj\n", obj_, TESSERACT_VERSION_STR, datestr, title());
  lept_free(datestr);
  if (n >= sizeof(buf)) return false;
  AppendPDFObject(buf);
  n = snprintf(buf, sizeof(buf),
               "xref\n"
               "0 %ld\n"
               "0000000000 65535 f \n", obj_);
  if (n >= sizeof(buf)) return false;
  AppendString(buf);
  for (int i = 1; i < obj_; i++) {
    n = snprintf(buf, sizeof(buf), "%010ld 00000 n \n", offsets_[i]);
    if (n >= sizeof(buf)) return false;
    AppendString(buf);
  }
  n = snprintf(buf, sizeof(buf),
               "trailer\n"
               "<<\n"
               "  /Size %ld\n"
               "  /Root %ld 0 R\n"
               "  /Info %ld 0 R\n"
               ">>\n"
               "startxref\n"
               "%ld\n"
               "%%%%EOF\n",
               obj_,
               1L,               // catalog
               obj_ - 1,         // info
               offsets_.back());
  if (n >= sizeof(buf)) return false;
  AppendString(buf);
  return true;
}
}  // namespace tesseract
