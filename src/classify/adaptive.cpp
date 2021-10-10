/******************************************************************************
 ** Filename:    adaptive.c
 ** Purpose:     Adaptive matcher.
 ** Author:      Dan Johnson
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

#include "adaptive.h"

#include "classify.h"

#include <cassert>
#include <cstdio>

namespace tesseract {

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
 */
void AddAdaptedClass(ADAPT_TEMPLATES_STRUCT *Templates, ADAPT_CLASS_STRUCT *Class, CLASS_ID ClassId) {
  assert(Templates != nullptr);
  assert(Class != nullptr);
  assert(LegalClassId(ClassId));
  assert(UnusedClassIdIn(Templates->Templates, ClassId));
  assert(Class->NumPermConfigs == 0);

  auto IntClass = new INT_CLASS_STRUCT(1, 1);
  AddIntClass(Templates->Templates, ClassId, IntClass);

  assert(Templates->Class[ClassId] == nullptr);
  Templates->Class[ClassId] = Class;

} /* AddAdaptedClass */

/*---------------------------------------------------------------------------*/

PERM_CONFIG_STRUCT::~PERM_CONFIG_STRUCT() {
  delete[] Ambigs;
}

ADAPT_CLASS_STRUCT::ADAPT_CLASS_STRUCT() {
  NumPermConfigs = 0;
  MaxNumTimesSeen = 0;
  TempProtos = NIL_LIST;

  PermProtos = NewBitVector(MAX_NUM_PROTOS);
  PermConfigs = NewBitVector(MAX_NUM_CONFIGS);
  zero_all_bits(PermProtos, WordsInVectorOfSize(MAX_NUM_PROTOS));
  zero_all_bits(PermConfigs, WordsInVectorOfSize(MAX_NUM_CONFIGS));

  for (int i = 0; i < MAX_NUM_CONFIGS; i++) {
    TempConfigFor(this, i) = nullptr;
  }
}

ADAPT_CLASS_STRUCT::~ADAPT_CLASS_STRUCT() {
  for (int i = 0; i < MAX_NUM_CONFIGS; i++) {
    if (ConfigIsPermanent(this, i) && PermConfigFor(this, i) != nullptr) {
      delete PermConfigFor(this, i);
    } else if (!ConfigIsPermanent(this, i) && TempConfigFor(this, i) != nullptr) {
      delete TempConfigFor(this, i);
    }
  }
  FreeBitVector(PermProtos);
  FreeBitVector(PermConfigs);
  auto list = TempProtos;
  while (list != nullptr) {
    delete reinterpret_cast<TEMP_PROTO_STRUCT *>(list->node);
    list = pop(list);
  }
}

/// Constructor for adapted templates.
/// Add an empty class for each char in unicharset to the newly created templates.
ADAPT_TEMPLATES_STRUCT::ADAPT_TEMPLATES_STRUCT(UNICHARSET &unicharset) {
  Templates = new INT_TEMPLATES_STRUCT;
  NumPermClasses = 0;
  NumNonEmptyClasses = 0;

  /* Insert an empty class for each unichar id in unicharset */
  for (unsigned i = 0; i < MAX_NUM_CLASSES; i++) {
    Class[i] = nullptr;
    if (i < unicharset.size()) {
      AddAdaptedClass(this, new ADAPT_CLASS_STRUCT, i);
    }
  }
}

ADAPT_TEMPLATES_STRUCT::~ADAPT_TEMPLATES_STRUCT() {
  for (unsigned i = 0; i < (Templates)->NumClasses; i++) {
    delete Class[i];
  }
  delete Templates;
}

// Returns FontinfoId of the given config of the given adapted class.
int Classify::GetFontinfoId(ADAPT_CLASS_STRUCT *Class, uint8_t ConfigId) {
  return (ConfigIsPermanent(Class, ConfigId) ? PermConfigFor(Class, ConfigId)->FontinfoId
                                             : TempConfigFor(Class, ConfigId)->FontinfoId);
}

/// This constructor allocates and returns a new temporary config.
///
/// @param MaxProtoId  max id of any proto in new config
/// @param FontinfoId font information from pre-trained templates
TEMP_CONFIG_STRUCT::TEMP_CONFIG_STRUCT(int maxProtoId, int fontinfoId) {
  int NumProtos = maxProtoId + 1;

  Protos = NewBitVector(NumProtos);

  NumTimesSeen = 1;
  MaxProtoId = maxProtoId;
  ProtoVectorSize = WordsInVectorOfSize(NumProtos);
  zero_all_bits(Protos, ProtoVectorSize);
  FontinfoId = fontinfoId;
}

TEMP_CONFIG_STRUCT::~TEMP_CONFIG_STRUCT() {
  FreeBitVector(Protos);
}

