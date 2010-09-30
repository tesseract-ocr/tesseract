/******************************************************************************
 ** Filename:    intproto.c
 ** Purpose:     Definition of data structures for integer protos.
 ** Author:      Dan Johnson
 ** History:     Thu Feb  7 14:38:16 1991, DSJ, Created.
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
#include "helpers.h"
#include "intproto.h"
#include "picofeat.h"
#include "mfoutline.h"
#include "emalloc.h"
#include "const.h"
#include "ndminx.h"
#include "svmnode.h"
#include "adaptmatch.h"
#include "globals.h"
#include "classify.h"
#include "genericvector.h"

//extern GetPicoFeatureLength();

#include <math.h>
#include <stdio.h>
#include <assert.h>
#ifdef __UNIX__
#include <unistd.h>
#endif

// Include automatically generated configuration file if running autoconf.
#ifdef HAVE_CONFIG_H
#include "config_auto.h"
#endif

/* match debug display constants*/
#define DISPLAY_OFFSET  (0.5  * INT_CHAR_NORM_RANGE)
#define PROTO_PRUNER_SCALE  (4.0)

#define INT_DESCENDER (0.0  * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)
#define INT_BASELINE  (0.25 * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)
#define INT_XHEIGHT (0.75 * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)
#define INT_CAPHEIGHT (1.0  * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)

#define INT_XCENTER (0.5  * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)
#define INT_YCENTER (0.5  * INT_CHAR_NORM_RANGE - DISPLAY_OFFSET)
#define INT_XRADIUS (0.2  * INT_CHAR_NORM_RANGE)
#define INT_YRADIUS (0.2  * INT_CHAR_NORM_RANGE)
#define INT_MIN_X (- DISPLAY_OFFSET)
#define INT_MIN_Y (- DISPLAY_OFFSET)
#define INT_MAX_X (  DISPLAY_OFFSET)
#define INT_MAX_Y (  DISPLAY_OFFSET)
#define DOUBLE_OFFSET 0.095

/** define pad used to snap near horiz/vertical protos to horiz/vertical */
#define HV_TOLERANCE  (0.0025)   /* approx 0.9 degrees */

typedef enum
{ StartSwitch, EndSwitch, LastSwitch }
SWITCH_TYPE;
#define MAX_NUM_SWITCHES  3

typedef struct
{
  SWITCH_TYPE Type;
  inT8 X, Y;
  inT16 YInit;
  inT16 Delta;
}


FILL_SWITCH;

typedef struct
{
  uinT8 NextSwitch;
  uinT8 AngleStart, AngleEnd;
  inT8 X;
  inT16 YStart, YEnd;
  inT16 StartDelta, EndDelta;
  FILL_SWITCH Switch[MAX_NUM_SWITCHES];
}


TABLE_FILLER;

typedef struct
{
  inT8 X;
  inT8 YStart, YEnd;
  uinT8 AngleStart, AngleEnd;
}


FILL_SPEC;


/* constants for conversion from old inttemp format */
#define OLD_MAX_NUM_CONFIGS      32
#define OLD_WERDS_PER_CONFIG_VEC ((OLD_MAX_NUM_CONFIGS + BITS_PER_WERD - 1) /\
                                  BITS_PER_WERD)

/*-----------------------------------------------------------------------------
            Macros
-----------------------------------------------------------------------------*/
/** macro for performing circular increments of bucket indices */
#define CircularIncrement(i,r)  (((i) < (r) - 1)?((i)++):((i) = 0))

/** macro for mapping floats to ints without bounds checking */
#define MapParam(P,O,N)   (floor (((P) + (O)) * (N)))

/*---------------------------------------------------------------------------
            Private Function Prototypes
----------------------------------------------------------------------------*/
FLOAT32 BucketStart(int Bucket, FLOAT32 Offset, int NumBuckets);

FLOAT32 BucketEnd(int Bucket, FLOAT32 Offset, int NumBuckets);

void DoFill(FILL_SPEC *FillSpec,
            CLASS_PRUNER Pruner,
            register uinT32 ClassMask,
            register uinT32 ClassCount,
            register uinT32 WordIndex);

BOOL8 FillerDone(TABLE_FILLER *Filler);

void FillPPCircularBits (uinT32
                         ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR],
                         int Bit, FLOAT32 Center, FLOAT32 Spread);

void FillPPLinearBits (uinT32 ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR],
                       int Bit, FLOAT32 Center, FLOAT32 Spread);

#ifndef GRAPHICS_DISABLED
CLASS_ID GetClassToDebug(const char *Prompt);
#endif

void GetCPPadsForLevel(int Level,
                       FLOAT32 *EndPad,
                       FLOAT32 *SidePad,
                       FLOAT32 *AnglePad);

C_COL GetMatchColorFor(FLOAT32 Evidence);

void GetNextFill(TABLE_FILLER *Filler, FILL_SPEC *Fill);

void InitTableFiller(FLOAT32 EndPad,
                     FLOAT32 SidePad,
                     FLOAT32 AnglePad,
                     PROTO Proto,
                     TABLE_FILLER *Filler);

#ifndef GRAPHICS_DISABLED
void RenderIntFeature(void *window, INT_FEATURE Feature, C_COL Color);

void RenderIntProto(void *window,
                    INT_CLASS Class,
                    PROTO_ID ProtoId,
                    C_COL Color);
#endif

int TruncateParam(FLOAT32 Param, int Min, int Max, char *Id);

/*-----------------------------------------------------------------------------
        Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/

/* global display lists used to display proto and feature match information*/
ScrollView *IntMatchWindow = NULL;
ScrollView *FeatureDisplayWindow = NULL;
ScrollView *ProtoDisplayWindow = NULL;

/*-----------------------------------------------------------------------------
        Variables
-----------------------------------------------------------------------------*/

/* control knobs */
INT_VAR(classify_num_cp_levels, 3, "Number of Class Pruner Levels");
double_VAR(classify_cp_angle_pad_loose, 45.0,
           "Class Pruner Angle Pad Loose");
double_VAR(classify_cp_angle_pad_medium, 20.0,
           "Class Pruner Angle Pad Medium");
double_VAR(classify_cp_angle_pad_tight, 10.0,
           "CLass Pruner Angle Pad Tight");
double_VAR(classify_cp_end_pad_loose, 0.5, "Class Pruner End Pad Loose");
double_VAR(classify_cp_end_pad_medium, 0.5, "Class Pruner End Pad Medium");
double_VAR(classify_cp_end_pad_tight, 0.5, "Class Pruner End Pad Tight");
double_VAR(classify_cp_side_pad_loose, 2.5, "Class Pruner Side Pad Loose");
double_VAR(classify_cp_side_pad_medium, 1.2, "Class Pruner Side Pad Medium");
double_VAR(classify_cp_side_pad_tight, 0.6, "Class Pruner Side Pad Tight");
double_VAR(classify_pp_angle_pad, 45.0, "Proto Pruner Angle Pad");
double_VAR(classify_pp_end_pad, 0.5, "Proto Prune End Pad");
double_VAR(classify_pp_side_pad, 2.5, "Proto Pruner Side Pad");

/*-----------------------------------------------------------------------------
              Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
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
 *
 * @note Exceptions: none
 * @note History: Mon Feb 11 11:52:08 1991, DSJ, Created.
 */
void AddIntClass(INT_TEMPLATES Templates, CLASS_ID ClassId, INT_CLASS Class) {
  int Pruner;
  uinT32 *Word;

  assert (LegalClassId (ClassId));
  if (ClassId != Templates->NumClasses) {
    fprintf(stderr, "Please make sure that classes are added to templates");
    fprintf(stderr, " in increasing order of ClassIds\n");
    exit(1);
  }
  ClassForClassId (Templates, ClassId) = Class;
  Templates->NumClasses++;

  if (Templates->NumClasses > MaxNumClassesIn (Templates)) {
    Pruner = Templates->NumClassPruners++;
    Templates->ClassPruner[Pruner] =
      (CLASS_PRUNER) Emalloc (sizeof (CLASS_PRUNER_STRUCT));

    for (Word = reinterpret_cast<uinT32*>(Templates->ClassPruner[Pruner]);
         Word < reinterpret_cast<uinT32*>(Templates->ClassPruner[Pruner]) +
                WERDS_PER_CP;
         *Word++ = 0);
  }
}                                /* AddIntClass */


/*---------------------------------------------------------------------------*/
/**
 * This routine returns the index of the next free config
 * in Class.
 *
 * @param Class class to add new configuration to
 *
 * Globals: none
 *
 * @return Index of next free config.
 * @note Exceptions: none
 * @note History: Mon Feb 11 14:44:40 1991, DSJ, Created.
 */
int AddIntConfig(INT_CLASS Class) {
  int Index;

  assert(Class->NumConfigs < MAX_NUM_CONFIGS);

  Index = Class->NumConfigs++;
  Class->ConfigLengths[Index] = 0;
  return Index;
}                                /* AddIntConfig */


/*---------------------------------------------------------------------------*/
/**
 * This routine allocates the next free proto in Class and
 * returns its index.
 *
 * @param Class class to add new proto to
 *
 * Globals: none
 *
 * @return Proto index of new proto.
 * @note Exceptions: none
 * @note History: Mon Feb 11 13:26:41 1991, DSJ, Created.
 */
