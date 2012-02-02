/******************************************************************************
 **      Filename:    intfx.c
 **      Purpose:     Integer character normalization & feature extraction
 **      Author:      Robert Moss
 **      History:     Tue May 21 15:51:57 MDT 1991, RWM, Created.
 **
 **      (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "intfx.h"
#include "intmatcher.h"
#include "const.h"
#include "helpers.h"
#include "ccutil.h"
#include "statistc.h"
#include "trainingsample.h"
#ifdef __UNIX__
#endif

using tesseract::TrainingSample;

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
int SaveFeature();
uinT8 BinaryAnglePlusPi(inT32 Y, inT32 X);
uinT8 MySqrt2();
void ClipRadius();

INT_VAR(classify_radius_gyr_min_man, 255,
        "Minimum Radius of Gyration Mantissa 0-255:        ");

INT_VAR(classify_radius_gyr_min_exp, 0,
        "Minimum Radius of Gyration Exponent 0-255:        ");

INT_VAR(classify_radius_gyr_max_man, 158,
        "Maximum Radius of Gyration Mantissa 0-255:        ");

INT_VAR(classify_radius_gyr_max_exp, 8,
        "Maximum Radius of Gyration Exponent 0-255:        ");

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
#define  ATAN_TABLE_SIZE    64

// Look up table for arc tangent containing:
//    atan(0.0) ... atan(ATAN_TABLE_SIZE - 1 / ATAN_TABLE_SIZE)
// The entries are in binary degrees where a full circle is 256 binary degrees.
static uinT8 AtanTable[ATAN_TABLE_SIZE];
// Look up table for cos and sin to turn the intfx feature angle to a vector.
// Also protected by atan_table_mutex.
static float cos_table[INT_CHAR_NORM_RANGE];
static float sin_table[INT_CHAR_NORM_RANGE];
// Guards write access to AtanTable so we dont create it more than once.
tesseract::CCUtilMutex atan_table_mutex;


/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void InitIntegerFX() {
  static bool atan_table_init = false;
  atan_table_mutex.Lock();
  if (!atan_table_init) {
    for (int i = 0; i < ATAN_TABLE_SIZE; i++) {
      AtanTable[i] =
          (uinT8) (atan ((i / (float) ATAN_TABLE_SIZE)) * 128.0 / PI + 0.5);
    }
    for (int i = 0; i < INT_CHAR_NORM_RANGE; ++i) {
      cos_table[i] = cos(i * 2 * PI / INT_CHAR_NORM_RANGE + PI);
      sin_table[i] = sin(i * 2 * PI / INT_CHAR_NORM_RANGE + PI);
    }
    atan_table_init = true;
  }
  atan_table_mutex.Unlock();
}

// Returns a vector representing the direction of a feature with the given
// theta direction in an INT_FEATURE_STRUCT.
FCOORD FeatureDirection(uinT8 theta) {
  return FCOORD(cos_table[theta], sin_table[theta]);
}

TrainingSample* GetIntFeatures(tesseract::NormalizationMode mode,
                               TBLOB *blob, const DENORM& denorm) {
  INT_FEATURE_ARRAY blfeatures;
  INT_FEATURE_ARRAY cnfeatures;
  INT_FX_RESULT_STRUCT fx_info;
  ExtractIntFeat(blob, denorm, blfeatures, cnfeatures, &fx_info, NULL);
  TrainingSample* sample = NULL;
  if (mode == tesseract::NM_CHAR_ANISOTROPIC) {
    int num_features = fx_info.NumCN;
    if (num_features > 0) {
      sample = TrainingSample::CopyFromFeatures(fx_info, cnfeatures,
                                                num_features);
    }
  } else if (mode == tesseract::NM_BASELINE) {
    int num_features = fx_info.NumBL;
    if (num_features > 0) {
      sample = TrainingSample::CopyFromFeatures(fx_info, blfeatures,
                                                num_features);
    }
  } else {
    ASSERT_HOST(!"Unsupported normalization mode!");
  }
  return sample;
}


/*--------------------------------------------------------------------------*/
// Extract a set of standard-sized features from Blobs and write them out in
// two formats: baseline normalized and character normalized.
//
// We presume the Blobs are already scaled so that x-height=128 units
//
// Standard Features:
//   We take all outline segments longer than 7 units and chop them into
//   standard-sized segments of approximately 13 = (64 / 5) units.
//   When writing these features out, we output their center and angle as
//   measured counterclockwise from the vector <-1, 0>
//
// Baseline Normalized Output:
//   We center the grapheme by aligning the x-coordinate of its centroid with
//   x=0 and subtracting 128 from the y-coordinate.
//
// Character Normalized Output:
//   We align the grapheme's centroid at the origin and scale it asymmetrically
//   in x and y so that the result is vaguely square.
//
int ExtractIntFeat(TBLOB *Blob,
                   const DENORM& denorm,
                   INT_FEATURE_ARRAY BLFeat,
                   INT_FEATURE_ARRAY CNFeat,
                   INT_FX_RESULT_STRUCT* Results,
                   inT32 *FeatureOutlineArray) {

  TESSLINE *OutLine;
  EDGEPT *Loop, *LoopStart, *Segment;
  inT16 LastX, LastY, Xmean, Ymean;
  inT32 NormX, NormY, DeltaX, DeltaY;
  inT32 Xsum, Ysum;
  uinT32 Ix, Iy, LengthSum;
  uinT16 n;
  // n - the number of features to extract from a given outline segment.
  //   We extract features from every outline segment longer than ~6 units.
  //   We chop these long segments into standard-sized features approximately
  //   13 (= 64 / 5) units in length.
  uinT8 Theta;
  uinT16 NumBLFeatures, NumCNFeatures;
  uinT8 RxInv, RyInv;            /* x.xxxxxxx  *  2^Exp  */
  uinT8 RxExp, RyExp;
                                 /* sxxxxxxxxxxxxxxxxxxxxxxx.xxxxxxxx */
  register inT32 pfX, pfY, dX, dY;
  uinT16 Length;
  register int i;

  Results->Length = 0;
  Results->Xmean = 0;
  Results->Ymean = 0;
  Results->Rx = 0;
  Results->Ry = 0;
  Results->NumBL = 0;
  Results->NumCN = 0;
  Results->YBottom = MAX_UINT8;
  Results->YTop = 0;

  // Calculate the centroid (Xmean, Ymean) for the blob.
  //   We use centroid (instead of center of bounding box or center of smallest
  //   enclosing circle) so the algorithm will not be too greatly influenced by
  //   small amounts of information at the edge of a character's bounding box.
  NumBLFeatures = 0;
  NumCNFeatures = 0;
  OutLine = Blob->outlines;
  Xsum = 0;
  Ysum = 0;
  LengthSum = 0;
  while (OutLine != NULL) {
    LoopStart = OutLine->loop;
    Loop = LoopStart;
    LastX = Loop->pos.x;
    LastY = Loop->pos.y;
    /* Check for bad loops */
    if ((Loop == NULL) || (Loop->next == NULL) || (Loop->next == LoopStart))
      return FALSE;
    do {
      Segment = Loop;
      Loop = Loop->next;
      NormX = Loop->pos.x;
      NormY = Loop->pos.y;

      n = 1;
      if (!Segment->IsHidden()) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt(DeltaX, DeltaY);
        n = ((Length << 2) + Length + 32) >> 6;
        if (n != 0) {
          Xsum += ((LastX << 1) + DeltaX) * (int) Length;
          Ysum += ((LastY << 1) + DeltaY) * (int) Length;
          LengthSum += Length;
        }
      }
      if (n != 0) {              /* Throw away a point that is too close */
        LastX = NormX;
        LastY = NormY;
      }
    }
    while (Loop != LoopStart);
    OutLine = OutLine->next;
  }
  if (LengthSum == 0)
    return FALSE;
  Xmean = (Xsum / (inT32) LengthSum) >> 1;
  Ymean = (Ysum / (inT32) LengthSum) >> 1;

  Results->Length = LengthSum;
  Results->Xmean = Xmean;
  Results->Ymean = Ymean;

  // Extract Baseline normalized features,
  // and find 2nd moments (Ix, Iy) & radius of gyration (Rx, Ry).
  //
  //   Ix = Sum y^2 dA, where:
  //     Ix: the second moment of area about the axis x
  //     dA = 1 for our standard-sized piece of outline
  //      y: the perependicular distance to the x axis
  //   Rx = sqrt(Ix / A)
  //      Note: 1 <= Rx <= height of blob / 2
  //   Ry = sqrt(Iy / A)
  //      Note: 1 <= Ry <=  width of blob / 2
  Ix = 0;
  Iy = 0;
  NumBLFeatures = 0;
  OutLine = Blob->outlines;
  int min_x = 0;
  int max_x = 0;
  while (OutLine != NULL) {
    LoopStart = OutLine->loop;
    Loop = LoopStart;
    LastX = Loop->pos.x - Xmean;
    LastY = Loop->pos.y;
    /* Check for bad loops */
    if ((Loop == NULL) || (Loop->next == NULL) || (Loop->next == LoopStart))
      return FALSE;
    do {
      Segment = Loop;
      Loop = Loop->next;
      NormX = Loop->pos.x - Xmean;
      NormY = Loop->pos.y;
      if (NormY < Results->YBottom)
        Results->YBottom = ClipToRange(NormY, 0, MAX_UINT8);
      if (NormY > Results->YTop)
        Results->YTop = ClipToRange(NormY, 0, MAX_UINT8);
      UpdateRange(NormX, &min_x, &max_x);

      n = 1;
      if (!Segment->IsHidden()) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt(DeltaX, DeltaY);
        n = ((Length << 2) + Length + 32) >> 6;
        if (n != 0) {
          Theta = BinaryAnglePlusPi(DeltaY, DeltaX);
          dX = (DeltaX << 8) / n;
          dY = (DeltaY << 8) / n;
          pfX = (LastX << 8) + (dX >> 1);
          pfY = (LastY << 8) + (dY >> 1);
          Ix += ((pfY >> 8) - Ymean) * ((pfY >> 8) - Ymean);
          // TODO(eger): Hmmm... Xmean is not necessarily 0.
          //   Figure out if we should center against Xmean for these
          //   features, and if so fix Iy & SaveFeature().
          Iy += (pfX >> 8) * (pfX >> 8);
          if (SaveFeature(BLFeat,
                          NumBLFeatures,
                          (inT16) (pfX >> 8),
                          (inT16) ((pfY >> 8) - 128),
                          Theta) == FALSE)
            return FALSE;
          NumBLFeatures++;
          for (i = 1; i < n; i++) {
            pfX += dX;
            pfY += dY;
            Ix += ((pfY >> 8) - Ymean) * ((pfY >> 8) - Ymean);
            Iy += (pfX >> 8) * (pfX >> 8);
            if (SaveFeature(BLFeat,
                            NumBLFeatures,
                            (inT16) (pfX >> 8),
                            (inT16) ((pfY >> 8) - 128),
                            Theta) == FALSE)
              return FALSE;
            NumBLFeatures++;
          }
        }
      }
      if (n != 0) {              /* Throw away a point that is too close */
        LastX = NormX;
        LastY = NormY;
      }
    }
    while (Loop != LoopStart);
    OutLine = OutLine->next;
  }
  Results->Width = max_x - min_x;
  if (Ix == 0)
    Ix = 1;
  if (Iy == 0)
    Iy = 1;
  RxInv = MySqrt2 (NumBLFeatures, Ix, &RxExp);
  RyInv = MySqrt2 (NumBLFeatures, Iy, &RyExp);
  ClipRadius(&RxInv, &RxExp, &RyInv, &RyExp);

  Results->Rx = (inT16) (51.2 / (double) RxInv * pow (2.0, (double) RxExp));
  Results->Ry = (inT16) (51.2 / (double) RyInv * pow (2.0, (double) RyExp));
  if (Results->Ry == 0) {
    /*
        This would result in features having 'nan' values.
        Since the expression is always > 0, assign a value of 1.
    */
    Results->Ry = 1;
  }
  Results->NumBL = NumBLFeatures;

  // Extract character normalized features
  //
  //   Rescale the co-ordinates to "equalize" distribution in X and Y, making
  //   all of the following unichars be sized to look similar:  , ' 1 i
  //
  //   We calculate co-ordinates relative to the centroid, and then scale them
  //   as follows (accomplishing a scale of up to 102.4 / dimension):
  //     y *= 51.2 / Rx    [ y scaled by 0.0 ... 102.4 / height of glyph  ]
  //     x *= 51.2 / Ry    [ x scaled by 0.0 ... 102.4 / width of glyph ]
  //   Although tempting to think so, this does not guarantee that our range
  //   is within [-102.4...102.4] x [-102.4...102.4] because (Xmean, Ymean)
  //   is the centroid, not the center of the bounding box.  Instead, we can
  //   only bound the result to [-204 ... 204] x [-204 ... 204]
  //
  NumCNFeatures = 0;
  OutLine = Blob->outlines;
  int OutLineIndex = -1;
  while (OutLine != NULL) {
    LoopStart = OutLine->loop;
    Loop = LoopStart;
    LastX = (Loop->pos.x - Xmean) * RyInv;
    LastY = (Loop->pos.y - Ymean) * RxInv;
    LastX >>= (inT8) RyExp;
    LastY >>= (inT8) RxExp;
    OutLineIndex++;

    /* Check for bad loops */
    if ((Loop == NULL) || (Loop->next == NULL) || (Loop->next == LoopStart))
      return FALSE;
    do {
      Segment = Loop;
      Loop = Loop->next;
      NormX = (Loop->pos.x - Xmean) * RyInv;
      NormY = (Loop->pos.y - Ymean) * RxInv;
      NormX >>= (inT8) RyExp;
      NormY >>= (inT8) RxExp;

      n = 1;
      if (!Segment->IsHidden()) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt(DeltaX, DeltaY);
        n = ((Length << 2) + Length + 32) >> 6;
        if (n != 0) {
          Theta = BinaryAnglePlusPi(DeltaY, DeltaX);
          dX = (DeltaX << 8) / n;
          dY = (DeltaY << 8) / n;
          pfX = (LastX << 8) + (dX >> 1);
          pfY = (LastY << 8) + (dY >> 1);
          if (SaveFeature(CNFeat,
                          NumCNFeatures,
                          (inT16) (pfX >> 8),
                          (inT16) (pfY >> 8),
                          Theta) == FALSE)
            return FALSE;
          if (FeatureOutlineArray) {
            FeatureOutlineArray[NumCNFeatures] = OutLineIndex;
          }
          NumCNFeatures++;
          for (i = 1; i < n; i++) {
            pfX += dX;
            pfY += dY;
            if (SaveFeature(CNFeat,
                            NumCNFeatures,
                            (inT16) (pfX >> 8),
                            (inT16) (pfY >> 8),
                            Theta) == FALSE)
              return FALSE;
            if (FeatureOutlineArray) {
              FeatureOutlineArray[NumCNFeatures] = OutLineIndex;
            }
            NumCNFeatures++;
          }
        }
      }
      if (n != 0) {              /* Throw away a point that is too close */
        LastX = NormX;
        LastY = NormY;
      }
    }
    while (Loop != LoopStart);
    OutLine = OutLine->next;
  }

  Results->NumCN = NumCNFeatures;
  return TRUE;
}


