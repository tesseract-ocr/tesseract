/* -*-C-*-
 ******************************************************************************
 *
 * File:        protos.cpp  (Formerly protos.c)
 * Description:
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Mon Mar  4 14:51:24 1991 (Dan Johnson) danj@hpgrlj
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "protos.h"
#include "emalloc.h"
#include "callcpp.h"
#include "tprintf.h"
#include "globals.h"
#include "classify.h"
#include "params.h"

#include <cstdio>
#include <cmath>

#define PROTO_INCREMENT   32
#define CONFIG_INCREMENT  16

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
CLASS_STRUCT TrainingData[NUMBER_OF_CLASSES];

STRING_VAR(classify_training_file, "MicroFeatures", "Training file");

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

  if (Class->NumConfigs >= Class->MaxNumConfigs) {
    /* add configs in CONFIG_INCREMENT chunks at a time */
    NewNumConfigs = (((Class->MaxNumConfigs + CONFIG_INCREMENT) /
      CONFIG_INCREMENT) * CONFIG_INCREMENT);

    Class->Configurations =
      (CONFIGS) Erealloc (Class->Configurations,
      sizeof (BIT_VECTOR) * NewNumConfigs);

    Class->MaxNumConfigs = NewNumConfigs;
  }
  NewConfig = Class->NumConfigs++;
  Config = NewBitVector (MaxNumProtos);
  Class->Configurations[NewConfig] = Config;
  zero_all_bits (Config, WordsInVectorOfSize (MaxNumProtos));

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
  int i;
  int Bit;
  int NewNumProtos;
  int NewProto;
  BIT_VECTOR Config;

  if (Class->NumProtos >= Class->MaxNumProtos) {
    /* add protos in PROTO_INCREMENT chunks at a time */
    NewNumProtos = (((Class->MaxNumProtos + PROTO_INCREMENT) /
      PROTO_INCREMENT) * PROTO_INCREMENT);

    Class->Prototypes = (PROTO) Erealloc (Class->Prototypes,
      sizeof (PROTO_STRUCT) *
      NewNumProtos);

    Class->MaxNumProtos = NewNumProtos;

    for (i = 0; i < Class->NumConfigs; i++) {
      Config = Class->Configurations[i];
      Class->Configurations[i] = ExpandBitVector (Config, NewNumProtos);

      for (Bit = Class->NumProtos; Bit < NewNumProtos; Bit++)
        reset_bit(Config, Bit);
    }
  }
  NewProto = Class->NumProtos++;
  if (Class->NumProtos > MAX_NUM_PROTOS) {
    tprintf("Ouch! number of protos = %d, vs max of %d!",
            Class->NumProtos, MAX_NUM_PROTOS);
  }
  return (NewProto);
}


/**
 * @name ClassConfigLength
 *
 * Return the length of all the protos in this class.
 *
 * @param Class The class to add to
 * @param Config FIXME
 */
float ClassConfigLength(CLASS_TYPE Class, BIT_VECTOR Config) {
  int16_t Pid;
  float TotalLength = 0;

  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    if (test_bit (Config, Pid)) {

      TotalLength += (ProtoIn (Class, Pid))->Length;
    }
  }
  return (TotalLength);
}


/**
 * @name ClassProtoLength
 *
 * Return the length of all the protos in this class.
 *
 * @param Class The class to use
 */
float ClassProtoLength(CLASS_TYPE Class) {
  int16_t Pid;
  float TotalLength = 0;

  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    TotalLength += (ProtoIn (Class, Pid))->Length;
  }
  return (TotalLength);
}


/**
 * @name CopyProto
 *
 * Copy the first proto into the second.
 *
 * @param Src Source
 * @param Dest Destination
 */
void CopyProto(PROTO Src, PROTO Dest) {
  Dest->X = Src->X;
  Dest->Y = Src->Y;
  Dest->Length = Src->Length;
  Dest->Angle = Src->Angle;
  Dest->A = Src->A;
  Dest->B = Src->B;
  Dest->C = Src->C;
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
    Class->Prototypes = (PROTO) Emalloc (NumProtos * sizeof (PROTO_STRUCT));

  if (NumConfigs > 0)
    Class->Configurations = (CONFIGS) Emalloc (NumConfigs *
      sizeof (BIT_VECTOR));
  Class->MaxNumProtos = NumProtos;
  Class->MaxNumConfigs = NumConfigs;
  Class->NumProtos = 0;
  Class->NumConfigs = 0;
  return (Class);

}


/**********************************************************************
 * PrintProtos
 *
 * Print the list of prototypes in this class type.
 **********************************************************************/
void PrintProtos(CLASS_TYPE Class) {
  int16_t Pid;

  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    cprintf ("Proto %d:\t", Pid);
    PrintProto (ProtoIn (Class, Pid));
    cprintf ("\t");
    PrintProtoLine (ProtoIn (Class, Pid));
    tprintf("\n");
  }
}
