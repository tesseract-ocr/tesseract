/*
 * Copyright 2010, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HYDROGEN_HYDROGENTEXTDETECTOR_H_
#define HYDROGEN_HYDROGENTEXTDETECTOR_H_

#include "leptonica.h"

class HydrogenTextDetector {
public:
  HydrogenTextDetector();

  ~HydrogenTextDetector();

  struct TextDetectorParameters {
    bool debug;
    char out_dir[255];

    // Edge-based thresholding
    l_int32 edge_tile_x;
    l_int32 edge_tile_y;
    l_int32 edge_thresh;
    l_int32 edge_avg_thresh;

    // Skew angle correction
    bool skew_enabled;
    l_float32 skew_min_angle;
    l_float32 skew_sweep_range;
    l_float32 skew_sweep_delta;
    l_int32 skew_sweep_reduction;
    l_int32 skew_search_reduction;
    l_float32 skew_search_min_delta;

    // Singleton filter
    l_float32 single_min_aspect;
    l_float32 single_max_aspect;
    l_int32 single_min_area;
    l_float32 single_min_density;

    // Quick pair filter
    l_float32 pair_h_ratio;
    l_float32 pair_d_ratio;
    l_float32 pair_h_dist_ratio;
    l_float32 pair_v_dist_ratio;
    l_float32 pair_h_shared;

    // Cluster pair filter
    l_int32 cluster_width_spacing;
    l_float32 cluster_shared_edge;
    l_float32 cluster_h_ratio;

    // Finalized cluster filter
    l_int32 cluster_min_blobs;
    l_float32 cluster_min_aspect;
    l_float32 cluster_min_fdr;
    l_int32 cluster_min_edge;
    l_int32 cluster_min_edge_avg;

    TextDetectorParameters()
        : debug(false),
          edge_tile_x(32),
          edge_tile_y(64),
          edge_thresh(64),
          edge_avg_thresh(4),
          skew_enabled(true),
          skew_min_angle(1.0),
          skew_sweep_range(30.0),
          skew_sweep_delta(5.0),
          skew_sweep_reduction(8),
          skew_search_reduction(4),
          skew_search_min_delta(0.01),
          single_min_aspect(0.1),
          single_max_aspect(4.0),
          single_min_area(4),
          single_min_density(0.2),
          pair_h_ratio(1.0),
          pair_d_ratio(1.5),
          pair_h_dist_ratio(2.0),
          pair_v_dist_ratio(0.25),
          pair_h_shared(0.25),
          cluster_width_spacing(2),
          cluster_shared_edge(0.5),
          cluster_h_ratio(1.0),
          cluster_min_blobs(5),
          cluster_min_aspect(2),
          cluster_min_fdr(2.5),
          cluster_min_edge(32),
          cluster_min_edge_avg(1)
          {
    }
  };

  // Function to set the original source image
  void SetSourceImage(PIX *);

  // Main text detection function
  void DetectText();

  // Clear recognition results between calls
  void Clear();

  // Function to return text area clippings of the original image
  PIXA *GetImageAreas();

  // Function to return binarized text areas
  PIXA *GetTextAreas();

  // Function to return text area confidences
  NUMA *GetTextConfs();

  // Function to return detected skew angle
  l_float32 GetSkewAngle();

  // Function to return the original source image
  PIX *GetSourceImage();

  TextDetectorParameters *GetMutableParameters();

private:
  TextDetectorParameters parameters_;

  // Source image
  PIX *pixs_;
  // Detected text areas
  PIXA *text_areas_;
  // Confidences of detected text areas
  NUMA *text_confs_;
  // Detected skew angle
  l_float32 skew_angle_;

  // Function to extract text areas from a PIX
  PIXA *ExtractTextRegions(PIX *pix8, PIX *edges, NUMA **pconfs);

  // Function to detect and fix text skew
  PIX *DetectAndFixSkew(PIX *pixs);
};

#endif /* HYDROGEN_HYDROGENTEXTDETECTOR_H_ */