/*--------------------------------------------------------------------------*/
// Return the "binary angle" [0..255]
//    made by vector <X, Y> as measured counterclockwise from <-1, 0>
// The order of the arguments follows the convention of atan2(3)
uinT8 BinaryAnglePlusPi(inT32 Y, inT32 X) {
  inT16 Angle, Atan;
  uinT16 Ratio;
  uinT32 AbsX, AbsY;

  assert ((X != 0) || (Y != 0));
  if (X < 0)
    AbsX = -X;
  else
    AbsX = X;
  if (Y < 0)
    AbsY = -Y;
  else
    AbsY = Y;
  if (AbsX > AbsY)
    Ratio = AbsY * ATAN_TABLE_SIZE / AbsX;
  else
    Ratio = AbsX * ATAN_TABLE_SIZE / AbsY;
  if (Ratio >= ATAN_TABLE_SIZE)
    Ratio = ATAN_TABLE_SIZE - 1;
  Atan = AtanTable[Ratio];
  if (X >= 0)
    if (Y >= 0)
      if (AbsX > AbsY)
        Angle = Atan;
      else
        Angle = 64 - Atan;
    else if (AbsX > AbsY)
      Angle = 256 - Atan;
    else
      Angle = 192 + Atan;
  else if (Y >= 0)
    if (AbsX > AbsY)
      Angle = 128 - Atan;
    else
      Angle = 64 + Atan;
  else if (AbsX > AbsY)
    Angle = 128 + Atan;
  else
    Angle = 192 - Atan;

  /* reverse angles to match old feature extractor:   Angle += PI */
  Angle += 128;
  Angle &= 255;
  return (uinT8) Angle;
}


