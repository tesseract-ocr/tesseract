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
#ifdef __UNIX__
#endif

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
int SaveFeature();
uinT8 TableLookup();
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

static uinT8 AtanTable[ATAN_TABLE_SIZE];

/**----------------------------------------------------------------------------
            Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void InitIntegerFX() {
  int i;

  for (i = 0; i < ATAN_TABLE_SIZE; i++)
    AtanTable[i] =
      (uinT8) (atan ((i / (float) ATAN_TABLE_SIZE)) * 128.0 / PI + 0.5);

}


/*--------------------------------------------------------------------------*/
int ExtractIntFeat(TBLOB *Blob,
                   INT_FEATURE_ARRAY BLFeat,
                   INT_FEATURE_ARRAY CNFeat,
                   INT_FX_RESULT Results) {

  TESSLINE *OutLine;
  EDGEPT *Loop, *LoopStart, *Segment;
  inT16 LastX, LastY, Xmean, Ymean;
  inT32 NormX, NormY, DeltaX, DeltaY;
  inT32 Xsum, Ysum;
  uinT32 Ix, Iy, LengthSum;
  uinT16 n;
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

  /* find Xmean, Ymean */
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
      if (!is_hidden_edge (Segment)) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt (DeltaX, DeltaY);
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

  /* extract Baseline normalized features,     */
  /* and find 2nd moments & radius of gyration */
  Ix = 0;
  Iy = 0;
  NumBLFeatures = 0;
  OutLine = Blob->outlines;
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

      n = 1;
      if (!is_hidden_edge (Segment)) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt (DeltaX, DeltaY);
        n = ((Length << 2) + Length + 32) >> 6;
        if (n != 0) {
          Theta = TableLookup (DeltaY, DeltaX);
          dX = (DeltaX << 8) / n;
          dY = (DeltaY << 8) / n;
          pfX = (LastX << 8) + (dX >> 1);
          pfY = (LastY << 8) + (dY >> 1);
          Ix += ((pfY >> 8) - Ymean) * ((pfY >> 8) - Ymean);
          Iy += (pfX >> 8) * (pfX >> 8);
          if (SaveFeature (BLFeat, NumBLFeatures, (inT16) (pfX >> 8),
            (inT16) ((pfY >> 8) - 128),
            Theta) == FALSE)
            return FALSE;
          NumBLFeatures++;
          for (i = 1; i < n; i++) {
            pfX += dX;
            pfY += dY;
            Ix += ((pfY >> 8) - Ymean) * ((pfY >> 8) - Ymean);
            Iy += (pfX >> 8) * (pfX >> 8);
            if (SaveFeature
              (BLFeat, NumBLFeatures, (inT16) (pfX >> 8),
              (inT16) ((pfY >> 8) - 128), Theta) == FALSE)
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

  /* extract character normalized features */
  NumCNFeatures = 0;
  OutLine = Blob->outlines;
  while (OutLine != NULL) {
    LoopStart = OutLine->loop;
    Loop = LoopStart;
    LastX = (Loop->pos.x - Xmean) * RyInv;
    LastY = (Loop->pos.y - Ymean) * RxInv;
    LastX >>= (inT8) RyExp;
    LastY >>= (inT8) RxExp;
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
      if (!is_hidden_edge (Segment)) {
        DeltaX = NormX - LastX;
        DeltaY = NormY - LastY;
        Length = MySqrt (DeltaX, DeltaY);
        n = ((Length << 2) + Length + 32) >> 6;
        if (n != 0) {
          Theta = TableLookup (DeltaY, DeltaX);
          dX = (DeltaX << 8) / n;
          dY = (DeltaY << 8) / n;
          pfX = (LastX << 8) + (dX >> 1);
          pfY = (LastY << 8) + (dY >> 1);
          if (SaveFeature (CNFeat, NumCNFeatures, (inT16) (pfX >> 8),
            (inT16) ((pfY >> 8)), Theta) == FALSE)
            return FALSE;
          NumCNFeatures++;
          for (i = 1; i < n; i++) {
            pfX += dX;
            pfY += dY;
            if (SaveFeature
              (CNFeat, NumCNFeatures, (inT16) (pfX >> 8),
              (inT16) ((pfY >> 8)), Theta) == FALSE)
              return FALSE;
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
uinT8 TableLookup(inT32 Y, inT32 X) {
  inT16 Angle;
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
  Angle = AtanTable[Ratio];
  if (X >= 0)
    if (Y >= 0)
      if (AbsX > AbsY)
        Angle = Angle;
  else
    Angle = 64 - Angle;
  else if (AbsX > AbsY)
    Angle = 256 - Angle;
  else
    Angle = 192 + Angle;
  else if (Y >= 0)
  if (AbsX > AbsY)
    Angle = 128 - Angle;
  else
    Angle = 64 + Angle;
  else if (AbsX > AbsY)
    Angle = 128 + Angle;
  else
    Angle = 192 - Angle;

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

  if (X > 255)
    Feature->X = 255;
  else if (X < 0)
    Feature->X = 0;
  else
    Feature->X = X;

  if (Y > 255)
    Feature->Y = 255;
  else if (Y < 0)
    Feature->Y = 0;
  else
    Feature->Y = Y;

  Feature->Theta = Theta;

  return TRUE;
}


/*---------------------------------------------------------------------------*/
uinT16 MySqrt(inT32 X, inT32 Y) {
  register uinT16 SqRoot;
  register uinT32 Square;
  register uinT16 BitLocation;
  register uinT32 Sum;

  if (X < 0)
    X = -X;
  if (Y < 0)
    Y = -Y;

  if (X > EvidenceMultMask)
    X = EvidenceMultMask;
  if (Y > EvidenceMultMask)
    Y = EvidenceMultMask;

  Sum = X * X + Y * Y;

  BitLocation = 1024;
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
