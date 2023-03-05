/******************************************************************************
 ** Filename:    intproto.c
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
 ******************************************************************************/
/*-----------------------------------------------------------------------------
          Include Files and Type Defines
-----------------------------------------------------------------------------*/

#define _USE_MATH_DEFINES // for M_PI

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#  include "config_auto.h"
#endif

#include "intproto.h"

#include "classify.h"
#include "fontinfo.h"
#include "mfoutline.h"
#include "picofeat.h"
#include "points.h"
#include "shapetable.h"
#ifndef GRAPHICS_DISABLED
#include "svmnode.h"
#endif

#include "helpers.h"

#include <algorithm>
#include <cassert>
#include <cmath> // for M_PI, std::floor
#include <cstdio>

namespace tesseract {

/* match debug display constants*/
#define PROTO_PRUNER_SCALE (4.0)

#define INT_DESCENDER (0.0 * INT_CHAR_NORM_RANGE)
#define INT_BASELINE (0.25 * INT_CHAR_NORM_RANGE)
#define INT_XHEIGHT (0.75 * INT_CHAR_NORM_RANGE)
#define INT_CAPHEIGHT (1.0 * INT_CHAR_NORM_RANGE)

#define INT_XCENTER (0.5 * INT_CHAR_NORM_RANGE)
#define INT_YCENTER (0.5 * INT_CHAR_NORM_RANGE)
#define INT_XRADIUS (0.2 * INT_CHAR_NORM_RANGE)
#define INT_YRADIUS (0.2 * INT_CHAR_NORM_RANGE)
#define INT_MIN_X 0
#define INT_MIN_Y 0
#define INT_MAX_X INT_CHAR_NORM_RANGE
#define INT_MAX_Y INT_CHAR_NORM_RANGE

/** define pad used to snap near horiz/vertical protos to horiz/vertical */
#define HV_TOLERANCE (0.0025) /* approx 0.9 degrees */

typedef enum { StartSwitch, EndSwitch, LastSwitch } SWITCH_TYPE;
#define MAX_NUM_SWITCHES 3

struct FILL_SWITCH {
  SWITCH_TYPE Type;
  int8_t X, Y;
  int16_t YInit;
  int16_t Delta;
};

struct TABLE_FILLER {
  uint8_t NextSwitch;
  uint8_t AngleStart, AngleEnd;
  int8_t X;
  int16_t YStart, YEnd;
  int16_t StartDelta, EndDelta;
  FILL_SWITCH Switch[MAX_NUM_SWITCHES];
};

struct FILL_SPEC {
  int8_t X;
  int8_t YStart, YEnd;
  uint8_t AngleStart, AngleEnd;
};

/* constants for conversion from old inttemp format */
#define OLD_MAX_NUM_CONFIGS 32
#define OLD_WERDS_PER_CONFIG_VEC ((OLD_MAX_NUM_CONFIGS + BITS_PER_WERD - 1) / BITS_PER_WERD)

/*-----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
/** macro for performing circular increments of bucket indices */
#define CircularIncrement(i, r) (((i) < (r)-1) ? ((i)++) : ((i) = 0))

/** macro for mapping floats to ints without bounds checking */
#define MapParam(P, O, N) (std::floor(((P) + (O)) * (N)))

/*---------------------------------------------------------------------------
            Private Function Prototypes
----------------------------------------------------------------------------*/
float BucketStart(int Bucket, float Offset, int NumBuckets);

float BucketEnd(int Bucket, float Offset, int NumBuckets);

void DoFill(FILL_SPEC *FillSpec, CLASS_PRUNER_STRUCT *Pruner, uint32_t ClassMask,
            uint32_t ClassCount, uint32_t WordIndex);

bool FillerDone(TABLE_FILLER *Filler);

void FillPPCircularBits(uint32_t ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR], int Bit,
                        float Center, float Spread, bool debug);

void FillPPLinearBits(uint32_t ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR], int Bit,
                      float Center, float Spread, bool debug);

void GetCPPadsForLevel(int Level, float *EndPad, float *SidePad, float *AnglePad);

ScrollView::Color GetMatchColorFor(float Evidence);

void GetNextFill(TABLE_FILLER *Filler, FILL_SPEC *Fill);

void InitTableFiller(float EndPad, float SidePad, float AnglePad, PROTO_STRUCT *Proto,
                     TABLE_FILLER *Filler);

#ifndef GRAPHICS_DISABLED
void RenderIntFeature(ScrollView *window, const INT_FEATURE_STRUCT *Feature,
                      ScrollView::Color color);

void RenderIntProto(ScrollView *window, INT_CLASS_STRUCT *Class, PROTO_ID ProtoId, ScrollView::Color color);
#endif // !GRAPHICS_DISABLED

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/

#ifndef GRAPHICS_DISABLED
/* global display lists used to display proto and feature match information*/
static ScrollView *IntMatchWindow = nullptr;
static ScrollView *FeatureDisplayWindow = nullptr;
static ScrollView *ProtoDisplayWindow = nullptr;
#endif

/*-----------------------------------------------------------------------------
        Variables
-----------------------------------------------------------------------------*/

/* control knobs */
static INT_VAR(classify_num_cp_levels, 3, "Number of Class Pruner Levels");
static double_VAR(classify_cp_angle_pad_loose, 45.0, "Class Pruner Angle Pad Loose");
static double_VAR(classify_cp_angle_pad_medium, 20.0, "Class Pruner Angle Pad Medium");
static double_VAR(classify_cp_angle_pad_tight, 10.0, "CLass Pruner Angle Pad Tight");
static double_VAR(classify_cp_end_pad_loose, 0.5, "Class Pruner End Pad Loose");
static double_VAR(classify_cp_end_pad_medium, 0.5, "Class Pruner End Pad Medium");
static double_VAR(classify_cp_end_pad_tight, 0.5, "Class Pruner End Pad Tight");
static double_VAR(classify_cp_side_pad_loose, 2.5, "Class Pruner Side Pad Loose");
static double_VAR(classify_cp_side_pad_medium, 1.2, "Class Pruner Side Pad Medium");
static double_VAR(classify_cp_side_pad_tight, 0.6, "Class Pruner Side Pad Tight");
static double_VAR(classify_pp_angle_pad, 45.0, "Proto Pruner Angle Pad");
static double_VAR(classify_pp_end_pad, 0.5, "Proto Prune End Pad");
static double_VAR(classify_pp_side_pad, 2.5, "Proto Pruner Side Pad");

/**
 * This routine truncates Param to lie within the range
 * of Min-Max inclusive.
 *
 * @param Param   parameter value to be truncated
 * @param Min, Max  parameter limits (inclusive)
 *
 * @return Truncated parameter.
 */
static int TruncateParam(float Param, int Min, int Max) {
  int result;
  if (Param < Min) {
    result = Min;
  } else if (Param > Max) {
    result = Max;
  } else {
    result = static_cast<int>(std::floor(Param));
  }
  return result;
}

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/// Builds a feature from an FCOORD for position with all the necessary
/// clipping and rounding.
INT_FEATURE_STRUCT::INT_FEATURE_STRUCT(const FCOORD &pos, uint8_t theta)
    : X(ClipToRange<int16_t>(static_cast<int16_t>(pos.x() + 0.5), 0, 255))
    , Y(ClipToRange<int16_t>(static_cast<int16_t>(pos.y() + 0.5), 0, 255))
    , Theta(theta)
    , CP_misses(0) {}
/** Builds a feature from ints with all the necessary clipping and casting. */
INT_FEATURE_STRUCT::INT_FEATURE_STRUCT(int x, int y, int theta)
    : X(static_cast<uint8_t>(ClipToRange<int>(x, 0, UINT8_MAX)))
    , Y(static_cast<uint8_t>(ClipToRange<int>(y, 0, UINT8_MAX)))
    , Theta(static_cast<uint8_t>(ClipToRange<int>(theta, 0, UINT8_MAX)))
    , CP_misses(0) {}

/**
 * This routine adds a new class structure to a set of
 * templates. Classes have to be added to Templates in
 * the order of increasing ClassIds.
 *
 * @param Templates templates to add new class to
 * @param ClassId   class id to associate new class with
 * @param Class   class data structure to add to templates
 *
 * Globals: none
 */