/*--------------------------------------------------------------------------*/
int SaveFeature(INT_FEATURE_ARRAY FeatureArray,
                uinT16 FeatureNum,
                inT16 X,
                inT16 Y,
                uinT8 Theta) {
  INT_FEATURE Feature;

  if (FeatureNum >= MAX_NUM_INT_FEATURES)
    return FALSE;

  Feature = &(FeatureArray[FeatureNum]);

  X = X + 128;
  Y = Y + 128;

  Feature->X = ClipToRange<inT16>(X, 0, 255);
  Feature->Y = ClipToRange<inT16>(Y, 0, 255);
  Feature->Theta = Theta;
  Feature->CP_misses = 0;

  return TRUE;
}


/*---------------------------------------------------------------------------*/
// Return floor(sqrt(min(emm, x)^2 + min(emm, y)^2))
//    where emm = EvidenceMultMask.
uinT16 MySqrt(inT32 X, inT32 Y) {
  register uinT16 SqRoot;
  register uinT32 Square;
  register uinT16 BitLocation;
  register uinT32 Sum;
  const uinT32 EvidenceMultMask =
    ((1 << IntegerMatcher::kIntEvidenceTruncBits) - 1);

  if (X < 0)
    X = -X;
  if (Y < 0)
    Y = -Y;

  if (X > EvidenceMultMask)
    X = EvidenceMultMask;
  if (Y > EvidenceMultMask)
    Y = EvidenceMultMask;

  Sum = X * X + Y * Y;

  BitLocation = (EvidenceMultMask + 1) << 1;
  SqRoot = 0;
  do {
    Square = (SqRoot | BitLocation) * (SqRoot | BitLocation);
    if (Square <= Sum)
      SqRoot |= BitLocation;
    BitLocation >>= 1;
  }
  while (BitLocation);

  return SqRoot;
}


