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
#include "debug.h"
#include "const.h"
#include "emalloc.h"
#include "freelist.h"
#include "callcpp.h"
#include "tprintf.h"
#include "adaptmatch.h"
#include "scanutils.h"
#include "globals.h"

#include <stdio.h>
#include <math.h>

#define PROTO_INCREMENT   32
#define CONFIG_INCREMENT  16

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
CLASS_STRUCT TrainingData[NUMBER_OF_CLASSES];

char *TrainingFile;

//extern int LearningDebugLevel;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * AddConfigToClass
 *
 * Add a new config to this class.  Malloc new space and copy the
 * old configs if necessary.  Return the config id for the new config.
 **********************************************************************/
int AddConfigToClass(CLASS_TYPE Class) {
  int NewNumConfigs;
  int NewConfig;
  int MaxNumProtos;
  BIT_VECTOR Config;

  MaxNumProtos = Class->MaxNumProtos;

  if (NumConfigsIn (Class) >= Class->MaxNumConfigs) {
    /* add configs in CONFIG_INCREMENT chunks at a time */
    NewNumConfigs = (((Class->MaxNumConfigs + CONFIG_INCREMENT) /
      CONFIG_INCREMENT) * CONFIG_INCREMENT);

    Class->Configurations =
      (CONFIGS) Erealloc (Class->Configurations,
      sizeof (BIT_VECTOR) * NewNumConfigs);

    Class->MaxNumConfigs = NewNumConfigs;
  }
  NewConfig = NumConfigsIn (Class);
  NumConfigsIn (Class)++;
  Config = NewBitVector (MaxNumProtos);
  ConfigIn (Class, NewConfig) = Config;
  zero_all_bits (Config, WordsInVectorOfSize (MaxNumProtos));

  return (NewConfig);
}


/**********************************************************************
 * AddProtoToClass
 *
 * Add a new proto to this class.  Malloc new space and copy the
 * old protos if necessary.  Return the proto id for the new proto.
 **********************************************************************/
int AddProtoToClass(CLASS_TYPE Class) {
  int i;
  int Bit;
  int NewNumProtos;
  int NewProto;
  BIT_VECTOR Config;

  if (NumProtosIn (Class) >= Class->MaxNumProtos) {
    /* add protos in PROTO_INCREMENT chunks at a time */
    NewNumProtos = (((Class->MaxNumProtos + PROTO_INCREMENT) /
      PROTO_INCREMENT) * PROTO_INCREMENT);

    Class->Prototypes = (PROTO) Erealloc (Class->Prototypes,
      sizeof (PROTO_STRUCT) *
      NewNumProtos);

    Class->MaxNumProtos = NewNumProtos;

    for (i = 0; i < NumConfigsIn (Class); i++) {
      Config = ConfigIn (Class, i);
      ConfigIn (Class, i) = ExpandBitVector (Config, NewNumProtos);

      for (Bit = NumProtosIn (Class); Bit < NewNumProtos; Bit++)
        reset_bit(Config, Bit);
    }
  }
  NewProto = NumProtosIn (Class);
  NumProtosIn (Class)++;
  if (NumProtosIn(Class) > MAX_NUM_PROTOS) {
    tprintf("Ouch! number of protos = %d, vs max of %d!",
            NumProtosIn(Class), MAX_NUM_PROTOS);
  }
  return (NewProto);
}


/**********************************************************************
 * ClassConfigLength
 *
 * Return the length of all the protos in this class.
 **********************************************************************/
FLOAT32 ClassConfigLength(CLASS_TYPE Class, BIT_VECTOR Config) {
  inT16 Pid;
  FLOAT32 TotalLength = 0;

  for (Pid = 0; Pid < NumProtosIn (Class); Pid++) {
    if (test_bit (Config, Pid)) {

      TotalLength += ProtoLength (ProtoIn (Class, Pid));
    }
  }
  return (TotalLength);
}