void AddIntClass(INT_TEMPLATES_STRUCT *Templates, CLASS_ID ClassId, INT_CLASS_STRUCT *Class) {
  int Pruner;

  assert(LegalClassId(ClassId));
  if (static_cast<unsigned>(ClassId) != Templates->NumClasses) {
    fprintf(stderr,
            "Please make sure that classes are added to templates"
            " in increasing order of ClassIds\n");
    exit(1);
  }
  ClassForClassId(Templates, ClassId) = Class;
  Templates->NumClasses++;

  if (Templates->NumClasses > MaxNumClassesIn(Templates)) {
    Pruner = Templates->NumClassPruners++;
    Templates->ClassPruners[Pruner] = new CLASS_PRUNER_STRUCT;
    memset(Templates->ClassPruners[Pruner], 0, sizeof(CLASS_PRUNER_STRUCT));
  }
} /* AddIntClass */

/**
 * This routine returns the index of the next free config
 * in Class.
 *
 * @param Class class to add new configuration to
 *
 * Globals: none
 *
 * @return Index of next free config.
 */
int AddIntConfig(INT_CLASS_STRUCT *Class) {
  int Index;

  assert(Class->NumConfigs < MAX_NUM_CONFIGS);

  Index = Class->NumConfigs++;
  Class->ConfigLengths[Index] = 0;
  return Index;
} /* AddIntConfig */

/**
 * This routine allocates the next free proto in Class and
 * returns its index.
 *
 * @param Class class to add new proto to
 *
 * Globals: none
 *
 * @return Proto index of new proto.
 */
int AddIntProto(INT_CLASS_STRUCT *Class) {
  if (Class->NumProtos >= MAX_NUM_PROTOS) {
    return (NO_PROTO);
  }

  int Index = Class->NumProtos++;

  if (Class->NumProtos > MaxNumIntProtosIn(Class)) {
    int ProtoSetId = Class->NumProtoSets++;
    auto ProtoSet = new PROTO_SET_STRUCT;
    Class->ProtoSets[ProtoSetId] = ProtoSet;
    memset(ProtoSet, 0, sizeof(*ProtoSet));

    /* reallocate space for the proto lengths and install in class */
    Class->ProtoLengths.resize(MaxNumIntProtosIn(Class));
  }

  /* initialize proto so its length is zero and it isn't in any configs */
  Class->ProtoLengths[Index] = 0;
  auto Proto = ProtoForProtoId(Class, Index);
  for (uint32_t *Word = Proto->Configs; Word < Proto->Configs + WERDS_PER_CONFIG_VEC; *Word++ = 0) {
  }

  return (Index);
}

/**
 * This routine adds Proto to the class pruning tables
 * for the specified class in Templates.
 *
 * Globals:
 *  - classify_num_cp_levels number of levels used in the class pruner
 * @param Proto   floating-pt proto to add to class pruner
 * @param ClassId   class id corresponding to Proto
 * @param Templates set of templates containing class pruner
 */
void AddProtoToClassPruner(PROTO_STRUCT *Proto, CLASS_ID ClassId, INT_TEMPLATES_STRUCT *Templates)
#define MAX_LEVEL 2
{
  CLASS_PRUNER_STRUCT *Pruner;
  uint32_t ClassMask;
  uint32_t ClassCount;
  uint32_t WordIndex;
  int Level;
  float EndPad, SidePad, AnglePad;
  TABLE_FILLER TableFiller;
  FILL_SPEC FillSpec;

  Pruner = CPrunerFor(Templates, ClassId);
  WordIndex = CPrunerWordIndexFor(ClassId);
  ClassMask = CPrunerMaskFor(MAX_LEVEL, ClassId);

  for (Level = classify_num_cp_levels - 1; Level >= 0; Level--) {
    GetCPPadsForLevel(Level, &EndPad, &SidePad, &AnglePad);
    ClassCount = CPrunerMaskFor(Level, ClassId);
    InitTableFiller(EndPad, SidePad, AnglePad, Proto, &TableFiller);

    while (!FillerDone(&TableFiller)) {
      GetNextFill(&TableFiller, &FillSpec);
      DoFill(&FillSpec, Pruner, ClassMask, ClassCount, WordIndex);
    }
  }
} /* AddProtoToClassPruner */

/**
 * This routine updates the proto pruner lookup tables
 * for Class to include a new proto identified by ProtoId
 * and described by Proto.
 * @param Proto floating-pt proto to be added to proto pruner
 * @param ProtoId id of proto
 * @param Class integer class that contains desired proto pruner
 * @param debug debug flag
 * @note Globals: none
 */
void AddProtoToProtoPruner(PROTO_STRUCT *Proto, int ProtoId, INT_CLASS_STRUCT *Class, bool debug) {
  float X, Y, Length;
  float Pad;

  if (ProtoId >= Class->NumProtos) {
    tprintf("AddProtoToProtoPruner:assert failed: %d < %d", ProtoId, Class->NumProtos);
  }
  assert(ProtoId < Class->NumProtos);

  int Index = IndexForProto(ProtoId);
  auto ProtoSet = Class->ProtoSets[SetForProto(ProtoId)];

  float Angle = Proto->Angle;
#ifndef _WIN32
  assert(!std::isnan(Angle));
#endif

  FillPPCircularBits(ProtoSet->ProtoPruner[PRUNER_ANGLE], Index, Angle + ANGLE_SHIFT,
                     classify_pp_angle_pad / 360.0, debug);

  Angle *= 2.0 * M_PI;
  Length = Proto->Length;

  X = Proto->X + X_SHIFT;
  Pad = std::max(fabs(std::cos(Angle)) * (Length / 2.0 + classify_pp_end_pad * GetPicoFeatureLength()),
                 fabs(std::sin(Angle)) * (classify_pp_side_pad * GetPicoFeatureLength()));

  FillPPLinearBits(ProtoSet->ProtoPruner[PRUNER_X], Index, X, Pad, debug);

  Y = Proto->Y + Y_SHIFT;
  Pad = std::max(fabs(std::sin(Angle)) * (Length / 2.0 + classify_pp_end_pad * GetPicoFeatureLength()),
                 fabs(std::cos(Angle)) * (classify_pp_side_pad * GetPicoFeatureLength()));

  FillPPLinearBits(ProtoSet->ProtoPruner[PRUNER_Y], Index, Y, Pad, debug);
} /* AddProtoToProtoPruner */

/**
 * Returns a quantized bucket for the given param shifted by offset,
 * notionally (param + offset) * num_buckets, but clipped and casted to the
 * appropriate type.
 */
uint8_t Bucket8For(float param, float offset, int num_buckets) {
  int bucket = IntCastRounded(MapParam(param, offset, num_buckets));
  return static_cast<uint8_t>(ClipToRange<int>(bucket, 0, num_buckets - 1));
}
uint16_t Bucket16For(float param, float offset, int num_buckets) {
  int bucket = IntCastRounded(MapParam(param, offset, num_buckets));
  return static_cast<uint16_t>(ClipToRange<int>(bucket, 0, num_buckets - 1));
}

/**
 * Returns a quantized bucket for the given circular param shifted by offset,
 * notionally (param + offset) * num_buckets, but modded and casted to the
 * appropriate type.
 */
uint8_t CircBucketFor(float param, float offset, int num_buckets) {
  int bucket = IntCastRounded(MapParam(param, offset, num_buckets));
  return static_cast<uint8_t>(Modulo(bucket, num_buckets));
} /* CircBucketFor */

#ifndef GRAPHICS_DISABLED
/**
 * This routine clears the global feature and proto
 * display lists.
 *
 * Globals:
 * - FeatureShapes display list for features
 * - ProtoShapes display list for protos
 */
void UpdateMatchDisplay() {
  if (IntMatchWindow != nullptr) {
    IntMatchWindow->Update();
  }
} /* ClearMatchDisplay */
#endif

/**
 * This operation updates the config vectors of all protos
 * in Class to indicate that the protos with 1's in Config
 * belong to a new configuration identified by ConfigId.
 * It is assumed that the length of the Config bit vector is
 * equal to the number of protos in Class.
 * @param Config    config to be added to class
 * @param ConfigId  id to be used for new config
 * @param Class   class to add new config to
 */
void ConvertConfig(BIT_VECTOR Config, int ConfigId, INT_CLASS_STRUCT *Class) {
  int ProtoId;
  INT_PROTO_STRUCT *Proto;
  int TotalLength;

  for (ProtoId = 0, TotalLength = 0; ProtoId < Class->NumProtos; ProtoId++) {
    if (test_bit(Config, ProtoId)) {
      Proto = ProtoForProtoId(Class, ProtoId);
      SET_BIT(Proto->Configs, ConfigId);
      TotalLength += Class->ProtoLengths[ProtoId];
    }
  }
  Class->ConfigLengths[ConfigId] = TotalLength;
} /* ConvertConfig */

