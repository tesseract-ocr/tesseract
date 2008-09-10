# -*- mode: python; -*-
#
# Copyright 2006 Google Inc.
# All Rights Reserved.
#
# Description:
#   The Helium text detector.

licenses(['unencumbered'])  # code authored by Google

# Applications
# =========================================================

cc_binary(name = "text_detect",
          srcs = [
                   "binarize.cc",
                   "box.cc",
                   "cluster.cc",
                   "clusterer.cc",
                   "color.cc",
                   "cstringutils.cc",
                   "contourdetector.cc",
                   "debugging.cc",
                   "edgedetector.cc",
                   "gaussiansmooth.cc",
                   "heliumbinarizer.cc",
                   "heliumtextdetector.cc",
                   "histogram.cc",
                   "image.cc",
                   "imageenhancer.cc",
                   "leptonica.cc",
                   "main_helium.cc",
                   "mask.cc",
                   "maxtracer.cc",
                   "mathfunctions.cc",
                   "perspectivedetection.cc",
                   "point.cc",
                   "quicksmooth.cc",
                   "refcount.cc",
                   "shape.cc",
                   "shapetree.cc",
                   "sobeledgedetector.cc",
                   "laplaceedgedetector.cc",
                   "surface.cc",
                   "stringutils.cc",
                   "tesseract.cc",
                   "textareas.cc",
                   "textclassifier.cc",
                   "textvalidator.cc",
                   "textrecognition.cc",
                   "thresholdbinarizer.cc",
                   "trace.cc",
                   "tracer.cc",
                   "unionfind.cc"
                   ],
          deps = [
                   "//third_party/leptonica",
                   "//third_party/tesseract:tesseract_main",
                   "//base"
                   ] )
                   
cc_library(name = "helium",
          srcs = [
                   "binarize.cc",
                   "box.cc",
                   "cluster.cc",
                   "clusterer.cc",
                   "color.cc",
                   "cstringutils.cc",
                   "contourdetector.cc",
                   "debugging.cc",
                   "edgedetector.cc",
                   "gaussiansmooth.cc",
                   "heliumbinarizer.cc",
                   "heliumtextdetector.cc",
                   "histogram.cc",
                   "image.cc",
                   "imageenhancer.cc",
                   "labeler.cc",
                   "leptonica.cc",
                   "mask.cc",
                   "maxtracer.cc",
                   "mathfunctions.cc",
                   "perspectivedetection.cc",
                   "point.cc",
                   "quicksmooth.cc",
                   "refcount.cc",
                   "shape.cc",
                   "shapetree.cc",
                   "sobeledgedetector.cc",
                   "laplaceedgedetector.cc",
                   "surface.cc",
                   "stringutils.cc",
                   "tesseract.cc",
                   "textareas.cc",
                   "textclassifier.cc",
                   "textvalidator.cc",
                   "textrecognition.cc",
                   "thresholdbinarizer.cc",
                   "trace.cc",
                   "tracer.cc",
                   "unionfind.cc"
                   ],
          deps = [
                   "//third_party/leptonica",
                   "//third_party/tesseract:tesseract_main",
                   "//base"
                   ] )
                   