/**********************************************************************
 * ClassProtoLength
 *
 * Return the length of all the protos in this class.
 **********************************************************************/
FLOAT32 ClassProtoLength(CLASS_TYPE Class) {
  inT16 Pid;
  FLOAT32 TotalLength = 0;

  for (Pid = 0; Pid < NumProtosIn (Class); Pid++) {
    TotalLength += ProtoLength (ProtoIn (Class, Pid));
  }
  return (TotalLength);
}


/**********************************************************************
 * CopyProto
 *
 * Copy the first proto into the second.
 **********************************************************************/
void CopyProto(PROTO Src, PROTO Dest) {
  ProtoX (Dest) = ProtoX (Src);
  ProtoY (Dest) = ProtoY (Src);
  ProtoLength (Dest) = ProtoLength (Src);
  ProtoAngle (Dest) = ProtoAngle (Src);
  CoefficientA (Dest) = CoefficientA (Src);
  CoefficientB (Dest) = CoefficientB (Src);
  CoefficientC (Dest) = CoefficientC (Src);
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
    memfree(Class);
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
      for (i = 0; i < NumConfigsIn (Class); i++)
        FreeBitVector (ConfigIn (Class, i));
      memfree (Class->Configurations);
    }
  }
}


/**********************************************************************
 * InitPrototypes
 *
 * Initialize anything that needs to be initialized to work with the
 * functions in this file.
 **********************************************************************/
void InitPrototypes() {
  string_variable (TrainingFile, "TrainingFile", "MicroFeatures");
}


/**********************************************************************
 * NewClass
 *
 * Allocate a new class with enough memory to hold the specified number
 * of prototypes and configurations.
 **********************************************************************/
CLASS_TYPE NewClass(int NumProtos, int NumConfigs) {
  CLASS_TYPE Class;

  Class = (CLASS_TYPE) Emalloc (sizeof (CLASS_STRUCT));

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

  for (Pid = 0; Pid < NumProtosIn (Class); Pid++) {
    cprintf ("Proto %d:\t", Pid);
    PrintProto (ProtoIn (Class, Pid));
    cprintf ("\t");
    PrintProtoLine (ProtoIn (Class, Pid));
    new_line();
  }
}


/**********************************************************************
 * ReadClassFile
 *
 * Read in the training data from a file.  All of the classes are read
 * in.  The results are stored in the global variable, 'TrainingData'.
 **********************************************************************/
void ReadClassFile() {
 FILE *File;
 char TextLine[CHARS_PER_LINE];
 char unichar[CHARS_PER_LINE];

 cprintf ("Reading training data from '%s' ...", TrainingFile);
 fflush(stdout);

 File = open_file (TrainingFile, "r");
 while (fgets (TextLine, CHARS_PER_LINE, File) != NULL) {

   sscanf(TextLine, "%s", unichar);
   ReadClassFromFile (File, unicharset.unichar_to_id(unichar));
   fgets(TextLine, CHARS_PER_LINE, File);
   fgets(TextLine, CHARS_PER_LINE, File);
 }
 fclose(File);
 new_line();
}

/**********************************************************************
 * ReadClassFromFile
 *
 * Read in a class description (protos and configs) from a file.  Update
 * the class structure record.
 **********************************************************************/
void ReadClassFromFile(FILE *File, UNICHAR_ID unichar_id) {
  CLASS_TYPE Class;

  Class = &TrainingData[unichar_id];

  ReadProtos(File, Class);

  ReadConfigs(File, Class);
}

/**********************************************************************
 * ReadConfigs
 *
 * Read the prototype configurations for this class from a file.  Read
 * the requested number of lines.
 **********************************************************************/