/**
 * This routine converts Proto to integer format and
 * installs it as ProtoId in Class.
 * @param Proto floating-pt proto to be converted to integer format
 * @param ProtoId id of proto
 * @param Class integer class to add converted proto to
 */
void Classify::ConvertProto(PROTO_STRUCT *Proto, int ProtoId, INT_CLASS_STRUCT *Class) {
  assert(ProtoId < Class->NumProtos);

  INT_PROTO_STRUCT *P = ProtoForProtoId(Class, ProtoId);

  float Param = Proto->A * 128;
  P->A = TruncateParam(Param, -128, 127);

  Param = -Proto->B * 256;
  P->B = TruncateParam(Param, 0, 255);

  Param = Proto->C * 128;
  P->C = TruncateParam(Param, -128, 127);

  Param = Proto->Angle * 256;
  if (Param < 0 || Param >= 256) {
    P->Angle = 0;
  } else {
    P->Angle = static_cast<uint8_t>(Param);
  }

  /* round proto length to nearest integer number of pico-features */
  Param = (Proto->Length / GetPicoFeatureLength()) + 0.5;
  Class->ProtoLengths[ProtoId] = TruncateParam(Param, 1, 255);
  if (classify_learning_debug_level >= 2) {
    tprintf("Converted ffeat to (A=%d,B=%d,C=%d,L=%d)", P->A, P->B, P->C,
            Class->ProtoLengths[ProtoId]);
  }
} /* ConvertProto */

/**
 * This routine converts from the old floating point format
 * to the new integer format.
 * @param FloatProtos prototypes in old floating pt format
 * @param target_unicharset the UNICHARSET to use
 * @return New set of training templates in integer format.
 * @note Globals: none
 */
INT_TEMPLATES_STRUCT *Classify::CreateIntTemplates(CLASSES FloatProtos,
                                           const UNICHARSET &target_unicharset) {
  CLASS_TYPE FClass;
  INT_CLASS_STRUCT *IClass;
  int ProtoId;
  int ConfigId;

  auto IntTemplates = new INT_TEMPLATES_STRUCT;

  for (unsigned ClassId = 0; ClassId < target_unicharset.size(); ClassId++) {
    FClass = &(FloatProtos[ClassId]);
    if (FClass->NumProtos == 0 && FClass->NumConfigs == 0 &&
        strcmp(target_unicharset.id_to_unichar(ClassId), " ") != 0) {
      tprintf("Warning: no protos/configs for %s in CreateIntTemplates()\n",
              target_unicharset.id_to_unichar(ClassId));
    }
    assert(UnusedClassIdIn(IntTemplates, ClassId));
    IClass = new INT_CLASS_STRUCT(FClass->NumProtos, FClass->NumConfigs);
    unsigned fs_size = FClass->font_set.size();
    FontSet fs;
    fs.reserve(fs_size);
    for (unsigned i = 0; i < fs_size; ++i) {
      fs.push_back(FClass->font_set[i]);
    }
    IClass->font_set_id = this->fontset_table_.push_back(fs);
    AddIntClass(IntTemplates, ClassId, IClass);

    for (ProtoId = 0; ProtoId < FClass->NumProtos; ProtoId++) {
      AddIntProto(IClass);
      ConvertProto(ProtoIn(FClass, ProtoId), ProtoId, IClass);
      AddProtoToProtoPruner(ProtoIn(FClass, ProtoId), ProtoId, IClass,
                            classify_learning_debug_level >= 2);
      AddProtoToClassPruner(ProtoIn(FClass, ProtoId), ClassId, IntTemplates);
    }

    for (ConfigId = 0; ConfigId < FClass->NumConfigs; ConfigId++) {
      AddIntConfig(IClass);
      ConvertConfig(FClass->Configurations[ConfigId], ConfigId, IClass);
    }
  }
  return (IntTemplates);
} /* CreateIntTemplates */

#ifndef GRAPHICS_DISABLED
/**
 * This routine renders the specified feature into a
 * global display list.
 *
 * Globals:
 * - FeatureShapes global display list for features
 * @param Feature   pico-feature to be displayed
 * @param Evidence  best evidence for this feature (0-1)
 */
void DisplayIntFeature(const INT_FEATURE_STRUCT *Feature, float Evidence) {
  ScrollView::Color color = GetMatchColorFor(Evidence);
  RenderIntFeature(IntMatchWindow, Feature, color);
  if (FeatureDisplayWindow) {
    RenderIntFeature(FeatureDisplayWindow, Feature, color);
  }
} /* DisplayIntFeature */

/**
 * This routine renders the specified proto into a
 * global display list.
 *
 * Globals:
 * - ProtoShapes global display list for protos
 * @param Class   class to take proto from
 * @param ProtoId   id of proto in Class to be displayed
 * @param Evidence  total evidence for proto (0-1)
 */
void DisplayIntProto(INT_CLASS_STRUCT *Class, PROTO_ID ProtoId, float Evidence) {
  ScrollView::Color color = GetMatchColorFor(Evidence);
  RenderIntProto(IntMatchWindow, Class, ProtoId, color);
  if (ProtoDisplayWindow) {
    RenderIntProto(ProtoDisplayWindow, Class, ProtoId, color);
  }
} /* DisplayIntProto */
#endif

/// This constructor creates a new integer class data structure
/// and returns it.  Sufficient space is allocated
/// to handle the specified number of protos and configs.
/// @param MaxNumProtos  number of protos to allocate space for
/// @param MaxNumConfigs number of configs to allocate space for
INT_CLASS_STRUCT::INT_CLASS_STRUCT(int MaxNumProtos, int MaxNumConfigs) :
  NumProtos(0),
  NumProtoSets((MaxNumProtos + PROTOS_PER_PROTO_SET - 1) / PROTOS_PER_PROTO_SET),
  NumConfigs(0),
  ProtoLengths(MaxNumIntProtosIn(this))
{
  assert(MaxNumConfigs <= MAX_NUM_CONFIGS);
  assert(NumProtoSets <= MAX_NUM_PROTO_SETS);

  for (int i = 0; i < NumProtoSets; i++) {
    /* allocate space for a proto set, install in class, and initialize */
    auto ProtoSet = new PROTO_SET_STRUCT;
    memset(ProtoSet, 0, sizeof(*ProtoSet));
    ProtoSets[i] = ProtoSet;

    /* allocate space for the proto lengths and install in class */
  }
  memset(ConfigLengths, 0, sizeof(ConfigLengths));
}

INT_CLASS_STRUCT::~INT_CLASS_STRUCT() {
  for (int i = 0; i < NumProtoSets; i++) {
    delete ProtoSets[i];
  }
}

/// This constructor allocates a new set of integer templates
/// initialized to hold 0 classes.
INT_TEMPLATES_STRUCT::INT_TEMPLATES_STRUCT() {
  NumClasses = 0;
  NumClassPruners = 0;

  for (int i = 0; i < MAX_NUM_CLASSES; i++) {
    ClassForClassId(this, i) = nullptr;
  }
}

INT_TEMPLATES_STRUCT::~INT_TEMPLATES_STRUCT() {
  for (unsigned i = 0; i < NumClasses; i++) {
    delete Class[i];
  }
  for (unsigned i = 0; i < NumClassPruners; i++) {
    delete ClassPruners[i];
  }
}

/**
 * This routine reads a set of integer templates from
 * File.  File must already be open and must be in the
 * correct binary format.
 * @param  fp open file to read templates from
 * @return Pointer to integer templates read from File.
 * @note Globals: none
 */