/*--------------------------------------------------------------------------*/
// Return two integers which can be used to express the sqrt(I/N):
//   sqrt(I/N) = 51.2 * 2^(*Exp) / retval
uinT8 MySqrt2(uinT16 N, uinT32 I, uinT8 *Exp) {
  register inT8 k;
  register uinT32 N2;
  register uinT8 SqRoot;
  register uinT16 Square;
  register uinT8 BitLocation;
  register uinT16 Ratio;

  N2 = N * 41943;

  k = 9;
  while ((N2 & 0xc0000000) == 0) {
    N2 <<= 2;
    k += 1;
  }

  while ((I & 0xc0000000) == 0) {
    I <<= 2;
    k -= 1;
  }

  if (((N2 & 0x80000000) == 0) && ((I & 0x80000000) == 0)) {
    N2 <<= 1;
    I <<= 1;
  }

  N2 &= 0xffff0000;
  I >>= 14;
  Ratio = N2 / I;

  BitLocation = 128;
  SqRoot = 0;
  do {
    Square = (SqRoot | BitLocation) * (SqRoot | BitLocation);
    if (Square <= Ratio)
      SqRoot |= BitLocation;
    BitLocation >>= 1;
  }
  while (BitLocation);

  if (k < 0) {
    *Exp = 0;
    return 255;
  }
  else {
    *Exp = k;
    return SqRoot;
  }
}


