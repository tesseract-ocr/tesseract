///////////////////////////////////////////////////////////////////////
// File:        networkio.cpp
// Description: Network input/output data, allowing float/int implementations.
// Author:      Ray Smith
// Created:     Thu Jun 19 13:01:31 PST 2014
//
// (C) Copyright 2014, Google Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////

#include "networkio.h"
#include <cfloat>        // for FLT_MAX

#include "allheaders.h"
#include "functions.h"
#include "statistc.h"
#include "tprintf.h"

namespace tesseract {

// Minimum value to output for certainty.
const float kMinCertainty = -20.0f;
// Probability corresponding to kMinCertainty.
const float kMinProb = exp(kMinCertainty);

// Resizes to a specific size as a 2-d temp buffer. No batches, no y-dim.
void NetworkIO::Resize2d(bool int_mode, int width, int num_features) {
  stride_map_ = StrideMap();
  int_mode_ = int_mode;
  if (int_mode_) {
    i_.ResizeNoInit(width, num_features, GetPadding(num_features));
  } else {
    f_.ResizeNoInit(width, num_features);
  }
}

// Resizes to a specific stride_map.
void NetworkIO::ResizeToMap(bool int_mode, const StrideMap& stride_map,
                            int num_features) {
  // If this method crashes with this == nullptr,
  // it most likely got here through an uninitialized scratch element,
  // ie call NetworkScratch::IO::Resizexxx() not NetworkIO::Resizexxx()!!
  stride_map_ = stride_map;
  int_mode_ = int_mode;
  if (int_mode_) {
    i_.ResizeNoInit(stride_map.Width(), num_features, GetPadding(num_features));
  } else {
    f_.ResizeNoInit(stride_map.Width(), num_features);
  }
  ZeroInvalidElements();
}

// Shrinks image size by x_scale,y_scale, and use given number of features.
void NetworkIO::ResizeScaled(const NetworkIO& src,
                             int x_scale, int y_scale, int num_features) {
  StrideMap stride_map = src.stride_map_;
  stride_map.ScaleXY(x_scale, y_scale);
  ResizeToMap(src.int_mode_, stride_map, num_features);
}

// Resizes to just 1 x-coord, whatever the input.
void NetworkIO::ResizeXTo1(const NetworkIO& src, int num_features) {
  StrideMap stride_map = src.stride_map_;
  stride_map.ReduceWidthTo1();
  ResizeToMap(src.int_mode_, stride_map, num_features);
}

// Initialize all the array to zero.
void NetworkIO::Zero() {
  int width = Width();
  // Zero out the everything. Column-by-column in case it is aligned.
  for (int t = 0; t < width; ++t) {
    ZeroTimeStep(t);
  }
}

// Initializes to zero all elements of the array that do not correspond to
// valid image positions. (If a batch of different-sized images are packed
// together, then there will be padding pixels.)
void NetworkIO::ZeroInvalidElements() {
  int num_features = NumFeatures();
  int full_width = stride_map_.Size(FD_WIDTH);
  int full_height = stride_map_.Size(FD_HEIGHT);
  StrideMap::Index b_index(stride_map_);
  do {
    int end_x = b_index.MaxIndexOfDim(FD_WIDTH) + 1;
    if (end_x < full_width) {
      // The width is small, so fill for every valid y.
      StrideMap::Index y_index(b_index);
      int fill_size = num_features * (full_width - end_x);
      do {
        StrideMap::Index z_index(y_index);
        z_index.AddOffset(end_x, FD_WIDTH);
        if (int_mode_) {
          ZeroVector(fill_size, i_[z_index.t()]);
        } else {
          ZeroVector(fill_size, f_[z_index.t()]);
        }
      } while (y_index.AddOffset(1, FD_HEIGHT));
    }
    int end_y = b_index.MaxIndexOfDim(FD_HEIGHT) + 1;
    if (end_y < full_height) {
      // The height is small, so fill in the space in one go.
      StrideMap::Index y_index(b_index);
      y_index.AddOffset(end_y, FD_HEIGHT);
      int fill_size = num_features * full_width * (full_height - end_y);
      if (int_mode_) {
        ZeroVector(fill_size, i_[y_index.t()]);
      } else {
        ZeroVector(fill_size, f_[y_index.t()]);
      }
    }
  } while (b_index.AddOffset(1, FD_BATCH));
}

// Helper computes a black point and white point to contrast-enhance an image.
// The computation is based on the assumption that the image is of a single line
// of text, so a horizontal line through the middle of the image passes through
// at least some of it, so local minima and maxima are a good proxy for black
// and white pixel samples.
static void ComputeBlackWhite(Pix* pix, float* black, float* white) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  STATS mins(0, 256), maxes(0, 256);
  if (width >= 3) {
    int y = height / 2;
    l_uint32* line = pixGetData(pix) + pixGetWpl(pix) * y;
    int prev = GET_DATA_BYTE(line, 0);
    int curr = GET_DATA_BYTE(line, 1);
    for (int x = 1; x + 1 < width; ++x) {
      int next = GET_DATA_BYTE(line, x + 1);
      if ((curr < prev && curr <= next) || (curr <= prev && curr < next)) {
        // Local minimum.
        mins.add(curr, 1);
      }
      if ((curr > prev && curr >= next) || (curr >= prev && curr > next)) {
        // Local maximum.
        maxes.add(curr, 1);
      }
      prev = curr;
      curr = next;
    }
  }
  if (mins.get_total() == 0) mins.add(0, 1);
  if (maxes.get_total() == 0) maxes.add(255, 1);
  *black = mins.ile(0.25);
  *white = maxes.ile(0.75);
}

