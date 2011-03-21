/******************************************************************************
 **	Filename:    intproto.h
 **	Purpose:     Definition of data structures for integer protos.
 **	Author:      Dan Johnson
 **	History:     Thu Feb  7 12:58:45 1991, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
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
#ifndef INTPROTO_H
#define INTPROTO_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "genericvector.h"
#include "matchdefs.h"
#include "mfoutline.h"
#include "protos.h"
#include "callcpp.h"
#include "scrollview.h"
#include "unicharset.h"

/* define order of params in pruners */
#define PRUNER_X      0
#define PRUNER_Y      1
#define PRUNER_ANGLE    2

/* definition of coordinate system offsets for each table parameter */
#define ANGLE_SHIFT (0.0)
#define X_SHIFT   (0.5)
#define Y_SHIFT   (0.5)

#define MAX_PROTO_INDEX   24
#define BITS_PER_WERD   (8 * sizeof (uinT32))
/* Script detection: increase this number to 128 */
#define MAX_NUM_CONFIGS   64
#define MAX_NUM_PROTOS    512
#define PROTOS_PER_PROTO_SET  64
#define MAX_NUM_PROTO_SETS  (MAX_NUM_PROTOS / PROTOS_PER_PROTO_SET)
#define NUM_PP_PARAMS   3
#define NUM_PP_BUCKETS    64
#define NUM_CP_BUCKETS    24
#define CLASSES_PER_CP    32
#define NUM_BITS_PER_CLASS  2
#define CLASS_PRUNER_CLASS_MASK (~(~0 << NUM_BITS_PER_CLASS))
#define CLASSES_PER_CP_WERD (CLASSES_PER_CP / NUM_BITS_PER_CLASS)
#define PROTOS_PER_PP_WERD  BITS_PER_WERD
#define BITS_PER_CP_VECTOR  (CLASSES_PER_CP * NUM_BITS_PER_CLASS)
#define MAX_NUM_CLASS_PRUNERS	((MAX_NUM_CLASSES + CLASSES_PER_CP - 1) /   \
				CLASSES_PER_CP)
#define WERDS_PER_CP_VECTOR (BITS_PER_CP_VECTOR / BITS_PER_WERD)
#define WERDS_PER_PP_VECTOR	((PROTOS_PER_PROTO_SET+BITS_PER_WERD-1)/    \
				BITS_PER_WERD)
#define WERDS_PER_PP		(NUM_PP_PARAMS * NUM_PP_BUCKETS *		\
				WERDS_PER_PP_VECTOR)
#define WERDS_PER_CP		(NUM_CP_BUCKETS * NUM_CP_BUCKETS *		\
				NUM_CP_BUCKETS * WERDS_PER_CP_VECTOR)
#define WERDS_PER_CONFIG_VEC	((MAX_NUM_CONFIGS + BITS_PER_WERD - 1) /    \
				BITS_PER_WERD)

/* The first 3 dimensions of the CLASS_PRUNER_STRUCT are the
 * 3 axes of the quantized feature space.
 * The position of the the bits recorded for each class in the
 * 4th dimension is determined by using CPrunerWordIndexFor(c),
 * where c is the corresponding class id. */
typedef uinT32 CLASS_PRUNER_STRUCT
[NUM_CP_BUCKETS][NUM_CP_BUCKETS][NUM_CP_BUCKETS][WERDS_PER_CP_VECTOR];

typedef
uinT32 (*CLASS_PRUNER)[NUM_CP_BUCKETS][NUM_CP_BUCKETS][WERDS_PER_CP_VECTOR];

typedef struct
{
  inT8 A;
  uinT8 B;
  inT8 C;
  uinT8 Angle;
  uinT32 Configs[WERDS_PER_CONFIG_VEC];
}


INT_PROTO_STRUCT, *INT_PROTO;

typedef uinT32 PROTO_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR];

typedef struct
{
  PROTO_PRUNER ProtoPruner;
  INT_PROTO_STRUCT Protos[PROTOS_PER_PROTO_SET];
}


PROTO_SET_STRUCT, *PROTO_SET;

typedef uinT32 CONFIG_PRUNER[NUM_PP_PARAMS][NUM_PP_BUCKETS][4];

// Struct for information about spacing between characters in a particular font.
struct FontSpacingInfo {
  inT16 x_gap_before;
  inT16 x_gap_after;
  GenericVector<UNICHAR_ID> kerned_unichar_ids;
  GenericVector<inT16> kerned_x_gaps;
};

/*
 * font_properties contains properties about boldness, italicness, fixed pitch,
 * serif, fraktur
 */
