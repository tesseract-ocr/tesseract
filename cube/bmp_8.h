/**********************************************************************
 * File:        bmp_8.h
 * Description: Declaration of an 8-bit Bitmap class
 * Author:    Ahmad Abdulkader
 * Created:   2007
 *
 * (C) Copyright 2008, Google Inc.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef BMP8_H
#define BMP8_H

// The Bmp8 class is an 8-bit bitmap that represents images of
// words, characters and segments throughout Cube
// It is meant to provide fast access to the bitmap bits and provide
// fast scaling, cropping, deslanting, connected components detection,
// loading and saving functionality

#include <stdlib.h>
#include <stdio.h>
#include "con_comp.h"
#include "cached_file.h"

namespace tesseract {

// Non-integral deslanting parameters.
static const float kMinDeslantAngle = -30.0f;
static const float kMaxDeslantAngle = 30.0f;
static const float kDeslantAngleDelta = 0.5f;

class Bmp8 {
 public:
  Bmp8(unsigned short wid, unsigned short hgt);
  ~Bmp8();
  // Clears the bitmap
  bool Clear();
  // accessors to bitmap dimensions
  inline unsigned short Width() const { return wid_; }
  inline unsigned short Stride() const { return stride_; }
  inline unsigned short Height() const { return hgt_; }
  inline unsigned char *RawData() const {
    return (line_buff_ == NULL ? NULL : line_buff_[0]);
  }
  // creates a scaled version of the specified bitmap
  // Optionally, scaling can be isotropic (preserving aspect ratio) or not
  bool ScaleFrom(Bmp8 *bmp, bool isotropic = true);
  // Deslant the bitmap vertically
  bool Deslant();
  // Deslant the bitmap horizontally
  bool HorizontalDeslant(double *deslant_angle);
  // Create a bitmap object from a file
  static Bmp8 *FromCharDumpFile(CachedFile *fp);
  static Bmp8 *FromCharDumpFile(FILE *fp);
  // are two bitmaps identical
  bool IsIdentical(Bmp8 *pBmp) const;
  // Detect connected components
  ConComp ** FindConComps(int *concomp_cnt, int min_size) const;
  // compute the foreground ratio
  float ForegroundRatio() const;
  // returns the mean horizontal histogram entropy of the bitmap
  float MeanHorizontalHistogramEntropy() const;
  // returns the horizontal histogram of the bitmap
  int *HorizontalHistogram() const;

 private:
  // Compute a look up tan table that will be used for fast slant computation
  static bool ComputeTanTable();
  // create a bitmap buffer (two flavors char & int) and init contents
  unsigned char ** CreateBmpBuffer(unsigned char init_val = 0xff);
  static unsigned int ** CreateBmpBuffer(int wid, int hgt,
            unsigned char init_val = 0xff);
  // Free a bitmap buffer
  static void FreeBmpBuffer(unsigned char **buff);
  static void FreeBmpBuffer(unsigned int **buff);

  // a static array that holds the tan lookup table
  static float *tan_table_;
  // bitmap 32-bit-aligned stride
  unsigned short stride_;
  // Bmp8 magic number used to validate saved bitmaps
  static const unsigned int kMagicNumber = 0xdeadbeef;

 protected:
  // bitmap dimensions
  unsigned short wid_;
  unsigned short hgt_;
  // bitmap contents
  unsigned char **line_buff_;
  // deslanting parameters
  static const int kConCompAllocChunk = 16;
  static const int kDeslantAngleCount;

  // Load dimensions & contents of bitmap from file
  bool LoadFromCharDumpFile(CachedFile *fp);
  bool LoadFromCharDumpFile(FILE *fp);
  // Load dimensions & contents of bitmap from raw data
  bool LoadFromCharDumpFile(unsigned char **raw_data);
  // Load contents of bitmap from raw data
  bool LoadFromRawData(unsigned char *data);
  // save bitmap to a file
  bool SaveBmp2CharDumpFile(FILE *fp) const;
  // checks if a row or a column are entirely blank
  bool IsBlankColumn(int x) const;
  bool IsBlankRow(int y) const;
  // crop the bitmap returning new dimensions
  void Crop(int *xst_src, int *yst_src, int *wid, int *hgt);
  // copy part of the specified bitmap
  void Copy(int x, int y, int wid, int hgt, Bmp8 *bmp_dest) const;
};
}

#endif  // BMP8_H