INT_TEMPLATES_STRUCT *Classify::ReadIntTemplates(TFile *fp) {
  int j, w, x, y, z;
  INT_TEMPLATES_STRUCT *Templates;
  CLASS_PRUNER_STRUCT *Pruner;
  INT_CLASS_STRUCT *Class;

  /* variables for conversion from older inttemp formats */
  int b, bit_number, last_cp_bit_number, new_b, new_i, new_w;
  CLASS_ID class_id, max_class_id;
  std::vector<CLASS_ID> ClassIdFor(MAX_NUM_CLASSES);
  std::vector<CLASS_PRUNER_STRUCT *> TempClassPruner(MAX_NUM_CLASS_PRUNERS);
  uint32_t SetBitsForMask =          // word with NUM_BITS_PER_CLASS
      (1 << NUM_BITS_PER_CLASS) - 1; // set starting at bit 0
  uint32_t Mask, NewMask, ClassBits;
  unsigned MaxNumConfigs = MAX_NUM_CONFIGS;
  unsigned WerdsPerConfigVec = WERDS_PER_CONFIG_VEC;

  /* first read the high level template struct */
  Templates = new INT_TEMPLATES_STRUCT;
  // Read Templates in parts for 64 bit compatibility.
  uint32_t unicharset_size;
  if (fp->FReadEndian(&unicharset_size, sizeof(unicharset_size), 1) != 1) {
    tprintf("Bad read of inttemp!\n");
  }
  int32_t version_id = 0;
  if (fp->FReadEndian(&version_id, sizeof(version_id), 1) != 1 ||
      fp->FReadEndian(&Templates->NumClassPruners, sizeof(Templates->NumClassPruners), 1) != 1) {
    tprintf("Bad read of inttemp!\n");
  }
  if (version_id < 0) {
    // This file has a version id!
    version_id = -version_id;
    if (fp->FReadEndian(&Templates->NumClasses, sizeof(Templates->NumClasses), 1) != 1) {
      tprintf("Bad read of inttemp!\n");
    }
  } else {
    Templates->NumClasses = version_id;
  }

  if (version_id < 3) {
    MaxNumConfigs = OLD_MAX_NUM_CONFIGS;
    WerdsPerConfigVec = OLD_WERDS_PER_CONFIG_VEC;
  }

  if (version_id < 2) {
    std::vector<int16_t> IndexFor(MAX_NUM_CLASSES);
    if (fp->FReadEndian(&IndexFor[0], sizeof(IndexFor[0]), unicharset_size) != unicharset_size) {
      tprintf("Bad read of inttemp!\n");
    }
    if (fp->FReadEndian(&ClassIdFor[0], sizeof(ClassIdFor[0]), Templates->NumClasses) !=
        Templates->NumClasses) {
      tprintf("Bad read of inttemp!\n");
    }
  }

  /* then read in the class pruners */
  const unsigned kNumBuckets = NUM_CP_BUCKETS * NUM_CP_BUCKETS * NUM_CP_BUCKETS * WERDS_PER_CP_VECTOR;
  for (unsigned i = 0; i < Templates->NumClassPruners; i++) {
    Pruner = new CLASS_PRUNER_STRUCT;
    if (fp->FReadEndian(Pruner, sizeof(Pruner->p[0][0][0][0]), kNumBuckets) != kNumBuckets) {
      tprintf("Bad read of inttemp!\n");
    }
    if (version_id < 2) {
      TempClassPruner[i] = Pruner;
    } else {
      Templates->ClassPruners[i] = Pruner;
    }
  }

  /* fix class pruners if they came from an old version of inttemp */
  if (version_id < 2) {
    // Allocate enough class pruners to cover all the class ids.
    max_class_id = 0;
    for (unsigned i = 0; i < Templates->NumClasses; i++) {
      if (ClassIdFor[i] > max_class_id) {
        max_class_id = ClassIdFor[i];
      }
    }
    for (int i = 0; i <= CPrunerIdFor(max_class_id); i++) {
      Templates->ClassPruners[i] = new CLASS_PRUNER_STRUCT;
      memset(Templates->ClassPruners[i], 0, sizeof(CLASS_PRUNER_STRUCT));
    }
    // Convert class pruners from the old format (indexed by class index)
    // to the new format (indexed by class id).
    last_cp_bit_number = NUM_BITS_PER_CLASS * Templates->NumClasses - 1;
    for (unsigned i = 0; i < Templates->NumClassPruners; i++) {
      for (x = 0; x < NUM_CP_BUCKETS; x++) {
        for (y = 0; y < NUM_CP_BUCKETS; y++) {
          for (z = 0; z < NUM_CP_BUCKETS; z++) {
            for (w = 0; w < WERDS_PER_CP_VECTOR; w++) {
              if (TempClassPruner[i]->p[x][y][z][w] == 0) {
                continue;
              }
              for (b = 0; b < BITS_PER_WERD; b += NUM_BITS_PER_CLASS) {
                bit_number = i * BITS_PER_CP_VECTOR + w * BITS_PER_WERD + b;
                if (bit_number > last_cp_bit_number) {
                  break; // the rest of the bits in this word are not used
                }
                class_id = ClassIdFor[bit_number / NUM_BITS_PER_CLASS];
                // Single out NUM_BITS_PER_CLASS bits relating to class_id.
                Mask = SetBitsForMask << b;
                ClassBits = TempClassPruner[i]->p[x][y][z][w] & Mask;
                // Move these bits to the new position in which they should
                // appear (indexed corresponding to the class_id).
                new_i = CPrunerIdFor(class_id);
                new_w = CPrunerWordIndexFor(class_id);
                new_b = CPrunerBitIndexFor(class_id) * NUM_BITS_PER_CLASS;
                if (new_b > b) {
                  ClassBits <<= (new_b - b);
                } else {
                  ClassBits >>= (b - new_b);
                }
                // Copy bits relating to class_id to the correct position
                // in Templates->ClassPruner.
                NewMask = SetBitsForMask << new_b;
                Templates->ClassPruners[new_i]->p[x][y][z][new_w] &= ~NewMask;
                Templates->ClassPruners[new_i]->p[x][y][z][new_w] |= ClassBits;
              }
            }
          }
        }
      }
    }
    for (unsigned i = 0; i < Templates->NumClassPruners; i++) {
      delete TempClassPruner[i];
    }
  }

  /* then read in each class */
  for (unsigned i = 0; i < Templates->NumClasses; i++) {
    /* first read in the high level struct for the class */
    Class = new INT_CLASS_STRUCT;
    if (fp->FReadEndian(&Class->NumProtos, sizeof(Class->NumProtos), 1) != 1 ||
        fp->FRead(&Class->NumProtoSets, sizeof(Class->NumProtoSets), 1) != 1 ||
        fp->FRead(&Class->NumConfigs, sizeof(Class->NumConfigs), 1) != 1) {
      tprintf("Bad read of inttemp!\n");
    }
    if (version_id == 0) {
      // Only version 0 writes 5 pointless pointers to the file.
      for (j = 0; j < 5; ++j) {
        int32_t junk;
        if (fp->FRead(&junk, sizeof(junk), 1) != 1) {
          tprintf("Bad read of inttemp!\n");
        }
      }
    }
    unsigned num_configs = version_id < 4 ? MaxNumConfigs : Class->NumConfigs;
    ASSERT_HOST(num_configs <= MaxNumConfigs);
    if (fp->FReadEndian(Class->ConfigLengths, sizeof(uint16_t), num_configs) != num_configs) {
      tprintf("Bad read of inttemp!\n");
    }
    if (version_id < 2) {
      ClassForClassId(Templates, ClassIdFor[i]) = Class;
    } else {
      ClassForClassId(Templates, i) = Class;
    }

    /* then read in the proto lengths */
    Class->ProtoLengths.clear();
    if (MaxNumIntProtosIn(Class) > 0) {
      Class->ProtoLengths.resize(MaxNumIntProtosIn(Class));
      if (fp->FRead(&Class->ProtoLengths[0], sizeof(uint8_t), MaxNumIntProtosIn(Class)) !=
          MaxNumIntProtosIn(Class)) {
        tprintf("Bad read of inttemp!\n");
      }
    }

    /* then read in the proto sets */
    for (j = 0; j < Class->NumProtoSets; j++) {
      auto ProtoSet = new PROTO_SET_STRUCT;
      unsigned num_buckets = NUM_PP_PARAMS * NUM_PP_BUCKETS * WERDS_PER_PP_VECTOR;
      if (fp->FReadEndian(&ProtoSet->ProtoPruner, sizeof(ProtoSet->ProtoPruner[0][0][0]),
                          num_buckets) != num_buckets) {
        tprintf("Bad read of inttemp!\n");
      }
      for (x = 0; x < PROTOS_PER_PROTO_SET; x++) {
        if (fp->FRead(&ProtoSet->Protos[x].A, sizeof(ProtoSet->Protos[x].A), 1) != 1 ||
            fp->FRead(&ProtoSet->Protos[x].B, sizeof(ProtoSet->Protos[x].B), 1) != 1 ||
            fp->FRead(&ProtoSet->Protos[x].C, sizeof(ProtoSet->Protos[x].C), 1) != 1 ||
            fp->FRead(&ProtoSet->Protos[x].Angle, sizeof(ProtoSet->Protos[x].Angle), 1) != 1) {
          tprintf("Bad read of inttemp!\n");
        }
        if (fp->FReadEndian(&ProtoSet->Protos[x].Configs, sizeof(ProtoSet->Protos[x].Configs[0]),
                            WerdsPerConfigVec) != WerdsPerConfigVec) {
          tprintf("Bad read of inttemp!\n");
        }
      }
      Class->ProtoSets[j] = ProtoSet;
    }
    if (version_id < 4) {
      Class->font_set_id = -1;
    } else {
      fp->FReadEndian(&Class->font_set_id, sizeof(Class->font_set_id), 1);
    }
  }

  if (version_id < 2) {
    /* add an empty nullptr class with class id 0 */
    assert(UnusedClassIdIn(Templates, 0));
    ClassForClassId(Templates, 0) = new INT_CLASS_STRUCT(1, 1);
    ClassForClassId(Templates, 0)->font_set_id = -1;
    Templates->NumClasses++;
    /* make sure the classes are contiguous */
    for (unsigned i = 0; i < MAX_NUM_CLASSES; i++) {
      if (i < Templates->NumClasses) {
        if (ClassForClassId(Templates, i) == nullptr) {
          fprintf(stderr, "Non-contiguous class ids in inttemp\n");
          exit(1);
        }
      } else {
        if (ClassForClassId(Templates, i) != nullptr) {
          fprintf(stderr, "Class id %u exceeds NumClassesIn (Templates) %u\n", i,
                  Templates->NumClasses);
          exit(1);
        }
      }
    }
  }
  if (version_id >= 4) {
    using namespace std::placeholders; // for _1, _2
    this->fontinfo_table_.read(fp, std::bind(read_info, _1, _2));
    if (version_id >= 5) {
      this->fontinfo_table_.read(fp, std::bind(read_spacing_info, _1, _2));
    }
    this->fontset_table_.read(fp, [](auto *f, auto *fs) { return f->DeSerialize(*fs); } );
  }

  return (Templates);
} /* ReadIntTemplates */

