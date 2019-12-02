///////////////////////////////////////////////////////////////////////
// File:        renderer.h
// Description: Rendering interface to inject into TessBaseAPI
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

#ifndef TESSERACT_API_RENDERER_H_
#define TESSERACT_API_RENDERER_H_

// To avoid collision with other typenames include the ABSOLUTE MINIMUM
// complexity of includes here. Use forward declarations wherever possible
// and hide includes of complex types in baseapi.cpp.
#include <string>  // for std::string

#include "genericvector.h"
#include "platform.h"
#include "strngs.h"  // for STRING

struct Pix;

namespace tesseract {

class TessBaseAPI;

/**
 * Interface for rendering tesseract results into a document, such as text,
 * HOCR or pdf. This class is abstract. Specific classes handle individual
 * formats. This interface is then used to inject the renderer class into
 * tesseract when processing images.
 *
 * For simplicity implementing this with tesseract version 3.01,
 * the renderer contains document state that is cleared from document
 * to document just as the TessBaseAPI is. This way the base API can just
 * delegate its rendering functionality to injected renderers, and the
 * renderers can manage the associated state needed for the specific formats
 * in addition to the heuristics for producing it.
 */
class TESS_API TessResultRenderer {
 public:
  virtual ~TessResultRenderer();

  // Takes ownership of pointer so must be new'd instance.
  // Renderers aren't ordered, but appends the sequences of next parameter
  // and existing next(). The renderers should be unique across both lists.
  void insert(TessResultRenderer* next);

  // Returns the next renderer or nullptr.
  TessResultRenderer* next() {
    return next_;
  }

  /**
   * Starts a new document with the given title.
   * This clears the contents of the output data.
   * Title should use UTF-8 encoding.
   */
  bool BeginDocument(const char* title);

  /**
   * Adds the recognized text from the source image to the current document.
   * Invalid if BeginDocument not yet called.
   *
   * Note that this API is a bit weird but is designed to fit into the
   * current TessBaseAPI implementation where the api has lots of state
   * information that we might want to add in.
   */
  bool AddImage(TessBaseAPI* api);

  /**
   * Finishes the document and finalizes the output data
   * Invalid if BeginDocument not yet called.
   */
  bool EndDocument();

  const char* file_extension() const {
    return file_extension_;
  }
  const char* title() const {
    return title_.c_str();
  }

  // Is everything fine? Otherwise something went wrong.
  bool happy() {
    return happy_;
  }

  /**
   * Returns the index of the last image given to AddImage
   * (i.e. images are incremented whether the image succeeded or not)
   *
   * This is always defined. It means either the number of the
   * current image, the last image ended, or in the completed document
   * depending on when in the document lifecycle you are looking at it.
   * Will return -1 if a document was never started.
   */
  int imagenum() const {
    return imagenum_;
  }

 protected:
  /**
   * Called by concrete classes.
   *
   * outputbase is the name of the output file excluding
   * extension. For example, "/path/to/chocolate-chip-cookie-recipe"
   *
   * extension indicates the file extension to be used for output
   * files. For example "pdf" will produce a .pdf file, and "hocr"
   * will produce .hocr files.
   */
  TessResultRenderer(const char* outputbase, const char* extension);

  // Hook for specialized handling in BeginDocument()
  virtual bool BeginDocumentHandler();

  // This must be overridden to render the OCR'd results
  virtual bool AddImageHandler(TessBaseAPI* api) = 0;

  // Hook for specialized handling in EndDocument()
  virtual bool EndDocumentHandler();

  // Renderers can call this to append '\0' terminated strings into
  // the output string returned by GetOutput.
  // This method will grow the output buffer if needed.
  void AppendString(const char* s);

  // Renderers can call this to append binary byte sequences into
  // the output string returned by GetOutput. Note that s is not necessarily
  // '\0' terminated (and can contain '\0' within it).
  // This method will grow the output buffer if needed.
  void AppendData(const char* s, int len);

 private:
  const char* file_extension_;  // standard extension for generated output
  STRING title_;                // title of document being rendered
  int imagenum_;                // index of last image added

