/******************************************************************************
 **	Filename:    adaptive.c
 **	Purpose:     Adaptive matcher.
 **	Author:      Dan Johnson
 **	History:     Fri Mar  8 10:00:21 1991, DSJ, Created.
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

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "adaptive.h"
#include "emalloc.h"
#include "freelist.h"
#include "globals.h"

#ifdef __UNIX__
#include <assert.h>
#endif
#include <stdio.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
int AddAdaptedClass(ADAPT_TEMPLATES Templates,
                    ADAPT_CLASS Class,
                    CLASS_ID ClassId) {
/*
 **	Parameters:
 **		Templates	set of templates to add new class to
 **		Class		new class to add to templates
 **		ClassId		class id to associate with new class
 **	Globals: none
 **	Operation: This routine adds a new adapted class to an existing
 **		set of adapted templates.
 **	Return: The class index of the new class.
 **	Exceptions: none
 **	History: Thu Mar 14 13:06:09 1991, DSJ, Created.
 */
  INT_CLASS IntClass;
  CLASS_INDEX ClassIndex;

  assert (Templates != NULL);
  assert (Class != NULL);
  assert (LegalClassId (ClassId));
  assert (UnusedClassIdIn (Templates->Templates, ClassId));
  assert (Class->NumPermConfigs == 0);

  IntClass = NewIntClass (1, 1);
  ClassIndex = AddIntClass (Templates->Templates, ClassId, IntClass);

  assert (Templates->Class[ClassIndex] == NULL);

  Templates->Class[ClassIndex] = Class;

  return (ClassIndex);

}                                /* AddAdaptedClass */


/*---------------------------------------------------------------------------*/
void FreeTempConfig(TEMP_CONFIG Config) {
/*
 **	Parameters:
 **		Config	config to be freed
 **	Globals: none
 **	Operation: This routine frees all memory consumed by a temporary
 **		configuration.
 **	Return: none
 **	Exceptions: none
 **	History: Thu Mar 14 13:34:23 1991, DSJ, Created.
 */
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


/*---------------------------------------------------------------------------*/
ADAPT_CLASS NewAdaptedClass() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: This operation allocates and initializes a new adapted
 **		class data structure and returns a ptr to it.
 **	Return: Ptr to new class data structure.
 **	Exceptions: none
 **	History: Thu Mar 14 12:58:13 1991, DSJ, Created.
 */
  ADAPT_CLASS Class;
  int i;

  Class = (ADAPT_CLASS) Emalloc (sizeof (ADAPT_CLASS_STRUCT));
  Class->NumPermConfigs = 0;
  Class->TempProtos = NIL;

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
      Efree (PermConfigFor (adapt_class, i));
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
ADAPT_TEMPLATES NewAdaptedTemplates() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation:
 **	Return: none
 **	Exceptions: none
 **	History: Fri Mar  8 10:15:28 1991, DSJ, Created.
 */
  ADAPT_TEMPLATES Templates;
  int i;

  Templates = (ADAPT_TEMPLATES) Emalloc (sizeof (ADAPT_TEMPLATES_STRUCT));

  Templates->Templates = NewIntTemplates ();
  Templates->NumPermClasses = 0;

  for (i = 0; i < MAX_NUM_CLASSES; i++)
    Templates->Class[i] = NULL;

  return (Templates);

}                                /* NewAdaptedTemplates */


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
TEMP_CONFIG NewTempConfig(int MaxProtoId) {
/*
 **	Parameters:
 **		MaxProtoId	max id of any proto in new config
 **	Globals: none
 **	Operation: This routine allocates and returns a new temporary
 **		config.
 **	Return: Ptr to new temp config.
 **	Exceptions: none
 **	History: Thu Mar 14 13:28:21 1991, DSJ, Created.
 */
  TEMP_CONFIG Config;
  int NumProtos = MaxProtoId + 1;

  Config =
    (TEMP_CONFIG) alloc_struct (sizeof (TEMP_CONFIG_STRUCT),
    "TEMP_CONFIG_STRUCT");
  Config->Protos = NewBitVector (NumProtos);

  Config->NumTimesSeen = 1;
  Config->MaxProtoId = MaxProtoId;
  Config->ProtoVectorSize = WordsInVectorOfSize (NumProtos);
  Config->ContextsSeen = NIL;
  zero_all_bits (Config->Protos, Config->ProtoVectorSize);

  return (Config);

}                                /* NewTempConfig */