struct FontInfo {
  FontInfo() : name(NULL), spacing_vec(NULL) {}
  ~FontInfo() {}
  // Reserves unicharset_size spots in spacing_vec.
  void init_spacing(int unicharset_size) {
    spacing_vec = new GenericVector<FontSpacingInfo *>();
    spacing_vec->init_to_size(unicharset_size, NULL);
  }
  // Adds the given pointer to FontSpacingInfo to spacing_vec member
  // (FontInfo class takes ownership of the pointer).
  // Note: init_spacing should be called before calling this function.
  void add_spacing(UNICHAR_ID uch_id, FontSpacingInfo *spacing_info) {
    ASSERT_HOST(spacing_vec != NULL && spacing_vec->size() > uch_id);
    (*spacing_vec)[uch_id] = spacing_info;
  }

  // Returns the pointer to FontSpacingInfo for the given UNICHAR_ID.
  const FontSpacingInfo *get_spacing(UNICHAR_ID uch_id) const {
    return (spacing_vec == NULL || spacing_vec->size() <= uch_id) ?
        NULL : (*spacing_vec)[uch_id];
  }

  // Fills spacing with the value of the x gap expected between the two given
  // UNICHAR_IDs. Returns true on success.
  bool get_spacing(UNICHAR_ID prev_uch_id,
                   UNICHAR_ID uch_id,
                   int *spacing) const {
    const FontSpacingInfo *prev_fsi = this->get_spacing(prev_uch_id);
    const FontSpacingInfo *fsi = this->get_spacing(uch_id);
    if (prev_fsi == NULL || fsi == NULL) return false;
    int i = 0;
    for (; i < prev_fsi->kerned_unichar_ids.size(); ++i) {
      if (prev_fsi->kerned_unichar_ids[i] == uch_id) break;
    }
    if (i < prev_fsi->kerned_unichar_ids.size()) {
      *spacing = prev_fsi->kerned_x_gaps[i];
    } else {
      *spacing = prev_fsi->x_gap_after + fsi->x_gap_before;
    }
    return true;
  }

  bool is_italic() const { return properties & 1; }
  bool is_bold() const { return (properties & 2) != 0; }
  bool is_fixed_pitch() const { return (properties & 4) != 0; }
  bool is_serif() const { return (properties & 8) != 0; }
  bool is_fraktur() const { return (properties & 16) != 0; }

  char* name;
  uinT32 properties;
  // Horizontal spacing between characters (indexed by UNICHAR_ID).
  GenericVector<FontSpacingInfo *> *spacing_vec;
};

// Every class (character) owns a FontSet that represents all the fonts that can
// render this character.
// Since almost all the characters from the same script share the same set of
// fonts, the sets are shared over multiple classes (see
// Classify::fontset_table_). Thus, a class only store an id to a set.
// Because some fonts cannot render just one character of a set, there are a
// lot of FontSet that differ only by one font. Rather than storing directly
// the FontInfo in the FontSet structure, it's better to share FontInfos among
// FontSets (Classify::fontinfo_table_).
struct FontSet {
  int           size;
  int*          configs;  // FontInfo ids
};


typedef struct
{
  uinT16 NumProtos;
  uinT8 NumProtoSets;
  uinT8 NumConfigs;
  PROTO_SET ProtoSets[MAX_NUM_PROTO_SETS];
  uinT8 *ProtoLengths;
  uinT16 ConfigLengths[MAX_NUM_CONFIGS];
  int font_set_id;  // FontSet id, see above
}


INT_CLASS_STRUCT, *INT_CLASS;

typedef struct
{
  int NumClasses;
  int NumClassPruners;
  INT_CLASS Class[MAX_NUM_CLASSES];
  CLASS_PRUNER ClassPruner[MAX_NUM_CLASS_PRUNERS];
}


INT_TEMPLATES_STRUCT, *INT_TEMPLATES;

/* definitions of integer features*/
#define MAX_NUM_INT_FEATURES 512
#define INT_CHAR_NORM_RANGE  256

struct INT_FEATURE_STRUCT
{
  uinT8 X;
  uinT8 Y;
  uinT8 Theta;
  inT8 CP_misses;

  void print() const {
    tprintf("(%d,%d):%d\n", X, Y, Theta);
  }
};

typedef INT_FEATURE_STRUCT *INT_FEATURE;

typedef INT_FEATURE_STRUCT INT_FEATURE_ARRAY[MAX_NUM_INT_FEATURES];

enum IntmatcherDebugAction {
  IDA_ADAPTIVE,
  IDA_STATIC,
  IDA_BOTH
};

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/

