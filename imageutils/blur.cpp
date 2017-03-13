/*
 * Copyright 2011, Google Inc.
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

// Author: Xiaotao Duan
//
// This library contains image processing method to detect
// image blurriness.
//
// This library is *not* thread safe because static memory is
// used for performance.
//
// A method to detect whether a given image is blurred or not.
// The algorithm is based on H. Tong, M. Li, H. Zhang, J. He,
// and C. Zhang. "Blur detection for digital images using wavelet
// transform".
//
// To achieve better performance on client side, the method
// is running on four 128x128 portions which compose the 256x256
// central area of the given image. On Nexus One, average time
// to process a single image is ~5 milliseconds.

#include <math.h>

#include "blur.h"
#include "utils.h"

static const int kDecomposition = 3;
static const int kThreshold = 35;
static const float kMinZero = 0.05;

static const int kMaximumWidth = 256;
static const int kMaximumHeight = 256;

static int32 _smatrix[kMaximumWidth * kMaximumHeight];
static int32 _arow[kMaximumWidth > kMaximumHeight ?
    kMaximumWidth : kMaximumHeight];

// Does Haar Wavelet Transformation in place on a given row of a matrix.
// The matrix is in size of matrix_height * matrix_width and represented
// in a linear array. Parameter offset_row indicates transformation is
// performed on which row. offset_column and num_columns indicate column
// range of the given row.
inline void Haar1DX(int* matrix, int matrix_height, int matrix_width,
    int offset_row, int offset_column, int num_columns) {
  int32* ptr_a = _arow;
  int32* ptr_matrix = matrix + offset_row * matrix_width + offset_column;
  int half_num_columns = num_columns / 2;

  int32* a_tmp = ptr_a;
  int32* matrix_tmp = ptr_matrix;
  for (int j = 0; j < half_num_columns; ++j) {
    *a_tmp++ = (matrix_tmp[0] + matrix_tmp[1]) / 2;
    matrix_tmp += 2;
  }

  int32* average = ptr_a;
  a_tmp = ptr_a + half_num_columns;
  matrix_tmp = ptr_matrix;
  for (int j = 0; j < half_num_columns; ++j) {
    *a_tmp++ = *matrix_tmp - *average++;
    matrix_tmp += 2;
  }

  memcpy(ptr_matrix, ptr_a, sizeof(int32) * num_columns);
}

// Does Haar Wavelet Transformation in place on a given column of a matrix.
inline void Haar1DY(int* matrix, int matrix_height, int matrix_width,
    int offset_column, int offset_row, int num_rows) {
  int32* ptr_a = _arow;
  int32* ptr_matrix = matrix + offset_row * matrix_width + offset_column;
  int half_num_rows = num_rows / 2;
  int two_line_width = matrix_width * 2;

  int32* a_tmp = ptr_a;
  int32* matrix_tmp = ptr_matrix;
  for (int j = 0; j < half_num_rows; ++j) {
    *a_tmp++ = (matrix_tmp[matrix_width] + matrix_tmp[0]) / 2;
    matrix_tmp += two_line_width;
  }

  int32* average = ptr_a;
  a_tmp = ptr_a + half_num_rows;
  matrix_tmp = ptr_matrix;
  for (int j = 0; j < num_rows; j += 2) {
    *a_tmp++ = *matrix_tmp - *average++;
    matrix_tmp += two_line_width;
  }

  for (int j = 0; j < num_rows; ++j) {
    *ptr_matrix = *ptr_a++;
    ptr_matrix += matrix_width;
  }
}

// Does Haar Wavelet Transformation in place for a specified area of
// a matrix. The matrix size is specified by matrix_width and matrix_height.
// The area on which the transformation is performed is specified by
// offset_column, num_columns, offset_row and num_rows.
void Haar2D(int* matrix, int matrix_height, int matrix_width,
    int offset_column, int num_columns, int offset_row, int num_rows) {
  for (int i = offset_row; i < offset_row + num_rows; ++i) {
    Haar1DX(matrix, matrix_height, matrix_width, i, offset_column, num_columns);
  }

  for (int i = offset_column; i < offset_column + num_columns; ++i){
    Haar1DY(matrix, matrix_height, matrix_width, i, offset_row, num_rows);
  }
}

// Reads in a given matrix, does first round HWT and outputs result
// matrix into target array. This function is used for optimization by
// avoiding a memory copy. The input matrix has height rows and width
// columns. The transformation is performed on the given area specified
// by offset_column, num_columns, offset_row, num_rows. After
// transformation, the output matrix has num_columns columns and
// num_rows rows.
void HwtFirstRound(const uint8* const data, int height, int width,
    int offset_column, int num_columns,
    int offset_row, int num_rows, int32* matrix) {
  int32* ptr_a = _arow;
  const uint8* ptr_data = data + offset_row * width + offset_column;
  int half_num_columns = num_columns / 2;
  for (int i = 0; i < num_rows; ++i) {
    int32* a_tmp = ptr_a;
    const uint8* data_tmp = ptr_data;
    for (int j = 0; j < half_num_columns; ++j) {
      *a_tmp++ = (int32) ((data_tmp[0] + data_tmp[1]) / 2);
      data_tmp += 2;
    }

    int32* average = ptr_a;
    a_tmp = ptr_a + half_num_columns;
    data_tmp = ptr_data;
    for (int j = 0; j < half_num_columns; ++j) {
      *a_tmp++ = *data_tmp - *average++;
      data_tmp += 2;
    }

    int32* ptr_matrix = matrix + i * num_columns;
    a_tmp = ptr_a;
    for (int j = 0; j < num_columns; ++j) {
      *ptr_matrix++ = *a_tmp++;
    }

    ptr_data += width;
  }

  // Column transformation does not involve input data.
  for (int i = 0; i < num_columns; ++i) {
    Haar1DY(matrix, num_rows, num_columns, i, 0, num_rows);
  }
}

// Returns the weight of a given point in a certain scale of a matrix
// after wavelet transformation.
// The point is specified by k and l which are y and x coordinate
// respectively. Parameter scale tells in which scale the weight is
// computed, must be 1, 2 or 3 which stands respectively for 1/2, 1/4
// and 1/8 of original size.
int ComputeEdgePointWeight(int* matrix, int width, int height,
    int k, int l, int scale) {
  int r = k >> scale;
  int c = l >> scale;
  int window_row = height >> scale;
  int window_column = width >> scale;

  int v_top_right = square(matrix[r * width + c + window_column]);
  int v_bot_left = square(matrix[(r + window_row) * width + c]);
  int v_bot_right =
      square(matrix[(r + window_row) * width + c + window_column]);

  int v = sqrt(v_top_right + v_bot_left + v_bot_right);
  return v;
}

// Computes point with maximum weight for a given local window for a
// given scale.
// Parameter scaled_width and scaled_height define scaled image size
// of a certain decomposition level. The window size is defined by
// window_size. Output value k and l store row (y coordinate) and
// column (x coordinate) respectively of the point with maximum weight.
// The maximum weight is returned.
int ComputeLocalMaximum(int* matrix, int width, int height,
    int scaled_width, int scaled_height,
    int top, int left, int window_size, int* k, int* l) {
  int max = -1;
  *k = top;
  *l = left;

  for (int i = 0; i < window_size; ++i) {
    for (int j = 0; j < window_size; ++j) {
      int r = top + i;
      int c = left + j;

      int v_top_right = abs(matrix[r * width + c + scaled_width]);
      int v_bot_left = abs(matrix[(r + scaled_height) * width + c]);
      int v_bot_right =
          abs(matrix[(r + scaled_height) * width + c + scaled_width]);
      int v = v_top_right + v_bot_left + v_bot_right;

      if (v > max) {
        max = v;
        *k = r;
        *l = c;
      }
    }
  }

  int r = *k;
  int c = *l;
  int v_top_right = square(matrix[r * width + c + scaled_width]);
  int v_bot_left = square(matrix[(r + scaled_height) * width + c]);
  int v_bot_right =
      square(matrix[(r + scaled_height) * width + c + scaled_width]);
  int v = sqrt(v_top_right + v_bot_left + v_bot_right);

  return v;
}

// Detects blurriness of a transformed matrix.
// Blur confidence and extent will be returned through blur_conf
// and blur_extent. 1 is returned while input matrix is blurred.
int DetectBlur(int* matrix, int width, int height,
    float* blur_conf, float* blur_extent) {
  int nedge = 0;
  int nda = 0;
  int nrg = 0;
  int nbrg = 0;

  // For each scale
  for (int current_scale = kDecomposition; current_scale > 0; --current_scale) {
    int scaled_width = width >> current_scale;
    int scaled_height = height >> current_scale;
    int window_size = 16 >> current_scale;  // 2, 4, 8
    // For each window
    for (int r = 0; r + window_size < scaled_height; r += window_size) {
      for (int c = 0; c + window_size < scaled_width; c += window_size) {
        int k, l;
        int emax = ComputeLocalMaximum(matrix, width, height,
            scaled_width, scaled_height, r, c, window_size, &k, &l);
        if (emax > kThreshold) {
          int emax1, emax2, emax3;
          switch (current_scale) {
            case 1:
              emax1 = emax;
              emax2 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 2);
              emax3 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 3);
              break;
            case 2:
              emax1 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 1);
              emax2 = emax;
              emax3 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 3);
              break;
            case 3:
              emax1 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 1);
              emax2 = ComputeEdgePointWeight(matrix, width, height,
                  k << current_scale, l << current_scale, 2);
              emax3 = emax;
              break;
          }

          nedge++;
          if (emax1 > emax2 && emax2 > emax3) {
            nda++;
          }
          if (emax1 < emax2 && emax2 < emax3) {
            nrg++;
            if (emax1 < kThreshold) {
              nbrg++;
            }
          }
          if (emax2 > emax1 && emax2 > emax3) {
            nrg++;
            if (emax1 < kThreshold) {
              nbrg++;
            }
          }
        }
      }
    }
  }

  // TODO(xiaotao): No edge point at all, blurred or not?
  float per = nedge == 0 ? 0 : (float)nda / nedge;

  *blur_conf = per;
  *blur_extent = (float)nbrg / nrg;

  return per < kMinZero;
}

// Detects blurriness of a given portion of a luminance matrix.
int IsBlurredInner(const uint8* const luminance,
    const int width, const int height,
    const int left, const int top,
    const int width_wanted, const int height_wanted,
    float* const blur, float* const extent) {
  int32* matrix = _smatrix;

  HwtFirstRound(luminance, height, width,
                left, width_wanted, top, height_wanted, matrix);
  Haar2D(matrix, height_wanted, width_wanted,
         0, width_wanted >> 1, 0, height_wanted >> 1);
  Haar2D(matrix, height_wanted, width_wanted,
         0, width_wanted >> 2, 0, height_wanted >> 2);

  int blurred = DetectBlur(matrix, width_wanted, height_wanted, blur, extent);

  return blurred;
}

int IsBlurred(const uint8* const luminance,
    const int width, const int height, float* const blur, float* const extent) {

  int desired_width = min(kMaximumWidth, width);
  int desired_height = min(kMaximumHeight, height);
  int left = (width - desired_width) >> 1;
  int top = (height - desired_height) >> 1;

  float conf1, extent1;
  int blur1 = IsBlurredInner(luminance, width, height,
      left, top, desired_width >> 1, desired_height >> 1, &conf1, &extent1);
  float conf2, extent2;
  int blur2 = IsBlurredInner(luminance, width, height,
      left + (desired_width >> 1), top, desired_width >> 1, desired_height >> 1,
      &conf2, &extent2);
  float conf3, extent3;
  int blur3 = IsBlurredInner(luminance, width, height,
      left, top + (desired_height >> 1), desired_width >> 1,
      desired_height >> 1, &conf3, &extent3);
  float conf4, extent4;
  int blur4 = IsBlurredInner(luminance, width, height,
      left + (desired_width >> 1), top + (desired_height >> 1),
      desired_width >> 1, desired_height >> 1, &conf4, &extent4);

  *blur = (conf1 + conf2 + conf3 + conf4) / 4;
  *extent = (extent1 + extent2 + extent3 + extent4) / 4;
  return *blur < kMinZero;
}