int AddIntProto(INT_CLASS Class) {
  int Index;
  int ProtoSetId;
  PROTO_SET ProtoSet;
  INT_PROTO Proto;
  register uinT32 *Word;

  if (Class->NumProtos >= MAX_NUM_PROTOS)
    return (NO_PROTO);

  Index = Class->NumProtos++;

  if (Class->NumProtos > MaxNumIntProtosIn(Class)) {
    ProtoSetId = Class->NumProtoSets++;

    ProtoSet = (PROTO_SET) Emalloc(sizeof(PROTO_SET_STRUCT));
    Class->ProtoSets[ProtoSetId] = ProtoSet;
    for (Word = reinterpret_cast<uinT32*>(ProtoSet->ProtoPruner);
         Word < reinterpret_cast<uinT32*>(ProtoSet->ProtoPruner) + WERDS_PER_PP;
         *Word++ = 0);

    /* reallocate space for the proto lengths and install in class */
    Class->ProtoLengths =
      (uinT8 *)Erealloc(Class->ProtoLengths,
                        MaxNumIntProtosIn(Class) * sizeof(uinT8));
  }

  /* initialize proto so its length is zero and it isn't in any configs */
  Class->ProtoLengths[Index] = 0;
  Proto = ProtoForProtoId (Class, Index);
  for (Word = Proto->Configs;
       Word < Proto->Configs + WERDS_PER_CONFIG_VEC; *Word++ = 0);

  return (Index);

}                                /* AddIntProto */


/*---------------------------------------------------------------------------*/
void AddProtoToClassPruner (PROTO Proto, CLASS_ID ClassId,
                            INT_TEMPLATES Templates)
/*
 ** Parameters:
 **   Proto   floating-pt proto to add to class pruner
 **   ClassId   class id corresponding to Proto
 **   Templates set of templates containing class pruner
 ** Globals:
 **   classify_num_cp_levels number of levels used in the class pruner
 ** Operation: This routine adds Proto to the class pruning tables
 **   for the specified class in Templates.
 ** Return: none
 ** Exceptions: none
 ** History: Wed Feb 13 08:49:54 1991, DSJ, Created.
 */
#define MAX_LEVEL     2
{
  CLASS_PRUNER Pruner;
  uinT32 ClassMask;
  uinT32 ClassCount;
  uinT32 WordIndex;
  int Level;
  FLOAT32 EndPad, SidePad, AnglePad;
  TABLE_FILLER TableFiller;
  FILL_SPEC FillSpec;

  Pruner = CPrunerFor (Templates, ClassId);
  WordIndex = CPrunerWordIndexFor (ClassId);
  ClassMask = CPrunerMaskFor (MAX_LEVEL, ClassId);

  for (Level = classify_num_cp_levels - 1; Level >= 0; Level--) {
    GetCPPadsForLevel(Level, &EndPad, &SidePad, &AnglePad);
    ClassCount = CPrunerMaskFor (Level, ClassId);
    InitTableFiller(EndPad, SidePad, AnglePad, Proto, &TableFiller);

    while (!FillerDone (&TableFiller)) {
      GetNextFill(&TableFiller, &FillSpec);
      DoFill(&FillSpec, Pruner, ClassMask, ClassCount, WordIndex);
    }
  }
}                                /* AddProtoToClassPruner */


/*---------------------------------------------------------------------------*/
void AddProtoToProtoPruner(PROTO Proto, int ProtoId, INT_CLASS Class) {
/*
 ** Parameters:
 **   Proto floating-pt proto to be added to proto pruner
 **   ProtoId id of proto
 **   Class integer class that contains desired proto pruner
 ** Globals: none
 ** Operation: This routine updates the proto pruner lookup tables
 **   for Class to include a new proto identified by ProtoId
 **   and described by Proto.
 ** Return: none
 ** Exceptions: none
 ** History: Fri Feb  8 13:07:19 1991, DSJ, Created.
 */
  FLOAT32 Angle, X, Y, Length;
  FLOAT32 Pad;
  int Index;
  PROTO_SET ProtoSet;

  if (ProtoId >= Class->NumProtos)
    cprintf("AddProtoToProtoPruner:assert failed: %d < %d",
            ProtoId, Class->NumProtos);
  assert(ProtoId < Class->NumProtos);

  Index = IndexForProto (ProtoId);
  ProtoSet = Class->ProtoSets[SetForProto (ProtoId)];

  Angle = Proto->Angle;
#ifndef __MSW32__
  assert(!isnan(Angle));
#endif

  FillPPCircularBits (ProtoSet->ProtoPruner[PRUNER_ANGLE], Index,
                      Angle + ANGLE_SHIFT, classify_pp_angle_pad / 360.0);

  Angle *= 2.0 * PI;
  Length = Proto->Length;

  X = Proto->X + X_SHIFT;
  Pad = MAX (fabs (cos (Angle)) * (Length / 2.0 +
                                   classify_pp_end_pad *
                                   GetPicoFeatureLength ()),
             fabs (sin (Angle)) * (classify_pp_side_pad *
                                   GetPicoFeatureLength ()));

  FillPPLinearBits (ProtoSet->ProtoPruner[PRUNER_X], Index, X, Pad);

  Y = Proto->Y + Y_SHIFT;
  Pad = MAX (fabs (sin (Angle)) * (Length / 2.0 +
                                   classify_pp_end_pad *
                                   GetPicoFeatureLength ()),
             fabs (cos (Angle)) * (classify_pp_side_pad *
                                   GetPicoFeatureLength ()));

  FillPPLinearBits(ProtoSet->ProtoPruner[PRUNER_Y], Index, Y, Pad);
}                                /* AddProtoToProtoPruner */


/*---------------------------------------------------------------------------*/
int BucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets) {
/*
 ** Parameters:
 **   Param   parameter value to map into a bucket number
 **   Offset    amount to shift param before mapping it
 **   NumBuckets  number of buckets to map param into
 ** Globals: none
 ** Operation: This routine maps a parameter value into a bucket between
 **   0 and NumBuckets-1.  Offset is added to the parameter
 **   before mapping it.  Values which map to buckets outside
 **   the range are truncated to fit within the range.  Mapping
 **   is done by truncating rather than rounding.
 ** Return: Bucket number corresponding to Param + Offset.
 ** Exceptions: none
 ** History: Thu Feb 14 13:24:33 1991, DSJ, Created.
 */
  return ClipToRange(static_cast<int>(MapParam(Param, Offset, NumBuckets)),
                     0, NumBuckets - 1);
}                                /* BucketFor */


/*---------------------------------------------------------------------------*/
int CircBucketFor(FLOAT32 Param, FLOAT32 Offset, int NumBuckets) {
/*
 ** Parameters:
 **   Param   parameter value to map into a circular bucket
 **   Offset    amount to shift param before mapping it
 **   NumBuckets  number of buckets to map param into
 ** Globals: none
 ** Operation: This routine maps a parameter value into a bucket between
 **   0 and NumBuckets-1.  Offset is added to the parameter
 **   before mapping it.  Values which map to buckets outside
 **   the range are wrapped to a new value in a circular fashion.
 **   Mapping is done by truncating rather than rounding.
 ** Return: Bucket number corresponding to Param + Offset.
 ** Exceptions: none
 ** History: Thu Feb 14 13:24:33 1991, DSJ, Created.
 */
  int Bucket;

  Bucket = static_cast<int>(MapParam(Param, Offset, NumBuckets));
  if (Bucket < 0)
    Bucket += NumBuckets;
  else if (Bucket >= NumBuckets)
    Bucket -= NumBuckets;
  return Bucket;
}                                /* CircBucketFor */


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void UpdateMatchDisplay() {
/*
 ** Parameters: none
 ** Globals:
 **   FeatureShapes display list for features
 **   ProtoShapes display list for protos
 ** Operation: This routine clears the global feature and proto
 **   display lists.
 ** Return: none
 ** Exceptions: none
 ** History: Thu Mar 21 15:40:19 1991, DSJ, Created.
 */
  if (IntMatchWindow != NULL)
    c_make_current(IntMatchWindow);
}                                /* ClearMatchDisplay */
#endif

/*---------------------------------------------------------------------------*/
void ConvertConfig(BIT_VECTOR Config, int ConfigId, INT_CLASS Class) {
/*
 ** Parameters:
 **   Config    config to be added to class
 **   ConfigId  id to be used for new config
 **   Class   class to add new config to
 ** Globals: none
 ** Operation: This operation updates the config vectors of all protos
 **   in Class to indicate that the protos with 1's in Config
 **   belong to a new configuration identified by ConfigId.
 **   It is assumed that the length of the Config bit vector is
 **   equal to the number of protos in Class.
 ** Return: none
 ** Exceptions: none
 ** History: Mon Feb 11 14:57:31 1991, DSJ, Created.
 */
  int ProtoId;
  INT_PROTO Proto;
  int TotalLength;

  for (ProtoId = 0, TotalLength = 0;
    ProtoId < Class->NumProtos; ProtoId++) {
    if (test_bit(Config, ProtoId)) {
      Proto = ProtoForProtoId(Class, ProtoId);
      SET_BIT(Proto->Configs, ConfigId);
      TotalLength += Class->ProtoLengths[ProtoId];
    }
  }
  Class->ConfigLengths[ConfigId] = TotalLength;
}                                /* ConvertConfig */