// Sets up the array from the given image, using the currently set int_mode_.
// If the image width doesn't match the shape, the image is truncated or padded
// with noise to match.
void NetworkIO::FromPix(const StaticShape& shape, const Pix* pix,
                        TRand* randomizer) {
  std::vector<const Pix*> pixes(1, pix);
  FromPixes(shape, pixes, randomizer);
}

// Sets up the array from the given set of images, using the currently set
// int_mode_. If the image width doesn't match the shape, the images are
// truncated or padded with noise to match.
void NetworkIO::FromPixes(const StaticShape& shape,
                          const std::vector<const Pix*>& pixes,
                          TRand* randomizer) {
  int target_height = shape.height();
  int target_width = shape.width();
  std::vector<std::pair<int, int>> h_w_pairs;
  for (auto pix : pixes) {
    Pix* var_pix = const_cast<Pix*>(pix);
    int width = pixGetWidth(var_pix);
    if (target_width != 0) width = target_width;
    int height = pixGetHeight(var_pix);
    if (target_height != 0) height = target_height;
    h_w_pairs.emplace_back(height, width);
  }
  stride_map_.SetStride(h_w_pairs);
  ResizeToMap(int_mode(), stride_map_, shape.depth());
  // Iterate over the images again to copy the data.
  for (size_t b = 0; b < pixes.size(); ++b) {
    Pix* pix = const_cast<Pix*>(pixes[b]);
    float black = 0.0f, white = 255.0f;
    if (shape.depth() != 3) ComputeBlackWhite(pix, &black, &white);
    float contrast = (white - black) / 2.0f;
    if (contrast <= 0.0f) contrast = 1.0f;
    if (shape.height() == 1) {
      Copy1DGreyImage(b, pix, black, contrast, randomizer);
    } else {
      Copy2DImage(b, pix, black, contrast, randomizer);
    }
  }
}

// Copies the given pix to *this at the given batch index, stretching and
// clipping the pixel values so that [black, black + 2*contrast] maps to the
// dynamic range of *this, ie [-1,1] for a float and (-127,127) for int.
// This is a 2-d operation in the sense that the output depth is the number
// of input channels, the height is the height of the image, and the width
// is the width of the image, or truncated/padded with noise if the width
// is a fixed size.
void NetworkIO::Copy2DImage(int batch, Pix* pix, float black, float contrast,
                            TRand* randomizer) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  int wpl = pixGetWpl(pix);
  StrideMap::Index index(stride_map_);
  index.AddOffset(batch, FD_BATCH);
  int t = index.t();
  int target_height = stride_map_.Size(FD_HEIGHT);
  int target_width = stride_map_.Size(FD_WIDTH);
  int num_features = NumFeatures();
  bool color = num_features == 3;
  if (width > target_width) width = target_width;
  uint32_t* line = pixGetData(pix);
  for (int y = 0; y < target_height; ++y, line += wpl) {
    int x = 0;
    if (y < height) {
      for (x = 0; x < width; ++x, ++t) {
        if (color) {
          int f = 0;
          for (int c = COLOR_RED; c <= COLOR_BLUE; ++c) {
            int pixel = GET_DATA_BYTE(line + x, c);
            SetPixel(t, f++, pixel, black, contrast);
          }
        } else {
          int pixel = GET_DATA_BYTE(line, x);
          SetPixel(t, 0, pixel, black, contrast);
        }
      }
    }
    for (; x < target_width; ++x) Randomize(t++, 0, num_features, randomizer);
  }
}

