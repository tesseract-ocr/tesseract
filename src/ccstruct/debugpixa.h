#ifndef TESSERACT_CCSTRUCT_DEBUGPIXA_H_
#define TESSERACT_CCSTRUCT_DEBUGPIXA_H_

#include "allheaders.h"

namespace tesseract {

// Class to hold a Pixa collection of debug images with captions and save them
// to a PDF file.
class DebugPixa {
 public:
  // TODO(rays) add another constructor with size control.
  DebugPixa() {
    pixa_ = pixaCreate(0);
#ifdef TESSERACT_DISABLE_DEBUG_FONTS
    fonts_ = NULL;
#else
    fonts_ = bmfCreate(nullptr, 14);
#endif
  }
  // If the filename_ has been set and there are any debug images, they are
  // written to the set filename_.
  ~DebugPixa() {
    pixaDestroy(&pixa_);
    bmfDestroy(&fonts_);
  }

  // Adds the given pix to the set of pages in the PDF file, with the given
  // caption added to the top.
  void AddPix(const Pix* pix, const char* caption) {
    int depth = pixGetDepth(const_cast<Pix*>(pix));
    int color = depth < 8 ? 1 : (depth > 8 ? 0x00ff0000 : 0x80);
    Pix* pix_debug = pixAddSingleTextblock(
        const_cast<Pix*>(pix), fonts_, caption, color, L_ADD_BELOW, nullptr);
    pixaAddPix(pixa_, pix_debug, L_INSERT);
  }

  // Sets the destination filename and enables images to be written to a PDF
  // on destruction.
  void WritePDF(const char* filename) {
    if (pixaGetCount(pixa_) > 0) {
      pixaConvertToPdf(pixa_, 300, 1.0f, 0, 0, "AllDebugImages", filename);
      pixaClear(pixa_);
    }
  }

 private:
  // The collection of images to put in the PDF.
  Pixa* pixa_;
  // The fonts used to draw text captions.
  L_Bmf* fonts_;
};

}  // namespace tesseract

#endif  // TESSERACT_CCSTRUCT_DEBUGPIXA_H_
