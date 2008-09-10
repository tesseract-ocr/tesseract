// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the abstract TextDetector base class, which provides a
// common interface to various text detectors. Currently, there are two
// implementations of this interface: OsmiumTextDetector and
// HeliumTextDetector.
//
#ifndef HELIUM_TEXTDETECTOR_H__
#define HELIUM_TEXTDETECTOR_H__

#include "mask.h"
#include "box.h"
#include "clusterer.h"

namespace helium {

  class TextAreas;

  // The abstract base class TextDetector provides a common interface to all
  // text detectors. The main method DetectText(Image&) is used to detect
  // text in the given image. GetTextAreas() returns the detected text areas.
  class TextDetector {
    public:

      // These constants identify the text detector type. They are returned
      // by the Type() method.
      enum TextDetectorType {
        HELIUM,
        OSMIUM,
        CARBON,
        DUMMY,
        OCR_DETECT,
        OTHER
      };

      // Virtual deconstructor for subclasses.
      virtual ~TextDetector() {
      }

      // This method should be called if parameters should not be passed
      // manually to the text detector. This superclass implementation does
      // nothing.
      virtual void SetDefaultParameters() {
      }

      // Detector-specific parameters setting.  This is guaranteed to be
      // called before detector use, so it can be used for initialization.
      // But it maybe called multiple times after initialization as well.
      // The parameter string may be empty, in which case it should implement
      // SetDefaultParameters().
      virtual void SetParameters(const char* parameter) {
      }

      // Specifies the data directory where classifier or parameter files
      // can be found.  This is often needed so worth separating from
      // SetParameters().
      virtual void SetDataDir(const char* data_dir) {
      }

      // Method to detect the text in the given image. The image can not be
      // modified during detection.  The result will be stored internally,
      // and can be accessed via the GetTextAreas() method.
      // Note: The HeliumTextDetector can also return the Clusters, which
      // contain the color information associated with each Cluster.
      virtual void DetectText(const Image& image) = 0;

      // Returns the detected text areas, with no textual information.
      // This method is non-const, so that the subclass may compute and store
      // the result during the call.
      virtual const TextAreas& GetTextAreas() = 0;

      // Returns a pointer to the textarea and allow modification.  This is
      // used to rescale the boxes to the original size if a downscaled image
      // was used.
      virtual TextAreas* GetMutableTextAreas() = 0;

      // Optional implementation to return a binary text mask for detected
      // text area, which must be the same size as the ith box in TextAreas,
      // and the calling function is responsible for freeing the memory.
      // The function returns true if the mask can be generated.
      virtual bool GetTextMask(int i, Mask *text_mask, Box *box) {
        return false;
      }

      // Returns the type of the text detector. See the TextDetectorType
      // enumeration for a list of available types.
      virtual TextDetectorType Type() const = 0;

      virtual void set_show_debug(bool flag) {}
  };

} // namespace

#endif  // HELIUM_TEXTDETECTOR_H__