/*---------------------------------------------------------------------------*/
void ConvertProto(PROTO Proto, int ProtoId, INT_CLASS Class) {
/*
 ** Parameters:
 **   Proto floating-pt proto to be converted to integer format
 **   ProtoId id of proto
 **   Class integer class to add converted proto to
 ** Globals: none
 ** Operation: This routine converts Proto to integer format and
 **   installs it as ProtoId in Class.
 ** Return: none
 ** Exceptions: none
 ** History: Fri Feb  8 11:22:43 1991, DSJ, Created.
 */
  INT_PROTO P;
  FLOAT32 Param;

  assert(ProtoId < Class->NumProtos);

  P = ProtoForProtoId(Class, ProtoId);

  Param = Proto->A * 128;
  P->A = TruncateParam(Param, -128, 127, NULL);

  Param = -Proto->B * 256;
  P->B = TruncateParam(Param, 0, 255, NULL);

  Param = Proto->C * 128;
  P->C = TruncateParam(Param, -128, 127, NULL);

  Param = Proto->Angle * 256;
  if (Param < 0 || Param >= 256)
    P->Angle = 0;
  else
    P->Angle = (uinT8) Param;

  /* round proto length to nearest integer number of pico-features */
  Param = (Proto->Length / GetPicoFeatureLength()) + 0.5;
  Class->ProtoLengths[ProtoId] = TruncateParam(Param, 1, 255, NULL);
  if (classify_learning_debug_level >= 2)
    cprintf("Converted ffeat to (A=%d,B=%d,C=%d,L=%d)",
            P->A, P->B, P->C, Class->ProtoLengths[ProtoId]);
}                                /* ConvertProto */