/*-------------------------------------------------------------------------*/
void ClipRadius(uinT8 *RxInv, uinT8 *RxExp, uinT8 *RyInv, uinT8 *RyExp) {
  register uinT8 AM, BM, AE, BE;
  register uinT8 BitN, LastCarry;
  int RxInvLarge, RyInvSmall;

  AM = classify_radius_gyr_min_man;
  AE = classify_radius_gyr_min_exp;
  BM = *RxInv;
  BE = *RxExp;
  LastCarry = 1;
  while ((AM != 0) || (BM != 0)) {
    if (AE > BE) {
      BitN = LastCarry + (AM & 1) + 1;
      AM >>= 1;
      AE--;
    }
    else if (AE < BE) {
      BitN = LastCarry + (!(BM & 1));
      BM >>= 1;
      BE--;
    }
    else {                       /* AE == BE */
      BitN = LastCarry + (AM & 1) + (!(BM & 1));
      AM >>= 1;
      BM >>= 1;
      AE--;
      BE--;
    }
    LastCarry = (BitN & 2) > 1;
    BitN = BitN & 1;
  }
  BitN = LastCarry + 1;
  LastCarry = (BitN & 2) > 1;
  BitN = BitN & 1;

  if (BitN == 1) {
    *RxInv = classify_radius_gyr_min_man;
    *RxExp = classify_radius_gyr_min_exp;
  }

  AM = classify_radius_gyr_min_man;
  AE = classify_radius_gyr_min_exp;
  BM = *RyInv;
  BE = *RyExp;
  LastCarry = 1;
  while ((AM != 0) || (BM != 0)) {
    if (AE > BE) {
      BitN = LastCarry + (AM & 1) + 1;
      AM >>= 1;
      AE--;
    }
    else if (AE < BE) {
      BitN = LastCarry + (!(BM & 1));
      BM >>= 1;
      BE--;
    }
    else {                       /* AE == BE */
      BitN = LastCarry + (AM & 1) + (!(BM & 1));
      AM >>= 1;
      BM >>= 1;
      AE--;
      BE--;
    }
    LastCarry = (BitN & 2) > 1;
    BitN = BitN & 1;
  }
  BitN = LastCarry + 1;
  LastCarry = (BitN & 2) > 1;
  BitN = BitN & 1;

  if (BitN == 1) {
    *RyInv = classify_radius_gyr_min_man;
    *RyExp = classify_radius_gyr_min_exp;
  }

  AM = classify_radius_gyr_max_man;
  AE = classify_radius_gyr_max_exp;
  BM = *RxInv;
  BE = *RxExp;
  LastCarry = 1;
  while ((AM != 0) || (BM != 0)) {
    if (AE > BE) {
      BitN = LastCarry + (AM & 1) + 1;
      AM >>= 1;
      AE--;
    }
    else if (AE < BE) {
      BitN = LastCarry + (!(BM & 1));
      BM >>= 1;
      BE--;
    }
    else {                       /* AE == BE */
      BitN = LastCarry + (AM & 1) + (!(BM & 1));
      AM >>= 1;
      BM >>= 1;
      AE--;
      BE--;
    }
    LastCarry = (BitN & 2) > 1;
    BitN = BitN & 1;
  }
  BitN = LastCarry + 1;
  LastCarry = (BitN & 2) > 1;
  BitN = BitN & 1;

  if (BitN == 1)
    RxInvLarge = 1;
  else
    RxInvLarge = 0;

  AM = *RyInv;
  AE = *RyExp;
  BM = classify_radius_gyr_max_man;
  BE = classify_radius_gyr_max_exp;
  LastCarry = 1;
  while ((AM != 0) || (BM != 0)) {
    if (AE > BE) {
      BitN = LastCarry + (AM & 1) + 1;
      AM >>= 1;
      AE--;
    }
    else if (AE < BE) {
      BitN = LastCarry + (!(BM & 1));
      BM >>= 1;
      BE--;
    }
    else {                       /* AE == BE */
      BitN = LastCarry + (AM & 1) + (!(BM & 1));
      AM >>= 1;
      BM >>= 1;
      AE--;
      BE--;
    }
    LastCarry = (BitN & 2) > 1;
    BitN = BitN & 1;
  }
  BitN = LastCarry + 1;
  LastCarry = (BitN & 2) > 1;
  BitN = BitN & 1;

  if (BitN == 1)
    RyInvSmall = 1;
  else
    RyInvSmall = 0;

  if (RxInvLarge && RyInvSmall) {
    *RyInv = classify_radius_gyr_max_man;
    *RyExp = classify_radius_gyr_max_exp;
  }

}