  FILE* fout_;                // output file pointer
  TessResultRenderer* next_;  // Can link multiple renderers together
  bool happy_;                // I get grumpy when the disk fills up, etc.
};

/**
 * Renders tesseract output into a plain UTF-8 text string
 */
class TESS_API TessTextRenderer : public TessResultRenderer {
 public:
  explicit TessTextRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

/**
 * Renders tesseract output into an hocr text string
 */
class TESS_API TessHOcrRenderer : public TessResultRenderer {
 public:
  explicit TessHOcrRenderer(const char* outputbase, bool font_info);
  explicit TessHOcrRenderer(const char* outputbase);

 protected:
  bool BeginDocumentHandler() override;
  bool AddImageHandler(TessBaseAPI* api) override;
  bool EndDocumentHandler() override;

 private:
  bool font_info_;  // whether to print font information
};

/**
 * Renders tesseract output into an alto text string
 */
class TESS_API TessAltoRenderer : public TessResultRenderer {
 public:
  explicit TessAltoRenderer(const char* outputbase);

 protected:
  bool BeginDocumentHandler() override;
  bool AddImageHandler(TessBaseAPI* api) override;
  bool EndDocumentHandler() override;
};

/**
 * Renders Tesseract output into a TSV string
 */
class TESS_API TessTsvRenderer : public TessResultRenderer {
 public:
  explicit TessTsvRenderer(const char* outputbase, bool font_info);
  explicit TessTsvRenderer(const char* outputbase);

 protected:
  bool BeginDocumentHandler() override;
  bool AddImageHandler(TessBaseAPI* api) override;
  bool EndDocumentHandler() override;

 private:
  bool font_info_;  // whether to print font information
};

/**
 * Renders tesseract output into searchable PDF
 */
class TESS_API TessPDFRenderer : public TessResultRenderer {
 public:
  // datadir is the location of the TESSDATA. We need it because
  // we load a custom PDF font from this location.
  TessPDFRenderer(const char* outputbase, const char* datadir,
                  bool textonly = false);

 protected:
  bool BeginDocumentHandler() override;
  bool AddImageHandler(TessBaseAPI* api) override;
  bool EndDocumentHandler() override;

 private:
  // We don't want to have every image in memory at once,
  // so we store some metadata as we go along producing
  // PDFs one page at a time. At the end, that metadata is
  // used to make everything that isn't easily handled in a
  // streaming fashion.
  long int obj_;                     // counter for PDF objects
  GenericVector<long int> offsets_;  // offset of every PDF object in bytes
  GenericVector<long int> pages_;    // object number for every /Page object
  std::string datadir_;              // where to find the custom font
  bool textonly_;                    // skip images if set
  // Bookkeeping only. DIY = Do It Yourself.
  void AppendPDFObjectDIY(size_t objectsize);
  // Bookkeeping + emit data.
  void AppendPDFObject(const char* data);
  // Create the /Contents object for an entire page.
  char* GetPDFTextObjects(TessBaseAPI* api, double width, double height);
  // Turn an image into a PDF object. Only transcode if we have to.
  static bool imageToPDFObj(Pix* pix, const char* filename, long int objnum,
                            char** pdf_object, long int* pdf_object_size,
                            int jpg_quality);
};

/**
 * Renders tesseract output into a plain UTF-8 text string
 */
class TESS_API TessUnlvRenderer : public TessResultRenderer {
 public:
  explicit TessUnlvRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

/**
 * Renders tesseract output into a plain UTF-8 text string for LSTMBox
 */
class TESS_API TessLSTMBoxRenderer : public TessResultRenderer {
 public:
  explicit TessLSTMBoxRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

/**
 * Renders tesseract output into a plain UTF-8 text string
 */
class TESS_API TessBoxTextRenderer : public TessResultRenderer {
 public:
  explicit TessBoxTextRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

/**
 * Renders tesseract output into a plain UTF-8 text string in WordStr format
 */
class TESS_API TessWordStrBoxRenderer : public TessResultRenderer {
 public:
  explicit TessWordStrBoxRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

#ifndef DISABLED_LEGACY_ENGINE

/**
 * Renders tesseract output into an osd text string
 */
class TESS_API TessOsdRenderer : public TessResultRenderer {
 public:
  explicit TessOsdRenderer(const char* outputbase);

 protected:
  bool AddImageHandler(TessBaseAPI* api) override;
};

#endif  // ndef DISABLED_LEGACY_ENGINE

}  // namespace tesseract.

#endif  // TESSERACT_API_RENDERER_H_