#ifndef GRAPHICS_DISABLED
/**
 * This routine sends the shapes in the global display
 * lists to the match debugger window.
 *
 * Globals:
 * - FeatureShapes display list containing feature matches
 * - ProtoShapes display list containing proto matches
 */
void Classify::ShowMatchDisplay() {
  InitIntMatchWindowIfReqd();
  if (ProtoDisplayWindow) {
    ProtoDisplayWindow->Clear();
  }
  if (FeatureDisplayWindow) {
    FeatureDisplayWindow->Clear();
  }
  ClearFeatureSpaceWindow(static_cast<NORM_METHOD>(static_cast<int>(classify_norm_method)),
                          IntMatchWindow);
  IntMatchWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y, INT_MAX_X, INT_MAX_Y);
  if (ProtoDisplayWindow) {
    ProtoDisplayWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y, INT_MAX_X, INT_MAX_Y);
  }
  if (FeatureDisplayWindow) {
    FeatureDisplayWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y, INT_MAX_X, INT_MAX_Y);
  }
} /* ShowMatchDisplay */

/// Clears the given window and draws the featurespace guides for the
/// appropriate normalization method.
void ClearFeatureSpaceWindow(NORM_METHOD norm_method, ScrollView *window) {
  window->Clear();

  window->Pen(ScrollView::GREY);
  // Draw the feature space limit rectangle.
  window->Rectangle(0, 0, INT_MAX_X, INT_MAX_Y);
  if (norm_method == baseline) {
    window->SetCursor(0, INT_DESCENDER);
    window->DrawTo(INT_MAX_X, INT_DESCENDER);
    window->SetCursor(0, INT_BASELINE);
    window->DrawTo(INT_MAX_X, INT_BASELINE);
    window->SetCursor(0, INT_XHEIGHT);
    window->DrawTo(INT_MAX_X, INT_XHEIGHT);
    window->SetCursor(0, INT_CAPHEIGHT);
    window->DrawTo(INT_MAX_X, INT_CAPHEIGHT);
  } else {
    window->Rectangle(INT_XCENTER - INT_XRADIUS, INT_YCENTER - INT_YRADIUS,
                      INT_XCENTER + INT_XRADIUS, INT_YCENTER + INT_YRADIUS);
  }
}
#endif

/**
 * This routine writes Templates to File.  The format
 * is an efficient binary format.  File must already be open
 * for writing.
 * @param File open file to write templates to
 * @param Templates templates to save into File
 * @param target_unicharset the UNICHARSET to use
 */
void Classify::WriteIntTemplates(FILE *File, INT_TEMPLATES_STRUCT *Templates,
                                 const UNICHARSET &target_unicharset) {
  INT_CLASS_STRUCT *Class;
  uint32_t unicharset_size = target_unicharset.size();
  int version_id = -5; // When negated by the reader -1 becomes +1 etc.

  if (Templates->NumClasses != unicharset_size) {
    tprintf(
        "Warning: executing WriteIntTemplates() with %d classes in"
        " Templates, while target_unicharset size is %" PRIu32 "\n",
        Templates->NumClasses, unicharset_size);
  }

  /* first write the high level template struct */
  fwrite(&unicharset_size, sizeof(unicharset_size), 1, File);
  fwrite(&version_id, sizeof(version_id), 1, File);
  fwrite(&Templates->NumClassPruners, sizeof(Templates->NumClassPruners), 1, File);
  fwrite(&Templates->NumClasses, sizeof(Templates->NumClasses), 1, File);

  /* then write out the class pruners */
  for (unsigned i = 0; i < Templates->NumClassPruners; i++) {
    fwrite(Templates->ClassPruners[i], sizeof(CLASS_PRUNER_STRUCT), 1, File);
  }

  /* then write out each class */
  for (unsigned i = 0; i < Templates->NumClasses; i++) {
    Class = Templates->Class[i];

    /* first write out the high level struct for the class */
    fwrite(&Class->NumProtos, sizeof(Class->NumProtos), 1, File);
    fwrite(&Class->NumProtoSets, sizeof(Class->NumProtoSets), 1, File);
    ASSERT_HOST(Class->NumConfigs == this->fontset_table_.at(Class->font_set_id).size());
    fwrite(&Class->NumConfigs, sizeof(Class->NumConfigs), 1, File);
    for (int j = 0; j < Class->NumConfigs; ++j) {
      fwrite(&Class->ConfigLengths[j], sizeof(uint16_t), 1, File);
    }

    /* then write out the proto lengths */
    if (MaxNumIntProtosIn(Class) > 0) {
      fwrite(&Class->ProtoLengths[0], sizeof(uint8_t), MaxNumIntProtosIn(Class), File);
    }

    /* then write out the proto sets */
    for (int j = 0; j < Class->NumProtoSets; j++) {
      fwrite(Class->ProtoSets[j], sizeof(PROTO_SET_STRUCT), 1, File);
    }

    /* then write the fonts info */
    fwrite(&Class->font_set_id, sizeof(int), 1, File);
  }

  /* Write the fonts info tables */
  using namespace std::placeholders; // for _1, _2
  this->fontinfo_table_.write(File, std::bind(write_info, _1, _2));
  this->fontinfo_table_.write(File, std::bind(write_spacing_info, _1, _2));
  this->fontset_table_.write(File, std::bind(write_set, _1, _2));
} /* WriteIntTemplates */

/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/**
 * This routine returns the parameter value which
 * corresponds to the beginning of the specified bucket.
 * The bucket number should have been generated using the
 * BucketFor() function with parameters Offset and NumBuckets.
 * @param Bucket    bucket whose start is to be computed
 * @param Offset    offset used to map params to buckets
 * @param NumBuckets  total number of buckets
 * @return Param value corresponding to start position of Bucket.
 * @note Globals: none
 */
float BucketStart(int Bucket, float Offset, int NumBuckets) {
  return static_cast<float>(Bucket) / NumBuckets - Offset;

} /* BucketStart */

/**
 * This routine returns the parameter value which
 * corresponds to the end of the specified bucket.
 * The bucket number should have been generated using the
 * BucketFor() function with parameters Offset and NumBuckets.
 * @param Bucket    bucket whose end is to be computed
 * @param Offset    offset used to map params to buckets
 * @param NumBuckets  total number of buckets
 * @return Param value corresponding to end position of Bucket.
 * @note Globals: none
 */
float BucketEnd(int Bucket, float Offset, int NumBuckets) {
  return static_cast<float>(Bucket + 1) / NumBuckets - Offset;
} /* BucketEnd */

/**
 * This routine fills in the section of a class pruner
 * corresponding to a single x value for a single proto of
 * a class.
 * @param FillSpec  specifies which bits to fill in pruner
 * @param Pruner    class pruner to be filled
 * @param ClassMask indicates which bits to change in each word
 * @param ClassCount  indicates what to change bits to
 * @param WordIndex indicates which word to change
 */