/*---------------------------------------------------------------------------*/
namespace tesseract {
INT_TEMPLATES Classify::CreateIntTemplates(CLASSES FloatProtos,
                                           const UNICHARSET&
                                           target_unicharset) {
/*
 ** Parameters:
 **   FloatProtos prototypes in old floating pt format
 ** Globals: none
 ** Operation: This routine converts from the old floating point format
 **   to the new integer format.
 ** Return: New set of training templates in integer format.
 ** Exceptions: none
 ** History: Thu Feb  7 14:40:42 1991, DSJ, Created.
 */
  INT_TEMPLATES IntTemplates;
  CLASS_TYPE FClass;
  INT_CLASS IClass;
  int ClassId;
  int ProtoId;
  int ConfigId;

  IntTemplates = NewIntTemplates();

  for (ClassId = 0; ClassId < target_unicharset.size(); ClassId++) {
    FClass = &(FloatProtos[ClassId]);
    if (FClass->NumProtos == 0 && FClass->NumConfigs == 0 &&
        strcmp(target_unicharset.id_to_unichar(ClassId), " ") != 0) {
      cprintf("Warning: no protos/configs for %s in CreateIntTemplates()\n",
              target_unicharset.id_to_unichar(ClassId));
    }
    assert(UnusedClassIdIn(IntTemplates, ClassId));
    IClass = NewIntClass(FClass->NumProtos, FClass->NumConfigs);
    FontSet fs;
    fs.size = FClass->font_set.size();
    fs.configs = new int[fs.size];
    for (int i = 0; i < fs.size; ++i) {
      fs.configs[i] = FClass->font_set.get(i);
    }
    if (this->fontset_table_.contains(fs)) {
      IClass->font_set_id = this->fontset_table_.get_id(fs);
      delete[] fs.configs;
    } else {
      IClass->font_set_id = this->fontset_table_.push_back(fs);
    }
    AddIntClass(IntTemplates, ClassId, IClass);

    for (ProtoId = 0; ProtoId < FClass->NumProtos; ProtoId++) {
      AddIntProto(IClass);
      ConvertProto(ProtoIn(FClass, ProtoId), ProtoId, IClass);
      AddProtoToProtoPruner(ProtoIn(FClass, ProtoId), ProtoId, IClass);
      AddProtoToClassPruner(ProtoIn(FClass, ProtoId), ClassId, IntTemplates);
    }

    for (ConfigId = 0; ConfigId < FClass->NumConfigs; ConfigId++) {
      AddIntConfig(IClass);
      ConvertConfig(FClass->Configurations[ConfigId], ConfigId, IClass);
    }
  }
  return (IntTemplates);
}                                /* CreateIntTemplates */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void DisplayIntFeature(INT_FEATURE Feature, FLOAT32 Evidence) {
/*
 ** Parameters:
 **   Feature   pico-feature to be displayed
 **   Evidence  best evidence for this feature (0-1)
 ** Globals:
 **   FeatureShapes global display list for features
 ** Operation: This routine renders the specified feature into a
 **   global display list.
 ** Return: none
 ** Exceptions: none
 ** History: Thu Mar 21 14:45:04 1991, DSJ, Created.
 */
  C_COL Color;

  Color = GetMatchColorFor(Evidence);
  RenderIntFeature(IntMatchWindow, Feature, Color);
  if (FeatureDisplayWindow) {
    RenderIntFeature(FeatureDisplayWindow, Feature, Color);
  }
}                                /* DisplayIntFeature */


/*---------------------------------------------------------------------------*/
void DisplayIntProto(INT_CLASS Class, PROTO_ID ProtoId, FLOAT32 Evidence) {
/*
 ** Parameters:
 **   Class   class to take proto from
 **   ProtoId   id of proto in Class to be displayed
 **   Evidence  total evidence for proto (0-1)
 ** Globals:
 **   ProtoShapes global display list for protos
 ** Operation: This routine renders the specified proto into a
 **   global display list.
 ** Return: none
 ** Exceptions: none
 ** History: Thu Mar 21 14:45:04 1991, DSJ, Created.
 */
  C_COL Color;

  Color = GetMatchColorFor(Evidence);
  RenderIntProto(IntMatchWindow, Class, ProtoId, Color);
  if (ProtoDisplayWindow) {
    RenderIntProto(ProtoDisplayWindow, Class, ProtoId, Color);
  }
}                                /* DisplayIntProto */
#endif

/*---------------------------------------------------------------------------*/
INT_CLASS NewIntClass(int MaxNumProtos, int MaxNumConfigs) {
/*
 ** Parameters:
 **   MaxNumProtos  number of protos to allocate space for
 **   MaxNumConfigs number of configs to allocate space for
 ** Globals: none
 ** Operation: This routine creates a new integer class data structure
 **   and returns it.  Sufficient space is allocated
 **   to handle the specified number of protos and configs.
 ** Return: New class created.
 ** Exceptions: none
 ** History: Fri Feb  8 10:51:23 1991, DSJ, Created.
 */
  INT_CLASS Class;
  PROTO_SET ProtoSet;
  int i;
  register uinT32 *Word;

  assert(MaxNumConfigs <= MAX_NUM_CONFIGS);

  Class = (INT_CLASS) Emalloc(sizeof(INT_CLASS_STRUCT));
  Class->NumProtoSets = ((MaxNumProtos + PROTOS_PER_PROTO_SET - 1) /
                            PROTOS_PER_PROTO_SET);

  assert(Class->NumProtoSets <= MAX_NUM_PROTO_SETS);

  Class->NumProtos = 0;
  Class->NumConfigs = 0;

  for (i = 0; i < Class->NumProtoSets; i++) {
    /* allocate space for a proto set, install in class, and initialize */
    ProtoSet = (PROTO_SET) Emalloc(sizeof(PROTO_SET_STRUCT));
    Class->ProtoSets[i] = ProtoSet;
    for (Word = reinterpret_cast<uinT32*>(ProtoSet->ProtoPruner);
         Word < reinterpret_cast<uinT32*>(ProtoSet->ProtoPruner) + WERDS_PER_PP;
         *Word++ = 0);

    /* allocate space for the proto lengths and install in class */
  }
  if (MaxNumIntProtosIn (Class) > 0) {
    Class->ProtoLengths =
      (uinT8 *)Emalloc(MaxNumIntProtosIn (Class) * sizeof (uinT8));
  }

  return (Class);

}                                /* NewIntClass */


/*-------------------------------------------------------------------------*/
void free_int_class(INT_CLASS int_class) {
  int i;

  for (i = 0; i < int_class->NumProtoSets; i++) {
    Efree (int_class->ProtoSets[i]);
  }
  if (int_class->ProtoLengths != NULL) {
    Efree (int_class->ProtoLengths);
  }
  Efree(int_class);
}


/*---------------------------------------------------------------------------*/
INT_TEMPLATES NewIntTemplates() {
/*
 ** Parameters: none
 ** Globals: none
 ** Operation: This routine allocates a new set of integer templates
 **   initialized to hold 0 classes.
 ** Return: The integer templates created.
 ** Exceptions: none
 ** History: Fri Feb  8 08:38:51 1991, DSJ, Created.
 */
  INT_TEMPLATES T;
  int i;

  T = (INT_TEMPLATES) Emalloc (sizeof (INT_TEMPLATES_STRUCT));
  T->NumClasses = 0;
  T->NumClassPruners = 0;

  for (i = 0; i < MAX_NUM_CLASSES; i++)
    ClassForClassId (T, i) = NULL;

  return (T);
}                                /* NewIntTemplates */


/*---------------------------------------------------------------------------*/
void free_int_templates(INT_TEMPLATES templates) {
  int i;

  for (i = 0; i < templates->NumClasses; i++)
    free_int_class(templates->Class[i]);
  for (i = 0; i < templates->NumClassPruners; i++)
    Efree(templates->ClassPruner[i]);
  Efree(templates);
}


/*---------------------------------------------------------------------------*/
// Code to read/write Classify::font*table structures.
namespace {
bool read_info(FILE* f, FontInfo* fi, bool swap) {
  inT32 size;
  if (fread(&size, sizeof(size), 1, f) != 1) return false;
  if (swap)
    Reverse32(&size);
  char* font_name = new char[size + 1];
  fi->name = font_name;
  if (fread(font_name, sizeof(*font_name), size, f) != size) return false;
  font_name[size] = '\0';
  if (fread(&fi->properties, sizeof(fi->properties), 1, f) != 1) return false;
  if (swap)
    Reverse32(&fi->properties);
  return true;
}

bool write_info(FILE* f, const FontInfo& fi) {
  inT32 size = strlen(fi.name);
  if (fwrite(&size, sizeof(size), 1, f) != 1) return false;
  if (fwrite(fi.name, sizeof(*fi.name), size, f) != size) return false;
  if (fwrite(&fi.properties, sizeof(fi.properties), 1, f) != 1) return false;
  return true;
}

bool read_set(FILE* f, FontSet* fs, bool swap) {
  if (fread(&fs->size, sizeof(fs->size), 1, f) != 1) return false;
  if (swap)
    Reverse32(&fs->size);
  fs->configs = new int[fs->size];
  for (int i = 0; i < fs->size; ++i) {
    if (fread(&fs->configs[i], sizeof(fs->configs[i]), 1, f) != 1) return false;
    if (swap)
      Reverse32(&fs->configs[i]);
  }
  return true;
}

bool write_set(FILE* f, const FontSet& fs) {
  if (fwrite(&fs.size, sizeof(fs.size), 1, f) != 1) return false;
  for (int i = 0; i < fs.size; ++i) {
    if (fwrite(&fs.configs[i], sizeof(fs.configs[i]), 1, f) != 1) return false;
  }
  return true;
}

}  // namespace.

namespace tesseract {
INT_TEMPLATES Classify::ReadIntTemplates(FILE *File) {
/*
 ** Parameters:
 **   File    open file to read templates from
 ** Globals: none
 ** Operation: This routine reads a set of integer templates from
 **   File.  File must already be open and must be in the
 **   correct binary format.
 ** Return: Pointer to integer templates read from File.
 ** Exceptions: none
 ** History: Wed Feb 27 11:48:46 1991, DSJ, Created.
 */
  int i, j, w, x, y, z;
  BOOL8 swap;
  int nread;
  int unicharset_size;
  int version_id = 0;
  INT_TEMPLATES Templates;
  CLASS_PRUNER Pruner;
  INT_CLASS Class;
  uinT8 *Lengths;
  PROTO_SET ProtoSet;

  /* variables for conversion from older inttemp formats */
  int b, bit_number, last_cp_bit_number, new_b, new_i, new_w;
  CLASS_ID class_id, max_class_id;
  inT16 *IndexFor = new inT16[MAX_NUM_CLASSES];
  CLASS_ID *ClassIdFor = new CLASS_ID[MAX_NUM_CLASSES];
  CLASS_PRUNER *TempClassPruner = new CLASS_PRUNER[MAX_NUM_CLASS_PRUNERS];
  uinT32 SetBitsForMask =           // word with NUM_BITS_PER_CLASS
    (1 << NUM_BITS_PER_CLASS) - 1;  // set starting at bit 0
  uinT32 Mask, NewMask, ClassBits;
  uinT32 *Word;
  int MaxNumConfigs = MAX_NUM_CONFIGS;
  int WerdsPerConfigVec = WERDS_PER_CONFIG_VEC;

  /* first read the high level template struct */
  Templates = NewIntTemplates();
  // Read Templates in parts for 64 bit compatibility.
  if (fread(&unicharset_size, sizeof(int), 1, File) != 1)
    cprintf("Bad read of inttemp!\n");
  if (fread(&Templates->NumClasses,
            sizeof(Templates->NumClasses), 1, File) != 1 ||
      fread(&Templates->NumClassPruners,
            sizeof(Templates->NumClassPruners), 1, File) != 1)
    cprintf("Bad read of inttemp!\n");
  // Swap status is determined automatically.
  swap = Templates->NumClassPruners < 0 ||
    Templates->NumClassPruners > MAX_NUM_CLASS_PRUNERS;
  if (swap) {
    Reverse32(&Templates->NumClassPruners);
    Reverse32(&Templates->NumClasses);
    Reverse32(&unicharset_size);
  }
  if (Templates->NumClasses < 0) {
    // This file has a version id!
    version_id = -Templates->NumClasses;
    if (fread(&Templates->NumClasses, sizeof(Templates->NumClasses),
              1, File) != 1)
      cprintf("Bad read of inttemp!\n");
    if (swap)
      Reverse32(&Templates->NumClasses);
  }

  if (version_id < 3) {
    MaxNumConfigs = OLD_MAX_NUM_CONFIGS;
    WerdsPerConfigVec = OLD_WERDS_PER_CONFIG_VEC;
  }

  if (version_id < 2) {
    for (i = 0; i < unicharset_size; ++i) {
      if (fread(&IndexFor[i], sizeof(inT16), 1, File) != 1)
        cprintf("Bad read of inttemp!\n");
    }
    for (i = 0; i < Templates->NumClasses; ++i) {
      if (fread(&ClassIdFor[i], sizeof(CLASS_ID), 1, File) != 1)
        cprintf("Bad read of inttemp!\n");
    }
    if (swap) {
      for (i = 0; i < Templates->NumClasses; i++)
        Reverse16(&IndexFor[i]);
      for (i = 0; i < Templates->NumClasses; i++)
        Reverse32(&ClassIdFor[i]);
    }
  }

  /* then read in the class pruners */
  for (i = 0; i < Templates->NumClassPruners; i++) {
    Pruner = (CLASS_PRUNER) Emalloc(sizeof(CLASS_PRUNER_STRUCT));
    if ((nread =
         fread((char *) Pruner, 1, sizeof(CLASS_PRUNER_STRUCT),
                File)) != sizeof(CLASS_PRUNER_STRUCT))
      cprintf("Bad read of inttemp!\n");
    if (swap) {
      for (x = 0; x < NUM_CP_BUCKETS; x++) {
        for (y = 0; y < NUM_CP_BUCKETS; y++) {
          for (z = 0; z < NUM_CP_BUCKETS; z++) {
            for (w = 0; w < WERDS_PER_CP_VECTOR; w++) {
              Reverse32(&Pruner[x][y][z][w]);
            }
          }
        }
      }
    }
    if (version_id < 2) {
      TempClassPruner[i] = Pruner;
    } else {
      Templates->ClassPruner[i] = Pruner;
    }
  }

  /* fix class pruners if they came from an old version of inttemp */
  if (version_id < 2) {
    // Allocate enough class pruners to cover all the class ids.
    max_class_id = 0;
    for (i = 0; i < Templates->NumClasses; i++)
      if (ClassIdFor[i] > max_class_id)
        max_class_id = ClassIdFor[i];
    for (i = 0; i <= CPrunerIdFor(max_class_id); i++) {
      Templates->ClassPruner[i] =
        (CLASS_PRUNER) Emalloc(sizeof(CLASS_PRUNER_STRUCT));
      for (Word = (uinT32 *) (Templates->ClassPruner[i]);
           Word < (uinT32 *) (Templates->ClassPruner[i]) + WERDS_PER_CP;
           *Word++ = 0);
    }
    // Convert class pruners from the old format (indexed by class index)
    // to the new format (indexed by class id).
    last_cp_bit_number = NUM_BITS_PER_CLASS * Templates->NumClasses - 1;
    for (i = 0; i < Templates->NumClassPruners; i++) {
      for (x = 0; x < NUM_CP_BUCKETS; x++)
        for (y = 0; y < NUM_CP_BUCKETS; y++)
          for (z = 0; z < NUM_CP_BUCKETS; z++)
            for (w = 0; w < WERDS_PER_CP_VECTOR; w++) {
              if (TempClassPruner[i][x][y][z][w] == 0)
                continue;
              for (b = 0; b < BITS_PER_WERD; b += NUM_BITS_PER_CLASS) {
                bit_number = i * BITS_PER_CP_VECTOR + w * BITS_PER_WERD + b;
                if (bit_number > last_cp_bit_number)
                  break; // the rest of the bits in this word are not used
                class_id = ClassIdFor[bit_number / NUM_BITS_PER_CLASS];
                // Single out NUM_BITS_PER_CLASS bits relating to class_id.
                Mask = SetBitsForMask << b;
                ClassBits = TempClassPruner[i][x][y][z][w] & Mask;
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
                Templates->ClassPruner[new_i][x][y][z][new_w] &= ~NewMask;
                Templates->ClassPruner[new_i][x][y][z][new_w] |= ClassBits;
              }
            }
    }
    for (i = 0; i < Templates->NumClassPruners; i++) {
      Efree (TempClassPruner[i]);
    }
  }

  /* then read in each class */
  for (i = 0; i < Templates->NumClasses; i++) {
    /* first read in the high level struct for the class */
    Class = (INT_CLASS) Emalloc (sizeof (INT_CLASS_STRUCT));
    if (fread(&Class->NumProtos, sizeof(Class->NumProtos), 1, File) != 1 ||
        fread(&Class->NumProtoSets, sizeof(Class->NumProtoSets), 1, File) != 1 ||
        fread(&Class->NumConfigs, sizeof(Class->NumConfigs), 1, File) != 1)
      cprintf ("Bad read of inttemp!\n");
    if (version_id == 0) {
      // Only version 0 writes 5 pointless pointers to the file.
      for (j = 0; j < 5; ++j) {
        int junk;
        if (fread(&junk, sizeof(junk), 1, File) != 1)
          cprintf ("Bad read of inttemp!\n");
      }
    }
    if (version_id < 4) {
      for (j = 0; j < MaxNumConfigs; ++j) {
        if (fread(&Class->ConfigLengths[j], sizeof(uinT16), 1, File) != 1)
          cprintf ("Bad read of inttemp!\n");
      }
      if (swap) {
        Reverse16(&Class->NumProtos);
        for (j = 0; j < MaxNumConfigs; j++)
          Reverse16(&Class->ConfigLengths[j]);
      }
    } else {
      ASSERT_HOST(Class->NumConfigs < MaxNumConfigs);
      for (j = 0; j < Class->NumConfigs; ++j) {
        if (fread(&Class->ConfigLengths[j], sizeof(uinT16), 1, File) != 1)
          cprintf ("Bad read of inttemp!\n");
      }
      if (swap) {
        Reverse16(&Class->NumProtos);
        for (j = 0; j < MaxNumConfigs; j++)
          Reverse16(&Class->ConfigLengths[j]);
      }
    }
    if (version_id < 2) {
      ClassForClassId (Templates, ClassIdFor[i]) = Class;
    } else {
      ClassForClassId (Templates, i) = Class;
    }

    /* then read in the proto lengths */
    Lengths = NULL;
    if (MaxNumIntProtosIn (Class) > 0) {
      Lengths = (uinT8 *)Emalloc(sizeof(uinT8) * MaxNumIntProtosIn(Class));
      if ((nread =
           fread((char *)Lengths, sizeof(uinT8),
                 MaxNumIntProtosIn(Class), File)) != MaxNumIntProtosIn (Class))
        cprintf ("Bad read of inttemp!\n");
    }
    Class->ProtoLengths = Lengths;

    /* then read in the proto sets */
    for (j = 0; j < Class->NumProtoSets; j++) {
      ProtoSet = (PROTO_SET)Emalloc(sizeof(PROTO_SET_STRUCT));
      if (version_id < 3) {
        if ((nread =
             fread((char *) &ProtoSet->ProtoPruner, 1,
                    sizeof(PROTO_PRUNER), File)) != sizeof(PROTO_PRUNER))
          cprintf("Bad read of inttemp!\n");
        for (x = 0; x < PROTOS_PER_PROTO_SET; x++) {
          if ((nread = fread((char *) &ProtoSet->Protos[x].A, 1,
                             sizeof(inT8), File)) != sizeof(inT8) ||
              (nread = fread((char *) &ProtoSet->Protos[x].B, 1,
                             sizeof(uinT8), File)) != sizeof(uinT8) ||
              (nread = fread((char *) &ProtoSet->Protos[x].C, 1,
                             sizeof(inT8), File)) != sizeof(inT8) ||
              (nread = fread((char *) &ProtoSet->Protos[x].Angle, 1,
                             sizeof(uinT8), File)) != sizeof(uinT8))
            cprintf("Bad read of inttemp!\n");
          for (y = 0; y < WerdsPerConfigVec; y++)
            if ((nread = fread((char *) &ProtoSet->Protos[x].Configs[y], 1,
                               sizeof(uinT32), File)) != sizeof(uinT32))
              cprintf("Bad read of inttemp!\n");
        }
      } else {
        if ((nread =
             fread((char *) ProtoSet, 1, sizeof(PROTO_SET_STRUCT),
                   File)) != sizeof(PROTO_SET_STRUCT))
          cprintf("Bad read of inttemp!\n");
      }
      if (swap) {
        for (x = 0; x < NUM_PP_PARAMS; x++)
          for (y = 0; y < NUM_PP_BUCKETS; y++)
            for (z = 0; z < WERDS_PER_PP_VECTOR; z++)
              Reverse32(&ProtoSet->ProtoPruner[x][y][z]);
        for (x = 0; x < PROTOS_PER_PROTO_SET; x++)
          for (y = 0; y < WerdsPerConfigVec; y++)
            Reverse32(&ProtoSet->Protos[x].Configs[y]);
      }
      Class->ProtoSets[j] = ProtoSet;
    }
    if (version_id < 4)
      Class->font_set_id = -1;
    else {
      fread(&Class->font_set_id, sizeof(int), 1, File);
      if (swap)
        Reverse32(&Class->font_set_id);
    }
  }

  if (version_id < 2) {
    /* add an empty NULL class with class id 0 */
    assert(UnusedClassIdIn (Templates, 0));
    ClassForClassId (Templates, 0) = NewIntClass (1, 1);
    ClassForClassId (Templates, 0)->font_set_id = -1;
    Templates->NumClasses++;
    /* make sure the classes are contiguous */
    for (i = 0; i < MAX_NUM_CLASSES; i++) {
      if (i < Templates->NumClasses) {
        if (ClassForClassId (Templates, i) == NULL) {
          fprintf(stderr, "Non-contiguous class ids in inttemp\n");
          exit(1);
        }
      } else {
        if (ClassForClassId (Templates, i) != NULL) {
          fprintf(stderr, "Class id %d exceeds NumClassesIn (Templates) %d\n",
                  i, Templates->NumClasses);
          exit(1);
        }
      }
    }
  }
  if (version_id >= 4) {
    this->fontinfo_table_.read(File, NewPermanentCallback(read_info), swap);
    this->fontset_table_.read(File, NewPermanentCallback(read_set), swap);
  }

  // Clean up.
  delete[] IndexFor;
  delete[] ClassIdFor;
  delete[] TempClassPruner;

  return (Templates);
}                                /* ReadIntTemplates */

} // namespace tesseract


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void ShowMatchDisplay() {
/*
 ** Parameters: none
 ** Globals:
 **   FeatureShapes display list containing feature matches
 **   ProtoShapes display list containing proto matches
 ** Operation: This routine sends the shapes in the global display
 **   lists to the match debugger window.
 ** Return: none
 ** Exceptions: none
 ** History: Thu Mar 21 15:47:33 1991, DSJ, Created.
 */
  void *window;
  /* Size of drawable */
  InitIntMatchWindowIfReqd();
  c_clear_window(IntMatchWindow);
  if (ProtoDisplayWindow) {
    c_clear_window(ProtoDisplayWindow);
  }
  if (FeatureDisplayWindow) {
    c_clear_window(FeatureDisplayWindow);
  }

  window = IntMatchWindow;
  c_line_color_index(window, Grey);
  /* Default size of drawing */
  if (classify_norm_method == baseline) {
    c_move (window, -1000.0, INT_BASELINE);
    c_draw (window, 1000.0, INT_BASELINE);
    c_move (window, -1000.0, INT_DESCENDER);
    c_draw (window, 1000.0, INT_DESCENDER);
    c_move (window, -1000.0, INT_XHEIGHT);
    c_draw (window, 1000.0, INT_XHEIGHT);
    c_move (window, -1000.0, INT_CAPHEIGHT);
    c_draw (window, 1000.0, INT_CAPHEIGHT);
    c_move (window, INT_MIN_X, -1000.0);
    c_draw (window, INT_MIN_X, 1000.0);
    c_move (window, INT_MAX_X, -1000.0);
    c_draw (window, INT_MAX_X, 1000.0);
  }
  else {
    c_move (window, INT_XCENTER - INT_XRADIUS, INT_YCENTER - INT_YRADIUS);
    c_draw (window, INT_XCENTER + INT_XRADIUS, INT_YCENTER - INT_YRADIUS);
    c_move (window, INT_XCENTER - INT_XRADIUS, INT_YCENTER + INT_YRADIUS);
    c_draw (window, INT_XCENTER + INT_XRADIUS, INT_YCENTER + INT_YRADIUS);
    c_move (window, INT_XCENTER - INT_XRADIUS, INT_YCENTER - INT_YRADIUS);
    c_draw (window, INT_XCENTER - INT_XRADIUS, INT_YCENTER + INT_YRADIUS);
    c_move (window, INT_XCENTER + INT_XRADIUS, INT_YCENTER - INT_YRADIUS);
    c_draw (window, INT_XCENTER + INT_XRADIUS, INT_YCENTER + INT_YRADIUS);
    c_move(window, INT_MIN_X, INT_MIN_Y);
    c_draw(window, INT_MIN_X, INT_MAX_Y);
    c_move(window, INT_MIN_X, INT_MIN_Y);
    c_draw(window, INT_MAX_X, INT_MIN_Y);
    c_move(window, INT_MAX_X, INT_MAX_Y);
    c_draw(window, INT_MIN_X, INT_MAX_Y);
    c_move(window, INT_MAX_X, INT_MAX_Y);
    c_draw(window, INT_MAX_X, INT_MIN_Y);
  }
  IntMatchWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y,
                                  INT_MAX_X, INT_MAX_Y);
  if (ProtoDisplayWindow) {
    ProtoDisplayWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y,
                                        INT_MAX_X, INT_MAX_Y);
  }
  if (FeatureDisplayWindow) {
    FeatureDisplayWindow->ZoomToRectangle(INT_MIN_X, INT_MIN_Y,
                                          INT_MAX_X, INT_MAX_Y);
  }
}                                /* ShowMatchDisplay */
#endif

