/******************************************************************************
 ** Filename:    adaptive.c
 ** Purpose:     Adaptive matcher.
 ** Author:      Dan Johnson
 ** History:     Fri Mar  8 10:00:21 1991, DSJ, Created.
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

/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "adaptive.h"
#include "emalloc.h"
#include "freelist.h"
#include "globals.h"
#include "classify.h"

#ifdef __UNIX__
#include <assert.h>
#endif
#include <stdio.h>

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine adds a new adapted class to an existing
 * set of adapted templates.
 *
 * @param Templates set of templates to add new class to
 * @param Class new class to add to templates
 * @param ClassId class id to associate with new class
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Thu Mar 14 13:06:09 1991, DSJ, Created.
 */
void AddAdaptedClass(ADAPT_TEMPLATES Templates,
                     ADAPT_CLASS Class,
                     CLASS_ID ClassId) {
  INT_CLASS IntClass;

  assert (Templates != NULL);
  assert (Class != NULL);
  assert (LegalClassId (ClassId));
  assert (UnusedClassIdIn (Templates->Templates, ClassId));
  assert (Class->NumPermConfigs == 0);

  IntClass = NewIntClass (1, 1);
  AddIntClass (Templates->Templates, ClassId, IntClass);

  assert (Templates->Class[ClassId] == NULL);
  Templates->Class[ClassId] = Class;

}                                /* AddAdaptedClass */


/*---------------------------------------------------------------------------*/
/**
 * This routine frees all memory consumed by a temporary
 * configuration.
 *
 * @param Config  config to be freed
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Thu Mar 14 13:34:23 1991, DSJ, Created.
 */
void FreeTempConfig(TEMP_CONFIG Config) {
  assert (Config != NULL);

  destroy_nodes (Config->ContextsSeen, memfree);
  FreeBitVector (Config->Protos);
  free_struct (Config, sizeof (TEMP_CONFIG_STRUCT), "TEMP_CONFIG_STRUCT");

}                                /* FreeTempConfig */

/*---------------------------------------------------------------------------*/
void FreeTempProto(void *arg) {
  PROTO proto = (PROTO) arg;

  free_struct (proto, sizeof (TEMP_PROTO_STRUCT), "TEMP_PROTO_STRUCT");
}

void FreePermConfig(PERM_CONFIG Config) {
  assert(Config != NULL);
  delete [] Config->Ambigs;
  free_struct(Config, sizeof(PERM_CONFIG_STRUCT), "PERM_CONFIG_STRUCT");
}

/*---------------------------------------------------------------------------*/
/**
 * This operation allocates and initializes a new adapted
 * class data structure and returns a ptr to it.
 *
 * @return Ptr to new class data structure.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Thu Mar 14 12:58:13 1991, DSJ, Created.
 */
ADAPT_CLASS NewAdaptedClass() {
  ADAPT_CLASS Class;
  int i;

  Class = (ADAPT_CLASS) Emalloc (sizeof (ADAPT_CLASS_STRUCT));
  Class->NumPermConfigs = 0;
  Class->MaxNumTimesSeen = 0;
  Class->TempProtos = NIL_LIST;

  Class->PermProtos = NewBitVector (MAX_NUM_PROTOS);
  Class->PermConfigs = NewBitVector (MAX_NUM_CONFIGS);
  zero_all_bits (Class->PermProtos, WordsInVectorOfSize (MAX_NUM_PROTOS));
  zero_all_bits (Class->PermConfigs, WordsInVectorOfSize (MAX_NUM_CONFIGS));

  for (i = 0; i < MAX_NUM_CONFIGS; i++)
    TempConfigFor (Class, i) = NULL;

  return (Class);

}                                /* NewAdaptedClass */


/*-------------------------------------------------------------------------*/
void free_adapted_class(ADAPT_CLASS adapt_class) {
  int i;

  for (i = 0; i < MAX_NUM_CONFIGS; i++) {
    if (ConfigIsPermanent (adapt_class, i)
      && PermConfigFor (adapt_class, i) != NULL)
      FreePermConfig (PermConfigFor (adapt_class, i));
    else if (!ConfigIsPermanent (adapt_class, i)
      && TempConfigFor (adapt_class, i) != NULL)
      FreeTempConfig (TempConfigFor (adapt_class, i));
  }
  FreeBitVector (adapt_class->PermProtos);
  FreeBitVector (adapt_class->PermConfigs);
  destroy_nodes (adapt_class->TempProtos, FreeTempProto);
  Efree(adapt_class);
}


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * Allocates memory for adapted tempates.
 * each char in unicharset to the newly created templates
 *
 * @param InitFromUnicharset if true, add an empty class for
 * @return Ptr to new adapted templates.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Fri Mar  8 10:15:28 1991, DSJ, Created.
 */