void DoFill(FILL_SPEC *FillSpec, CLASS_PRUNER_STRUCT *Pruner, uint32_t ClassMask,
            uint32_t ClassCount, uint32_t WordIndex) {
  int X, Y, Angle;
  uint32_t OldWord;

  X = FillSpec->X;
  if (X < 0) {
    X = 0;
  }
  if (X >= NUM_CP_BUCKETS) {
    X = NUM_CP_BUCKETS - 1;
  }

  if (FillSpec->YStart < 0) {
    FillSpec->YStart = 0;
  }
  if (FillSpec->YEnd >= NUM_CP_BUCKETS) {
    FillSpec->YEnd = NUM_CP_BUCKETS - 1;
  }

  for (Y = FillSpec->YStart; Y <= FillSpec->YEnd; Y++) {
    for (Angle = FillSpec->AngleStart;; CircularIncrement(Angle, NUM_CP_BUCKETS)) {
      OldWord = Pruner->p[X][Y][Angle][WordIndex];
      if (ClassCount > (OldWord & ClassMask)) {
        OldWord &= ~ClassMask;
        OldWord |= ClassCount;
        Pruner->p[X][Y][Angle][WordIndex] = OldWord;
      }
      if (Angle == FillSpec->AngleEnd) {
        break;
      }
    }
  }
} /* DoFill */

/**
 * Return true if the specified table filler is done, i.e.
 * if it has no more lines to fill.
 * @param Filler    table filler to check if done
 * @return true if no more lines to fill, false otherwise.
 * @note Globals: none
 */
bool FillerDone(TABLE_FILLER *Filler) {
  FILL_SWITCH *Next;

  Next = &(Filler->Switch[Filler->NextSwitch]);

  return Filler->X > Next->X && Next->Type == LastSwitch;

} /* FillerDone */

/**
 * This routine sets Bit in each bit vector whose
 * bucket lies within the range Center +- Spread.  The fill
 * is done for a circular dimension, i.e. bucket 0 is adjacent
 * to the last bucket.  It is assumed that Center and Spread
 * are expressed in a circular coordinate system whose range
 * is 0 to 1.
 * @param ParamTable  table of bit vectors, one per param bucket
 * @param Bit bit position in vectors to be filled
 * @param Center center of filled area
 * @param Spread spread of filled area
 * @param debug debug flag
 */
void FillPPCircularBits(uint32_t ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR], int Bit,
                        float Center, float Spread, bool debug) {
  int i, FirstBucket, LastBucket;

  if (Spread > 0.5) {
    Spread = 0.5;
  }

  FirstBucket = static_cast<int>(std::floor((Center - Spread) * NUM_PP_BUCKETS));
  if (FirstBucket < 0) {
    FirstBucket += NUM_PP_BUCKETS;
  }

  LastBucket = static_cast<int>(std::floor((Center + Spread) * NUM_PP_BUCKETS));
  if (LastBucket >= NUM_PP_BUCKETS) {
    LastBucket -= NUM_PP_BUCKETS;
  }
  if (debug) {
    tprintf("Circular fill from %d to %d", FirstBucket, LastBucket);
  }
  for (i = FirstBucket; true; CircularIncrement(i, NUM_PP_BUCKETS)) {
    SET_BIT(ParamTable[i], Bit);

    /* exit loop after we have set the bit for the last bucket */
    if (i == LastBucket) {
      break;
    }
  }

} /* FillPPCircularBits */

/**
 * This routine sets Bit in each bit vector whose
 * bucket lies within the range Center +- Spread.  The fill
 * is done for a linear dimension, i.e. there is no wrap-around
 * for this dimension.  It is assumed that Center and Spread
 * are expressed in a linear coordinate system whose range
 * is approximately 0 to 1.  Values outside this range will
 * be clipped.
 * @param ParamTable table of bit vectors, one per param bucket
 * @param Bit bit number being filled
 * @param Center center of filled area
 * @param Spread spread of filled area
 * @param debug debug flag
 */
void FillPPLinearBits(uint32_t ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR], int Bit,
                      float Center, float Spread, bool debug) {
  int i, FirstBucket, LastBucket;

  FirstBucket = static_cast<int>(std::floor((Center - Spread) * NUM_PP_BUCKETS));
  if (FirstBucket < 0) {
    FirstBucket = 0;
  }

  LastBucket = static_cast<int>(std::floor((Center + Spread) * NUM_PP_BUCKETS));
  if (LastBucket >= NUM_PP_BUCKETS) {
    LastBucket = NUM_PP_BUCKETS - 1;
  }

  if (debug) {
    tprintf("Linear fill from %d to %d", FirstBucket, LastBucket);
  }
  for (i = FirstBucket; i <= LastBucket; i++) {
    SET_BIT(ParamTable[i], Bit);
  }

} /* FillPPLinearBits */

/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
/**
 * This routine prompts the user with Prompt and waits
 * for the user to enter something in the debug window.
 * @param Prompt prompt to print while waiting for input from window
 * @param adaptive_on
 * @param pretrained_on
 * @param shape_id
 * @return Character entered in the debug window.
 * @note Globals: none
 */
CLASS_ID Classify::GetClassToDebug(const char *Prompt, bool *adaptive_on, bool *pretrained_on,
                                   int *shape_id) {
  tprintf("%s\n", Prompt);
  SVEventType ev_type;
  int unichar_id = INVALID_UNICHAR_ID;
  // Wait until a click or popup event.
  do {
    auto ev = IntMatchWindow->AwaitEvent(SVET_ANY);
    ev_type = ev->type;
    if (ev_type == SVET_POPUP) {
      if (ev->command_id == IDA_SHAPE_INDEX) {
        if (shape_table_ != nullptr) {
          *shape_id = atoi(ev->parameter);
          *adaptive_on = false;
          *pretrained_on = true;
          if (*shape_id >= 0 && static_cast<unsigned>(*shape_id) < shape_table_->NumShapes()) {
            int font_id;
            shape_table_->GetFirstUnicharAndFont(*shape_id, &unichar_id, &font_id);
            tprintf("Shape %d, first unichar=%d, font=%d\n", *shape_id, unichar_id, font_id);
            return unichar_id;
          }
          tprintf("Shape index '%s' not found in shape table\n", ev->parameter);
        } else {
          tprintf("No shape table loaded!\n");
        }
      } else {
        if (unicharset.contains_unichar(ev->parameter)) {
          unichar_id = unicharset.unichar_to_id(ev->parameter);
          if (ev->command_id == IDA_ADAPTIVE) {
            *adaptive_on = true;
            *pretrained_on = false;
            *shape_id = -1;
          } else if (ev->command_id == IDA_STATIC) {
            *adaptive_on = false;
            *pretrained_on = true;
          } else {
            *adaptive_on = true;
            *pretrained_on = true;
          }
          if (ev->command_id == IDA_ADAPTIVE || shape_table_ == nullptr) {
            *shape_id = -1;
            return unichar_id;
          }
          for (unsigned s = 0; s < shape_table_->NumShapes(); ++s) {
            if (shape_table_->GetShape(s).ContainsUnichar(unichar_id)) {
              tprintf("%s\n", shape_table_->DebugStr(s).c_str());
            }
          }
        } else {
          tprintf("Char class '%s' not found in unicharset", ev->parameter);
        }
      }
    }
  } while (ev_type != SVET_CLICK);
  return 0;
} /* GetClassToDebug */

#endif

/**
 * This routine copies the appropriate global pad variables
 * into EndPad, SidePad, and AnglePad.  This is a kludge used
 * to get around the fact that global control variables cannot
 * be arrays.  If the specified level is illegal, the tightest
 * possible pads are returned.
 * @param Level   "tightness" level to return pads for
 * @param EndPad    place to put end pad for Level
 * @param SidePad   place to put side pad for Level
 * @param AnglePad  place to put angle pad for Level
 */
void GetCPPadsForLevel(int Level, float *EndPad, float *SidePad, float *AnglePad) {
  switch (Level) {
    case 0:
      *EndPad = classify_cp_end_pad_loose * GetPicoFeatureLength();
      *SidePad = classify_cp_side_pad_loose * GetPicoFeatureLength();
      *AnglePad = classify_cp_angle_pad_loose / 360.0;
      break;

    case 1:
      *EndPad = classify_cp_end_pad_medium * GetPicoFeatureLength();
      *SidePad = classify_cp_side_pad_medium * GetPicoFeatureLength();
      *AnglePad = classify_cp_angle_pad_medium / 360.0;
      break;

    case 2:
      *EndPad = classify_cp_end_pad_tight * GetPicoFeatureLength();
      *SidePad = classify_cp_side_pad_tight * GetPicoFeatureLength();
      *AnglePad = classify_cp_angle_pad_tight / 360.0;
      break;

    default:
      *EndPad = classify_cp_end_pad_tight * GetPicoFeatureLength();
      *SidePad = classify_cp_side_pad_tight * GetPicoFeatureLength();
      *AnglePad = classify_cp_angle_pad_tight / 360.0;
      break;
  }
  if (*AnglePad > 0.5) {
    *AnglePad = 0.5;
  }

} /* GetCPPadsForLevel */

