// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// This file contains the HeliumTextDetector facade, which is used for simple
// access to the Helium text detection process.
// The output of this step are the detected text areas. Normally, you will want
// to pass the detected areas to a Binarizer, and pass the Binarizer to the
// TextRecognition module.
//
#ifndef HELIUM_HELIUMTEXTDETECTOR_H__
#define HELIUM_HELIUMTEXTDETECTOR_H__

#include "shapetree.h"
#include "textareas.h"
#include "textdetector.h"

namespace helium {

  class EdgeDetector;
  class Image;
  class Tracer;

  // This class is a facade to the complex workings of the Helium text
  // detection process. For standard use, it should be sufficient to use this
  // class. However, do note that there are many more ways to use and assort
  // the Helium components than this class allows you to.
  class HeliumTextDetector : public TextDetector {
    public:

      // Structure for the parameters of the text detector.
      struct TextDetectorParameters {
        // Whether to use Gaussian smoothing or quick smoothing
        bool use_gaussian_smoothing;

        // How strong to smooth, in case of using Gaussian smoothing
        uint8 gaussian_strength;

        // Which EdgeDetector to use
        EdgeDetector* edge_detector;

        // Which Tracer to use
        Tracer* tracer;

        // The Trace initiation threshold
        uint8 trace_threshold;

        TextDetectorParameters()
          : use_gaussian_smoothing(false),
            gaussian_strength(0),
            edge_detector(NULL),
            tracer(NULL),
            trace_threshold(0) {
        }
      };

      HeliumTextDetector();
      ~HeliumTextDetector();

      // Set the parameters of the text detector to the default values.
      // Specifically, this sets the parameters to:
      // use_gaussian_smoothing:  false
      // gaussian_strength: 3
      // edge_detector: SobelEdgeDetector()
      // tracer: MaxTracer(24)
      // trace_threshold: 16
      void SetDefaultParameters();

      inline void set_show_debug(bool flag) {
        show_debug_ = flag;
      }

      // Run the Helium text detection on the given Image using the
      // previously set parameters.
      void DetectText(const Image& image);

      // Accessor to the set of parameters.
      inline TextDetectorParameters& parameters() {
        return parameters_;
      }

      // Returns the detected clusters. Use this if you require color
      // information along with the detected boxes, for instance when passing
      // the detected areas to the HeliumBinarizer.
      const ClusterArray& GetClusters() {
        return clusterer_.clusters();
      }

      // Returns the detected text areas. This provides no textual information.
      // Use this when color information is not important, for instance when
      // passing the detected areas to the ThresholdBinarizer.
      const TextAreas& GetTextAreas();

      TextAreas* GetMutableTextAreas();

      // Returns the type of the text detector, here: MOBILE.
      TextDetectorType Type() const {
        return TextDetector::HELIUM;
      }

    private:
      static const int kMinDynamicRange = 128;

      TextDetectorParameters parameters_;
      Clusterer clusterer_;
      ShapeTree shapemaker_;
      TextAreas* text_areas_;
      bool show_debug_;
  };
} // namespace

#endif  // HELIUM_HELIUMTEXTDETECTOR_H__