/*---------------------------------------------------------------------------*/
namespace tesseract {
void Classify::WriteIntTemplates(FILE *File, INT_TEMPLATES Templates,
                                 const UNICHARSET& target_unicharset) {
/*
 ** Parameters:
 **   File    open file to write templates to
 **   Templates templates to save into File
 ** Globals: none
 ** Operation: This routine writes Templates to File.  The format
 **   is an efficient binary format.  File must already be open
 **   for writing.
 ** Return: none
 ** Exceptions: none
 ** History: Wed Feb 27 11:48:46 1991, DSJ, Created.
 */
  int i, j;
  INT_CLASS Class;
  int unicharset_size = target_unicharset.size();
  int version_id = -4;  // When negated by the reader -1 becomes +1 etc.

  if (Templates->NumClasses != unicharset_size) {
    cprintf("Warning: executing WriteIntTemplates() with %d classes in"
            " Templates, while target_unicharset size is %d\n",
            Templates->NumClasses, unicharset_size);
  }

  /* first write the high level template struct */
  fwrite(&unicharset_size, sizeof(unicharset_size), 1, File);
  fwrite(&version_id, sizeof(version_id), 1, File);
  fwrite(&Templates->NumClassPruners, sizeof(Templates->NumClassPruners),
         1, File);
  fwrite(&Templates->NumClasses, sizeof(Templates->NumClasses), 1, File);

  /* then write out the class pruners */
  for (i = 0; i < Templates->NumClassPruners; i++)
    fwrite(Templates->ClassPruner[i],
           sizeof(CLASS_PRUNER_STRUCT), 1, File);

  /* then write out each class */
  for (i = 0; i < Templates->NumClasses; i++) {
    Class = Templates->Class[i];

    /* first write out the high level struct for the class */
    fwrite(&Class->NumProtos, sizeof(Class->NumProtos), 1, File);
    fwrite(&Class->NumProtoSets, sizeof(Class->NumProtoSets), 1, File);
    ASSERT_HOST(Class->NumConfigs == this->fontset_table_.get(Class->font_set_id).size);
    fwrite(&Class->NumConfigs, sizeof(Class->NumConfigs), 1, File);
    for (j = 0; j < Class->NumConfigs; ++j) {
      fwrite(&Class->ConfigLengths[j], sizeof(uinT16), 1, File);
    }

    /* then write out the proto lengths */
    if (MaxNumIntProtosIn (Class) > 0) {
      fwrite ((char *) (Class->ProtoLengths), sizeof (uinT8),
              MaxNumIntProtosIn (Class), File);
    }

    /* then write out the proto sets */
    for (j = 0; j < Class->NumProtoSets; j++)
      fwrite ((char *) Class->ProtoSets[j],
              sizeof (PROTO_SET_STRUCT), 1, File);

    /* then write the fonts info */
    fwrite(&Class->font_set_id, sizeof(int), 1, File);
  }

  /* Write the fonts info tables */
  this->fontinfo_table_.write(File, NewPermanentCallback(write_info));
  this->fontset_table_.write(File, NewPermanentCallback(write_set));
}                                /* WriteIntTemplates */
} // namespace tesseract


