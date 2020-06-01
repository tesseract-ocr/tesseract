/******************************************************************************
 *
 * File:         protos.h
 * Author:       Mark Seaman, SW Productivity
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *****************************************************************************/

#ifndef PROTOS_H
#define PROTOS_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "bitvec.h"
#include "params.h"
#include <tesseract/unichar.h>
#include "unicity_table.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
using CONFIGS = BIT_VECTOR*;

typedef struct {
  float A;
  float B;
  float C;
  float X;
  float Y;
  float Angle;
  float Length;
} PROTO_STRUCT;
using PROTO = PROTO_STRUCT*;

struct CLASS_STRUCT {
  CLASS_STRUCT()
      : NumProtos(0),
        MaxNumProtos(0),
        Prototypes(nullptr),
        NumConfigs(0),
        MaxNumConfigs(0),
        Configurations(nullptr) {}
  int16_t NumProtos;
  int16_t MaxNumProtos;
  PROTO Prototypes;
  int16_t NumConfigs;
  int16_t MaxNumConfigs;
  CONFIGS Configurations;
  UnicityTableEqEq<int> font_set;
};
using CLASS_TYPE = CLASS_STRUCT*;
using CLASSES = CLASS_STRUCT*;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**
 * AddProtoToConfig
 *
 * Set a single proto bit in the specified configuration.
 */

#define AddProtoToConfig(Pid, Config) (SET_BIT(Config, Pid))

/**
 * ProtoIn
 *
 * Choose the selected prototype in this class record.  Return the
 * pointer to it (type PROTO).
 */

#define ProtoIn(Class, Pid) (&(Class)->Prototypes[Pid])

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int AddConfigToClass(CLASS_TYPE Class);

int AddProtoToClass(CLASS_TYPE Class);

void FillABC(PROTO Proto);

void FreeClass(CLASS_TYPE Class);

void FreeClassFields(CLASS_TYPE Class);

void InitPrototypes();

CLASS_TYPE NewClass(int NumProtos, int NumConfigs);

#endif