/**
 * @param Evidence  evidence value to return color for
 * @return Color which corresponds to specified Evidence value.
 * @note Globals: none
 */
ScrollView::Color GetMatchColorFor(float Evidence) {
  assert(Evidence >= 0.0);
  assert(Evidence <= 1.0);

  if (Evidence >= 0.90) {
    return ScrollView::WHITE;
  } else if (Evidence >= 0.75) {
    return ScrollView::GREEN;
  } else if (Evidence >= 0.50) {
    return ScrollView::RED;
  } else {
    return ScrollView::BLUE;
  }
} /* GetMatchColorFor */

/**
 * This routine returns (in Fill) the specification of
 * the next line to be filled from Filler.  FillerDone() should
 * always be called before GetNextFill() to ensure that we
 * do not run past the end of the fill table.
 * @param Filler    filler to get next fill spec from
 * @param Fill    place to put spec for next fill
 */
void GetNextFill(TABLE_FILLER *Filler, FILL_SPEC *Fill) {
  FILL_SWITCH *Next;

  /* compute the fill assuming no switches will be encountered */
  Fill->AngleStart = Filler->AngleStart;
  Fill->AngleEnd = Filler->AngleEnd;
  Fill->X = Filler->X;
  Fill->YStart = Filler->YStart >> 8;
  Fill->YEnd = Filler->YEnd >> 8;

  /* update the fill info and the filler for ALL switches at this X value */
  Next = &(Filler->Switch[Filler->NextSwitch]);
  while (Filler->X >= Next->X) {
    Fill->X = Filler->X = Next->X;
    if (Next->Type == StartSwitch) {
      Fill->YStart = Next->Y;
      Filler->StartDelta = Next->Delta;
      Filler->YStart = Next->YInit;
    } else if (Next->Type == EndSwitch) {
      Fill->YEnd = Next->Y;
      Filler->EndDelta = Next->Delta;
      Filler->YEnd = Next->YInit;
    } else { /* Type must be LastSwitch */
      break;
    }
    Filler->NextSwitch++;
    Next = &(Filler->Switch[Filler->NextSwitch]);
  }

  /* prepare the filler for the next call to this routine */
  Filler->X++;
  Filler->YStart += Filler->StartDelta;
  Filler->YEnd += Filler->EndDelta;

} /* GetNextFill */

/**
 * This routine computes a data structure (Filler)
 * which can be used to fill in a rectangle surrounding
 * the specified Proto. Results are returned in Filler.
 *
 * @param EndPad, SidePad, AnglePad padding to add to proto
 * @param Proto       proto to create a filler for
 * @param Filler        place to put table filler
 */
void InitTableFiller(float EndPad, float SidePad, float AnglePad, PROTO_STRUCT *Proto, TABLE_FILLER *Filler)
#define XS X_SHIFT
#define YS Y_SHIFT
#define AS ANGLE_SHIFT
#define NB NUM_CP_BUCKETS
{
  float Angle;
  float X, Y, HalfLength;
  float Cos, Sin;
  float XAdjust, YAdjust;
  FPOINT Start, Switch1, Switch2, End;
  int S1 = 0;
  int S2 = 1;

  Angle = Proto->Angle;
  X = Proto->X;
  Y = Proto->Y;
  HalfLength = Proto->Length / 2.0;

  Filler->AngleStart = CircBucketFor(Angle - AnglePad, AS, NB);
  Filler->AngleEnd = CircBucketFor(Angle + AnglePad, AS, NB);
  Filler->NextSwitch = 0;

  if (fabs(Angle - 0.0) < HV_TOLERANCE || fabs(Angle - 0.5) < HV_TOLERANCE) {
    /* horizontal proto - handle as special case */
    Filler->X = Bucket8For(X - HalfLength - EndPad, XS, NB);
    Filler->YStart = Bucket16For(Y - SidePad, YS, NB * 256);
    Filler->YEnd = Bucket16For(Y + SidePad, YS, NB * 256);
    Filler->StartDelta = 0;
    Filler->EndDelta = 0;
    Filler->Switch[0].Type = LastSwitch;
    Filler->Switch[0].X = Bucket8For(X + HalfLength + EndPad, XS, NB);
  } else if (fabs(Angle - 0.25) < HV_TOLERANCE || fabs(Angle - 0.75) < HV_TOLERANCE) {
    /* vertical proto - handle as special case */
    Filler->X = Bucket8For(X - SidePad, XS, NB);
    Filler->YStart = Bucket16For(Y - HalfLength - EndPad, YS, NB * 256);
    Filler->YEnd = Bucket16For(Y + HalfLength + EndPad, YS, NB * 256);
    Filler->StartDelta = 0;
    Filler->EndDelta = 0;
    Filler->Switch[0].Type = LastSwitch;
    Filler->Switch[0].X = Bucket8For(X + SidePad, XS, NB);
  } else {
    /* diagonal proto */

    if ((Angle > 0.0 && Angle < 0.25) || (Angle > 0.5 && Angle < 0.75)) {
      /* rising diagonal proto */
      Angle *= 2.0 * M_PI;
      Cos = fabs(std::cos(Angle));
      Sin = fabs(std::sin(Angle));

      /* compute the positions of the corners of the acceptance region */
      Start.x = X - (HalfLength + EndPad) * Cos - SidePad * Sin;
      Start.y = Y - (HalfLength + EndPad) * Sin + SidePad * Cos;
      End.x = 2.0 * X - Start.x;
      End.y = 2.0 * Y - Start.y;
      Switch1.x = X - (HalfLength + EndPad) * Cos + SidePad * Sin;
      Switch1.y = Y - (HalfLength + EndPad) * Sin - SidePad * Cos;
      Switch2.x = 2.0 * X - Switch1.x;
      Switch2.y = 2.0 * Y - Switch1.y;

      if (Switch1.x > Switch2.x) {
        S1 = 1;
        S2 = 0;
      }

      /* translate into bucket positions and deltas */
      Filler->X = Bucket8For(Start.x, XS, NB);
      Filler->StartDelta = -static_cast<int16_t>((Cos / Sin) * 256);
      Filler->EndDelta = static_cast<int16_t>((Sin / Cos) * 256);

      XAdjust = BucketEnd(Filler->X, XS, NB) - Start.x;
      YAdjust = XAdjust * Cos / Sin;
      Filler->YStart = Bucket16For(Start.y - YAdjust, YS, NB * 256);
      YAdjust = XAdjust * Sin / Cos;
      Filler->YEnd = Bucket16For(Start.y + YAdjust, YS, NB * 256);

      Filler->Switch[S1].Type = StartSwitch;
      Filler->Switch[S1].X = Bucket8For(Switch1.x, XS, NB);
      Filler->Switch[S1].Y = Bucket8For(Switch1.y, YS, NB);
      XAdjust = Switch1.x - BucketStart(Filler->Switch[S1].X, XS, NB);
      YAdjust = XAdjust * Sin / Cos;
      Filler->Switch[S1].YInit = Bucket16For(Switch1.y - YAdjust, YS, NB * 256);
      Filler->Switch[S1].Delta = Filler->EndDelta;

      Filler->Switch[S2].Type = EndSwitch;
      Filler->Switch[S2].X = Bucket8For(Switch2.x, XS, NB);
      Filler->Switch[S2].Y = Bucket8For(Switch2.y, YS, NB);
      XAdjust = Switch2.x - BucketStart(Filler->Switch[S2].X, XS, NB);
      YAdjust = XAdjust * Cos / Sin;
      Filler->Switch[S2].YInit = Bucket16For(Switch2.y + YAdjust, YS, NB * 256);
      Filler->Switch[S2].Delta = Filler->StartDelta;

      Filler->Switch[2].Type = LastSwitch;
      Filler->Switch[2].X = Bucket8For(End.x, XS, NB);
    } else {
      /* falling diagonal proto */
      Angle *= 2.0 * M_PI;
      Cos = fabs(std::cos(Angle));
      Sin = fabs(std::sin(Angle));

      /* compute the positions of the corners of the acceptance region */
      Start.x = X - (HalfLength + EndPad) * Cos - SidePad * Sin;
      Start.y = Y + (HalfLength + EndPad) * Sin - SidePad * Cos;
      End.x = 2.0 * X - Start.x;
      End.y = 2.0 * Y - Start.y;
      Switch1.x = X - (HalfLength + EndPad) * Cos + SidePad * Sin;
      Switch1.y = Y + (HalfLength + EndPad) * Sin + SidePad * Cos;
      Switch2.x = 2.0 * X - Switch1.x;
      Switch2.y = 2.0 * Y - Switch1.y;

      if (Switch1.x > Switch2.x) {
        S1 = 1;
        S2 = 0;
      }

      /* translate into bucket positions and deltas */
      Filler->X = Bucket8For(Start.x, XS, NB);
      Filler->StartDelta = static_cast<int16_t>(
          ClipToRange<int>(-IntCastRounded((Sin / Cos) * 256), INT16_MIN, INT16_MAX));
      Filler->EndDelta = static_cast<int16_t>(
          ClipToRange<int>(IntCastRounded((Cos / Sin) * 256), INT16_MIN, INT16_MAX));

      XAdjust = BucketEnd(Filler->X, XS, NB) - Start.x;
      YAdjust = XAdjust * Sin / Cos;
      Filler->YStart = Bucket16For(Start.y - YAdjust, YS, NB * 256);
      YAdjust = XAdjust * Cos / Sin;
      Filler->YEnd = Bucket16For(Start.y + YAdjust, YS, NB * 256);

      Filler->Switch[S1].Type = EndSwitch;
      Filler->Switch[S1].X = Bucket8For(Switch1.x, XS, NB);
      Filler->Switch[S1].Y = Bucket8For(Switch1.y, YS, NB);
      XAdjust = Switch1.x - BucketStart(Filler->Switch[S1].X, XS, NB);
      YAdjust = XAdjust * Sin / Cos;
      Filler->Switch[S1].YInit = Bucket16For(Switch1.y + YAdjust, YS, NB * 256);
      Filler->Switch[S1].Delta = Filler->StartDelta;

      Filler->Switch[S2].Type = StartSwitch;
      Filler->Switch[S2].X = Bucket8For(Switch2.x, XS, NB);
      Filler->Switch[S2].Y = Bucket8For(Switch2.y, YS, NB);
      XAdjust = Switch2.x - BucketStart(Filler->Switch[S2].X, XS, NB);
      YAdjust = XAdjust * Cos / Sin;
      Filler->Switch[S2].YInit = Bucket16For(Switch2.y - YAdjust, YS, NB * 256);
      Filler->Switch[S2].Delta = Filler->EndDelta;

      Filler->Switch[2].Type = LastSwitch;
      Filler->Switch[2].X = Bucket8For(End.x, XS, NB);
    }
  }
} /* InitTableFiller */