/*---------------------------------------------------------------------------*/
TEMP_PROTO NewTempProto() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: This routine allocates and returns a new temporary proto.
 **	Return: Ptr to new temporary proto.
 **	Exceptions: none
 **	History: Thu Mar 14 13:31:31 1991, DSJ, Created.
 */
  return ((TEMP_PROTO)
    alloc_struct (sizeof (TEMP_PROTO_STRUCT), "TEMP_PROTO_STRUCT"));
}                                /* NewTempProto */


/*---------------------------------------------------------------------------*/
void PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates) {
/*
 **	Parameters:
 **		File		open text file to print Templates to
 **		Templates	adapted templates to print to File
 **	Globals: none
 **	Operation: This routine prints a summary of the adapted templates
 **		in Templates to File.
 **	Return: none
 **	Exceptions: none
 **	History: Wed Mar 20 13:35:29 1991, DSJ, Created.
 */
  int i;
  INT_CLASS IClass;
  ADAPT_CLASS AClass;

  #ifndef SECURE_NAMES
  fprintf (File, "\n\nSUMMARY OF ADAPTED TEMPLATES:\n\n");
  fprintf (File, "Num classes = %d;  Num permanent classes = %d\n\n",
    (Templates->Templates)->NumClasses, Templates->NumPermClasses);
  fprintf (File, "Index Id  NC NPC  NP NPP\n");
  fprintf (File, "------------------------\n");

  for (i = 0; i < (Templates->Templates)->NumClasses; i++) {
    IClass = Templates->Templates->Class[i];
    AClass = Templates->Class[i];

    fprintf (File, "%5d  %s %3d %3d %3d %3d\n",
      i, unicharset.id_to_unichar(Templates->Templates->ClassIdFor[i]),
      IClass->NumConfigs, AClass->NumPermConfigs,
      IClass->NumProtos,
      IClass->NumProtos - count (AClass->TempProtos));
  }
  #endif
  fprintf (File, "\n");

}                                /* PrintAdaptedTemplates */


/*---------------------------------------------------------------------------*/
ADAPT_CLASS ReadAdaptedClass(FILE *File) {
/*
 **	Parameters:
 **		File	open file to read adapted class from
 **	Globals: none
 **	Operation: Read an adapted class description from File and return
 **		a ptr to the adapted class.
 **	Return: Ptr to new adapted class.
 **	Exceptions: none
 **	History: Tue Mar 19 14:11:01 1991, DSJ, Created.
 */
  int NumTempProtos;
  int NumConfigs;
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
  fread ((char *) &NumTempProtos, sizeof (int), 1, File);
  Class->TempProtos = NIL;
  for (i = 0; i < NumTempProtos; i++) {
    TempProto =
      (TEMP_PROTO) alloc_struct (sizeof (TEMP_PROTO_STRUCT),
      "TEMP_PROTO_STRUCT");
    fread ((char *) TempProto, sizeof (TEMP_PROTO_STRUCT), 1, File);
    Class->TempProtos = push_last (Class->TempProtos, TempProto);
  }

  /* then read in the adapted configs */
  fread ((char *) &NumConfigs, sizeof (int), 1, File);
  for (i = 0; i < NumConfigs; i++)
    if (test_bit (Class->PermConfigs, i))
      Class->Config[i].Perm = ReadPermConfig (File);
    else
      Class->Config[i].Temp = ReadTempConfig (File);

  return (Class);

}                                /* ReadAdaptedClass */


/*---------------------------------------------------------------------------*/
ADAPT_TEMPLATES ReadAdaptedTemplates(FILE *File) {
/*
 **	Parameters:
 **		File	open text file to read adapted templates from
 **	Globals: none
 **	Operation: Read a set of adapted templates from File and return
 **		a ptr to the templates.
 **	Return: Ptr to adapted templates read from File.
 **	Exceptions: none
 **	History: Mon Mar 18 15:18:10 1991, DSJ, Created.
 */
  int i;
  ADAPT_TEMPLATES Templates;

  /* first read the high level adaptive template struct */
  Templates = (ADAPT_TEMPLATES) Emalloc (sizeof (ADAPT_TEMPLATES_STRUCT));
  fread ((char *) Templates, sizeof (ADAPT_TEMPLATES_STRUCT), 1, File);

  /* then read in the basic integer templates */
  Templates->Templates = ReadIntTemplates (File, FALSE);

  /* then read in the adaptive info for each class */
  for (i = 0; i < (Templates->Templates)->NumClasses; i++) {
    Templates->Class[i] = ReadAdaptedClass (File);
  }
  return (Templates);

}                                /* ReadAdaptedTemplates */