/*---------------------------------------------------------------------------*/
/**
 * This routine prints a summary of the adapted templates
 *  in Templates to File.
 *
 * @param File    open text file to print Templates to
 * @param Templates adapted templates to print to File
 *
 * @note Globals: none
 */
void Classify::PrintAdaptedTemplates(FILE *File, ADAPT_TEMPLATES_STRUCT *Templates) {
  INT_CLASS_STRUCT *IClass;
  ADAPT_CLASS_STRUCT *AClass;

  fprintf(File, "\n\nSUMMARY OF ADAPTED TEMPLATES:\n\n");
  fprintf(File, "Num classes = %d;  Num permanent classes = %d\n\n", Templates->NumNonEmptyClasses,
          Templates->NumPermClasses);
  fprintf(File, "   Id  NC NPC  NP NPP\n");
  fprintf(File, "------------------------\n");

  for (unsigned i = 0; i < (Templates->Templates)->NumClasses; i++) {
    IClass = Templates->Templates->Class[i];
    AClass = Templates->Class[i];
    if (!IsEmptyAdaptedClass(AClass)) {
      fprintf(File, "%5u  %s %3d %3d %3d %3zd\n", i, unicharset.id_to_unichar(i), IClass->NumConfigs,
              AClass->NumPermConfigs, IClass->NumProtos,
              IClass->NumProtos - AClass->TempProtos->size());
    }
  }
  fprintf(File, "\n");

} /* PrintAdaptedTemplates */

/*---------------------------------------------------------------------------*/
/**
 * Read an adapted class description from file and return
 * a ptr to the adapted class.
 *
 * @param fp open file to read adapted class from
 * @return Ptr to new adapted class.
 *
 * @note Globals: none
 */
ADAPT_CLASS_STRUCT *ReadAdaptedClass(TFile *fp) {
  int NumTempProtos;
  int NumConfigs;
  int i;
  ADAPT_CLASS_STRUCT *Class;

  /* first read high level adapted class structure */
  Class = new ADAPT_CLASS_STRUCT;
  fp->FRead(Class, sizeof(ADAPT_CLASS_STRUCT), 1);

  /* then read in the definitions of the permanent protos and configs */
  Class->PermProtos = NewBitVector(MAX_NUM_PROTOS);
  Class->PermConfigs = NewBitVector(MAX_NUM_CONFIGS);
  fp->FRead(Class->PermProtos, sizeof(uint32_t), WordsInVectorOfSize(MAX_NUM_PROTOS));
  fp->FRead(Class->PermConfigs, sizeof(uint32_t), WordsInVectorOfSize(MAX_NUM_CONFIGS));

  /* then read in the list of temporary protos */
  fp->FRead(&NumTempProtos, sizeof(int), 1);
  Class->TempProtos = NIL_LIST;
  for (i = 0; i < NumTempProtos; i++) {
    auto TempProto = new TEMP_PROTO_STRUCT;
    fp->FRead(TempProto, sizeof(TEMP_PROTO_STRUCT), 1);
    Class->TempProtos = push_last(Class->TempProtos, TempProto);
  }

  /* then read in the adapted configs */
  fp->FRead(&NumConfigs, sizeof(int), 1);
  for (i = 0; i < NumConfigs; i++) {
    if (test_bit(Class->PermConfigs, i)) {
      Class->Config[i].Perm = ReadPermConfig(fp);
    } else {
      Class->Config[i].Temp = ReadTempConfig(fp);
    }
  }

  return (Class);

} /* ReadAdaptedClass */

/*---------------------------------------------------------------------------*/
/**
 * Read a set of adapted templates from file and return
 * a ptr to the templates.
 *
 * @param fp open text file to read adapted templates from
 * @return Ptr to adapted templates read from file.
 *
 * @note Globals: none
 */
ADAPT_TEMPLATES_STRUCT *Classify::ReadAdaptedTemplates(TFile *fp) {
  auto Templates = new ADAPT_TEMPLATES_STRUCT;

  /* first read the high level adaptive template struct */
  fp->FRead(Templates, sizeof(ADAPT_TEMPLATES_STRUCT), 1);

  /* then read in the basic integer templates */
  Templates->Templates = ReadIntTemplates(fp);

  /* then read in the adaptive info for each class */
  for (unsigned i = 0; i < (Templates->Templates)->NumClasses; i++) {
    Templates->Class[i] = ReadAdaptedClass(fp);
  }
  return (Templates);

} /* ReadAdaptedTemplates */

/*---------------------------------------------------------------------------*/
/**
 * Read a permanent configuration description from file
 * and return a ptr to it.
 *
 * @param fp open file to read permanent config from
 * @return Ptr to new permanent configuration description.
 *
 * @note Globals: none
 */
