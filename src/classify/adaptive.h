/******************************************************************************
 ** Filename:   adaptive.h
 ** Purpose:    Interface to adaptive matcher.
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
 ******************************************************************************/
#ifndef ADAPTIVE_H
#define ADAPTIVE_H

/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include <cstdio>
#include "intproto.h"
#include "oldlist.h"

typedef struct {
  uint16_t ProtoId;
  PROTO_STRUCT Proto;
}

TEMP_PROTO_STRUCT;
using TEMP_PROTO = TEMP_PROTO_STRUCT*;

typedef struct {
  uint8_t NumTimesSeen;
  uint8_t ProtoVectorSize;
  PROTO_ID MaxProtoId;
  BIT_VECTOR Protos;
  int FontinfoId;  // font information inferred from pre-trained templates
} TEMP_CONFIG_STRUCT;
using TEMP_CONFIG = TEMP_CONFIG_STRUCT*;

typedef struct {
  UNICHAR_ID* Ambigs;
  int FontinfoId;  // font information inferred from pre-trained templates
} PERM_CONFIG_STRUCT;
using PERM_CONFIG = PERM_CONFIG_STRUCT*;

typedef union {
  TEMP_CONFIG Temp;
  PERM_CONFIG Perm;
} ADAPTED_CONFIG;

typedef struct {
  uint8_t NumPermConfigs;
  uint8_t MaxNumTimesSeen;  // maximum number of times any TEMP_CONFIG was seen
                            // (cut at matcher_min_examples_for_prototyping)
  BIT_VECTOR PermProtos;
  BIT_VECTOR PermConfigs;
  LIST TempProtos;
  ADAPTED_CONFIG Config[MAX_NUM_CONFIGS];
} ADAPT_CLASS_STRUCT;
using ADAPT_CLASS = ADAPT_CLASS_STRUCT*;

typedef struct {
  INT_TEMPLATES Templates;
  int NumNonEmptyClasses;
  uint8_t NumPermClasses;
  ADAPT_CLASS Class[MAX_NUM_CLASSES];
} ADAPT_TEMPLATES_STRUCT;
using ADAPT_TEMPLATES = ADAPT_TEMPLATES_STRUCT*;

/*----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------*/
#define NumNonEmptyClassesIn(Template) ((Template)->NumNonEmptyClasses)

#define IsEmptyAdaptedClass(Class) \
  ((Class)->NumPermConfigs == 0 && (Class)->TempProtos == NIL_LIST)

#define ConfigIsPermanent(Class, ConfigId) \
  (test_bit((Class)->PermConfigs, ConfigId))

#define MakeConfigPermanent(Class, ConfigId) \
  (SET_BIT((Class)->PermConfigs, ConfigId))

#define MakeProtoPermanent(Class, ProtoId) \
  (SET_BIT((Class)->PermProtos, ProtoId))

#define TempConfigFor(Class, ConfigId) ((Class)->Config[ConfigId].Temp)

#define PermConfigFor(Class, ConfigId) ((Class)->Config[ConfigId].Perm)

#define IncreaseConfidence(TempConfig) ((TempConfig)->NumTimesSeen++)

void AddAdaptedClass(ADAPT_TEMPLATES Templates, ADAPT_CLASS Class,
                     CLASS_ID ClassId);

void FreeTempProto(void* arg);

void FreeTempConfig(TEMP_CONFIG Config);

ADAPT_CLASS NewAdaptedClass();

void free_adapted_class(ADAPT_CLASS adapt_class);

void free_adapted_templates(ADAPT_TEMPLATES templates);

TEMP_CONFIG NewTempConfig(int MaxProtoId, int FontinfoId);

TEMP_PROTO NewTempProto();

ADAPT_CLASS ReadAdaptedClass(tesseract::TFile* File);

PERM_CONFIG ReadPermConfig(tesseract::TFile* File);

TEMP_CONFIG ReadTempConfig(tesseract::TFile* File);

void WriteAdaptedClass(FILE* File, ADAPT_CLASS Class, int NumConfigs);

void WritePermConfig(FILE* File, PERM_CONFIG Config);

void WriteTempConfig(FILE* File, TEMP_CONFIG Config);

#endif