ADAPT_TEMPLATES Classify::NewAdaptedTemplates(bool InitFromUnicharset) {
  ADAPT_TEMPLATES Templates;
  int i;

  Templates = (ADAPT_TEMPLATES) Emalloc (sizeof (ADAPT_TEMPLATES_STRUCT));

  Templates->Templates = NewIntTemplates ();
  Templates->NumPermClasses = 0;
  Templates->NumNonEmptyClasses = 0;

  /* Insert an empty class for each unichar id in unicharset */
  for (i = 0; i < MAX_NUM_CLASSES; i++) {
    Templates->Class[i] = NULL;
    if (InitFromUnicharset && i < unicharset.size()) {
      AddAdaptedClass(Templates, NewAdaptedClass(), i);
    }
  }

  return (Templates);

}                                /* NewAdaptedTemplates */

// Returns FontinfoId of the given config of the given adapted class.
int Classify::GetFontinfoId(ADAPT_CLASS Class, uinT8 ConfigId) {
  return (ConfigIsPermanent(Class, ConfigId) ?
      PermConfigFor(Class, ConfigId)->FontinfoId :
      TempConfigFor(Class, ConfigId)->FontinfoId);
}

}  // namespace tesseract

/*----------------------------------------------------------------------------*/
void free_adapted_templates(ADAPT_TEMPLATES templates) {

  if (templates != NULL) {
    int i;
    for (i = 0; i < (templates->Templates)->NumClasses; i++)
      free_adapted_class (templates->Class[i]);
    free_int_templates (templates->Templates);
    Efree(templates);
  }
}


/*---------------------------------------------------------------------------*/
/**
 * This routine allocates and returns a new temporary config.
 *
 * @param MaxProtoId  max id of any proto in new config
 * @param FontinfoId font information from pre-trained templates
 * @return Ptr to new temp config.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Thu Mar 14 13:28:21 1991, DSJ, Created.
 */
TEMP_CONFIG NewTempConfig(int MaxProtoId, int FontinfoId) {
  TEMP_CONFIG Config;
  int NumProtos = MaxProtoId + 1;

  Config =
    (TEMP_CONFIG) alloc_struct (sizeof (TEMP_CONFIG_STRUCT),
    "TEMP_CONFIG_STRUCT");
  Config->Protos = NewBitVector (NumProtos);

  Config->NumTimesSeen = 1;
  Config->MaxProtoId = MaxProtoId;
  Config->ProtoVectorSize = WordsInVectorOfSize (NumProtos);
  Config->ContextsSeen = NIL_LIST;
  zero_all_bits (Config->Protos, Config->ProtoVectorSize);
  Config->FontinfoId = FontinfoId;

  return (Config);

}                                /* NewTempConfig */


/*---------------------------------------------------------------------------*/
/**
 * This routine allocates and returns a new temporary proto.
 *
 * @return Ptr to new temporary proto.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Thu Mar 14 13:31:31 1991, DSJ, Created.
 */
TEMP_PROTO NewTempProto() {
  return ((TEMP_PROTO)
    alloc_struct (sizeof (TEMP_PROTO_STRUCT), "TEMP_PROTO_STRUCT"));
}                                /* NewTempProto */


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine prints a summary of the adapted templates
 *  in Templates to File.
 *
 * @param File    open text file to print Templates to
 * @param Templates adapted templates to print to File
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Wed Mar 20 13:35:29 1991, DSJ, Created.
 */
void Classify::PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates) {
  int i;
  INT_CLASS IClass;
  ADAPT_CLASS AClass;

  fprintf (File, "\n\nSUMMARY OF ADAPTED TEMPLATES:\n\n");
  fprintf (File, "Num classes = %d;  Num permanent classes = %d\n\n",
           Templates->NumNonEmptyClasses, Templates->NumPermClasses);
  fprintf (File, "   Id  NC NPC  NP NPP\n");
  fprintf (File, "------------------------\n");

  for (i = 0; i < (Templates->Templates)->NumClasses; i++) {
    IClass = Templates->Templates->Class[i];
    AClass = Templates->Class[i];
    if (!IsEmptyAdaptedClass (AClass)) {
      fprintf (File, "%5d  %s %3d %3d %3d %3d\n",
        i, unicharset.id_to_unichar(i),
      IClass->NumConfigs, AClass->NumPermConfigs,
      IClass->NumProtos,
      IClass->NumProtos - count (AClass->TempProtos));
    }
  }
  fprintf (File, "\n");

}                                /* PrintAdaptedTemplates */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
/**
 * Read an adapted class description from File and return
 * a ptr to the adapted class.
 *
 * @param File  open file to read adapted class from
 * @return Ptr to new adapted class.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 14:11:01 1991, DSJ, Created.
 */