// Copies the given pix to *this at the given batch index, as Copy2DImage
// above, except that the output depth is the height of the input image, the
// output height is 1, and the output width as for Copy2DImage.
// The image is thus treated as a 1-d set of vertical pixel strips.
void NetworkIO::Copy1DGreyImage(int batch, Pix* pix, float black,
                                float contrast, TRand* randomizer) {
  int width = pixGetWidth(pix);
  int height = pixGetHeight(pix);
  ASSERT_HOST(height == NumFeatures());
  int wpl = pixGetWpl(pix);
  StrideMap::Index index(stride_map_);
  index.AddOffset(batch, FD_BATCH);
  int t = index.t();
  int target_width = stride_map_.Size(FD_WIDTH);
  if (width > target_width) width = target_width;
  int x;
  for (x = 0; x < width; ++x, ++t) {
    for (int y = 0; y < height; ++y) {
      uint32_t* line = pixGetData(pix) + wpl * y;
      int pixel = GET_DATA_BYTE(line, x);
      SetPixel(t, y, pixel, black, contrast);
    }
  }
  for (; x < target_width; ++x) Randomize(t++, 0, height, randomizer);
}

// Helper stores the pixel value in i_ or f_ according to int_mode_.
// t: is the index from the StrideMap corresponding to the current
//   [batch,y,x] position
// f: is the index into the depth/channel
// pixel: the value of the pixel from the image (in one channel)
// black: the pixel value to map to the lowest of the range of *this
// contrast: the range of pixel values to stretch to half the range of *this.
void NetworkIO::SetPixel(int t, int f, int pixel, float black, float contrast) {
  float float_pixel = (pixel - black) / contrast - 1.0f;
  if (int_mode_) {
    i_[t][f] = ClipToRange<int>(IntCastRounded((INT8_MAX + 1) * float_pixel),
                           -INT8_MAX, INT8_MAX);
  } else {
    f_[t][f] = float_pixel;
  }
}

// Converts the array to a Pix. Must be pixDestroyed after use.
Pix* NetworkIO::ToPix() const {
  // Count the width of the image, and find the max multiplication factor.
  int im_width = stride_map_.Size(FD_WIDTH);
  int im_height = stride_map_.Size(FD_HEIGHT);
  int num_features = NumFeatures();
  int feature_factor = 1;
  if (num_features == 3) {
    // Special hack for color.
    num_features = 1;
    feature_factor = 3;
  }
  Pix* pix = pixCreate(im_width, im_height * num_features, 32);
  StrideMap::Index index(stride_map_);
  do {
    int im_x = index.index(FD_WIDTH);
    int top_im_y = index.index(FD_HEIGHT);
    int im_y = top_im_y;
    int t = index.t();
    if (int_mode_) {
      const int8_t* features = i_[t];
      for (int y = 0; y < num_features; ++y, im_y += im_height) {
        int pixel = features[y * feature_factor];
        // 1 or 2 features use greyscale.
        int red = ClipToRange<int>(pixel + 128, 0, 255);
        int green = red, blue = red;
        if (feature_factor == 3) {
          // With 3 features assume RGB color.
          green = ClipToRange<int>(features[y * feature_factor + 1] + 128, 0, 255);
          blue = ClipToRange<int>(features[y * feature_factor + 2] + 128, 0, 255);
        } else if (num_features > 3) {
          // More than 3 features use false yellow/blue color, assuming a signed
          // input in the range [-1,1].
          red = abs(pixel) * 2;
          if (pixel >= 0) {
            green = red;
            blue = 0;
          } else {
            blue = red;
            green = red = 0;
          }
        }
        pixSetPixel(pix, im_x, im_y, (red << L_RED_SHIFT) |
                                         (green << L_GREEN_SHIFT) |
                                         (blue << L_BLUE_SHIFT));
      }
    } else {
      const float* features = f_[t];
      for (int y = 0; y < num_features; ++y, im_y += im_height) {
        float pixel = features[y * feature_factor];
        // 1 or 2 features use greyscale.
        int red = ClipToRange<int>(IntCastRounded((pixel + 1.0f) * 127.5f), 0, 255);
        int green = red, blue = red;
        if (feature_factor == 3) {
          // With 3 features assume RGB color.
          pixel = features[y * feature_factor + 1];
          green = ClipToRange<int>(IntCastRounded((pixel + 1.0f) * 127.5f), 0, 255);
          pixel = features[y * feature_factor + 2];
          blue = ClipToRange<int>(IntCastRounded((pixel + 1.0f) * 127.5f), 0, 255);
        } else if (num_features > 3) {
          // More than 3 features use false yellow/blue color, assuming a signed
          // input in the range [-1,1].
          red = ClipToRange<int>(IntCastRounded(fabs(pixel) * 255), 0, 255);
          if (pixel >= 0) {
            green = red;
            blue = 0;
          } else {
            blue = red;
            green = red = 0;
          }
        }
        pixSetPixel(pix, im_x, im_y, (red << L_RED_SHIFT) |
                                         (green << L_GREEN_SHIFT) |
                                         (blue << L_BLUE_SHIFT));
      }
    }
  } while (index.Increment());
  return pix;
}