/*-----------------------------------------------------------------------------
              Private Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
FLOAT32 BucketStart(int Bucket, FLOAT32 Offset, int NumBuckets) {
/*
 ** Parameters:
 **   Bucket    bucket whose start is to be computed
 **   Offset    offset used to map params to buckets
 **   NumBuckets  total number of buckets
 ** Globals: none
 ** Operation: This routine returns the parameter value which
 **   corresponds to the beginning of the specified bucket.
 **   The bucket number should have been generated using the
 **   BucketFor() function with parameters Offset and NumBuckets.
 ** Return: Param value corresponding to start position of Bucket.
 ** Exceptions: none
 ** History: Thu Feb 14 13:24:33 1991, DSJ, Created.
 */
  return (((FLOAT32) Bucket / NumBuckets) - Offset);

}                                /* BucketStart */


/*---------------------------------------------------------------------------*/
FLOAT32 BucketEnd(int Bucket, FLOAT32 Offset, int NumBuckets) {
/*
 ** Parameters:
 **   Bucket    bucket whose end is to be computed
 **   Offset    offset used to map params to buckets
 **   NumBuckets  total number of buckets
 ** Globals: none
 ** Operation: This routine returns the parameter value which
 **   corresponds to the end of the specified bucket.
 **   The bucket number should have been generated using the
 **   BucketFor() function with parameters Offset and NumBuckets.
 ** Return: Param value corresponding to end position of Bucket.
 ** Exceptions: none
 ** History: Thu Feb 14 13:24:33 1991, DSJ, Created.
 */
  return (((FLOAT32) (Bucket + 1) / NumBuckets) - Offset);
}                                /* BucketEnd */


/*---------------------------------------------------------------------------*/
void DoFill(FILL_SPEC *FillSpec,
            CLASS_PRUNER Pruner,
            register uinT32 ClassMask,
            register uinT32 ClassCount,
            register uinT32 WordIndex) {
/*
 ** Parameters:
 **   FillSpec  specifies which bits to fill in pruner
 **   Pruner    class pruner to be filled
 **   ClassMask indicates which bits to change in each word
 **   ClassCount  indicates what to change bits to
 **   WordIndex indicates which word to change
 ** Globals: none
 ** Operation: This routine fills in the section of a class pruner
 **   corresponding to a single x value for a single proto of
 **   a class.
 ** Return: none
 ** Exceptions: none
 ** History: Tue Feb 19 11:11:29 1991, DSJ, Created.
 */
  register int X, Y, Angle;
  register uinT32 OldWord;

  X = FillSpec->X;
  if (X < 0)
    X = 0;
  if (X >= NUM_CP_BUCKETS)
    X = NUM_CP_BUCKETS - 1;

  if (FillSpec->YStart < 0)
    FillSpec->YStart = 0;
  if (FillSpec->YEnd >= NUM_CP_BUCKETS)
    FillSpec->YEnd = NUM_CP_BUCKETS - 1;

  for (Y = FillSpec->YStart; Y <= FillSpec->YEnd; Y++)
    for (Angle = FillSpec->AngleStart;
         TRUE; CircularIncrement (Angle, NUM_CP_BUCKETS)) {
      OldWord = Pruner[X][Y][Angle][WordIndex];
      if (ClassCount > (OldWord & ClassMask)) {
        OldWord &= ~ClassMask;
        OldWord |= ClassCount;
        Pruner[X][Y][Angle][WordIndex] = OldWord;
      }
      if (Angle == FillSpec->AngleEnd)
        break;
    }
}                                /* DoFill */


/*---------------------------------------------------------------------------*/
BOOL8 FillerDone(TABLE_FILLER *Filler) {
/*
 ** Parameters:
 **   Filler    table filler to check if done
 ** Globals: none
 ** Operation: Return TRUE if the specified table filler is done, i.e.
 **   if it has no more lines to fill.
 ** Return: TRUE if no more lines to fill, FALSE otherwise.
 ** Exceptions: none
 ** History: Tue Feb 19 10:08:05 1991, DSJ, Created.
 */
  FILL_SWITCH *Next;

  Next = &(Filler->Switch[Filler->NextSwitch]);

  if (Filler->X > Next->X && Next->Type == LastSwitch)
    return (TRUE);
  else
    return (FALSE);

}                                /* FillerDone */


/*---------------------------------------------------------------------------*/
void
FillPPCircularBits (uinT32 ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR],
                    int Bit, FLOAT32 Center, FLOAT32 Spread) {
/*
 ** Parameters:
 **   ParamTable  table of bit vectors, one per param bucket
 **   Bit   bit position in vectors to be filled
 **   Center    center of filled area
 **   Spread    spread of filled area
 ** Globals: none
 ** Operation: This routine sets Bit in each bit vector whose
 **   bucket lies within the range Center +- Spread.  The fill
 **   is done for a circular dimension, i.e. bucket 0 is adjacent
 **   to the last bucket.  It is assumed that Center and Spread
 **   are expressed in a circular coordinate system whose range
 **   is 0 to 1.
 ** Return: none
 ** Exceptions: none
 ** History: Tue Oct 16 09:26:54 1990, DSJ, Created.
 */
  int i, FirstBucket, LastBucket;

  if (Spread > 0.5)
    Spread = 0.5;

  FirstBucket = (int) floor ((Center - Spread) * NUM_PP_BUCKETS);
  if (FirstBucket < 0)
    FirstBucket += NUM_PP_BUCKETS;

  LastBucket = (int) floor ((Center + Spread) * NUM_PP_BUCKETS);
  if (LastBucket >= NUM_PP_BUCKETS)
    LastBucket -= NUM_PP_BUCKETS;
  if (classify_learning_debug_level >= 2)
    cprintf ("Circular fill from %d to %d", FirstBucket, LastBucket);
  for (i = FirstBucket; TRUE; CircularIncrement (i, NUM_PP_BUCKETS)) {
    SET_BIT (ParamTable[i], Bit);

    /* exit loop after we have set the bit for the last bucket */
    if (i == LastBucket)
      break;
  }

}                                /* FillPPCircularBits */


