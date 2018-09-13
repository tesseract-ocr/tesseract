/* -*-C-*-
 ******************************************************************************
 *
 * File:         protos.h
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Jul 12 10:06:55 1991 (Dan Johnson) danj@hpgrlj
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
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
#include "unichar.h"
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
              C o n s t a n t s
----------------------------------------------------------------------*/
#define NUMBER_OF_CLASSES MAX_NUM_CLASSES
#define Y_OFFSET -40.0
#define FEATURE_SCALE 100.0

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern CLASS_STRUCT TrainingData[];

extern STRING_VAR_H(classify_training_file, "MicroFeatures", "Training file");

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
 * RemoveProtoFromConfig
 *
 * Clear a single proto bit in the specified configuration.
 */

#define RemoveProtoFromConfig(Pid, Config) (reset_bit(Config, Pid))

/**
 * ClassOfChar
 *
 * Return the class of a particular ASCII character value.
 */

#define ClassOfChar(Char) \
  ((TrainingData[Char].NumProtos) ? (&TrainingData[Char]) : NO_CLASS)

/**
 * ProtoIn
 *
 * Choose the selected prototype in this class record.  Return the
 * pointer to it (type PROTO).
 */

#define ProtoIn(Class, Pid) (&(Class)->Prototypes[Pid])

/**
 * PrintProto
 *
 * Print out the contents of a prototype.   The 'Proto' argument is of
 * type 'PROTO'.
 */

#define PrintProto(Proto)                                                     \
  (tprintf("X=%4.2f, Y=%4.2f, Length=%4.2f, Angle=%4.2f", Proto->X, Proto->Y, \
           Proto->Length, Proto->Angle))

/**
 * PrintProtoLine
 *
 * Print out the contents of a prototype.   The 'Proto' argument is of
 * type 'PROTO'.
 */

#define PrintProtoLine(Proto) \
  (cprintf("A=%4.2f, B=%4.2f, C=%4.2f", Proto->A, Proto->B, Proto->C))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
int AddConfigToClass(CLASS_TYPE Class);

int AddProtoToClass(CLASS_TYPE Class);

float ClassConfigLength(CLASS_TYPE Class, BIT_VECTOR Config);

float ClassProtoLength(CLASS_TYPE Class);

void CopyProto(PROTO Src, PROTO Dest);

void FillABC(PROTO Proto);

void FreeClass(CLASS_TYPE Class);

void FreeClassFields(CLASS_TYPE Class);

void InitPrototypes();

CLASS_TYPE NewClass(int NumProtos, int NumConfigs);

void PrintProtos(CLASS_TYPE Class);

#endif