// Prints the first and last num timesteps of the array for each feature.
void NetworkIO::Print(int num) const {
  int num_features = NumFeatures();
  for (int y = 0; y < num_features; ++y) {
    for (int t = 0; t < Width(); ++t) {
      if (num == 0 || t < num || t + num >= Width()) {
        if (int_mode_) {
          tprintf(" %g", static_cast<float>(i_[t][y]) / INT8_MAX);
        } else {
          tprintf(" %g", f_[t][y]);
        }
      }
    }
    tprintf("\n");
  }
}

// Copies a single time step from src.
void NetworkIO::CopyTimeStepFrom(int dest_t, const NetworkIO& src, int src_t) {
  ASSERT_HOST(int_mode_ == src.int_mode_);
  if (int_mode_) {
    memcpy(i_[dest_t], src.i_[src_t], i_.dim2() * sizeof(i_[0][0]));
  } else {
    memcpy(f_[dest_t], src.f_[src_t], f_.dim2() * sizeof(f_[0][0]));
  }
}

// Copies a part of single time step from src.
void NetworkIO::CopyTimeStepGeneral(int dest_t, int dest_offset,
                                    int num_features, const NetworkIO& src,
                                    int src_t, int src_offset) {
  ASSERT_HOST(int_mode_ == src.int_mode_);
  if (int_mode_) {
    memcpy(i_[dest_t] + dest_offset, src.i_[src_t] + src_offset,
           num_features * sizeof(i_[0][0]));
  } else {
    memcpy(f_[dest_t] + dest_offset, src.f_[src_t] + src_offset,
           num_features * sizeof(f_[0][0]));
  }
}

// Zeroes a single time step.
void NetworkIO::ZeroTimeStepGeneral(int t, int offset, int num_features) {
  if (int_mode_) {
    ZeroVector(num_features, i_[t] + offset);
  } else {
    ZeroVector(num_features, f_[t] + offset);
  }
}

// Sets the given range to random values.
void NetworkIO::Randomize(int t, int offset, int num_features,
                          TRand* randomizer) {
  if (int_mode_) {
    int8_t* line = i_[t] + offset;
    for (int i = 0; i < num_features; ++i)
      line[i] = IntCastRounded(randomizer->SignedRand(INT8_MAX));
  } else {
    // float mode.
    float* line = f_[t] + offset;
    for (int i = 0; i < num_features; ++i)
      line[i] = randomizer->SignedRand(1.0);
  }
}

// Helper returns the label and score of the best choice over a range.
int NetworkIO::BestChoiceOverRange(int t_start, int t_end, int not_this,
                                   int null_ch, float* rating,
                                   float* certainty) const {
  if (t_end <= t_start) return -1;
  int max_char = -1;
  float min_score = 0.0f;
  for (int c = 0; c < NumFeatures(); ++c) {
    if (c == not_this || c == null_ch) continue;
    ScoresOverRange(t_start, t_end, c, null_ch, rating, certainty);
    if (max_char < 0 || *rating < min_score) {
      min_score = *rating;
      max_char = c;
    }
  }
  ScoresOverRange(t_start, t_end, max_char, null_ch, rating, certainty);
  return max_char;
}

// Helper returns the rating and certainty of the choice over a range in output.
void NetworkIO::ScoresOverRange(int t_start, int t_end, int choice, int null_ch,
                                float* rating, float* certainty) const {
  ASSERT_HOST(!int_mode_);
  *rating = 0.0f;
  *certainty = 0.0f;
  if (t_end <= t_start || t_end <= 0) return;
  float ratings[3] = {0.0f, 0.0f, 0.0f};
  float certs[3] = {0.0f, 0.0f, 0.0f};
  for (int t = t_start; t < t_end; ++t) {
    const float* line = f_[t];
    float score = ProbToCertainty(line[choice]);
    float zero = ProbToCertainty(line[null_ch]);
    if (t == t_start) {
      ratings[2] = FLT_MAX;
      ratings[1] = -score;
      certs[1] = score;
    } else {
      for (int i = 2; i >= 1; --i) {
        if (ratings[i] > ratings[i - 1]) {
          ratings[i] = ratings[i - 1];
          certs[i] = certs[i - 1];
        }
      }
      ratings[2] -= zero;
      if (zero < certs[2]) certs[2] = zero;
      ratings[1] -= score;
      if (score < certs[1]) certs[1] = score;
    }
    ratings[0] -= zero;
    if (zero < certs[0]) certs[0] = zero;
  }
  int best_i = ratings[2] < ratings[1] ? 2 : 1;
  *rating = ratings[best_i] + t_end - t_start;
  *certainty = certs[best_i];
}

