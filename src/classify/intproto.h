/******************************************************************************
 ** Filename:    intproto.h
 ** Purpose:     Definition of data structures for integer protos.
 ** Author:      Dan Johnson
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

#ifndef INTPROTO_H
#define INTPROTO_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "matchdefs.h"
#include "mfoutline.h"
#include "protos.h"
#include "scrollview.h"
#include "unicharset.h"

namespace tesseract {

class FCOORD;

/* define order of params in pruners */
#define PRUNER_X 0
#define PRUNER_Y 1
#define PRUNER_ANGLE 2

/* definition of coordinate system offsets for each table parameter */
#define ANGLE_SHIFT (0.0)
#define X_SHIFT (0.5)
#define Y_SHIFT (0.5)

#define MAX_PROTO_INDEX 24
#define BITS_PER_WERD static_cast<int>(8 * sizeof(uint32_t))
/* Script detection: increase this number to 128 */
#define MAX_NUM_CONFIGS 64
#define MAX_NUM_PROTOS 512
#define PROTOS_PER_PROTO_SET 64
#define MAX_NUM_PROTO_SETS (MAX_NUM_PROTOS / PROTOS_PER_PROTO_SET)
#define NUM_PP_PARAMS 3
#define NUM_PP_BUCKETS 64
#define NUM_CP_BUCKETS 24
#define CLASSES_PER_CP 32
#define NUM_BITS_PER_CLASS 2
#define CLASS_PRUNER_CLASS_MASK (~(~0u << NUM_BITS_PER_CLASS))
#define CLASSES_PER_CP_WERD (CLASSES_PER_CP / NUM_BITS_PER_CLASS)
#define PROTOS_PER_PP_WERD BITS_PER_WERD
#define BITS_PER_CP_VECTOR (CLASSES_PER_CP * NUM_BITS_PER_CLASS)
#define MAX_NUM_CLASS_PRUNERS ((MAX_NUM_CLASSES + CLASSES_PER_CP - 1) / CLASSES_PER_CP)
#define WERDS_PER_CP_VECTOR (BITS_PER_CP_VECTOR / BITS_PER_WERD)
#define WERDS_PER_PP_VECTOR ((PROTOS_PER_PROTO_SET + BITS_PER_WERD - 1) / BITS_PER_WERD)
#define WERDS_PER_PP (NUM_PP_PARAMS * NUM_PP_BUCKETS * WERDS_PER_PP_VECTOR)
#define WERDS_PER_CP (NUM_CP_BUCKETS * NUM_CP_BUCKETS * NUM_CP_BUCKETS * WERDS_PER_CP_VECTOR)
#define WERDS_PER_CONFIG_VEC ((MAX_NUM_CONFIGS + BITS_PER_WERD - 1) / BITS_PER_WERD)

/* The first 3 dimensions of the CLASS_PRUNER_STRUCT are the
 * 3 axes of the quantized feature space.
 * The position of the bits recorded for each class in the
 * 4th dimension is determined by using CPrunerWordIndexFor(c),
 * where c is the corresponding class id. */
struct CLASS_PRUNER_STRUCT {
  uint32_t p[NUM_CP_BUCKETS][NUM_CP_BUCKETS][NUM_CP_BUCKETS][WERDS_PER_CP_VECTOR];
};

struct INT_PROTO_STRUCT {
  int8_t A;
  uint8_t B;
  int8_t C;
  uint8_t Angle;
  uint32_t Configs[WERDS_PER_CONFIG_VEC];
};

typedef uint32_t PROTO_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR];

struct PROTO_SET_STRUCT {
  PROTO_PRUNER ProtoPruner;
  INT_PROTO_STRUCT Protos[PROTOS_PER_PROTO_SET];
};

typedef uint32_t CONFIG_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][4];

struct INT_CLASS_STRUCT {
  INT_CLASS_STRUCT() = default;
  INT_CLASS_STRUCT(int MaxNumProtos, int MaxNumConfigs);
  ~INT_CLASS_STRUCT();
  uint16_t NumProtos = 0;
  uint8_t NumProtoSets = 0;
  uint8_t NumConfigs = 0;
  PROTO_SET_STRUCT *ProtoSets[MAX_NUM_PROTO_SETS];
  std::vector<uint8_t> ProtoLengths;
  uint16_t ConfigLengths[MAX_NUM_CONFIGS];
  int font_set_id = 0; // FontSet id, see above
};

struct TESS_API INT_TEMPLATES_STRUCT {
  INT_TEMPLATES_STRUCT();
  ~INT_TEMPLATES_STRUCT();
  unsigned NumClasses;
  unsigned NumClassPruners;
  INT_CLASS_STRUCT *Class[MAX_NUM_CLASSES];
  CLASS_PRUNER_STRUCT *ClassPruners[MAX_NUM_CLASS_PRUNERS];
};

/* definitions of integer features*/
#define MAX_NUM_INT_FEATURES 512
#define INT_CHAR_NORM_RANGE 256

struct INT_FEATURE_STRUCT {
  INT_FEATURE_STRUCT() : X(0), Y(0), Theta(0), CP_misses(0) {}
  // Builds a feature from an FCOORD for position with all the necessary
  // clipping and rounding.
  INT_FEATURE_STRUCT(const FCOORD &pos, uint8_t theta);
  // Builds a feature from ints with all the necessary clipping and casting.
  INT_FEATURE_STRUCT(int x, int y, int theta);

