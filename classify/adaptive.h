/******************************************************************************
 **	Filename:    adaptive.h
 **	Purpose:     Interface to adaptive matcher.
 **	Author:      Dan Johnson
 **	History:     Fri Mar  8 10:00:49 1991, DSJ, Created.
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
#ifndef ADAPTIVE_H
#define ADAPTIVE_H

/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "oldlist.h"
#include "intproto.h"
#include <stdio.h>

typedef struct
{
  uinT16 ProtoId;
  uinT16 dummy;
  PROTO_STRUCT Proto;
}


TEMP_PROTO_STRUCT;
typedef TEMP_PROTO_STRUCT *TEMP_PROTO;

typedef struct
{
  uinT8 NumTimesSeen;
  uinT8 ProtoVectorSize;
  PROTO_ID MaxProtoId;
  LIST ContextsSeen;
  BIT_VECTOR Protos;
  int FontinfoId;  // font information inferred from pre-trained templates
} TEMP_CONFIG_STRUCT;
typedef TEMP_CONFIG_STRUCT *TEMP_CONFIG;

typedef struct
{
  UNICHAR_ID *Ambigs;
  int FontinfoId;  // font information inferred from pre-trained templates
} PERM_CONFIG_STRUCT;
typedef PERM_CONFIG_STRUCT *PERM_CONFIG;

typedef union
{
  TEMP_CONFIG Temp;
  PERM_CONFIG Perm;
} ADAPTED_CONFIG;

typedef struct
{
  uinT8 NumPermConfigs;
  uinT8 MaxNumTimesSeen;  // maximum number of times any TEMP_CONFIG was seen
  uinT8 dummy[2];         // (cut at matcher_min_examples_for_prototyping)
  BIT_VECTOR PermProtos;
  BIT_VECTOR PermConfigs;
  LIST TempProtos;
  ADAPTED_CONFIG Config[MAX_NUM_CONFIGS];
} ADAPT_CLASS_STRUCT;
typedef ADAPT_CLASS_STRUCT *ADAPT_CLASS;

typedef struct
{
  INT_TEMPLATES Templates;
  int NumNonEmptyClasses;
  uinT8 NumPermClasses;
  uinT8 dummy[3];
  ADAPT_CLASS Class[MAX_NUM_CLASSES];
} ADAPT_TEMPLATES_STRUCT;
typedef ADAPT_TEMPLATES_STRUCT *ADAPT_TEMPLATES;

/*----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------*/
#define NumNonEmptyClassesIn(Template) ((Template)->NumNonEmptyClasses)

#define IsEmptyAdaptedClass(Class) ((Class)->NumPermConfigs == 0 &&      \
(Class)->TempProtos == NIL_LIST)

#define ConfigIsPermanent(Class,ConfigId)		\
(test_bit ((Class)->PermConfigs, ConfigId))

#define MakeConfigPermanent(Class,ConfigId)	\
(SET_BIT ((Class)->PermConfigs, ConfigId))

#define MakeProtoPermanent(Class,ProtoId)	\
(SET_BIT ((Class)->PermProtos, ProtoId))

#define TempConfigFor(Class,ConfigId)	\
((Class)->Config[ConfigId].Temp)

#define PermConfigFor(Class,ConfigId)	\
((Class)->Config[ConfigId].Perm)

#define IncreaseConfidence(TempConfig)	\
((TempConfig)->NumTimesSeen++)

void AddAdaptedClass(ADAPT_TEMPLATES Templates,
                    ADAPT_CLASS Class,
                    CLASS_ID ClassId);

void FreeTempProto(void *arg);

void FreeTempConfig(TEMP_CONFIG Config);

ADAPT_CLASS NewAdaptedClass();

void free_adapted_class(ADAPT_CLASS adapt_class);

void free_adapted_templates(ADAPT_TEMPLATES templates);

TEMP_CONFIG NewTempConfig(int MaxProtoId, int FontinfoId);

TEMP_PROTO NewTempProto();

ADAPT_CLASS ReadAdaptedClass(FILE *File);

PERM_CONFIG ReadPermConfig(FILE *File);

TEMP_CONFIG ReadTempConfig(FILE *File);

void WriteAdaptedClass(FILE *File, ADAPT_CLASS Class, int NumConfigs);

void WritePermConfig(FILE *File, PERM_CONFIG Config);

void WriteTempConfig(FILE *File, TEMP_CONFIG Config);

#endif