// Returns the index (label) of the best value at the given timestep,
// excluding not_this and not_that, and if not null, sets the score to the
// log of the corresponding value.
int NetworkIO::BestLabel(int t, int not_this, int not_that,
                         float* score) const {
  ASSERT_HOST(!int_mode_);
  int best_index = -1;
  float best_score = -FLT_MAX;
  const float* line = f_[t];
  for (int i = 0; i < f_.dim2(); ++i) {
    if (line[i] > best_score && i != not_this && i != not_that) {
      best_score = line[i];
      best_index = i;
    }
  }
  if (score != nullptr) *score = ProbToCertainty(best_score);
  return best_index;
}

// Returns the best start position out of [start, end) (into which all labels
// must fit) to obtain the highest cumulative score for the given labels.
int NetworkIO::PositionOfBestMatch(const GenericVector<int>& labels, int start,
                                   int end) const {
  int length = labels.size();
  int last_start = end - length;
  int best_start = -1;
  double best_score = 0.0;
  for (int s = start; s <= last_start; ++s) {
    double score = ScoreOfLabels(labels, s);
    if (score > best_score || best_start < 0) {
      best_score = score;
      best_start = s;
    }
  }
  return best_start;
}

// Returns the cumulative score of the given labels starting at start, and
// using one label per time-step.
double NetworkIO::ScoreOfLabels(const GenericVector<int>& labels,
                                int start) const {
  int length = labels.size();
  double score = 0.0;
  for (int i = 0; i < length; ++i) {
    score += f_(start + i, labels[i]);
  }
  return score;
}

// Helper function sets all the outputs for a single timestep, such that
// label has value ok_score, and the other labels share 1 - ok_score.
void NetworkIO::SetActivations(int t, int label, float ok_score) {
  ASSERT_HOST(!int_mode_);
  int num_classes = NumFeatures();
  float bad_score = (1.0f - ok_score) / (num_classes - 1);
  float* targets = f_[t];
  for (int i = 0; i < num_classes; ++i)
    targets[i] = bad_score;
  targets[label] = ok_score;
}

// Modifies the values, only if needed, so that the given label is
// the winner at the given time step t.
void NetworkIO::EnsureBestLabel(int t, int label) {
  ASSERT_HOST(!int_mode_);
  if (BestLabel(t, nullptr) != label) {
    // Output value needs enhancing. Third all the other elements and add the
    // remainder to best_label.
    int num_classes = NumFeatures();
    float* targets = f_[t];
    for (int c = 0; c < num_classes; ++c) {
      if (c == label) {
        targets[c] += (1.0 - targets[c]) * (2 / 3.0);
      } else {
        targets[c] /= 3.0;
      }
    }
  }
}

// Helper function converts prob to certainty taking the minimum into account.
/* static */
float NetworkIO::ProbToCertainty(float prob) {
  return prob > kMinProb ? log(prob) : kMinCertainty;
}

// Returns true if there is any bad value that is suspiciously like a GT
// error. Assuming that *this is the difference(gradient) between target
// and forward output, returns true if there is a large negative value
// (correcting a very confident output) for which there is no corresponding
// positive value in an adjacent timestep for the same feature index. This
// allows the box-truthed samples to make fine adjustments to position while
// stopping other disagreements of confident output with ground truth.
bool NetworkIO::AnySuspiciousTruth(float confidence_thr) const {
  int num_features = NumFeatures();
  for (int t = 0; t < Width(); ++t) {
    const float* features = f_[t];
    for (int y = 0; y < num_features; ++y) {
      float grad = features[y];
      if (grad < -confidence_thr) {
        // Correcting strong output. Check for movement.
        if ((t == 0 || f_[t - 1][y] < confidence_thr / 2) &&
            (t + 1 == Width() || f_[t + 1][y] < confidence_thr / 2)) {
          return true;  // No strong positive on either side.
        }
      }
    }
  }
  return false;
}

// Reads a single timestep to floats in the range [-1, 1].
void NetworkIO::ReadTimeStep(int t, double* output) const {
  if (int_mode_) {
    const int8_t* line = i_[t];
    for (int i = 0; i < i_.dim2(); ++i) {
      output[i] = static_cast<double>(line[i]) / INT8_MAX;
    }
  } else {
    const float* line = f_[t];
    for (int i = 0; i < f_.dim2(); ++i) {
      output[i] = static_cast<double>(line[i]);
    }
  }
}

// Adds a single timestep to floats.
void NetworkIO::AddTimeStep(int t, double* inout) const {
  int num_features = NumFeatures();
  if (int_mode_) {
    const int8_t* line = i_[t];
    for (int i = 0; i < num_features; ++i) {
      inout[i] += static_cast<double>(line[i]) / INT8_MAX;
    }
  } else {
    const float* line = f_[t];
    for (int i = 0; i < num_features; ++i) {
      inout[i] += line[i];
    }
  }
}