/*---------------------------------------------------------------------------*/
void
FillPPLinearBits (uinT32 ParamTable[NUM_PP_BUCKETS][WERDS_PER_PP_VECTOR],
                  int Bit, FLOAT32 Center, FLOAT32 Spread) {
/*
 ** Parameters:
 **   ParamTable  table of bit vectors, one per param bucket
 **   Bit   bit number being filled
 **   Center    center of filled area
 **   Spread    spread of filled area
 ** Globals: none
 ** Operation: This routine sets Bit in each bit vector whose
 **   bucket lies within the range Center +- Spread.  The fill
 **   is done for a linear dimension, i.e. there is no wrap-around
 **   for this dimension.  It is assumed that Center and Spread
 **   are expressed in a linear coordinate system whose range
 **   is approximately 0 to 1.  Values outside this range will
 **   be clipped.
 ** Return: none
 ** Exceptions: none
 ** History: Tue Oct 16 09:26:54 1990, DSJ, Created.
 */
  int i, FirstBucket, LastBucket;

  FirstBucket = (int) floor ((Center - Spread) * NUM_PP_BUCKETS);
  if (FirstBucket < 0)
    FirstBucket = 0;

  LastBucket = (int) floor ((Center + Spread) * NUM_PP_BUCKETS);
  if (LastBucket >= NUM_PP_BUCKETS)
    LastBucket = NUM_PP_BUCKETS - 1;

  if (classify_learning_debug_level >= 2)
    cprintf ("Linear fill from %d to %d", FirstBucket, LastBucket);
  for (i = FirstBucket; i <= LastBucket; i++)
    SET_BIT (ParamTable[i], Bit);

}                                /* FillPPLinearBits */


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
namespace tesseract {
CLASS_ID Classify::GetClassToDebug(const char *Prompt) {
/*
 ** Parameters:
 **   Prompt  prompt to print while waiting for input from window
 ** Globals: none
 ** Operation: This routine prompts the user with Prompt and waits
 **   for the user to enter something in the debug window.
 ** Return: Character entered in the debug window.
 ** Exceptions: none
 ** History: Thu Mar 21 16:55:13 1991, DSJ, Created.
 */
  tprintf("%s\n", Prompt);
  SVEvent* ev;
  SVEventType ev_type;
  // Wait until a click or popup event.
  do {
    ev = IntMatchWindow->AwaitEvent(SVET_ANY);
    ev_type = ev->type;
    if (ev_type == SVET_POPUP) {
      // TODO(rays) must return which menu item was selected, but
      // that can't be done in this CL without dragging in a lot of
      // other changes.
      if (unicharset.contains_unichar(ev->parameter))
        return unicharset.unichar_to_id(ev->parameter);
      tprintf("Char class '%s' not found in unicharset",
              ev->parameter);
    }
    delete ev;
  } while (ev_type != SVET_CLICK);
  return 0;
}                                /* GetClassToDebug */

}  // namespace tesseract
#endif

/*---------------------------------------------------------------------------*/
void GetCPPadsForLevel(int Level,
                       FLOAT32 *EndPad,
                       FLOAT32 *SidePad,
                       FLOAT32 *AnglePad) {
/*
 ** Parameters:
 **   Level   "tightness" level to return pads for
 **   EndPad    place to put end pad for Level
 **   SidePad   place to put side pad for Level
 **   AnglePad  place to put angle pad for Level
 ** Globals: none
 ** Operation: This routine copies the appropriate global pad variables
 **   into EndPad, SidePad, and AnglePad.  This is a kludge used
 **   to get around the fact that global control variables cannot
 **   be arrays.  If the specified level is illegal, the tightest
 **   possible pads are returned.
 ** Return: none (results are returned in EndPad, SidePad, and AnglePad.
 ** Exceptions: none
 ** History: Thu Feb 14 08:26:49 1991, DSJ, Created.
 */
  switch (Level) {
    case 0:
      *EndPad = classify_cp_end_pad_loose * GetPicoFeatureLength ();
      *SidePad = classify_cp_side_pad_loose * GetPicoFeatureLength ();
      *AnglePad = classify_cp_angle_pad_loose / 360.0;
      break;

    case 1:
      *EndPad = classify_cp_end_pad_medium * GetPicoFeatureLength ();
      *SidePad = classify_cp_side_pad_medium * GetPicoFeatureLength ();
      *AnglePad = classify_cp_angle_pad_medium / 360.0;
      break;

    case 2:
      *EndPad = classify_cp_end_pad_tight * GetPicoFeatureLength ();
      *SidePad = classify_cp_side_pad_tight * GetPicoFeatureLength ();
      *AnglePad = classify_cp_angle_pad_tight / 360.0;
      break;

    default:
      *EndPad = classify_cp_end_pad_tight * GetPicoFeatureLength ();
      *SidePad = classify_cp_side_pad_tight * GetPicoFeatureLength ();
      *AnglePad = classify_cp_angle_pad_tight / 360.0;
      break;
  }
  if (*AnglePad > 0.5)
    *AnglePad = 0.5;

}                                /* GetCPPadsForLevel */


/*---------------------------------------------------------------------------*/
C_COL GetMatchColorFor(FLOAT32 Evidence) {
/*
 ** Parameters:
 **   Evidence  evidence value to return color for
 ** Globals: none
 ** Operation:
 ** Return: Color which corresponds to specified Evidence value.
 ** Exceptions: none
 ** History: Thu Mar 21 15:24:52 1991, DSJ, Created.
 */

  assert (Evidence >= 0.0);
  assert (Evidence <= 1.0);

  if (Evidence >= 0.90)
    return White;
  else if (Evidence >= 0.75)
    return Green;
  else if (Evidence >= 0.50)
    return Red;
  else
    return Blue;
}                                /* GetMatchColorFor */


/*---------------------------------------------------------------------------*/
void GetNextFill(TABLE_FILLER *Filler, FILL_SPEC *Fill) {
/*
 ** Parameters:
 **   Filler    filler to get next fill spec from
 **   Fill    place to put spec for next fill
 ** Globals: none
 ** Operation: This routine returns (in Fill) the specification of
 **   the next line to be filled from Filler.  FillerDone() should
 **   always be called before GetNextFill() to ensure that we
 **   do not run past the end of the fill table.
 ** Return: none (results are returned in Fill)
 ** Exceptions: none
 ** History: Tue Feb 19 10:17:42 1991, DSJ, Created.
 */
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
    }
    else if (Next->Type == EndSwitch) {
      Fill->YEnd = Next->Y;
      Filler->EndDelta = Next->Delta;
      Filler->YEnd = Next->YInit;
    }
    else {                       /* Type must be LastSwitch */
      break;
    }
    Filler->NextSwitch++;
    Next = &(Filler->Switch[Filler->NextSwitch]);
  }

  /* prepare the filler for the next call to this routine */
  Filler->X++;
  Filler->YStart += Filler->StartDelta;
  Filler->YEnd += Filler->EndDelta;

}                                /* GetNextFill */


/*---------------------------------------------------------------------------*/
/**
 * This routine computes a data structure (Filler)
 * which can be used to fill in a rectangle surrounding
 * the specified Proto.
 *
 * @param EndPad, SidePad, AnglePad padding to add to proto
 * @param Proto       proto to create a filler for
 * @param Filler        place to put table filler
 *
 * Globals: none
 *
 * @return none (results are returned in Filler)
 * @note Exceptions: none
 * @note History: Thu Feb 14 09:27:05 1991, DSJ, Created.
 */
void InitTableFiller (FLOAT32 EndPad, FLOAT32 SidePad,
                      FLOAT32 AnglePad, PROTO Proto, TABLE_FILLER * Filler)
