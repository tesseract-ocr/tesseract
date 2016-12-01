/**********************************************************************
 * File:        bmp_8.cpp
 * Description: Implementation of an 8-bit Bitmap class
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

#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <algorithm>
#include "bmp_8.h"
#include "con_comp.h"
#include "platform.h"
#ifdef USE_STD_NAMESPACE
using std::min;
using std::max;
#endif

namespace tesseract {

const int Bmp8::kDeslantAngleCount = (1 + static_cast<int>(0.5f +
    (kMaxDeslantAngle - kMinDeslantAngle) / kDeslantAngleDelta));
float *Bmp8::tan_table_ = NULL;

Bmp8::Bmp8(unsigned short wid, unsigned short hgt)
    : wid_(wid)
    , hgt_(hgt) {
  line_buff_ = CreateBmpBuffer();
}

Bmp8::~Bmp8() {
  FreeBmpBuffer(line_buff_);
}

// free buffer
void Bmp8::FreeBmpBuffer(unsigned char **buff) {
  if (buff != NULL) {
    delete []buff[0];
    delete []buff;
  }
}

void Bmp8::FreeBmpBuffer(unsigned int **buff) {
  if (buff != NULL) {
    delete []buff[0];
    delete []buff;
  }
}

// init bmp buffers
unsigned char **Bmp8::CreateBmpBuffer(unsigned char init_val) {
  unsigned char **buff;

  // Check valid sizes
  if (!hgt_ || !wid_)
    return NULL;

  // compute stride (align on 4 byte boundries)
  stride_ = ((wid_ % 4) == 0) ? wid_ : (4 * (1 + (wid_ / 4)));

  buff = (unsigned char **) new unsigned char *[hgt_ * sizeof(*buff)];

  // alloc and init memory for buffer and line buffer
  buff[0] = (unsigned char *)
      new unsigned char[stride_ * hgt_ * sizeof(*buff[0])];

  memset(buff[0], init_val, stride_ * hgt_ * sizeof(*buff[0]));

  for (int y = 1; y < hgt_; y++) {
    buff[y] = buff[y -1] + stride_;
  }

  return buff;
}

// init bmp buffers
unsigned int ** Bmp8::CreateBmpBuffer(int wid, int hgt,
                                      unsigned char init_val) {
  unsigned int **buff;

  // compute stride (align on 4 byte boundries)
  buff = (unsigned int **) new unsigned int *[hgt * sizeof(*buff)];

  // alloc and init memory for buffer and line buffer
  buff[0] = (unsigned int *) new unsigned int[wid * hgt * sizeof(*buff[0])];

  memset(buff[0], init_val, wid * hgt * sizeof(*buff[0]));

  for (int y = 1; y < hgt; y++) {
    buff[y] = buff[y -1] + wid;
  }

  return buff;
}

// clears the contents of the bmp
bool Bmp8::Clear() {
  if (line_buff_ == NULL) {
    return false;
  }

  memset(line_buff_[0], 0xff, stride_ * hgt_ * sizeof(*line_buff_[0]));
  return true;
}

bool Bmp8::LoadFromCharDumpFile(CachedFile *fp) {
  unsigned short wid;
  unsigned short hgt;
  unsigned short x;
  unsigned short y;
  int buf_size;
  int pix;
  int pix_cnt;
  unsigned int val32;
  unsigned char *buff;

  // read and check 32 bit marker
  if (fp->Read(&val32, sizeof(val32)) != sizeof(val32)) {
    return false;
  }

  if (val32 != kMagicNumber) {
    return false;
  }

  // read wid and hgt
  if (fp->Read(&wid, sizeof(wid)) != sizeof(wid)) {
    return false;
  }

  if (fp->Read(&hgt, sizeof(hgt)) != sizeof(hgt)) {
    return false;
  }

  // read buf size
  if (fp->Read(&buf_size, sizeof(buf_size)) != sizeof(buf_size)) {
    return false;
  }

  // validate buf size: for now, only 3 channel (RBG) is supported
  pix_cnt = wid * hgt;
  if (buf_size != (3 * pix_cnt)) {
    return false;
  }

  // alloc memory & read the 3 channel buffer
  buff = new unsigned char[buf_size];

  if (fp->Read(buff, buf_size) != buf_size) {
    delete []buff;
    return false;
  }

  // create internal buffers
  wid_ = wid;
  hgt_ = hgt;

  line_buff_ = CreateBmpBuffer();
  if (line_buff_ == NULL) {
    delete []buff;
    return false;
  }

  // copy the data
  for (y = 0, pix = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++, pix += 3) {
      // for now we only support gray scale,
      // so we expect R = G = B, it this is not the case, bail out
      if  (buff[pix] != buff[pix + 1] || buff[pix] != buff[pix + 2]) {
        delete []buff;
        return false;
      }
      line_buff_[y][x] = buff[pix];
    }
  }

  // delete temp buffer
  delete[]buff;

  return true;
}

Bmp8 * Bmp8::FromCharDumpFile(CachedFile *fp) {
  // create a Bmp8 object
  Bmp8 *bmp_obj = new Bmp8(0, 0);

  if (bmp_obj->LoadFromCharDumpFile(fp) == false) {
    delete bmp_obj;
    return NULL;
  }

  return bmp_obj;
}

bool Bmp8::LoadFromCharDumpFile(FILE *fp) {
  unsigned short wid;
  unsigned short hgt;
  unsigned short x;
  unsigned short y;
  int buf_size;
  int pix;
  int pix_cnt;
  unsigned int val32;
  unsigned char *buff;

  // read and check 32 bit marker
  if (fread(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return false;
  }

  if (val32 != kMagicNumber) {
    return false;
  }

  // read wid and hgt
  if (fread(&wid, 1, sizeof(wid), fp) != sizeof(wid)) {
    return false;
  }

  if (fread(&hgt, 1, sizeof(hgt), fp) != sizeof(hgt)) {
    return false;
  }

  // read buf size
  if (fread(&buf_size, 1, sizeof(buf_size), fp) != sizeof(buf_size)) {
    return false;
  }

  // validate buf size: for now, only 3 channel (RBG) is supported
  pix_cnt = wid * hgt;
  if (buf_size != (3 * pix_cnt)) {
    return false;
  }

  // alloc memory & read the 3 channel buffer
  buff = new unsigned char[buf_size];

  if (fread(buff, 1, buf_size, fp) != buf_size) {
    delete []buff;
    return false;
  }

  // create internal buffers
  wid_ = wid;
  hgt_ = hgt;

  line_buff_ = CreateBmpBuffer();
  if (line_buff_ == NULL) {
    delete []buff;
    return false;
  }

  // copy the data
  for (y = 0, pix = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++, pix += 3) {
      // for now we only support gray scale,
      // so we expect R = G = B, it this is not the case, bail out
      if  (buff[pix] != buff[pix + 1] || buff[pix] != buff[pix + 2]) {
        delete []buff;
        return false;
      }
      line_buff_[y][x] = buff[pix];
    }
  }

  // delete temp buffer
  delete[]buff;

  return true;
}

Bmp8 * Bmp8::FromCharDumpFile(FILE *fp) {
  // create a Bmp8 object
  Bmp8 *bmp_obj = new Bmp8(0, 0);

  if (bmp_obj->LoadFromCharDumpFile(fp) == false) {
    delete bmp_obj;
    return NULL;
  }

  return bmp_obj;
}

bool Bmp8::IsBlankColumn(int x) const {
  for (int y = 0; y < hgt_; y++) {
    if (line_buff_[y][x] != 0xff) {
      return false;
    }
  }

  return true;
}

bool Bmp8::IsBlankRow(int y) const {
  for (int x = 0; x < wid_; x++) {
    if (line_buff_[y][x] != 0xff) {
      return false;
    }
  }

  return true;
}

// crop the bitmap returning new dimensions
void Bmp8::Crop(int *xst, int *yst, int *wid, int *hgt) {
  (*xst) = 0;
  (*yst) = 0;

  int xend = wid_ - 1;
  int yend = hgt_ - 1;

  while ((*xst) < (wid_ - 1) && (*xst) <= xend) {
    // column is not empty
    if (!IsBlankColumn((*xst))) {
      break;
    }
    (*xst)++;
  }

  while (xend > 0 && xend >= (*xst)) {
    // column is not empty
    if (!IsBlankColumn(xend)) {
      break;
    }
    xend--;
  }

  while ((*yst) < (hgt_ - 1) && (*yst) <= yend) {
    // column is not empty
    if (!IsBlankRow((*yst))) {
      break;
    }
    (*yst)++;
  }

  while (yend > 0 && yend >= (*yst)) {
    // column is not empty
    if (!IsBlankRow(yend)) {
      break;
    }
    yend--;
  }

  (*wid) = xend - (*xst) + 1;
  (*hgt) = yend - (*yst) + 1;
}

// generates a scaled bitmap with dimensions the new bmp will have the
// same aspect ratio and will be centered in the box
bool Bmp8::ScaleFrom(Bmp8 *bmp, bool isotropic) {
  int x_num;
  int x_denom;
  int y_num;
  int y_denom;
  int xoff;
  int yoff;
  int xsrc;
  int ysrc;
  int xdest;
  int ydest;
  int xst_src = 0;
  int yst_src = 0;
  int xend_src = bmp->wid_ - 1;
  int yend_src = bmp->hgt_ - 1;
  int wid_src;
  int hgt_src;

  // src dimensions
  wid_src = xend_src - xst_src + 1,
  hgt_src = yend_src - yst_src + 1;

  // scale to maintain aspect ratio if required
  if (isotropic) {
    if ((wid_ * hgt_src) > (hgt_ * wid_src)) {
      x_num = y_num = hgt_;
      x_denom = y_denom = hgt_src;
    } else {
      x_num = y_num = wid_;
      x_denom = y_denom = wid_src;
    }
  } else {
    x_num = wid_;
    y_num = hgt_;
    x_denom = wid_src;
    y_denom = hgt_src;
  }

  // compute offsets needed to center new bmp
  xoff = (wid_ - ((x_num * wid_src) / x_denom)) / 2;
  yoff = (hgt_ - ((y_num * hgt_src) / y_denom)) / 2;

  // scale up
  if (y_num > y_denom) {
    for (ydest = yoff; ydest < (hgt_ - yoff); ydest++) {
      // compute un-scaled y
      ysrc = static_cast<int>(0.5 + (1.0 * (ydest - yoff) *
          y_denom / y_num));
      if (ysrc < 0 || ysrc >= hgt_src) {
        continue;
      }

      for (xdest = xoff; xdest < (wid_ - xoff); xdest++) {
        // compute un-scaled y
        xsrc = static_cast<int>(0.5 + (1.0 * (xdest - xoff) *
            x_denom / x_num));
        if (xsrc < 0 || xsrc >= wid_src) {
          continue;
        }

        line_buff_[ydest][xdest] =
            bmp->line_buff_[ysrc + yst_src][xsrc + xst_src];
      }
    }
  } else {
    // or scale down
    // scaling down is a bit tricky: we'll accumulate pixels
    // and then compute the means
    unsigned int **dest_line_buff = CreateBmpBuffer(wid_, hgt_, 0),
      **dest_pix_cnt =  CreateBmpBuffer(wid_, hgt_, 0);

    for (ysrc = 0; ysrc < hgt_src; ysrc++) {
      // compute scaled y
      ydest = yoff + static_cast<int>(0.5 + (1.0 * ysrc * y_num / y_denom));
      if (ydest < 0 || ydest >= hgt_) {
        continue;
      }

      for (xsrc = 0; xsrc < wid_src; xsrc++) {
        // compute scaled y
        xdest = xoff + static_cast<int>(0.5 + (1.0 * xsrc * x_num / x_denom));
        if (xdest < 0 || xdest >= wid_) {
          continue;
        }

        dest_line_buff[ydest][xdest] +=
            bmp->line_buff_[ysrc + yst_src][xsrc + xst_src];
        dest_pix_cnt[ydest][xdest]++;
      }
    }

    for (ydest = 0; ydest < hgt_; ydest++) {
      for (xdest = 0; xdest < wid_; xdest++) {
        if (dest_pix_cnt[ydest][xdest] > 0) {
          unsigned int pixval =
              dest_line_buff[ydest][xdest] / dest_pix_cnt[ydest][xdest];

          line_buff_[ydest][xdest] =
              (unsigned char) min((unsigned int)255, pixval);
        }
      }
    }

    // we no longer need these temp buffers
    FreeBmpBuffer(dest_line_buff);
    FreeBmpBuffer(dest_pix_cnt);
  }

  return true;
}

bool Bmp8::LoadFromRawData(unsigned char *data) {
  unsigned char *pline_data = data;

  // copy the data
  for (int y = 0; y < hgt_; y++, pline_data += wid_) {
    memcpy(line_buff_[y], pline_data, wid_ * sizeof(*pline_data));
  }

  return true;
}

bool Bmp8::SaveBmp2CharDumpFile(FILE *fp) const {
  unsigned short wid;
  unsigned short hgt;
  unsigned short x;
  unsigned short y;
  int buf_size;
  int pix;
  int pix_cnt;
  unsigned int val32;
  unsigned char *buff;

  // write and check 32 bit marker
  val32 = kMagicNumber;
  if (fwrite(&val32, 1, sizeof(val32), fp) != sizeof(val32)) {
    return false;
  }

  // write wid and hgt
  wid = wid_;
  if (fwrite(&wid, 1, sizeof(wid), fp) != sizeof(wid)) {
    return false;
  }

  hgt = hgt_;
  if (fwrite(&hgt, 1, sizeof(hgt), fp) != sizeof(hgt)) {
    return false;
  }

  // write buf size
  pix_cnt = wid * hgt;
  buf_size = 3 * pix_cnt;
  if (fwrite(&buf_size, 1, sizeof(buf_size), fp) != sizeof(buf_size)) {
    return false;
  }

  // alloc memory & write the 3 channel buffer
  buff = new unsigned char[buf_size];

  // copy the data
  for (y = 0, pix = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++, pix += 3) {
      buff[pix] =
      buff[pix + 1] =
      buff[pix + 2] = line_buff_[y][x];
    }
  }

  if (fwrite(buff, 1, buf_size, fp) != buf_size) {
    delete []buff;
    return false;
  }

  // delete temp buffer
  delete[]buff;

  return true;
}

// copy part of the specified bitmap to the top of the bitmap
// does any necessary clipping
void Bmp8::Copy(int x_st, int y_st, int wid, int hgt, Bmp8 *bmp_dest) const {
  int x_end = min(x_st + wid, static_cast<int>(wid_)),
  y_end = min(y_st + hgt, static_cast<int>(hgt_));

  for (int y = y_st; y < y_end; y++) {
    for (int x = x_st; x < x_end; x++) {
      bmp_dest->line_buff_[y - y_st][x - x_st] =
          line_buff_[y][x];
    }
  }
}

bool Bmp8::IsIdentical(Bmp8 *pBmp) const {
  if (wid_ != pBmp->wid_ || hgt_ != pBmp->hgt_) {
    return false;
  }

  for (int y = 0; y < hgt_; y++) {
    if (memcmp(line_buff_[y], pBmp->line_buff_[y], wid_) != 0) {
      return false;
    }
  }

  return true;
}

// Detect connected components in the bitmap
ConComp ** Bmp8::FindConComps(int *concomp_cnt, int min_size) const {
  (*concomp_cnt) = 0;

  unsigned int **out_bmp_array = CreateBmpBuffer(wid_, hgt_, 0);
  if (out_bmp_array == NULL) {
    fprintf(stderr, "Cube ERROR (Bmp8::FindConComps): could not allocate "
            "bitmap array\n");
    return NULL;
  }

  // listed of connected components
  ConComp **concomp_array = NULL;

  int x;
  int y;
  int x_nbr;
  int y_nbr;
  int concomp_id;
  int alloc_concomp_cnt = 0;

  // neighbors to check
  const int nbr_cnt = 4;

  // relative coordinates of nbrs
  int x_del[nbr_cnt] = {-1, 0, 1, -1},
    y_del[nbr_cnt] = {-1, -1, -1, 0};


  for (y = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++) {
      // is this a foreground pix
      if (line_buff_[y][x] != 0xff) {
        int master_concomp_id = 0;
        ConComp *master_concomp = NULL;

        // checkout the nbrs
        for (int nbr = 0; nbr < nbr_cnt; nbr++) {
          x_nbr = x + x_del[nbr];
          y_nbr = y + y_del[nbr];

          if (x_nbr < 0 || y_nbr < 0 || x_nbr >= wid_ || y_nbr >= hgt_) {
            continue;
          }

          // is this nbr a foreground pix
          if (line_buff_[y_nbr][x_nbr] != 0xff) {
            // get its concomp ID
            concomp_id = out_bmp_array[y_nbr][x_nbr];

            // this should not happen
            if (concomp_id < 1 || concomp_id > alloc_concomp_cnt) {
              fprintf(stderr, "Cube ERROR (Bmp8::FindConComps): illegal "
                      "connected component id: %d\n", concomp_id);
              FreeBmpBuffer(out_bmp_array);
              delete []concomp_array;
              return NULL;
            }

            // if we has previously found a component then merge the two
            // and delete the latest one
            if (master_concomp != NULL && concomp_id != master_concomp_id) {
              // relabel all the pts
              ConCompPt *pt_ptr = concomp_array[concomp_id - 1]->Head();
              while (pt_ptr != NULL) {
                out_bmp_array[pt_ptr->y()][pt_ptr->x()] = master_concomp_id;
                pt_ptr = pt_ptr->Next();
              }

              // merge the two concomp
              if (!master_concomp->Merge(concomp_array[concomp_id - 1])) {
                fprintf(stderr, "Cube ERROR (Bmp8::FindConComps): could not "
                        "merge connected component: %d\n", concomp_id);
                FreeBmpBuffer(out_bmp_array);
                delete []concomp_array;
                return NULL;
              }

              // delete the merged concomp
              delete concomp_array[concomp_id - 1];
              concomp_array[concomp_id - 1] = NULL;
            } else {
              // this is the first concomp we encounter
              master_concomp_id = concomp_id;
              master_concomp = concomp_array[master_concomp_id - 1];

              out_bmp_array[y][x] = master_concomp_id;

              if (!master_concomp->Add(x, y)) {
                fprintf(stderr, "Cube ERROR (Bmp8::FindConComps): could not "
                        "add connected component (%d,%d)\n", x, y);
                FreeBmpBuffer(out_bmp_array);
                delete []concomp_array;
                return NULL;
              }
            }
          }  // foreground nbr
        }  // nbrs

        // if there was no foreground pix, then create a new concomp
        if (master_concomp == NULL) {
          master_concomp = new ConComp();
          if (master_concomp->Add(x, y) == false) {
            fprintf(stderr, "Cube ERROR (Bmp8::FindConComps): could not "
                    "allocate or add a connected component\n");
            FreeBmpBuffer(out_bmp_array);
            delete []concomp_array;
            return NULL;
          }

          // extend the list of concomps if needed
          if ((alloc_concomp_cnt % kConCompAllocChunk) == 0) {
            ConComp **temp_con_comp =
                new ConComp *[alloc_concomp_cnt + kConCompAllocChunk];

            if (alloc_concomp_cnt > 0) {
              memcpy(temp_con_comp, concomp_array,
                     alloc_concomp_cnt * sizeof(*concomp_array));

              delete []concomp_array;
            }

            concomp_array = temp_con_comp;
          }

          concomp_array[alloc_concomp_cnt++] = master_concomp;
          out_bmp_array[y][x] = alloc_concomp_cnt;
        }
      }  // foreground pix
    }  // x
  }  // y

  // free the concomp bmp
  FreeBmpBuffer(out_bmp_array);

  if (alloc_concomp_cnt > 0 && concomp_array != NULL) {
    // scan the array of connected components and color
    // the o/p buffer with the corresponding concomps
    (*concomp_cnt) = 0;
    ConComp *concomp = NULL;

    for (int concomp_idx = 0; concomp_idx < alloc_concomp_cnt; concomp_idx++) {
      concomp = concomp_array[concomp_idx];

      // found a concomp
      if (concomp != NULL) {
        // add the connected component if big enough
        if (concomp->PtCnt() > min_size) {
          concomp->SetLeftMost(true);
          concomp->SetRightMost(true);
          concomp->SetID((*concomp_cnt));
          concomp_array[(*concomp_cnt)++] = concomp;
        } else {
          delete concomp;
        }
      }
    }
  }

  return concomp_array;
}

// precompute the tan table to speedup deslanting
bool Bmp8::ComputeTanTable() {
  int ang_idx;
  float ang_val;

  // alloc memory for tan table
  delete []tan_table_;
  tan_table_ = new float[kDeslantAngleCount];

  for (ang_idx = 0, ang_val = kMinDeslantAngle;
       ang_idx < kDeslantAngleCount; ang_idx++) {
    tan_table_[ang_idx] = tan(ang_val * M_PI / 180.0f);
    ang_val += kDeslantAngleDelta;
  }

  return true;
}

// generates a deslanted bitmap from the passed bitmap.
bool Bmp8::Deslant() {
  int x;
  int y;
  int des_x;
  int des_y;
  int ang_idx;
  int best_ang;
  int min_des_x;
  int max_des_x;
  int des_wid;

  // only do deslanting if bitmap is wide enough
  // otherwise it slant estimate might not be reliable
  if (wid_ < (hgt_ * 2)) {
    return true;
  }

  // compute tan table if needed
  if (tan_table_ == NULL && !ComputeTanTable()) {
    return false;
  }

  // compute min and max values for x after deslant
  min_des_x = static_cast<int>(0.5f + (hgt_ - 1) * tan_table_[0]);
  max_des_x = (wid_ - 1) +
      static_cast<int>(0.5f + (hgt_ - 1) * tan_table_[kDeslantAngleCount - 1]);

  des_wid = max_des_x - min_des_x + 1;

  // alloc memory for histograms
  int **angle_hist = new int*[kDeslantAngleCount];
  for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
    angle_hist[ang_idx] = new int[des_wid];
    memset(angle_hist[ang_idx], 0, des_wid * sizeof(*angle_hist[ang_idx]));
  }

  // compute histograms
  for (y = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++) {
      // find a non-bkgrnd pixel
      if (line_buff_[y][x] != 0xff) {
        des_y = hgt_ - y - 1;
        // stamp all histograms
        for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
          des_x = x + static_cast<int>(0.5f + (des_y * tan_table_[ang_idx]));
          if (des_x >= min_des_x && des_x <= max_des_x) {
            angle_hist[ang_idx][des_x - min_des_x]++;
          }
        }
      }
    }
  }

  // find the histogram with the lowest entropy
  float entropy;
  double best_entropy = 0.0f;
  double norm_val;

  best_ang = -1;
  for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
    entropy = 0.0f;

    for (x = min_des_x; x <= max_des_x; x++) {
      if (angle_hist[ang_idx][x - min_des_x] > 0) {
        norm_val = (1.0f * angle_hist[ang_idx][x - min_des_x] / hgt_);
        entropy += (-1.0f * norm_val * log(norm_val));
      }
    }

    if (best_ang == -1 || entropy < best_entropy) {
      best_ang = ang_idx;
      best_entropy = entropy;
    }

    // free the histogram
    delete[] angle_hist[ang_idx];
  }
  delete[] angle_hist;

  // deslant
  if (best_ang != -1) {
    unsigned char **dest_lines;
    int old_wid = wid_;

    // create a new buffer
    wid_ = des_wid;
    dest_lines = CreateBmpBuffer();
    if (dest_lines == NULL) {
      return false;
    }

    for (y = 0; y < hgt_; y++) {
      for (x = 0; x < old_wid; x++) {
        // find a non-bkgrnd pixel
        if (line_buff_[y][x] != 0xff) {
          des_y = hgt_ - y - 1;
          // compute new pos
          des_x = x + static_cast<int>(0.5f + (des_y * tan_table_[best_ang]));
          dest_lines[y][des_x - min_des_x] = 0;
        }
      }
    }

    // free old buffer
    FreeBmpBuffer(line_buff_);
    line_buff_ = dest_lines;
  }
  return true;
}

// Load dimensions & contents of bitmap from raw data
bool Bmp8::LoadFromCharDumpFile(unsigned char **raw_data_ptr) {
  unsigned short wid;
  unsigned short hgt;
  unsigned short x;
  unsigned short y;
  unsigned char *raw_data = (*raw_data_ptr);
  int buf_size;
  int pix;
  unsigned int val32;

  // read and check 32 bit marker
  memcpy(&val32, raw_data, sizeof(val32));
  raw_data += sizeof(val32);

  if (val32 != kMagicNumber) {
    return false;
  }

  // read wid and hgt
  memcpy(&wid, raw_data, sizeof(wid));
  raw_data += sizeof(wid);

  memcpy(&hgt, raw_data, sizeof(hgt));
  raw_data += sizeof(hgt);

  // read buf size
  memcpy(&buf_size, raw_data, sizeof(buf_size));
  raw_data += sizeof(buf_size);

  // validate buf size: for now, only 3 channel (RBG) is supported
  if (buf_size != (3 * wid * hgt)) {
    return false;
  }

  wid_ = wid;
  hgt_ = hgt;

  line_buff_ = CreateBmpBuffer();
  if (line_buff_ == NULL) {
    return false;
  }

  // copy the data
  for (y = 0, pix = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++, pix += 3) {
      // for now we only support gray scale,
      // so we expect R = G = B, it this is not the case, bail out
      if  (raw_data[pix] != raw_data[pix + 1] ||
           raw_data[pix] != raw_data[pix + 2]) {
        return false;
      }

      line_buff_[y][x] = raw_data[pix];
    }
  }

  (*raw_data_ptr) = raw_data + buf_size;
  return true;
}

float Bmp8::ForegroundRatio() const {
  int fore_cnt = 0;

  if (wid_ == 0 || hgt_ == 0) {
    return 1.0;
  }

  for (int y = 0; y < hgt_; y++) {
    for (int x = 0; x < wid_; x++) {
      fore_cnt += (line_buff_[y][x] == 0xff ? 0 : 1);
    }
  }

  return (1.0 * (fore_cnt / hgt_) / wid_);
}

// generates a deslanted bitmap from the passed bitmap
bool Bmp8::HorizontalDeslant(double *deslant_angle) {
  int x;
  int y;
  int des_y;
  int ang_idx;
  int best_ang;
  int min_des_y;
  int max_des_y;
  int des_hgt;

  // compute tan table if necess.
  if (tan_table_ == NULL && !ComputeTanTable()) {
    return false;
  }

  // compute min and max values for x after deslant
  min_des_y = min(0, static_cast<int>((wid_ - 1) * tan_table_[0]));
  max_des_y = (hgt_ - 1) +
      max(0, static_cast<int>((wid_ - 1) * tan_table_[kDeslantAngleCount - 1]));

  des_hgt = max_des_y - min_des_y + 1;

  // alloc memory for histograms
  int **angle_hist = new int*[kDeslantAngleCount];
  for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
    angle_hist[ang_idx] = new int[des_hgt];
    memset(angle_hist[ang_idx], 0, des_hgt * sizeof(*angle_hist[ang_idx]));
  }

  // compute histograms
  for (y = 0; y < hgt_; y++) {
    for (x = 0; x < wid_; x++) {
      // find a non-bkgrnd pixel
      if (line_buff_[y][x] != 0xff) {
        // stamp all histograms
        for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
          des_y = y - static_cast<int>(x * tan_table_[ang_idx]);
          if (des_y >= min_des_y && des_y <= max_des_y) {
            angle_hist[ang_idx][des_y - min_des_y]++;
          }
        }
      }
    }
  }

  // find the histogram with the lowest entropy
  float entropy;
  float best_entropy =  0.0f;
  float norm_val;

  best_ang = -1;
  for (ang_idx = 0; ang_idx < kDeslantAngleCount; ang_idx++) {
    entropy = 0.0f;

    for (y = min_des_y; y <= max_des_y; y++) {
      if (angle_hist[ang_idx][y - min_des_y] > 0) {
        norm_val = (1.0f * angle_hist[ang_idx][y - min_des_y] / wid_);
        entropy += (-1.0f * norm_val * log(norm_val));
      }
    }

    if (best_ang == -1 || entropy < best_entropy) {
      best_ang = ang_idx;
      best_entropy = entropy;
    }

    // free the histogram
    delete[] angle_hist[ang_idx];
  }
  delete[] angle_hist;

  (*deslant_angle) = 0.0;

  // deslant
  if (best_ang != -1) {
    unsigned char **dest_lines;
    int old_hgt = hgt_;

    // create a new buffer
    min_des_y = min(0, static_cast<int>((wid_ - 1) * -tan_table_[best_ang]));
    max_des_y = (hgt_ - 1) +
        max(0, static_cast<int>((wid_ - 1) * -tan_table_[best_ang]));
    hgt_ = max_des_y - min_des_y + 1;
    dest_lines = CreateBmpBuffer();
    if (dest_lines == NULL) {
      return false;
    }

    for (y = 0; y < old_hgt; y++) {
      for (x = 0; x < wid_; x++) {
        // find a non-bkgrnd pixel
        if (line_buff_[y][x] != 0xff) {
          // compute new pos
          des_y = y - static_cast<int>((x * tan_table_[best_ang]));
          dest_lines[des_y - min_des_y][x] = 0;
        }
      }
    }

    // free old buffer
    FreeBmpBuffer(line_buff_);
    line_buff_ = dest_lines;

    (*deslant_angle) = kMinDeslantAngle + (best_ang * kDeslantAngleDelta);
  }

  return true;
}

float Bmp8::MeanHorizontalHistogramEntropy() const {
  float entropy = 0.0f;

  // compute histograms
  for (int y = 0; y < hgt_; y++) {
    int pix_cnt = 0;

    for (int x = 0; x < wid_; x++) {
      // find a non-bkgrnd pixel
      if (line_buff_[y][x] != 0xff) {
        pix_cnt++;
      }
    }

    if (pix_cnt > 0) {
      float norm_val = (1.0f * pix_cnt / wid_);
      entropy += (-1.0f * norm_val * log(norm_val));
    }
  }

  return entropy / hgt_;
}

int *Bmp8::HorizontalHistogram() const {
  int *hist = new int[hgt_];

  // compute histograms
  for (int y = 0; y < hgt_; y++) {
    hist[y] = 0;

    for (int x = 0; x < wid_; x++) {
      // find a non-bkgrnd pixel
      if (line_buff_[y][x] != 0xff) {
        hist[y]++;
      }
    }
  }

  return hist;
}

}  // namespace tesseract