/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
/**
 * This routine renders the specified feature into ShapeList.
 * @param window to add feature rendering to
 * @param Feature feature to be rendered
 * @param color color to use for feature rendering
 * @return New shape list with rendering of Feature added.
 * @note Globals: none
 */
void RenderIntFeature(ScrollView *window, const INT_FEATURE_STRUCT *Feature,
                      ScrollView::Color color) {
  float X, Y, Dx, Dy, Length;

  window->Pen(color);
  assert(Feature != nullptr);
  assert(color != 0);

  X = Feature->X;
  Y = Feature->Y;
  Length = GetPicoFeatureLength() * 0.7 * INT_CHAR_NORM_RANGE;
  // The -PI has no significant effect here, but the value of Theta is computed
  // using BinaryAnglePlusPi in intfx.cpp.
  Dx = (Length / 2.0) * cos((Feature->Theta / 256.0) * 2.0 * M_PI - M_PI);
  Dy = (Length / 2.0) * sin((Feature->Theta / 256.0) * 2.0 * M_PI - M_PI);

  window->SetCursor(X, Y);
  window->DrawTo(X + Dx, Y + Dy);
} /* RenderIntFeature */

/**
 * This routine extracts the parameters of the specified
 * proto from the class description and adds a rendering of
 * the proto onto the ShapeList.
 *
 * @param window ScrollView instance
 * @param Class class that proto is contained in
 * @param ProtoId id of proto to be rendered
 * @param color color to render proto in
 *
 * Globals: none
 *
 * @return New shape list with a rendering of one proto added.
 */
void RenderIntProto(ScrollView *window, INT_CLASS_STRUCT *Class, PROTO_ID ProtoId,
                    ScrollView::Color color) {
  INT_PROTO_STRUCT *Proto;
  int ProtoSetIndex;
  int ProtoWordIndex;
  float Length;
  int Xmin, Xmax, Ymin, Ymax;
  float X, Y, Dx, Dy;
  uint32_t ProtoMask;
  int Bucket;

  assert(ProtoId >= 0);
  assert(Class != nullptr);
  assert(ProtoId < Class->NumProtos);
  assert(color != 0);
  window->Pen(color);

  auto ProtoSet = Class->ProtoSets[SetForProto(ProtoId)];
  ProtoSetIndex = IndexForProto(ProtoId);
  Proto = &(ProtoSet->Protos[ProtoSetIndex]);
  Length = (Class->ProtoLengths[ProtoId] * GetPicoFeatureLength() * INT_CHAR_NORM_RANGE);
  ProtoMask = PPrunerMaskFor(ProtoId);
  ProtoWordIndex = PPrunerWordIndexFor(ProtoId);

  // find the x and y extent of the proto from the proto pruning table
  Xmin = Ymin = NUM_PP_BUCKETS;
  Xmax = Ymax = 0;
  for (Bucket = 0; Bucket < NUM_PP_BUCKETS; Bucket++) {
    if (ProtoMask & ProtoSet->ProtoPruner[PRUNER_X][Bucket][ProtoWordIndex]) {
      UpdateRange(Bucket, &Xmin, &Xmax);
    }

    if (ProtoMask & ProtoSet->ProtoPruner[PRUNER_Y][Bucket][ProtoWordIndex]) {
      UpdateRange(Bucket, &Ymin, &Ymax);
    }
  }
  X = (Xmin + Xmax + 1) / 2.0 * PROTO_PRUNER_SCALE;
  Y = (Ymin + Ymax + 1) / 2.0 * PROTO_PRUNER_SCALE;
  // The -PI has no significant effect here, but the value of Theta is computed
  // using BinaryAnglePlusPi in intfx.cpp.
  Dx = (Length / 2.0) * cos((Proto->Angle / 256.0) * 2.0 * M_PI - M_PI);
  Dy = (Length / 2.0) * sin((Proto->Angle / 256.0) * 2.0 * M_PI - M_PI);

  window->SetCursor(X - Dx, Y - Dy);
  window->DrawTo(X + Dx, Y + Dy);
} /* RenderIntProto */
#endif

#ifndef GRAPHICS_DISABLED
/**
 * Initializes the int matcher window if it is not already
 * initialized.
 */
void InitIntMatchWindowIfReqd() {
  if (IntMatchWindow == nullptr) {
    IntMatchWindow = CreateFeatureSpaceWindow("IntMatchWindow", 50, 200);
    auto *popup_menu = new SVMenuNode();

    popup_menu->AddChild("Debug Adapted classes", IDA_ADAPTIVE, "x", "Class to debug");
    popup_menu->AddChild("Debug Static classes", IDA_STATIC, "x", "Class to debug");
    popup_menu->AddChild("Debug Both", IDA_BOTH, "x", "Class to debug");
    popup_menu->AddChild("Debug Shape Index", IDA_SHAPE_INDEX, "0", "Index to debug");
    popup_menu->BuildMenu(IntMatchWindow, false);
  }
}

/**
 * Initializes the proto display window if it is not already
 * initialized.
 */
void InitProtoDisplayWindowIfReqd() {
  if (ProtoDisplayWindow == nullptr) {
    ProtoDisplayWindow = CreateFeatureSpaceWindow("ProtoDisplayWindow", 550, 200);
  }
}

/**
 * Initializes the feature display window if it is not already
 * initialized.
 */
void InitFeatureDisplayWindowIfReqd() {
  if (FeatureDisplayWindow == nullptr) {
    FeatureDisplayWindow = CreateFeatureSpaceWindow("FeatureDisplayWindow", 50, 700);
  }
}

/// Creates a window of the appropriate size for displaying elements
/// in feature space.
ScrollView *CreateFeatureSpaceWindow(const char *name, int xpos, int ypos) {
  return new ScrollView(name, xpos, ypos, 520, 520, 260, 260, true);
}
#endif // !GRAPHICS_DISABLED

} // namespace tesseract