ADAPT_CLASS ReadAdaptedClass(FILE *File) {
  int32_t NumTempProtos;
  int32_t NumConfigs;
  int i;
  ADAPT_CLASS Class;
  TEMP_PROTO TempProto;

  /* first read high level adapted class structure */
  Class = (ADAPT_CLASS) Emalloc (sizeof (ADAPT_CLASS_STRUCT));
  fread ((char *) Class, sizeof (ADAPT_CLASS_STRUCT), 1, File);

  /* then read in the definitions of the permanent protos and configs */
  Class->PermProtos = NewBitVector (MAX_NUM_PROTOS);
  Class->PermConfigs = NewBitVector (MAX_NUM_CONFIGS);
  fread ((char *) Class->PermProtos, sizeof (uinT32),
    WordsInVectorOfSize (MAX_NUM_PROTOS), File);
  fread ((char *) Class->PermConfigs, sizeof (uinT32),
    WordsInVectorOfSize (MAX_NUM_CONFIGS), File);

  /* then read in the list of temporary protos */
  fread (&NumTempProtos, sizeof(NumTempProtos), 1, File);
  Class->TempProtos = NIL_LIST;
  for (i = 0; i < NumTempProtos; i++) {
    TempProto =
      (TEMP_PROTO) alloc_struct (sizeof (TEMP_PROTO_STRUCT),
      "TEMP_PROTO_STRUCT");
    fread ((char *) TempProto, sizeof (TEMP_PROTO_STRUCT), 1, File);
    Class->TempProtos = push_last (Class->TempProtos, TempProto);
  }

  /* then read in the adapted configs */
  fread (&NumConfigs, sizeof(NumConfigs), 1, File);
  for (i = 0; i < NumConfigs; i++)
    if (test_bit (Class->PermConfigs, i))
      Class->Config[i].Perm = ReadPermConfig (File);
    else
      Class->Config[i].Temp = ReadTempConfig (File);

  return (Class);

}                                /* ReadAdaptedClass */


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * Read a set of adapted templates from File and return
 * a ptr to the templates.
 *
 * @param File  open text file to read adapted templates from
 * @return Ptr to adapted templates read from File.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Mon Mar 18 15:18:10 1991, DSJ, Created.
 */
ADAPT_TEMPLATES Classify::ReadAdaptedTemplates(FILE *File) {
  int i;
  ADAPT_TEMPLATES Templates;

  /* first read the high level adaptive template struct */
  Templates = (ADAPT_TEMPLATES) Emalloc (sizeof (ADAPT_TEMPLATES_STRUCT));
  fread ((char *) Templates, sizeof (ADAPT_TEMPLATES_STRUCT), 1, File);

  /* then read in the basic integer templates */
  Templates->Templates = ReadIntTemplates (File);

  /* then read in the adaptive info for each class */
  for (i = 0; i < (Templates->Templates)->NumClasses; i++) {
    Templates->Class[i] = ReadAdaptedClass (File);
  }
  return (Templates);

}                                /* ReadAdaptedTemplates */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
/**
 * Read a permanent configuration description from File
 * and return a ptr to it.
 *
 * @param File  open file to read permanent config from
 * @return Ptr to new permanent configuration description.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 14:25:26 1991, DSJ, Created.
 */
PERM_CONFIG ReadPermConfig(FILE *File) {
  PERM_CONFIG Config = (PERM_CONFIG) alloc_struct(sizeof(PERM_CONFIG_STRUCT),
                                                  "PERM_CONFIG_STRUCT");
  uinT8 NumAmbigs;
  fread ((char *) &NumAmbigs, sizeof(uinT8), 1, File);
  Config->Ambigs = new UNICHAR_ID[NumAmbigs + 1];
  fread(Config->Ambigs, sizeof(UNICHAR_ID), NumAmbigs, File);
  Config->Ambigs[NumAmbigs] = -1;
  fread(&(Config->FontinfoId), sizeof(int), 1, File);

  return (Config);

}                                /* ReadPermConfig */


/*---------------------------------------------------------------------------*/
/**
 * Read a temporary configuration description from File
 * and return a ptr to it.
 *
 * @param File  open file to read temporary config from
 * @return Ptr to new temporary configuration description.
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 14:29:59 1991, DSJ, Created.
 */