/*---------------------------------------------------------------------------*/
PERM_CONFIG ReadPermConfig(FILE *File) {
/*
 **	Parameters:
 **		File	open file to read permanent config from
 **	Globals: none
 **	Operation: Read a permanent configuration description from File
 **		and return a ptr to it.
 **	Return: Ptr to new permanent configuration description.
 **	Exceptions: none
 **	History: Tue Mar 19 14:25:26 1991, DSJ, Created.
 */
  PERM_CONFIG Config;
  uinT8 NumAmbigs;

  fread ((char *) &NumAmbigs, sizeof (uinT8), 1, File);
  Config = (PERM_CONFIG) Emalloc (sizeof (char) * (NumAmbigs + 1));
  fread (Config, sizeof (UNICHAR_ID), NumAmbigs, File);
  Config[NumAmbigs] = -1;

  return (Config);

}                                /* ReadPermConfig */


/*---------------------------------------------------------------------------*/
TEMP_CONFIG ReadTempConfig(FILE *File) {
/*
 **	Parameters:
 **		File	open file to read temporary config from
 **	Globals: none
 **	Operation:  Read a temporary configuration description from File
 **		and return a ptr to it.
 **	Return: Ptr to new temporary configuration description.
 **	Exceptions: none
 **	History: Tue Mar 19 14:29:59 1991, DSJ, Created.
 */
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
void WriteAdaptedClass(FILE *File, ADAPT_CLASS Class, int NumConfigs) {
/*
 **	Parameters:
 **		File		open file to write Class to
 **		Class		adapted class to write to File
 **		NumConfigs	number of configs in Class
 **	Globals: none
 **	Operation: This routine writes a binary representation of Class
 **		to File.
 **	Return: none
 **	Exceptions: none
 **	History: Tue Mar 19 13:33:51 1991, DSJ, Created.
 */
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
void WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES Templates) {
/*
 **	Parameters:
 **		File		open text file to write Templates to
 **		Templates	set of adapted templates to write to File
 **	Globals: none
 **	Operation: This routine saves Templates to File in a binary format.
 **	Return: none
 **	Exceptions: none
 **	History: Mon Mar 18 15:07:32 1991, DSJ, Created.
 */
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


/*---------------------------------------------------------------------------*/
void WritePermConfig(FILE *File, PERM_CONFIG Config) {
/*
 **	Parameters:
 **		File	open file to write Config to
 **		Config	permanent config to write to File
 **	Globals: none
 **	Operation: This routine writes a binary representation of a
 **		permanent configuration to File.
 **	Return: none
 **	Exceptions: none
 **	History: Tue Mar 19 13:55:44 1991, DSJ, Created.
 */
  uinT8 NumAmbigs = 0;

  assert (Config != NULL);
  while (Config[NumAmbigs] > 0)
    ++NumAmbigs;

  fwrite ((char *) &NumAmbigs, sizeof (uinT8), 1, File);
  fwrite (Config, sizeof (UNICHAR_ID), NumAmbigs, File);

}                                /* WritePermConfig */


/*---------------------------------------------------------------------------*/
void WriteTempConfig(FILE *File, TEMP_CONFIG Config) {
/*
 **	Parameters:
 **		File	open file to write Config to
 **		Config	temporary config to write to File
 **	Globals: none
 **	Operation: This routine writes a binary representation of a
 **		temporary configuration to File.
 **	Return: none
 **	Exceptions: none
 **	History: Tue Mar 19 14:00:28 1991, DSJ, Created.
 */
  assert (Config != NULL);
                                 /* contexts not yet implemented */
  assert (Config->ContextsSeen == NULL);

  fwrite ((char *) Config, sizeof (TEMP_CONFIG_STRUCT), 1, File);
  fwrite ((char *) Config->Protos, sizeof (uinT32),
    Config->ProtoVectorSize, File);

}                                /* WriteTempConfig */