#define XS          X_SHIFT
#define YS          Y_SHIFT
#define AS          ANGLE_SHIFT
#define NB          NUM_CP_BUCKETS
{
  FLOAT32 Angle;
  FLOAT32 X, Y, HalfLength;
  FLOAT32 Cos, Sin;
  FLOAT32 XAdjust, YAdjust;
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

  if (fabs (Angle - 0.0) < HV_TOLERANCE || fabs (Angle - 0.5) < HV_TOLERANCE) {
    /* horizontal proto - handle as special case */
    Filler->X = BucketFor(X - HalfLength - EndPad, XS, NB);
    Filler->YStart = BucketFor(Y - SidePad, YS, NB * 256);
    Filler->YEnd = BucketFor(Y + SidePad, YS, NB * 256);
    Filler->StartDelta = 0;
    Filler->EndDelta = 0;
    Filler->Switch[0].Type = LastSwitch;
    Filler->Switch[0].X = BucketFor(X + HalfLength + EndPad, XS, NB);
  } else if (fabs(Angle - 0.25) < HV_TOLERANCE ||
           fabs(Angle - 0.75) < HV_TOLERANCE) {
    /* vertical proto - handle as special case */
    Filler->X = BucketFor(X - SidePad, XS, NB);
    Filler->YStart = BucketFor(Y - HalfLength - EndPad, YS, NB * 256);
    Filler->YEnd = BucketFor(Y + HalfLength + EndPad, YS, NB * 256);
    Filler->StartDelta = 0;
    Filler->EndDelta = 0;
    Filler->Switch[0].Type = LastSwitch;
    Filler->Switch[0].X = BucketFor(X + SidePad, XS, NB);
  } else {
    /* diagonal proto */

    if ((Angle > 0.0 && Angle < 0.25) || (Angle > 0.5 && Angle < 0.75)) {
      /* rising diagonal proto */
      Angle *= 2.0 * PI;
      Cos = fabs(cos(Angle));
      Sin = fabs(sin(Angle));

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
      Filler->X = (inT8) MapParam(Start.x, XS, NB);
      Filler->StartDelta = -(inT16) ((Cos / Sin) * 256);
      Filler->EndDelta = (inT16) ((Sin / Cos) * 256);

      XAdjust = BucketEnd(Filler->X, XS, NB) - Start.x;
      YAdjust = XAdjust * Cos / Sin;
      Filler->YStart = (inT16) MapParam(Start.y - YAdjust, YS, NB * 256);
      YAdjust = XAdjust * Sin / Cos;
      Filler->YEnd = (inT16) MapParam(Start.y + YAdjust, YS, NB * 256);

      Filler->Switch[S1].Type = StartSwitch;
      Filler->Switch[S1].X = (inT8) MapParam(Switch1.x, XS, NB);
      Filler->Switch[S1].Y = (inT8) MapParam(Switch1.y, YS, NB);
      XAdjust = Switch1.x - BucketStart(Filler->Switch[S1].X, XS, NB);
      YAdjust = XAdjust * Sin / Cos;
      Filler->Switch[S1].YInit =
        (inT16) MapParam(Switch1.y - YAdjust, YS, NB * 256);
      Filler->Switch[S1].Delta = Filler->EndDelta;

      Filler->Switch[S2].Type = EndSwitch;
      Filler->Switch[S2].X = (inT8) MapParam(Switch2.x, XS, NB);
      Filler->Switch[S2].Y = (inT8) MapParam(Switch2.y, YS, NB);
      XAdjust = Switch2.x - BucketStart(Filler->Switch[S2].X, XS, NB);
      YAdjust = XAdjust * Cos / Sin;
      Filler->Switch[S2].YInit =
        (inT16) MapParam(Switch2.y + YAdjust, YS, NB * 256);
      Filler->Switch[S2].Delta = Filler->StartDelta;

      Filler->Switch[2].Type = LastSwitch;
      Filler->Switch[2].X = (inT8)MapParam(End.x, XS, NB);
    } else {
      /* falling diagonal proto */
      Angle *= 2.0 * PI;
      Cos = fabs(cos(Angle));
      Sin = fabs(sin(Angle));

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
      Filler->X = (inT8) MapParam(Start.x, XS, NB);
      Filler->StartDelta = -(inT16) ((Sin / Cos) * 256);
      Filler->EndDelta = (inT16) ((Cos / Sin) * 256);

      XAdjust = BucketEnd(Filler->X, XS, NB) - Start.x;
      YAdjust = XAdjust * Sin / Cos;
      Filler->YStart = (inT16) MapParam(Start.y - YAdjust, YS, NB * 256);
      YAdjust = XAdjust * Cos / Sin;
      Filler->YEnd = (inT16) MapParam(Start.y + YAdjust, YS, NB * 256);

      Filler->Switch[S1].Type = EndSwitch;
      Filler->Switch[S1].X = (inT8) MapParam(Switch1.x, XS, NB);
      Filler->Switch[S1].Y = (inT8) MapParam(Switch1.y, YS, NB);
      XAdjust = Switch1.x - BucketStart(Filler->Switch[S1].X, XS, NB);
      YAdjust = XAdjust * Sin / Cos;
      Filler->Switch[S1].YInit =
        (inT16) MapParam(Switch1.y + YAdjust, YS, NB * 256);
      Filler->Switch[S1].Delta = Filler->StartDelta;

      Filler->Switch[S2].Type = StartSwitch;
      Filler->Switch[S2].X = (inT8) MapParam(Switch2.x, XS, NB);
      Filler->Switch[S2].Y = (inT8) MapParam(Switch2.y, YS, NB);
      XAdjust = Switch2.x - BucketStart(Filler->Switch[S2].X, XS, NB);
      YAdjust = XAdjust * Cos / Sin;
      Filler->Switch[S2].YInit =
        (inT16) MapParam(Switch2.y - YAdjust, YS, NB * 256);
      Filler->Switch[S2].Delta = Filler->EndDelta;

      Filler->Switch[2].Type = LastSwitch;
      Filler->Switch[2].X = (inT8) MapParam(End.x, XS, NB);
    }
  }
}                                /* InitTableFiller */


/*---------------------------------------------------------------------------*/
#ifndef GRAPHICS_DISABLED
void RenderIntFeature(void *window, INT_FEATURE Feature, C_COL Color) {
/*
 ** Parameters:
 **   ShapeList shape list to add feature rendering to
 **   Feature   feature to be rendered
 **   Color   color to use for feature rendering
 ** Globals: none
 ** Operation: This routine renders the specified feature into ShapeList.
 ** Return: New shape list with rendering of Feature added.
 ** Exceptions: none
 ** History: Thu Mar 21 14:57:41 1991, DSJ, Created.
 */
  FLOAT32 X, Y, Dx, Dy, Length;

  c_line_color_index(window, Color);
  assert(Feature != NULL);
  assert(Color != 0);

  X = Feature->X - DISPLAY_OFFSET;
  Y = Feature->Y - DISPLAY_OFFSET;
  Length = GetPicoFeatureLength() * 0.7 * INT_CHAR_NORM_RANGE;
  Dx = (Length / 2.0) * cos((Feature->Theta / 256.0) * 2.0 * PI);
  Dy = (Length / 2.0) * sin((Feature->Theta / 256.0) * 2.0 * PI);

  c_move(window, X - Dx, Y - Dy);
  c_draw(window, X + Dx, Y + Dy);
  c_move(window, X - Dx - Dy * DOUBLE_OFFSET, Y - Dy + Dx * DOUBLE_OFFSET);
  c_draw(window, X + Dx - Dy * DOUBLE_OFFSET, Y + Dy + Dx * DOUBLE_OFFSET);
}                                /* RenderIntFeature */


/*---------------------------------------------------------------------------*/
/**
 * This routine extracts the parameters of the specified
 * proto from the class description and adds a rendering of
 * the proto onto the ShapeList.
 *
 * @param ShapeList shape list to append proto rendering onto
 * @param Class   class that proto is contained in
 * @param ProtoId   id of proto to be rendered
 * @param Color   color to render proto in
 *
 * Globals: none
 *
 * @return New shape list with a rendering of one proto added.
 * @note Exceptions: none
 * @note History: Thu Mar 21 10:21:09 1991, DSJ, Created.
 */
void RenderIntProto(void *window,
                    INT_CLASS Class,
                    PROTO_ID ProtoId,
                    C_COL Color) {
  PROTO_SET ProtoSet;
  INT_PROTO Proto;
  int ProtoSetIndex;
  int ProtoWordIndex;
  FLOAT32 Length;
  int Xmin, Xmax, Ymin, Ymax;
  FLOAT32 X, Y, Dx, Dy;
  uinT32 ProtoMask;
  int Bucket;

  assert(ProtoId >= 0);
  assert(Class != NULL);
  assert(ProtoId < Class->NumProtos);
  assert(Color != 0);
  c_line_color_index(window, Color);

  ProtoSet = Class->ProtoSets[SetForProto(ProtoId)];
  ProtoSetIndex = IndexForProto(ProtoId);
  Proto = &(ProtoSet->Protos[ProtoSetIndex]);
  Length = (Class->ProtoLengths[ProtoId] *
    GetPicoFeatureLength() * INT_CHAR_NORM_RANGE);
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
  X = (Xmin + Xmax + 1) / 2.0 * PROTO_PRUNER_SCALE - DISPLAY_OFFSET;
  Y = (Ymin + Ymax + 1) / 2.0 * PROTO_PRUNER_SCALE - DISPLAY_OFFSET;
  Dx = (Length / 2.0) * cos((Proto->Angle / 256.0) * 2.0 * PI);
  Dy = (Length / 2.0) * sin((Proto->Angle / 256.0) * 2.0 * PI);

  c_move(window, X - Dx, Y - Dy);
  c_draw(window, X + Dx, Y + Dy);
}                                /* RenderIntProto */
#endif

/*---------------------------------------------------------------------------*/
/**
 * This routine truncates Param to lie within the range
 * of Min-Max inclusive.  If a truncation is performed, and
 * Id is not null, an warning message is printed.
 *
 * @param Param   parameter value to be truncated
 * @param Min, Max  parameter limits (inclusive)
 * @param Id    string id of parameter for error messages
 *
 * Globals: none
 *
 * @return Truncated parameter.
 * @note Exceptions: none
 * @note History: Fri Feb  8 11:54:28 1991, DSJ, Created.
 */
int TruncateParam(FLOAT32 Param, int Min, int Max, char *Id) {
  if (Param < Min) {
    if (Id)
      cprintf("Warning: Param %s truncated from %f to %d!\n",
              Id, Param, Min);
    Param = Min;
  } else if (Param > Max) {
    if (Id)
      cprintf("Warning: Param %s truncated from %f to %d!\n",
              Id, Param, Max);
    Param = Max;
  }
  return static_cast<int>(floor(Param));
}                                /* TruncateParam */


/*---------------------------------------------------------------------------*/

/**
 * Initializes the int matcher window if it is not already
 * initialized.
 */
void InitIntMatchWindowIfReqd() {
  if (IntMatchWindow == NULL) {
    IntMatchWindow = c_create_window("IntMatchWindow", 50, 200,
                                     520, 520,
                                     -130.0, 130.0, -130.0, 130.0);
    SVMenuNode* popup_menu = new SVMenuNode();

    popup_menu->AddChild("Debug Adapted classes", IDA_ADAPTIVE,
                         "x", "Class to debug");
    popup_menu->AddChild("Debug Static classes", IDA_STATIC,
                         "x", "Class to debug");
    popup_menu->AddChild("Debug Both", IDA_BOTH,
                         "x", "Class to debug");
    popup_menu->BuildMenu(IntMatchWindow, false);
  }
}

/**
 * Initializes the proto display window if it is not already
 * initialized.
 */
void InitProtoDisplayWindowIfReqd() {
  if (ProtoDisplayWindow == NULL) {
    ProtoDisplayWindow = c_create_window("ProtoDisplayWindow", 50, 200,
                                         520, 520,
                                         -130.0, 130.0, -130.0, 130.0);
  }
}

/**
 * Initializes the feature display window if it is not already
 * initialized.
 */
void InitFeatureDisplayWindowIfReqd() {
  if (FeatureDisplayWindow == NULL) {
    FeatureDisplayWindow = c_create_window("FeatureDisplayWindow", 50, 200,
                                           520, 520,
                                           -130.0, 130.0, -130.0, 130.0);
  }
}