// Adds part of a single timestep to floats.
void NetworkIO::AddTimeStepPart(int t, int offset, int num_features,
                                float* inout) const {
  if (int_mode_) {
    const int8_t* line = i_[t] + offset;
    for (int i = 0; i < num_features; ++i) {
      inout[i] += static_cast<float>(line[i]) / INT8_MAX;
    }
  } else {
    const float* line = f_[t] + offset;
    for (int i = 0; i < num_features; ++i) {
      inout[i] += line[i];
    }
  }
}

// Writes a single timestep from floats in the range [-1, 1].
void NetworkIO::WriteTimeStep(int t, const double* input) {
  WriteTimeStepPart(t, 0, NumFeatures(), input);
}

// Writes a single timestep from floats in the range [-1, 1] writing only
// num_features elements of input to (*this)[t], starting at offset.
void NetworkIO::WriteTimeStepPart(int t, int offset, int num_features,
                                  const double* input) {
  if (int_mode_) {
    int8_t* line = i_[t] + offset;
    for (int i = 0; i < num_features; ++i) {
      line[i] = ClipToRange<int>(IntCastRounded(input[i] * INT8_MAX),
                                 -INT8_MAX, INT8_MAX);
    }
  } else {
    float* line = f_[t] + offset;
    for (int i = 0; i < num_features; ++i) {
      line[i] = static_cast<float>(input[i]);
    }
  }
}

// Maxpools a single time step from src.
void NetworkIO::MaxpoolTimeStep(int dest_t, const NetworkIO& src, int src_t,
                                int* max_line) {
  ASSERT_HOST(int_mode_ == src.int_mode_);
  if (int_mode_) {
    int dim = i_.dim2();
    int8_t* dest_line = i_[dest_t];
    const int8_t* src_line = src.i_[src_t];
    for (int i = 0; i < dim; ++i) {
      if (dest_line[i] < src_line[i]) {
        dest_line[i] = src_line[i];
        max_line[i] = src_t;
      }
    }
  } else {
    int dim = f_.dim2();
    float* dest_line = f_[dest_t];
    const float* src_line = src.f_[src_t];
    for (int i = 0; i < dim; ++i) {
      if (dest_line[i] < src_line[i]) {
        dest_line[i] = src_line[i];
        max_line[i] = src_t;
      }
    }
  }
}

// Runs maxpool backward, using maxes to index timesteps in *this.
void NetworkIO::MaxpoolBackward(const NetworkIO& fwd,
                                const GENERIC_2D_ARRAY<int>& maxes) {
  ASSERT_HOST(!int_mode_);
  Zero();
  StrideMap::Index index(fwd.stride_map_);
  do {
    int t = index.t();
    const int* max_line = maxes[t];
    const float* fwd_line = fwd.f_[t];
    int num_features = fwd.f_.dim2();
    for (int i = 0; i < num_features; ++i) {
      f_[max_line[i]][i] = fwd_line[i];
    }
  } while (index.Increment());
}

// Returns the min over time of the maxes over features of the outputs.
float NetworkIO::MinOfMaxes() const {
  float min_max = 0.0f;
  int width = Width();
  int num_features = NumFeatures();
  for (int t = 0; t < width; ++t) {
    float max_value = -FLT_MAX;
    if (int_mode_) {
      const int8_t* column = i_[t];
      for (int i = 0; i < num_features; ++i) {
        if (column[i] > max_value) max_value = column[i];
      }
    } else {
      const float* column = f_[t];
      for (int i = 0; i < num_features; ++i) {
        if (column[i] > max_value) max_value = column[i];
      }
    }
    if (t == 0 || max_value < min_max) min_max = max_value;
  }
  return min_max;
}

// Computes combined results for a combiner that chooses between an existing
// input and itself, with an additional output to indicate the choice.
void NetworkIO::CombineOutputs(const NetworkIO& base_output,
                               const NetworkIO& combiner_output) {
  int no = base_output.NumFeatures();
  ASSERT_HOST(combiner_output.NumFeatures() == no + 1);
  Resize(base_output, no);
  int width = Width();
  if (int_mode_) {
    // Number of outputs from base and final result.
    for (int t = 0; t < width; ++t) {
      int8_t* out_line = i_[t];
      const int8_t* base_line = base_output.i_[t];
      const int8_t* comb_line = combiner_output.i_[t];
      float base_weight = static_cast<float>(comb_line[no]) / INT8_MAX;
      float boost_weight = 1.0f - base_weight;
      for (int i = 0; i < no; ++i) {
        out_line[i] = IntCastRounded(base_line[i] * base_weight +
                                     comb_line[i] * boost_weight);
      }
    }
  } else {
    for (int t = 0; t < width; ++t) {
      float* out_line = f_[t];
      const float* base_line = base_output.f_[t];
      const float* comb_line = combiner_output.f_[t];
      float base_weight = comb_line[no];
      float boost_weight = 1.0f - base_weight;
      for (int i = 0; i < no; ++i) {
        out_line[i] = base_line[i] * base_weight + comb_line[i] * boost_weight;
      }
    }
  }
}