#define MaxNumIntProtosIn(C)  (C->NumProtoSets * PROTOS_PER_PROTO_SET)
#define SetForProto(P)    (P / PROTOS_PER_PROTO_SET)
#define IndexForProto(P)  (P % PROTOS_PER_PROTO_SET)
#define ProtoForProtoId(C,P)	(&((C->ProtoSets[SetForProto (P)])->	\
					Protos [IndexForProto (P)]))
#define PPrunerWordIndexFor(I)	(((I) % PROTOS_PER_PROTO_SET) /		\
				PROTOS_PER_PP_WERD)
#define PPrunerBitIndexFor(I) ((I) % PROTOS_PER_PP_WERD)
#define PPrunerMaskFor(I) (1 << PPrunerBitIndexFor (I))

#define MaxNumClassesIn(T)    (T->NumClassPruners * CLASSES_PER_CP)
#define LegalClassId(c)   ((c) >= 0 && (c) <= MAX_CLASS_ID)
#define UnusedClassIdIn(T,c)  ((T)->Class[c] == NULL)
#define ClassForClassId(T,c) ((T)->Class[c])
#define ClassPrunersFor(T)  ((T)->ClassPruner)
#define CPrunerIdFor(c)   ((c) / CLASSES_PER_CP)
#define CPrunerFor(T,c)   ((T)->ClassPruner [CPrunerIdFor (c)])
#define CPrunerWordIndexFor(c)  (((c) % CLASSES_PER_CP) / CLASSES_PER_CP_WERD)
#define CPrunerBitIndexFor(c) (((c) % CLASSES_PER_CP) % CLASSES_PER_CP_WERD)
#define CPrunerMaskFor(L,c) (((L)+1) << CPrunerBitIndexFor (c) * NUM_BITS_PER_CLASS)

/* DEBUG macros*/
#define PRINT_MATCH_SUMMARY 0x001
#define DISPLAY_FEATURE_MATCHES 0x002
#define DISPLAY_PROTO_MATCHES 0x004
#define PRINT_FEATURE_MATCHES 0x008
#define PRINT_PROTO_MATCHES 0x010
#define CLIP_MATCH_EVIDENCE 0x020

#define MatchDebuggingOn(D)   (D)
#define PrintMatchSummaryOn(D)    ((D) & PRINT_MATCH_SUMMARY)
#define DisplayFeatureMatchesOn(D)  ((D) & DISPLAY_FEATURE_MATCHES)
#define DisplayProtoMatchesOn(D)  ((D) & DISPLAY_PROTO_MATCHES)
#define PrintFeatureMatchesOn(D)  ((D) & PRINT_FEATURE_MATCHES)
#define PrintProtoMatchesOn(D)    ((D) & PRINT_PROTO_MATCHES)
#define ClipMatchEvidenceOn(D)    ((D) & CLIP_MATCH_EVIDENCE)

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
void AddIntClass(INT_TEMPLATES Templates, CLASS_ID ClassId, INT_CLASS Class);

int AddIntConfig(INT_CLASS Class);

int AddIntProto(INT_CLASS Class);

void AddProtoToClassPruner(PROTO Proto,
                           CLASS_ID ClassId,
                           INT_TEMPLATES Templates);

void AddProtoToProtoPruner(PROTO Proto, int ProtoId,
                           INT_CLASS Class, bool debug);

int BucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets);

int CircBucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets);

void UpdateMatchDisplay();

void ConvertConfig(BIT_VECTOR Config, int ConfigId, INT_CLASS Class);

void DisplayIntFeature(INT_FEATURE Feature, FLOAT32 Evidence);

void DisplayIntProto(INT_CLASS Class, PROTO_ID ProtoId, FLOAT32 Evidence);

INT_CLASS NewIntClass(int MaxNumProtos, int MaxNumConfigs);

INT_TEMPLATES NewIntTemplates();

void free_int_templates(INT_TEMPLATES templates);

void ShowMatchDisplay();

namespace tesseract {

// Clears the given window and draws the featurespace guides for the
// appropriate normalization method.
void ClearFeatureSpaceWindow(NORM_METHOD norm_method, ScrollView* window);

}  // namespace tesseract.

/*----------------------------------------------------------------------------*/
void RenderIntFeature(ScrollView *window, const INT_FEATURE_STRUCT* Feature,
                      ScrollView::Color color);

void InitIntMatchWindowIfReqd();

void InitProtoDisplayWindowIfReqd();

void InitFeatureDisplayWindowIfReqd();

// Creates a window of the appropriate size for displaying elements
// in feature space.
ScrollView* CreateFeatureSpaceWindow(const char* name, int xpos, int ypos);

#endif