void ReadConfigs(register FILE *File, CLASS_TYPE Class) {
  inT16 Cid;
  register inT16 Wid;
  register BIT_VECTOR ThisConfig;
  int NumWords;
  int NumConfigs;

  fscanf (File, "%d %d\n", &NumConfigs, &NumWords);
  NumConfigsIn (Class) = NumConfigs;
  Class->MaxNumConfigs = NumConfigs;
  Class->Configurations =
    (CONFIGS) Emalloc (sizeof (BIT_VECTOR) * NumConfigs);
  NumWords = WordsInVectorOfSize (NumProtosIn (Class));

  for (Cid = 0; Cid < NumConfigs; Cid++) {

    ThisConfig = NewBitVector (NumProtosIn (Class));
    for (Wid = 0; Wid < NumWords; Wid++)
      fscanf (File, "%x", &ThisConfig[Wid]);
    ConfigIn (Class, Cid) = ThisConfig;
  }
}


/**********************************************************************
 * ReadProtos
 *
 * Read in all the prototype information from a file.  Read the number
 * of lines requested.
 **********************************************************************/
void ReadProtos(register FILE *File, CLASS_TYPE Class) {
  register inT16 Pid;
  register PROTO Proto;
  int NumProtos;

  fscanf (File, "%d\n", &NumProtos);
  NumProtosIn (Class) = NumProtos;
  Class->MaxNumProtos = NumProtos;
  Class->Prototypes = (PROTO) Emalloc (sizeof (PROTO_STRUCT) * NumProtos);

  for (Pid = 0; Pid < NumProtos; Pid++) {
    Proto = ProtoIn (Class, Pid);
    fscanf (File, "%f %f %f %f %f %f %f\n",
      &ProtoX (Proto),
      &ProtoY (Proto),
      &ProtoLength (Proto),
      &ProtoAngle (Proto),
      &CoefficientA (Proto),
      &CoefficientB (Proto), &CoefficientC (Proto));
  }
}


/**********************************************************************
 * SplitProto
 *
 * Add a new proto to this class.  Malloc new space and copy the
 * old protos if necessary.  Return the proto id for the new proto.
 * Update all configurations so that each config which contained the
 * specified old proto will also contain the new proto.  The caller
 * is responsible for actually filling in the appropriate proto params.
 **********************************************************************/
int SplitProto(CLASS_TYPE Class, int OldPid) {
  int i;
  int NewPid;
  BIT_VECTOR Config;

  NewPid = AddProtoToClass (Class);

  for (i = 0; i < NumConfigsIn (Class); i++) {
    Config = ConfigIn (Class, i);
    if (test_bit (Config, OldPid))
      SET_BIT(Config, NewPid);
  }
  return (NewPid);
}


/**********************************************************************
 * WriteOldConfigFile
 *
 * Write the configs in the given class to the specified file in the
 * old config format.
 **********************************************************************/
void WriteOldConfigFile(FILE *File, CLASS_TYPE Class) {
  int Cid, Pid;
  BIT_VECTOR Config;

  fprintf (File, "%d %d\n", NumConfigsIn (Class), NumProtosIn (Class));

  for (Cid = 0; Cid < NumConfigsIn (Class); Cid++) {
    fprintf (File, "1 ");

    Config = ConfigIn (Class, Cid);

    for (Pid = 0; Pid < NumProtosIn (Class); Pid++) {
      if (test_bit (Config, Pid))
        fprintf (File, "1");
      else
        fprintf (File, "0");
    }
    fprintf (File, "\n");
  }
}


/**********************************************************************
 * WriteOldProtoFile
 *
 * Write the protos in the given class to the specified file in the
 * old proto format.
 **********************************************************************/
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

  for (Pid = 0; Pid < NumProtosIn (Class); Pid++) {
    Proto = ProtoIn (Class, Pid);

    fprintf (File, "significant   elliptical   1\n");
    fprintf (File, "     %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
      ProtoX (Proto), ProtoY (Proto),
      ProtoLength (Proto), ProtoAngle (Proto), 0.0, 0.0);
    fprintf (File, "     %9.6f %9.6f %9.6f %9.6f %9.6f %9.6f\n",
      0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001);
  }
}