// Computes deltas for a combiner that chooses between 2 sets of inputs.
void NetworkIO::ComputeCombinerDeltas(const NetworkIO& fwd_deltas,
                                      const NetworkIO& base_output) {
  ASSERT_HOST(!int_mode_);
  // Compute the deltas for the combiner.
  int width = Width();
  int no = NumFeatures() - 1;
  ASSERT_HOST(fwd_deltas.NumFeatures() == no);
  ASSERT_HOST(base_output.NumFeatures() == no);
  // Number of outputs from base and final result.
  for (int t = 0; t < width; ++t) {
    const float* delta_line = fwd_deltas.f_[t];
    const float* base_line = base_output.f_[t];
    float* comb_line = f_[t];
    float base_weight = comb_line[no];
    float boost_weight = 1.0f - base_weight;
    float max_base_delta = 0.0;
    for (int i = 0; i < no; ++i) {
      // What did the combiner actually produce?
      float output = base_line[i] * base_weight + comb_line[i] * boost_weight;
      // Reconstruct the target from the delta.
      float comb_target = delta_line[i] + output;
      comb_line[i] = comb_target - comb_line[i];
      float base_delta = fabs(comb_target - base_line[i]);
      if (base_delta > max_base_delta) max_base_delta = base_delta;
    }
    if (max_base_delta >= 0.5) {
      // The base network got it wrong. The combiner should output the right
      // answer and 0 for the base network.
      comb_line[no] = 0.0 - base_weight;
    } else {
      // The base network was right. The combiner should flag that.
      for (int i = 0; i < no; ++i) {
        // All other targets are 0.
        if (comb_line[i] > 0.0) comb_line[i] -= 1.0;
      }
      comb_line[no] = 1.0 - base_weight;
    }
  }
}

// Copies the array checking that the types match.
void NetworkIO::CopyAll(const NetworkIO& src) {
  ASSERT_HOST(src.int_mode_ == int_mode_);
  f_ = src.f_;
}

// Checks that both are floats and adds the src array to *this.
void NetworkIO::AddAllToFloat(const NetworkIO& src) {
  ASSERT_HOST(!int_mode_);
  ASSERT_HOST(!src.int_mode_);
  f_ += src.f_;
}

// Subtracts the array from a float array. src must also be float.
void NetworkIO::SubtractAllFromFloat(const NetworkIO& src) {
  ASSERT_HOST(!int_mode_);
  ASSERT_HOST(!src.int_mode_);
  f_ -= src.f_;
}

// Copies src to *this, with maxabs normalization to match scale.
void NetworkIO::CopyWithNormalization(const NetworkIO& src,
                                      const NetworkIO& scale) {
  ASSERT_HOST(!int_mode_);
  ASSERT_HOST(!src.int_mode_);
  ASSERT_HOST(!scale.int_mode_);
  float src_max = src.f_.MaxAbs();
  ASSERT_HOST(std::isfinite(src_max));
  float scale_max = scale.f_.MaxAbs();
  ASSERT_HOST(std::isfinite(scale_max));
  if (src_max > 0.0f) {
    float factor = scale_max / src_max;
    for (int t = 0; t < src.Width(); ++t) {
      const float* src_ptr = src.f_[t];
      float* dest_ptr = f_[t];
      for (int i = 0; i < src.f_.dim2(); ++i) dest_ptr[i] = src_ptr[i] * factor;
    }
  } else {
    f_.Clear();
  }
}

// Copies src to *this with independent reversal of the y dimension.
void NetworkIO::CopyWithYReversal(const NetworkIO& src) {
  int num_features = src.NumFeatures();
  Resize(src, num_features);
  StrideMap::Index b_index(src.stride_map_);
  do {
    int width = b_index.MaxIndexOfDim(FD_WIDTH) + 1;
    StrideMap::Index fwd_index(b_index);
    StrideMap::Index rev_index(b_index);
    rev_index.AddOffset(rev_index.MaxIndexOfDim(FD_HEIGHT), FD_HEIGHT);
    do {
      int fwd_t = fwd_index.t();
      int rev_t = rev_index.t();
      for (int x = 0; x < width; ++x) CopyTimeStepFrom(rev_t++, src, fwd_t++);
    } while (fwd_index.AddOffset(1, FD_HEIGHT) &&
             rev_index.AddOffset(-1, FD_HEIGHT));
  } while (b_index.AddOffset(1, FD_BATCH));
}