TEMP_CONFIG ReadTempConfig(FILE *File) {
  TEMP_CONFIG Config;

  Config =
    (TEMP_CONFIG) alloc_struct (sizeof (TEMP_CONFIG_STRUCT),
    "TEMP_CONFIG_STRUCT");
  fread ((char *) Config, sizeof (TEMP_CONFIG_STRUCT), 1, File);

  Config->Protos = NewBitVector (Config->ProtoVectorSize * BITSINLONG);
  fread ((char *) Config->Protos, sizeof (uinT32),
    Config->ProtoVectorSize, File);

  return (Config);

}                                /* ReadTempConfig */


/*---------------------------------------------------------------------------*/
/**
 * This routine writes a binary representation of Class
 * to File.
 *
 * @param File    open file to write Class to
 * @param Class   adapted class to write to File
 * @param NumConfigs  number of configs in Class
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 13:33:51 1991, DSJ, Created.
 */
void WriteAdaptedClass(FILE *File, ADAPT_CLASS Class, int NumConfigs) {
  int NumTempProtos;
  LIST TempProtos;
  int i;

  /* first write high level adapted class structure */
  fwrite ((char *) Class, sizeof (ADAPT_CLASS_STRUCT), 1, File);

  /* then write out the definitions of the permanent protos and configs */
  fwrite ((char *) Class->PermProtos, sizeof (uinT32),
    WordsInVectorOfSize (MAX_NUM_PROTOS), File);
  fwrite ((char *) Class->PermConfigs, sizeof (uinT32),
    WordsInVectorOfSize (MAX_NUM_CONFIGS), File);

  /* then write out the list of temporary protos */
  NumTempProtos = count (Class->TempProtos);
  fwrite ((char *) &NumTempProtos, sizeof (int), 1, File);
  TempProtos = Class->TempProtos;
  iterate (TempProtos) {
    void* proto = first_node(TempProtos);
    fwrite ((char *) proto, sizeof (TEMP_PROTO_STRUCT), 1, File);
  }

  /* then write out the adapted configs */
  fwrite ((char *) &NumConfigs, sizeof (int), 1, File);
  for (i = 0; i < NumConfigs; i++)
    if (test_bit (Class->PermConfigs, i))
      WritePermConfig (File, Class->Config[i].Perm);
    else
      WriteTempConfig (File, Class->Config[i].Temp);

}                                /* WriteAdaptedClass */


/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine saves Templates to File in a binary format.
 *
 * @param File    open text file to write Templates to
 * @param Templates set of adapted templates to write to File
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Mon Mar 18 15:07:32 1991, DSJ, Created.
 */
void Classify::WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates) {
  int i;

  /* first write the high level adaptive template struct */
  fwrite ((char *) Templates, sizeof (ADAPT_TEMPLATES_STRUCT), 1, File);

  /* then write out the basic integer templates */
  WriteIntTemplates (File, Templates->Templates, unicharset);

  /* then write out the adaptive info for each class */
  for (i = 0; i < (Templates->Templates)->NumClasses; i++) {
    WriteAdaptedClass (File, Templates->Class[i],
      Templates->Templates->Class[i]->NumConfigs);
  }
}                                /* WriteAdaptedTemplates */
}  // namespace tesseract


/*---------------------------------------------------------------------------*/
/**
 * This routine writes a binary representation of a
 * permanent configuration to File.
 * 
 * @param File  open file to write Config to
 * @param Config  permanent config to write to File
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 13:55:44 1991, DSJ, Created.
 */
void WritePermConfig(FILE *File, PERM_CONFIG Config) {
  uinT8 NumAmbigs = 0;

  assert (Config != NULL);
  while (Config->Ambigs[NumAmbigs] > 0) ++NumAmbigs;

  fwrite((char *) &NumAmbigs, sizeof(uinT8), 1, File);
  fwrite(Config->Ambigs, sizeof(UNICHAR_ID), NumAmbigs, File);
  fwrite(&(Config->FontinfoId), sizeof(int), 1, File);
}                                /* WritePermConfig */


/*---------------------------------------------------------------------------*/
/**
 * This routine writes a binary representation of a
 * temporary configuration to File.
 *
 * @param File  open file to write Config to
 * @param Config  temporary config to write to File
 *
 * @note Globals: none
 * @note Exceptions: none
 * @note History: Tue Mar 19 14:00:28 1991, DSJ, Created.
 */
void WriteTempConfig(FILE *File, TEMP_CONFIG Config) {
  assert (Config != NULL);
                                 /* contexts not yet implemented */
  assert (Config->ContextsSeen == NULL);

  fwrite ((char *) Config, sizeof (TEMP_CONFIG_STRUCT), 1, File);
  fwrite ((char *) Config->Protos, sizeof (uinT32),
    Config->ProtoVectorSize, File);

}                                /* WriteTempConfig */