PERM_CONFIG_STRUCT *ReadPermConfig(TFile *fp) {
  auto Config = new PERM_CONFIG_STRUCT;
  uint8_t NumAmbigs;
  fp->FRead(&NumAmbigs, sizeof(NumAmbigs), 1);
  Config->Ambigs = new UNICHAR_ID[NumAmbigs + 1];
  fp->FRead(Config->Ambigs, sizeof(UNICHAR_ID), NumAmbigs);
  Config->Ambigs[NumAmbigs] = -1;
  fp->FRead(&(Config->FontinfoId), sizeof(int), 1);

  return (Config);

} /* ReadPermConfig */

/*---------------------------------------------------------------------------*/
/**
 * Read a temporary configuration description from file
 * and return a ptr to it.
 *
 * @param fp open file to read temporary config from
 * @return Ptr to new temporary configuration description.
 *
 * @note Globals: none
 */
TEMP_CONFIG_STRUCT *ReadTempConfig(TFile *fp) {
  auto Config = new TEMP_CONFIG_STRUCT;
  fp->FRead(Config, sizeof(TEMP_CONFIG_STRUCT), 1);

  Config->Protos = NewBitVector(Config->ProtoVectorSize * BITSINLONG);
  fp->FRead(Config->Protos, sizeof(uint32_t), Config->ProtoVectorSize);

  return (Config);

} /* ReadTempConfig */

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
 */
void WriteAdaptedClass(FILE *File, ADAPT_CLASS_STRUCT *Class, int NumConfigs) {
  /* first write high level adapted class structure */
  fwrite(Class, sizeof(ADAPT_CLASS_STRUCT), 1, File);

  /* then write out the definitions of the permanent protos and configs */
  fwrite(Class->PermProtos, sizeof(uint32_t), WordsInVectorOfSize(MAX_NUM_PROTOS), File);
  fwrite(Class->PermConfigs, sizeof(uint32_t), WordsInVectorOfSize(MAX_NUM_CONFIGS), File);

  /* then write out the list of temporary protos */
  uint32_t NumTempProtos = Class->TempProtos->size();
  fwrite(&NumTempProtos, sizeof(NumTempProtos), 1, File);
  auto TempProtos = Class->TempProtos;
  iterate(TempProtos) {
    void *proto = TempProtos->node;
    fwrite(proto, sizeof(TEMP_PROTO_STRUCT), 1, File);
  }

  /* then write out the adapted configs */
  fwrite(&NumConfigs, sizeof(int), 1, File);
  for (int i = 0; i < NumConfigs; i++) {
    if (test_bit(Class->PermConfigs, i)) {
      WritePermConfig(File, Class->Config[i].Perm);
    } else {
      WriteTempConfig(File, Class->Config[i].Temp);
    }
  }

} /* WriteAdaptedClass */

/*---------------------------------------------------------------------------*/
/**
 * This routine saves Templates to File in a binary format.
 *
 * @param File    open text file to write Templates to
 * @param Templates set of adapted templates to write to File
 *
 * @note Globals: none
 */
void Classify::WriteAdaptedTemplates(FILE *File, ADAPT_TEMPLATES_STRUCT *Templates) {
  /* first write the high level adaptive template struct */
  fwrite(Templates, sizeof(ADAPT_TEMPLATES_STRUCT), 1, File);

  /* then write out the basic integer templates */
  WriteIntTemplates(File, Templates->Templates, unicharset);

  /* then write out the adaptive info for each class */
  for (unsigned i = 0; i < (Templates->Templates)->NumClasses; i++) {
    WriteAdaptedClass(File, Templates->Class[i], Templates->Templates->Class[i]->NumConfigs);
  }
} /* WriteAdaptedTemplates */

/*---------------------------------------------------------------------------*/
/**
 * This routine writes a binary representation of a
 * permanent configuration to File.
 *
 * @param File  open file to write Config to
 * @param Config  permanent config to write to File
 *
 * @note Globals: none
 */
void WritePermConfig(FILE *File, PERM_CONFIG_STRUCT *Config) {
  uint8_t NumAmbigs = 0;

  assert(Config != nullptr);
  while (Config->Ambigs[NumAmbigs] > 0) {
    ++NumAmbigs;
  }

  fwrite(&NumAmbigs, sizeof(uint8_t), 1, File);
  fwrite(Config->Ambigs, sizeof(UNICHAR_ID), NumAmbigs, File);
  fwrite(&(Config->FontinfoId), sizeof(int), 1, File);
} /* WritePermConfig */

/*---------------------------------------------------------------------------*/
/**
 * This routine writes a binary representation of a
 * temporary configuration to File.
 *
 * @param File  open file to write Config to
 * @param Config  temporary config to write to File
 *
 * @note Globals: none
 */
void WriteTempConfig(FILE *File, TEMP_CONFIG_STRUCT *Config) {
  assert(Config != nullptr);

  fwrite(Config, sizeof(TEMP_CONFIG_STRUCT), 1, File);
  fwrite(Config->Protos, sizeof(uint32_t), Config->ProtoVectorSize, File);

} /* WriteTempConfig */

} // namespace tesseract
