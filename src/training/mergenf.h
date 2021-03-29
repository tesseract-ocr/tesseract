/******************************************************************************
 ** Filename:   MergeNF.c
 ** Purpose:    Program for merging similar nano-feature protos
 ** Author:     Dan Johnson
 **
 ** (c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *****************************************************************************/

#ifndef TESSERACT_TRAINING_MERGENF_H_
#define TESSERACT_TRAINING_MERGENF_H_

/**----------------------------------------------------------------------------
     Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "cluster.h"
#include "ocrfeatures.h"
#include "picofeat.h"
#include "protos.h"

#define WORST_MATCH_ALLOWED (0.9)
#define WORST_EVIDENCE (1.0)
#define MAX_LENGTH_MISMATCH (2.0 * GetPicoFeatureLength())

#define PROTO_SUFFIX ".mf.p"
#define CONFIG_SUFFIX ".cl"
#define NO_PROTO (-1)
#define XPOSITION 0
#define YPOSITION 1
#define MFLENGTH 2
#define ORIENTATION 3

struct FRECT {
  float MinX, MaxX, MinY, MaxY;
};

/**----------------------------------------------------------------------------
      Public Macros
----------------------------------------------------------------------------**/
#define CenterX(M) ((M)[XPOSITION])
#define CenterY(M) ((M)[YPOSITION])
#define LengthOf(M) ((M)[MFLENGTH])
#define OrientationOf(M) ((M)[ORIENTATION])

/**----------------------------------------------------------------------------
     Public Function Prototypes
----------------------------------------------------------------------------**/
float CompareProtos(tesseract::PROTO_STRUCT *p1, tesseract::PROTO_STRUCT *p2);

void ComputeMergedProto(tesseract::PROTO_STRUCT *p1, tesseract::PROTO_STRUCT *p2, float w1, float w2,
                        tesseract::PROTO_STRUCT *MergedProto);

int FindClosestExistingProto(tesseract::CLASS_TYPE Class, int NumMerged[],
                             tesseract::PROTOTYPE *Prototype);

void MakeNewFromOld(tesseract::PROTO_STRUCT *New, tesseract::PROTOTYPE *Old);

float SubfeatureEvidence(tesseract::FEATURE Feature, tesseract::PROTO_STRUCT *Proto);

double EvidenceOf(double Similarity);

bool DummyFastMatch(tesseract::FEATURE Feature, tesseract::PROTO_STRUCT *Proto);

void ComputePaddedBoundingBox(tesseract::PROTO_STRUCT *Proto, float TangentPad, float OrthogonalPad,
                              FRECT *BoundingBox);

bool PointInside(FRECT *Rectangle, float X, float Y);

#endif // TESSERACT_TRAINING_MERGENF_H_
