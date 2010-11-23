/* -*-C-*-
 ********************************************************************************
 *
 * File:        protos.c  (Formerly protos.c)
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
 *********************************************************************************/
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "protos.h"
#include "const.h"
#include "emalloc.h"
#include "freelist.h"
#include "callcpp.h"
#include "tprintf.h"
#include "scanutils.h"
#include "globals.h"
#include "classify.h"
#include "params.h"

#include <stdio.h>
#include <math.h>

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
FLOAT32 ClassConfigLength(CLASS_TYPE Class, BIT_VECTOR Config) {
  inT16 Pid;
  FLOAT32 TotalLength = 0;

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
FLOAT32 ClassProtoLength(CLASS_TYPE Class) {
  inT16 Pid;
  FLOAT32 TotalLength = 0;

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
  FLOAT32 Slope, Intercept, Normalizer;

  Slope = tan (Proto->Angle * 2.0 * PI);
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
    if (Class->MaxNumProtos > 0)
      memfree (Class->Prototypes);
    if (Class->MaxNumConfigs > 0) {
      for (i = 0; i < Class->NumConfigs; i++)
        FreeBitVector (Class->Configurations[i]);
      memfree (Class->Configurations);
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
  inT16 Pid;

  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    cprintf ("Proto %d:\t", Pid);
    PrintProto (ProtoIn (Class, Pid));
    cprintf ("\t");
    PrintProtoLine (ProtoIn (Class, Pid));
    new_line();
  }
}


namespace tesseract {
/**
 * @name ReadClassFile
 *
 * Read in the training data from a file.  All of the classes are read
 * in.  The results are stored in the global variable, 'TrainingData'.
 */
void Classify::ReadClassFile() {
 FILE *File;
 char TextLine[CHARS_PER_LINE];
 char unichar[CHARS_PER_LINE];

 cprintf ("Reading training data from '%s' ...",
          static_cast<STRING>(classify_training_file).string());
 fflush(stdout);

 File = open_file(static_cast<STRING>(classify_training_file).string(), "r");
 while (fgets (TextLine, CHARS_PER_LINE, File) != NULL) {

   sscanf(TextLine, "%s", unichar);
   ReadClassFromFile (File, unicharset.unichar_to_id(unichar));
   fgets(TextLine, CHARS_PER_LINE, File);
   fgets(TextLine, CHARS_PER_LINE, File);
 }
 fclose(File);
 new_line();
}
}  // namespace tesseract

/**
 * ReadClassFromFile
 *
 * Read in a class description (protos and configs) from a file.  Update
 * the class structure record.
 */
void ReadClassFromFile(FILE *File, UNICHAR_ID unichar_id) {
  CLASS_TYPE Class;

  Class = &TrainingData[unichar_id];

  ReadProtos(File, Class);

  ReadConfigs(File, Class);
}

/**
 * ReadConfigs
 *
 * Read the prototype configurations for this class from a file.  Read
 * the requested number of lines.
 */
void ReadConfigs(register FILE *File, CLASS_TYPE Class) {
  inT16 Cid;
  register inT16 Wid;
  register BIT_VECTOR ThisConfig;
  int NumWords;
  int NumConfigs;

  fscanf (File, "%d %d\n", &NumConfigs, &NumWords);
  Class->NumConfigs = NumConfigs;
  Class->MaxNumConfigs = NumConfigs;
  Class->Configurations =
    (CONFIGS) Emalloc (sizeof (BIT_VECTOR) * NumConfigs);
  NumWords = WordsInVectorOfSize (Class->NumProtos);

  for (Cid = 0; Cid < NumConfigs; Cid++) {

    ThisConfig = NewBitVector (Class->NumProtos);
    for (Wid = 0; Wid < NumWords; Wid++)
      fscanf (File, "%x", &ThisConfig[Wid]);
    Class->Configurations[Cid] = ThisConfig;
  }
}


/**
 * ReadProtos
 *
 * Read in all the prototype information from a file.  Read the number
 * of lines requested.
 */
void ReadProtos(register FILE *File, CLASS_TYPE Class) {
  register inT16 Pid;
  register PROTO Proto;
  int NumProtos;

  fscanf (File, "%d\n", &NumProtos);
  Class->NumProtos = NumProtos;
  Class->MaxNumProtos = NumProtos;
  Class->Prototypes = (PROTO) Emalloc (sizeof (PROTO_STRUCT) * NumProtos);

  for (Pid = 0; Pid < NumProtos; Pid++) {
    Proto = ProtoIn (Class, Pid);
    fscanf (File, "%f %f %f %f %f %f %f\n",
      &Proto->X,
      &Proto->Y,
      &Proto->Length,
      &Proto->Angle,
      &Proto->A,
      &Proto->B, &Proto->C);
  }
}


/**
 * @name SplitProto
 *
 * Add a new proto to this class.  Malloc new space and copy the
 * old protos if necessary.  Return the proto id for the new proto.
 * Update all configurations so that each config which contained the
 * specified old proto will also contain the new proto.  The caller
 * is responsible for actually filling in the appropriate proto params.
 */
int SplitProto(CLASS_TYPE Class, int OldPid) {
  int i;
  int NewPid;
  BIT_VECTOR Config;

  NewPid = AddProtoToClass (Class);

  for (i = 0; i < Class->NumConfigs; i++) {
    Config = Class->Configurations[i];
    if (test_bit (Config, OldPid))
      SET_BIT(Config, NewPid);
  }
  return (NewPid);
}


/**
 * @deprecated
 * @nameWriteOldConfigFile
 *
 * Write the configs in the given class to the specified file in the
 * old config format.
 * 
 * @param File The file to write to
 * @param Class The class to write
 */
void WriteOldConfigFile(FILE *File, CLASS_TYPE Class) {
  int Cid, Pid;
  BIT_VECTOR Config;

  fprintf (File, "%d %d\n", Class->NumConfigs, Class->NumProtos);

  for (Cid = 0; Cid < Class->NumConfigs; Cid++) {
    fprintf (File, "1 ");

    Config = Class->Configurations[Cid];

    for (Pid = 0; Pid < Class->NumProtos; Pid++) {
      if (test_bit (Config, Pid))
        fprintf (File, "1");
      else
        fprintf (File, "0");
    }
    fprintf (File, "\n");
  }
}


/**
 * @deprecated
 * @name WriteOldProtoFile
 *
 * Write the protos in the given class to the specified file in the
 * old proto format.
 * 
 * @param File The file to write to
 * @param Class The class to write
 */
void WriteOldProtoFile(FILE *File, CLASS_TYPE Class) {
  int Pid;
  PROTO Proto;

  /* print old header */
  fprintf (File, "6\n");
  fprintf (File, "linear   essential      -0.500000   0.500000\n");
  fprintf (File, "linear   essential      -0.250000   0.750000\n");
  fprintf (File, "linear   essential       0.000000   1.000000\n");
  fprintf (File, "circular essential       0.000000   1.000000\n");
  fprintf (File, "linear   non-essential  -0.500000   0.500000\n");
  fprintf (File, "linear   non-essential  -0.500000   0.500000\n");

  for (Pid = 0; Pid < Class->NumProtos; Pid++) {
    Proto = ProtoIn (Class, Pid);

    fprintf (File, "significant   elliptical   1\n");
    fprintf (File, "     %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
      Proto->X, Proto->Y,
      Proto->Length, Proto->Angle, 0.0, 0.0);
    fprintf (File, "     %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
      0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001);
  }
}
