// Copyright 2006 Google Inc.
// All Rights Reserved.
// Author: <renn@google.com> (Marius Renn)
//
// Local includes
#include <time.h>
#include "clusterer.h"
#include "contourdetector.h"
#include "debugging.h"
#include "edgedetector.h"
#include "gaussiansmooth.h"
#include "heliumtextdetector.h"
#include "maxtracer.h"
#include "imageenhancer.h"
#include "helium_image.h"
#include "quicksmooth.h"
#include "shapetree.h"
#include "sobeledgedetector.h"
#include "laplaceedgedetector.h"
#include "textclassifier.h"
#include "textvalidator.h"

using namespace helium;


static void ShowTrace(ContourDetector& outliner, int trace_type,
                      int width, int height) {
  Mask mask(width, height);
  outliner.PlotTracesOnto(mask, trace_type);
  Image trace_image = Image::FromMask(mask);
  // Leptonica::DisplayImage(trace_image);
}

static void ShowShapeTree(Clusterer& clusterer, ShapeTree& shapes,
                          int width, int height) {
  Image image(width, height);
  image.Clear();
  shapes.PaintShapes(image);
  clusterer.DrawClusterBounds(image);
  // Leptonica::DisplayImage(image);
}

HeliumTextDetector::HeliumTextDetector()
  : parameters_(),
    clusterer_(),
    shapemaker_(),
    text_areas_(NULL),
    show_debug_(false) {
}

HeliumTextDetector::~HeliumTextDetector() {
  delete text_areas_;
  delete parameters_.edge_detector;
  delete parameters_.tracer;
}

void HeliumTextDetector::SetDefaultParameters() {
  parameters_.use_gaussian_smoothing = false;
  parameters_.gaussian_strength = 3;
  parameters_.edge_detector = new SobelEdgeDetector();
  // parameters_.edge_detector = new LaplaceEdgeDetector();
  parameters_.tracer = new MaxTracer(24);
  parameters_.trace_threshold = 16;
}

const TextAreas& HeliumTextDetector::GetTextAreas() {
  GetMutableTextAreas();
  return *text_areas_;
}

TextAreas* HeliumTextDetector::GetMutableTextAreas() {
  if (!text_areas_) {
    text_areas_ = new TextAreas(clusterer_.clusters());
  }
  return text_areas_;
}

void HeliumTextDetector::DetectText(const Image& image) {
  ASSERT(image.Valid());

  // Clean up any remains from last detection
  delete(text_areas_);
  text_areas_ = NULL;

  // Parameters set?
  ASSERT(parameters_.edge_detector && parameters_.tracer);

  // Smooth image
  Image smooth_image;
  if (parameters_.use_gaussian_smoothing) {
    GaussianSmooth gaussian_blur(parameters_.gaussian_strength);
    smooth_image = gaussian_blur.Smooth(image);
  } else {
    smooth_image.Copy(image);
    QuickSmooth::Smooth(smooth_image);
  }

  ImageEnhancer::EnhanceColors(smooth_image, kMinDynamicRange);

  // Edge detection
  EdgeDetector* detector = parameters_.edge_detector;
  GrayMap edges = detector->DetectEdges(smooth_image);
  ImageEnhancer::LocalContrast(edges);

  // Tracing
  TextClassifier text_classifier;
  Tracer* tracer = parameters_.tracer;
  tracer->set_trace_map(&edges);
  ContourDetector outliner(*tracer, text_classifier);
  outliner.DetectContours(edges,
                          parameters_.trace_threshold,
                          smooth_image);

  // Labeling
  shapemaker_.ClearShapes();
  shapemaker_.ConvertTraces(outliner.contours(), smooth_image);

  // Clustering
  TextValidator text_validator;
  clusterer_.ClearClusters();
  clusterer_.ClusterShapes(shapemaker_.shapes(), text_validator);

  if (show_debug_) {
    // Leptonica::DisplayImage(smooth_image);
    // Leptonica::DisplayGrayMap(edges);
    ShowTrace(outliner, TRACECLASS_TEXT, image.width(), image.height());
    ShowShapeTree(clusterer_, shapemaker_, image.width(), image.height());
  }
}
