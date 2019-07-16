/* -*-C-*-
 ******************************************************************************
 *
 * File:        protos.cpp  (Formerly protos.c)
 * Author:      Mark Seaman, OCR Technology
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#define _USE_MATH_DEFINES       // for M_PI
#include "protos.h"
#include <cmath>                // for M_PI
#include <cstdio>
#include "emalloc.h"
#include "callcpp.h"
#include "tprintf.h"
#include "classify.h"
#include "params.h"
#include "intproto.h"

#define PROTO_INCREMENT   32
#define CONFIG_INCREMENT  16

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**
 * @name AddConfigToClass
 *
 * Add a new config to this class.  Malloc new space and copy the
 * old configs if necessary.  Return the config id for the new config.
 *
 * @param Class The class to add to
 */
int AddConfigToClass(CLASS_TYPE Class) {
  int NewNumConfigs;
  int NewConfig;
  int MaxNumProtos;
  BIT_VECTOR Config;

  MaxNumProtos = Class->MaxNumProtos;
  ASSERT_HOST(MaxNumProtos <= MAX_NUM_PROTOS);

  if (Class->NumConfigs >= Class->MaxNumConfigs) {
    /* add configs in CONFIG_INCREMENT chunks at a time */
    NewNumConfigs = (((Class->MaxNumConfigs + CONFIG_INCREMENT) /
      CONFIG_INCREMENT) * CONFIG_INCREMENT);

    Class->Configurations =
      static_cast<CONFIGS>(Erealloc (Class->Configurations,
      sizeof (BIT_VECTOR) * NewNumConfigs));

    Class->MaxNumConfigs = NewNumConfigs;
  }
  NewConfig = Class->NumConfigs++;
  Config = NewBitVector(MAX_NUM_PROTOS);
  Class->Configurations[NewConfig] = Config;
  zero_all_bits (Config, WordsInVectorOfSize(MAX_NUM_PROTOS));

  return (NewConfig);
}


/**
 * @name AddProtoToClass
 *
 * Add a new proto to this class.  Malloc new space and copy the
 * old protos if necessary.  Return the proto id for the new proto.
 *
 * @param Class The class to add to
 */
int AddProtoToClass(CLASS_TYPE Class) {
  if (Class->NumProtos >= Class->MaxNumProtos) {
    /* add protos in PROTO_INCREMENT chunks at a time */
    int NewNumProtos = (((Class->MaxNumProtos + PROTO_INCREMENT) /
      PROTO_INCREMENT) * PROTO_INCREMENT);

    Class->Prototypes = static_cast<PROTO>(Erealloc (Class->Prototypes,
      sizeof (PROTO_STRUCT) *
      NewNumProtos));

    Class->MaxNumProtos = NewNumProtos;
    ASSERT_HOST(NewNumProtos <= MAX_NUM_PROTOS);
  }
  int NewProto = Class->NumProtos++;
  ASSERT_HOST(Class->NumProtos <= MAX_NUM_PROTOS);
  return (NewProto);
}


/**********************************************************************
 * FillABC
 *
 * Fill in Protos A, B, C fields based on the X, Y, Angle fields.
 **********************************************************************/
void FillABC(PROTO Proto) {
  float Slope, Intercept, Normalizer;

  Slope = tan(Proto->Angle * 2.0 * M_PI);
  Intercept = Proto->Y - Slope * Proto->X;
  Normalizer = 1.0 / sqrt (Slope * Slope + 1.0);
  Proto->A = Slope * Normalizer;
  Proto->B = -Normalizer;
  Proto->C = Intercept * Normalizer;
}


/**********************************************************************
 * FreeClass
 *
 * Deallocate the memory consumed by the specified class.
 **********************************************************************/
void FreeClass(CLASS_TYPE Class) {
  if (Class) {
    FreeClassFields(Class);
    delete Class;
  }
}


/**********************************************************************
 * FreeClassFields
 *
 * Deallocate the memory consumed by subfields of the specified class.
 **********************************************************************/
void FreeClassFields(CLASS_TYPE Class) {
  int i;

  if (Class) {
    if (Class->MaxNumProtos > 0) free(Class->Prototypes);
    if (Class->MaxNumConfigs > 0) {
      for (i = 0; i < Class->NumConfigs; i++)
        FreeBitVector (Class->Configurations[i]);
      free(Class->Configurations);
    }
  }
}

/**********************************************************************
 * NewClass
 *
 * Allocate a new class with enough memory to hold the specified number
 * of prototypes and configurations.
 **********************************************************************/
CLASS_TYPE NewClass(int NumProtos, int NumConfigs) {
  CLASS_TYPE Class;

  Class = new CLASS_STRUCT;

  if (NumProtos > 0)
    Class->Prototypes = static_cast<PROTO>(Emalloc (NumProtos * sizeof (PROTO_STRUCT)));

  if (NumConfigs > 0)
    Class->Configurations = static_cast<CONFIGS>(Emalloc (NumConfigs *
      sizeof (BIT_VECTOR)));
  Class->MaxNumProtos = NumProtos;
  Class->MaxNumConfigs = NumConfigs;
  Class->NumProtos = 0;
  Class->NumConfigs = 0;
  return (Class);

}