// Copies src to *this with independent reversal of the x dimension.
void NetworkIO::CopyWithXReversal(const NetworkIO& src) {
  int num_features = src.NumFeatures();
  Resize(src, num_features);
  StrideMap::Index b_index(src.stride_map_);
  do {
    StrideMap::Index y_index(b_index);
    do {
      StrideMap::Index fwd_index(y_index);
      StrideMap::Index rev_index(y_index);
      rev_index.AddOffset(rev_index.MaxIndexOfDim(FD_WIDTH), FD_WIDTH);
      do {
        CopyTimeStepFrom(rev_index.t(), src, fwd_index.t());
      } while (fwd_index.AddOffset(1, FD_WIDTH) &&
               rev_index.AddOffset(-1, FD_WIDTH));
    } while (y_index.AddOffset(1, FD_HEIGHT));
  } while (b_index.AddOffset(1, FD_BATCH));
}

// Copies src to *this with independent transpose of the x and y dimensions.
void NetworkIO::CopyWithXYTranspose(const NetworkIO& src) {
  int num_features = src.NumFeatures();
  stride_map_ = src.stride_map_;
  stride_map_.TransposeXY();
  ResizeToMap(src.int_mode(), stride_map_, num_features);
  StrideMap::Index src_b_index(src.stride_map_);
  StrideMap::Index dest_b_index(stride_map_);
  do {
    StrideMap::Index src_y_index(src_b_index);
    StrideMap::Index dest_x_index(dest_b_index);
    do {
      StrideMap::Index src_x_index(src_y_index);
      StrideMap::Index dest_y_index(dest_x_index);
      do {
        CopyTimeStepFrom(dest_y_index.t(), src, src_x_index.t());
      } while (src_x_index.AddOffset(1, FD_WIDTH) &&
               dest_y_index.AddOffset(1, FD_HEIGHT));
    } while (src_y_index.AddOffset(1, FD_HEIGHT) &&
             dest_x_index.AddOffset(1, FD_WIDTH));
  } while (src_b_index.AddOffset(1, FD_BATCH) &&
           dest_b_index.AddOffset(1, FD_BATCH));
}

// Copies src to *this, at the given feature_offset, returning the total
// feature offset after the copy. Multiple calls will stack outputs from
// multiple sources in feature space.
int NetworkIO::CopyPacking(const NetworkIO& src, int feature_offset) {
  ASSERT_HOST(int_mode_ == src.int_mode_);
  int width = src.Width();
  ASSERT_HOST(width <= Width());
  int num_features = src.NumFeatures();
  ASSERT_HOST(num_features + feature_offset <= NumFeatures());
  if (int_mode_) {
    for (int t = 0; t < width; ++t) {
      memcpy(i_[t] + feature_offset, src.i_[t],
             num_features * sizeof(i_[t][0]));
    }
    for (int t = width; t < i_.dim1(); ++t) {
      memset(i_[t], 0, num_features * sizeof(i_[t][0]));
    }
  } else {
    for (int t = 0; t < width; ++t) {
      memcpy(f_[t] + feature_offset, src.f_[t],
             num_features * sizeof(f_[t][0]));
    }
    for (int t = width; t < f_.dim1(); ++t) {
      memset(f_[t], 0, num_features * sizeof(f_[t][0]));
    }
  }
  return num_features + feature_offset;
}

// Opposite of CopyPacking, fills *this with a part of src, starting at
// feature_offset, and picking num_features.
void NetworkIO::CopyUnpacking(const NetworkIO& src, int feature_offset,
                              int num_features) {
  Resize(src, num_features);
  int width = src.Width();
  ASSERT_HOST(num_features + feature_offset <= src.NumFeatures());
  if (int_mode_) {
    for (int t = 0; t < width; ++t) {
      memcpy(i_[t], src.i_[t] + feature_offset,
             num_features * sizeof(i_[t][0]));
    }
  } else {
    for (int t = 0; t < width; ++t) {
      memcpy(f_[t], src.f_[t] + feature_offset,
             num_features * sizeof(f_[t][0]));
    }
  }
}

// Transposes the float part of *this into dest.
void NetworkIO::Transpose(TransposedArray* dest) const {
  int width = Width();
  dest->ResizeNoInit(NumFeatures(), width);
  for (int t = 0; t < width; ++t) dest->WriteStrided(t, f_[t]);
}

// Clips the content of a single time-step to +/-range.
void NetworkIO::ClipVector(int t, float range) {
  ASSERT_HOST(!int_mode_);
  float* v = f_[t];
  int dim = f_.dim2();
  for (int i = 0; i < dim; ++i)
    v[i] = ClipToRange<float>(v[i], -range, range);
}

// Returns the padding required for the given number of features in order
// for the SIMD operations to be safe.
/* static */
int NetworkIO::GetPadding(int num_features) {
  int padding = 0;
  if (IntSimdMatrix::intSimdMatrix) {
    padding =
      IntSimdMatrix::intSimdMatrix->RoundInputs(num_features) - num_features;
  }
  return padding;
}

}  // namespace tesseract.
