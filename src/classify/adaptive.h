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

#include "intproto.h"
#include "oldlist.h"

#include <cstdio>

namespace tesseract {

struct TEMP_PROTO_STRUCT {
  uint16_t ProtoId;
  PROTO_STRUCT Proto;
};

struct TEMP_CONFIG_STRUCT {
  TEMP_CONFIG_STRUCT() = default;
  TEMP_CONFIG_STRUCT(int MaxProtoId, int FontinfoId);
  ~TEMP_CONFIG_STRUCT();
  uint8_t NumTimesSeen;
  uint8_t ProtoVectorSize;
  PROTO_ID MaxProtoId;
  BIT_VECTOR Protos;
  int FontinfoId; // font information inferred from pre-trained templates
};

struct PERM_CONFIG_STRUCT {
  PERM_CONFIG_STRUCT() = default;
  ~PERM_CONFIG_STRUCT();
  UNICHAR_ID *Ambigs;
  int FontinfoId; // font information inferred from pre-trained templates
};

union ADAPTED_CONFIG {
  TEMP_CONFIG_STRUCT *Temp;
  PERM_CONFIG_STRUCT *Perm;
};

struct ADAPT_CLASS_STRUCT {
  ADAPT_CLASS_STRUCT();
  ~ADAPT_CLASS_STRUCT();
  uint8_t NumPermConfigs;
  uint8_t MaxNumTimesSeen; // maximum number of times any TEMP_CONFIG_STRUCT was seen
                           // (cut at matcher_min_examples_for_prototyping)
  BIT_VECTOR PermProtos;
  BIT_VECTOR PermConfigs;
  LIST TempProtos;
  ADAPTED_CONFIG Config[MAX_NUM_CONFIGS];
};

class ADAPT_TEMPLATES_STRUCT {
public:
  ADAPT_TEMPLATES_STRUCT() = default;
  ADAPT_TEMPLATES_STRUCT(UNICHARSET &unicharset);
  ~ADAPT_TEMPLATES_STRUCT();
  INT_TEMPLATES_STRUCT *Templates;
  int NumNonEmptyClasses;
  uint8_t NumPermClasses;
  ADAPT_CLASS_STRUCT *Class[MAX_NUM_CLASSES];
};

/*----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------*/
#define NumNonEmptyClassesIn(Template) ((Template)->NumNonEmptyClasses)

#define IsEmptyAdaptedClass(Class) ((Class)->NumPermConfigs == 0 && (Class)->TempProtos == NIL_LIST)

#define ConfigIsPermanent(Class, ConfigId) (test_bit((Class)->PermConfigs, ConfigId))

#define MakeConfigPermanent(Class, ConfigId) (SET_BIT((Class)->PermConfigs, ConfigId))

#define MakeProtoPermanent(Class, ProtoId) (SET_BIT((Class)->PermProtos, ProtoId))

#define TempConfigFor(Class, ConfigId) ((Class)->Config[ConfigId].Temp)

#define PermConfigFor(Class, ConfigId) ((Class)->Config[ConfigId].Perm)

#define IncreaseConfidence(TempConfig) ((TempConfig)->NumTimesSeen++)

void AddAdaptedClass(ADAPT_TEMPLATES_STRUCT *Templates, ADAPT_CLASS_STRUCT *Class, CLASS_ID ClassId);

ADAPT_CLASS_STRUCT *ReadAdaptedClass(tesseract::TFile *File);

PERM_CONFIG_STRUCT *ReadPermConfig(tesseract::TFile *File);

TEMP_CONFIG_STRUCT *ReadTempConfig(tesseract::TFile *File);

void WriteAdaptedClass(FILE *File, ADAPT_CLASS_STRUCT *Class, int NumConfigs);

void WritePermConfig(FILE *File, PERM_CONFIG_STRUCT *Config);

void WriteTempConfig(FILE *File, TEMP_CONFIG_STRUCT *Config);

} // namespace tesseract

#endif