  uint8_t X;
  uint8_t Y;
  uint8_t Theta;
  int8_t CP_misses;

  void print() const {
    tprintf("(%d,%d):%d\n", X, Y, Theta);
  }
};

typedef INT_FEATURE_STRUCT INT_FEATURE_ARRAY[MAX_NUM_INT_FEATURES];

enum IntmatcherDebugAction { IDA_ADAPTIVE, IDA_STATIC, IDA_SHAPE_INDEX, IDA_BOTH };

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/

#define MaxNumIntProtosIn(C) (C->NumProtoSets * PROTOS_PER_PROTO_SET)
#define SetForProto(P) (P / PROTOS_PER_PROTO_SET)
#define IndexForProto(P) (P % PROTOS_PER_PROTO_SET)
#define ProtoForProtoId(C, P) (&((C->ProtoSets[SetForProto(P)])->Protos[IndexForProto(P)]))
#define PPrunerWordIndexFor(I) (((I) % PROTOS_PER_PROTO_SET) / PROTOS_PER_PP_WERD)
#define PPrunerBitIndexFor(I) ((I) % PROTOS_PER_PP_WERD)
#define PPrunerMaskFor(I) (1 << PPrunerBitIndexFor(I))

#define MaxNumClassesIn(T) (T->NumClassPruners * CLASSES_PER_CP)
#define LegalClassId(c) ((c) >= 0 && (c) < MAX_NUM_CLASSES)
#define UnusedClassIdIn(T, c) ((T)->Class[c] == nullptr)
#define ClassForClassId(T, c) ((T)->Class[c])
#define ClassPrunersFor(T) ((T)->ClassPruner)
#define CPrunerIdFor(c) ((c) / CLASSES_PER_CP)
#define CPrunerFor(T, c) ((T)->ClassPruners[CPrunerIdFor(c)])
#define CPrunerWordIndexFor(c) (((c) % CLASSES_PER_CP) / CLASSES_PER_CP_WERD)
#define CPrunerBitIndexFor(c) (((c) % CLASSES_PER_CP) % CLASSES_PER_CP_WERD)
#define CPrunerMaskFor(L, c) (((L) + 1) << CPrunerBitIndexFor(c) * NUM_BITS_PER_CLASS)

/* DEBUG macros*/
#define PRINT_MATCH_SUMMARY 0x001
#define DISPLAY_FEATURE_MATCHES 0x002
#define DISPLAY_PROTO_MATCHES 0x004
#define PRINT_FEATURE_MATCHES 0x008
#define PRINT_PROTO_MATCHES 0x010
#define CLIP_MATCH_EVIDENCE 0x020

#define MatchDebuggingOn(D) (D)
#define PrintMatchSummaryOn(D) ((D)&PRINT_MATCH_SUMMARY)
#define DisplayFeatureMatchesOn(D) ((D)&DISPLAY_FEATURE_MATCHES)
#define DisplayProtoMatchesOn(D) ((D)&DISPLAY_PROTO_MATCHES)
#define PrintFeatureMatchesOn(D) ((D)&PRINT_FEATURE_MATCHES)
#define PrintProtoMatchesOn(D) ((D)&PRINT_PROTO_MATCHES)
#define ClipMatchEvidenceOn(D) ((D)&CLIP_MATCH_EVIDENCE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void AddIntClass(INT_TEMPLATES_STRUCT *Templates, CLASS_ID ClassId, INT_CLASS_STRUCT *Class);

int AddIntConfig(INT_CLASS_STRUCT *Class);

int AddIntProto(INT_CLASS_STRUCT *Class);

void AddProtoToClassPruner(PROTO_STRUCT *Proto, CLASS_ID ClassId, INT_TEMPLATES_STRUCT *Templates);

void AddProtoToProtoPruner(PROTO_STRUCT *Proto, int ProtoId, INT_CLASS_STRUCT *Class, bool debug);

uint8_t Bucket8For(float param, float offset, int num_buckets);
uint16_t Bucket16For(float param, float offset, int num_buckets);

uint8_t CircBucketFor(float param, float offset, int num_buckets);

void UpdateMatchDisplay();

void ConvertConfig(BIT_VECTOR Config, int ConfigId, INT_CLASS_STRUCT *Class);

void DisplayIntFeature(const INT_FEATURE_STRUCT *Feature, float Evidence);

void DisplayIntProto(INT_CLASS_STRUCT *Class, PROTO_ID ProtoId, float Evidence);

void ShowMatchDisplay();

#ifndef GRAPHICS_DISABLED
// Clears the given window and draws the featurespace guides for the
// appropriate normalization method.
TESS_API
void ClearFeatureSpaceWindow(NORM_METHOD norm_method, ScrollView *window);
#endif // !GRAPHICS_DISABLED

/*----------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
TESS_API
void RenderIntFeature(ScrollView *window, const INT_FEATURE_STRUCT *Feature,
                      ScrollView::Color color);

void InitIntMatchWindowIfReqd();

void InitProtoDisplayWindowIfReqd();

void InitFeatureDisplayWindowIfReqd();

// Creates a window of the appropriate size for displaying elements
// in feature space.
TESS_API
ScrollView *CreateFeatureSpaceWindow(const char *name, int xpos, int ypos);
#endif // !GRAPHICS_DISABLED

} // namespace tesseract

#endif
