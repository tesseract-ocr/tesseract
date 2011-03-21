/******************************************************************************
**  Filename:  mftraining.c
**  Purpose:  Separates training pages into files for each character.
**        Strips from files only the features and there parameters of
        the feature type mf.
**  Author:    Dan Johnson
**  Revisment:  Christy Russon
**  Environment: HPUX 6.5
**  Library:     HPUX 6.5
**  History:     Fri Aug 18 08:53:50 1989, DSJ, Created.
**         5/25/90, DSJ, Adapted to multiple feature types.
**        Tuesday, May 17, 1998 Changes made to make feature specific and
**        simplify structures. First step in simplifying training process.
**
 **  (c) Copyright Hewlett-Packard Company, 1988.
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
#include "oldlist.h"
#include "efio.h"
#include "emalloc.h"
#include "featdefs.h"
#include "tessopt.h"
#include "ocrfeatures.h"
#include "mf.h"
#include "clusttool.h"
#include "cluster.h"
#include "protos.h"
#include "ndminx.h"
#include "tprintf.h"
#include "const.h"
#include "mergenf.h"
#include "intproto.h"
#include "freelist.h"
#include "efio.h"
#include "danerror.h"
#include "globals.h"
#include "commontraining.h"
#include "unicity_table.h"
#include "genericvector.h"
#include "classify.h"

#include <string.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#ifdef WIN32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#endif

#define PROGRAM_FEATURE_TYPE "mf"

static const char* kInputUnicharsetFile = "unicharset";
static const char* kOutputUnicharsetFile = "mfunicharset";
/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
int main (
     int  argc,
     char  **argv);

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/

void WriteMicrofeat(
    char  *Directory,
  LIST  ClassList);

void WriteProtos(
  FILE* File,
  MERGE_CLASS MergeClass);

void WriteConfigs(
  FILE* File,
  CLASS_TYPE Class);

/*
PARAMDESC *ConvertToPARAMDESC(
  PARAM_DESC* Param_Desc,
  int N);
*/

void WritePFFMTable(INT_TEMPLATES Templates, const UNICHARSET& unicharset,
                    const char* filename);

void InitXHeights(const char *filename,
                  const UnicityTable<FontInfo> &fontinfo_table,
                  int xheights[]);

void AddSpacingInfo(const char *filename, int fontinfo_id,
                    const UNICHARSET &unicharset, const int xheights[],
                     UnicityTable<FontInfo> *fontinfo_table);

// global variable to hold configuration parameters to control clustering
// -M 0.40   -B 0.05   -I 1.0   -C 1e-6.
CLUSTERCONFIG Config =
{ elliptical, 0.625, 0.05, 1.0, 1e-6, 0 };

/*----------------------------------------------------------------------------
            Public Code
-----------------------------------------------------------------------------*/
void DisplayProtoList(const char* ch, LIST protolist) {
  void* window = c_create_window("Char samples", 50, 200,
                                 520, 520, -130.0, 130.0, -130.0, 130.0);
  LIST proto = protolist;
  iterate(proto) {
    PROTOTYPE* prototype = reinterpret_cast<PROTOTYPE *>(first_node(proto));
    if (prototype->Significant)
      c_line_color_index(window, Green);
    else if (prototype->NumSamples == 0)
      c_line_color_index(window, Blue);
    else if (prototype->Merged)
      c_line_color_index(window, Magenta);
    else
      c_line_color_index(window, Red);
    float x = CenterX(prototype->Mean);
    float y = CenterY(prototype->Mean);
    double angle = OrientationOf(prototype->Mean) * 2 * M_PI;
    float dx = static_cast<float>(LengthOf(prototype->Mean) * cos(angle) / 2);
    float dy = static_cast<float>(LengthOf(prototype->Mean) * sin(angle) / 2);
    c_move(window, (x - dx) * 256, (y - dy) * 256);
    c_draw(window, (x + dx) * 256, (y + dy) * 256);
    if (prototype->Significant)
      tprintf("Green proto at (%g,%g)+(%g,%g) %d samples\n",
              x, y, dx, dy, prototype->NumSamples);
    else if (prototype->NumSamples > 0 && !prototype->Merged)
      tprintf("Red proto at (%g,%g)+(%g,%g) %d samples\n",
              x, y, dx, dy, prototype->NumSamples);
  }
  c_make_current(window);
}

char* new_dup(const char* str) {
  int len = strlen(str);
  char* new_str = new char[len + 1];
  strcpy(new_str, str);
  return new_str;
}

/*---------------------------------------------------------------------------*/
int main (int argc, char **argv) {
/*
**  Parameters:
**    argc  number of command line arguments
**    argv  array of command line arguments
**  Globals: none
**  Operation:
**    This program reads in a text file consisting of feature
**    samples from a training page in the following format:
**
**      FontName CharName NumberOfFeatureTypes(N)
**         FeatureTypeName1 NumberOfFeatures(M)
**            Feature1
**            ...
**            FeatureM
**         FeatureTypeName2 NumberOfFeatures(M)
**            Feature1
**            ...
**            FeatureM
**         ...
**         FeatureTypeNameN NumberOfFeatures(M)
**            Feature1
**            ...
**            FeatureM
**      FontName CharName ...
**
**    The result of this program is a binary inttemp file used by
**    the OCR engine.
**  Return: none
**  Exceptions: none
**  History:  Fri Aug 18 08:56:17 1989, DSJ, Created.
**        Mon May 18 1998, Christy Russson, Revistion started.
*/
  char  *PageName;
  FILE  *TrainingPage;
  FILE  *OutFile;
  CLUSTERER  *Clusterer = NULL;
  LIST    ProtoList = NIL_LIST;
  LABELEDLIST CharSample;
  PROTOTYPE  *Prototype;
  LIST     ClassList = NIL_LIST;
  int    Cid, Pid;
  PROTO    Proto;
  PROTO_STRUCT  DummyProto;
  BIT_VECTOR  Config2;
  MERGE_CLASS  MergeClass;
  INT_TEMPLATES  IntTemplates;
  LIST pCharList, pProtoList;
  char Filename[MAXNAMESIZE];
  tesseract::Classify *classify = new tesseract::Classify();
  FEATURE_DEFS_STRUCT FeatureDefs;
  InitFeatureDefs(&FeatureDefs);

  ParseArguments (argc, argv);
  if (InputUnicharsetFile == NULL) {
    InputUnicharsetFile = kInputUnicharsetFile;
  }
  if (OutputUnicharsetFile == NULL) {
    OutputUnicharsetFile = kOutputUnicharsetFile;
  }

  UNICHARSET unicharset_training;
  if (!unicharset_training.load_from_file(InputUnicharsetFile, true)) {
    fprintf(stderr, "Failed to load unicharset from file %s\n"
            "Building unicharset for mftraining from scratch...\n",
            InputUnicharsetFile);
    unicharset_training.clear();
    // Space character needed to represent NIL_LIST classification.
    unicharset_training.unichar_insert(" ");
  }


  // Populate fontinfo_table with font properties.
  if (InputFontInfoFile != NULL) {
    FILE* f = fopen(InputFontInfoFile, "r");
    if (f == NULL) {
      fprintf(stderr, "Failed to load font_properties\n");
    } else {
      int italic, bold, fixed, serif, fraktur;
      while (!feof(f)) {
        FontInfo fontinfo;
        fontinfo.name = new char[1024];
        fontinfo.properties = 0;
        if (fscanf(f, "%1024s %i %i %i %i %i\n", fontinfo.name,
                   &italic, &bold, &fixed, &serif, &fraktur) != 6)
          continue;
        fontinfo.properties =
            (italic << 0) +
            (bold << 1) +
            (fixed << 2) +
            (serif << 3) +
            (fraktur << 4);
        if (!classify->get_fontinfo_table().contains(fontinfo)) {
          classify->get_fontinfo_table().push_back(fontinfo);
        } else {
          fprintf(stderr, "Font %s already defined\n", fontinfo.name);
        }
      }
      fclose(f);
    }
  }

  int *xheights = new int[classify->get_fontinfo_table().size()];
  InitXHeights(InputXHeightsFile, classify->get_fontinfo_table(), xheights);

  while ((PageName = GetNextFilename(argc, argv)) != NULL) {
    tprintf ("Reading %s ...\n", PageName);
    char *short_name = strrchr(PageName, '/');
    if (short_name == NULL)
      short_name = PageName;
    else
      ++short_name;
    // filename is expected to be of the form [lang].[fontname].exp[num].tr
    // If it is, then set short_name to be the [fontname]. Otherwise it is just
    // the file basename with the .tr extension removed.
    char *font_dot = strchr(short_name, '.');
    char *exp_dot = (font_dot != NULL) ? strstr(font_dot, ".exp") : NULL;
    if (font_dot != NULL && exp_dot != NULL && font_dot != exp_dot) {
      short_name = new_dup(font_dot + 1);
      short_name[exp_dot - font_dot - 1] = '\0';
    } else {
      short_name = new_dup(short_name);
      int len = strlen(short_name);
      if (!strcmp(short_name + len - 3, ".tr"))
        short_name[len - 3] = '\0';
    }
    int fontinfo_id;
    FontInfo fontinfo;
    fontinfo.name = short_name;
    fontinfo.properties = 0;  // not used to lookup in the table
    if (!classify->get_fontinfo_table().contains(fontinfo)) {
      fontinfo_id = classify->get_fontinfo_table().push_back(fontinfo);
      tprintf("%s has no defined properties.\n", short_name);
      ASSERT_HOST(!"Missing font_properties entry is a fatal error!");
    } else {
      fontinfo_id = classify->get_fontinfo_table().get_id(fontinfo);
      // Update the properties field.
      fontinfo = classify->get_fontinfo_table().get(fontinfo_id);
      delete[] short_name;
    }

    TrainingPage = Efopen (PageName, "r");
    LIST char_list = NIL_LIST;
    ReadTrainingSamples(FeatureDefs, PROGRAM_FEATURE_TYPE,
                        0, 1.0f / 128.0f, 1.0f / 64.0f, &unicharset_training,
                        TrainingPage, &char_list);
    fclose (TrainingPage);
    pCharList = char_list;
    iterate(pCharList) {
      // Cluster.
      CharSample = (LABELEDLIST) first_node (pCharList);
      Clusterer =
        SetUpForClustering(FeatureDefs, CharSample, PROGRAM_FEATURE_TYPE);
      Config.MagicSamples = CharSample->SampleCount;
      ProtoList = ClusterSamples(Clusterer, &Config);
      CleanUpUnusedData(ProtoList);

      // Merge.
      MergeInsignificantProtos(ProtoList, CharSample->Label,
                               Clusterer, &Config);
      if (strcmp(test_ch, CharSample->Label) == 0)
        DisplayProtoList(test_ch, ProtoList);
      ProtoList = RemoveInsignificantProtos(ProtoList, true,
                                            false,
                                            Clusterer->SampleSize);
      FreeClusterer(Clusterer);
      MergeClass = FindClass (ClassList, CharSample->Label);
      if (MergeClass == NULL) {
        MergeClass = NewLabeledClass (CharSample->Label);
        ClassList = push (ClassList, MergeClass);
      }
      Cid = AddConfigToClass(MergeClass->Class);
      MergeClass->Class->font_set.push_back(fontinfo_id);
      pProtoList = ProtoList;
      iterate (pProtoList) {
        Prototype = (PROTOTYPE *) first_node (pProtoList);

        // See if proto can be approximated by existing proto.
        Pid = FindClosestExistingProto(MergeClass->Class,
                                       MergeClass->NumMerged, Prototype);
        if (Pid == NO_PROTO) {
          Pid = AddProtoToClass (MergeClass->Class);
          Proto = ProtoIn (MergeClass->Class, Pid);
          MakeNewFromOld (Proto, Prototype);
          MergeClass->NumMerged[Pid] = 1;
        }
        else {
          MakeNewFromOld (&DummyProto, Prototype);
          ComputeMergedProto (ProtoIn (MergeClass->Class, Pid), &DummyProto,
              (FLOAT32) MergeClass->NumMerged[Pid], 1.0,
              ProtoIn (MergeClass->Class, Pid));
          MergeClass->NumMerged[Pid] ++;
        }
        Config2 = MergeClass->Class->Configurations[Cid];
        AddProtoToConfig (Pid, Config2);
      }
      FreeProtoList (&ProtoList);
    }
    FreeTrainingSamples(char_list);

    // If there is a file with [lang].[fontname].exp[num].fontinfo present,
    // write font spacing information to fontinfo_table.
    int pagename_len = strlen(PageName);
    char *fontinfo_file_name = new char[pagename_len + 7];
    strncpy(fontinfo_file_name, PageName, pagename_len-2);  // remove "tr"
    strcpy(fontinfo_file_name+pagename_len-2, "fontinfo");  // add "fontinfo"
    AddSpacingInfo(fontinfo_file_name, fontinfo_id, unicharset_training,
                   xheights, &(classify->get_fontinfo_table()));
    delete[] fontinfo_file_name;
  }
  WriteMicrofeat(Directory, ClassList);
  SetUpForFloat2Int(unicharset_training, ClassList);
  IntTemplates = classify->CreateIntTemplates(TrainingData,
                                              unicharset_training);
  strcpy (Filename, "");
  if (Directory != NULL) {
    strcat (Filename, Directory);
    strcat (Filename, "/");
  }
  strcat (Filename, "inttemp");
#ifdef __UNIX__
  OutFile = Efopen (Filename, "w");
#else
  OutFile = Efopen (Filename, "wb");
#endif
  classify->WriteIntTemplates(OutFile, IntTemplates, unicharset_training);
  fclose (OutFile);
  strcpy (Filename, "");
  if (Directory != NULL) {
    strcat (Filename, Directory);
    strcat (Filename, "/");
  }
  strcat (Filename, "pffmtable");
  // Now create pffmtable.
  WritePFFMTable(IntTemplates, unicharset_training, Filename);
  // Write updated unicharset to a file.
  if (!unicharset_training.save_to_file(OutputUnicharsetFile)) {
    fprintf(stderr, "Failed to save unicharset to file %s\n",
            OutputUnicharsetFile);
    exit(1);
  }
  printf ("Done!\n"); /**/
  FreeLabeledClassList (ClassList);
  delete classify;
  if (test_ch[0] != '\0') {
    // If we are displaying debug window(s), wait for the user to look at them.
    while (getchar() != '\n');
  }
  return 0;
}  /* main */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/


/*--------------------------------------------------------------------------*/
void WriteMicrofeat(
    char  *Directory,
  LIST  ClassList)

{
  FILE    *File;
  char    Filename[MAXNAMESIZE];
  MERGE_CLASS MergeClass;

  strcpy (Filename, "");
  if (Directory != NULL)
  {
    strcat (Filename, Directory);
    strcat (Filename, "/");
  }
  strcat (Filename, "Microfeat");
  File = Efopen (Filename, "w");
  printf ("\nWriting Merged %s ...", Filename);
  iterate(ClassList)
  {
    MergeClass = (MERGE_CLASS) first_node (ClassList);
    WriteProtos(File, MergeClass);
    WriteConfigs(File, MergeClass->Class);
  }
  fclose (File);
} // WriteMicrofeat

/*---------------------------------------------------------------------------*/
void WriteProtos(
  FILE* File,
  MERGE_CLASS MergeClass)
{
  float Values[3];
  int i;
  PROTO Proto;

  fprintf(File, "%s\n", MergeClass->Label);
  fprintf(File, "%d\n", MergeClass->Class->NumProtos);
  for(i=0; i < MergeClass->Class->NumProtos; i++)
  {
    Proto = ProtoIn(MergeClass->Class,i);
    fprintf(File, "\t%8.4f %8.4f %8.4f %8.4f ", Proto->X, Proto->Y,
      Proto->Length, Proto->Angle);
    Values[0] = Proto->X;
    Values[1] = Proto->Y;
    Values[2] = Proto->Angle;
    Normalize(Values);
    fprintf(File, "%8.4f %8.4f %8.4f\n", Values[0], Values[1], Values[2]);
  }
} // WriteProtos

/*----------------------------------------------------------------------------*/
void WriteConfigs(
  FILE* File,
  CLASS_TYPE Class)
{
  BIT_VECTOR Config;
  int i, j, WordsPerConfig;

  WordsPerConfig = WordsInVectorOfSize(Class->NumProtos);
  fprintf(File, "%d %d\n", Class->NumConfigs,WordsPerConfig);
  for(i=0; i < Class->NumConfigs; i++)
  {
    Config = Class->Configurations[i];
    for(j=0; j < WordsPerConfig; j++)
      fprintf(File, "%08x ", Config[j]);
    fprintf(File, "\n");
  }
  fprintf(File, "\n");
} // WriteConfigs

/*--------------------------------------------------------------------------*/
void WritePFFMTable(INT_TEMPLATES Templates, const UNICHARSET& unicharset,
                    const char* filename) {
  FILE* fp = Efopen(filename, "wb");
  /* then write out each class */
  for (int i = 0; i < Templates->NumClasses; i++) {
    INT_CLASS Class = ClassForClassId (Templates, i);
    // Todo: Test with min instead of max
    // int MaxLength = LengthForConfigId(Class, 0);
    int MaxLength = 0;
    const char *unichar = unicharset.id_to_unichar(i);
    if (strcmp(unichar, " ") == 0) {
      unichar = "NULL";
    } else if (Class->NumConfigs == 0) {
      tprintf("Error: no configs for class %s in mftraining\n", unichar);
    }
    for (int ConfigId = 0; ConfigId < Class->NumConfigs; ConfigId++) {
      // Todo: Test with min instead of max
      // if (LengthForConfigId (Class, ConfigId) < MaxLength)
      if (Class->ConfigLengths[ConfigId] > MaxLength)
        MaxLength = Class->ConfigLengths[ConfigId];
    }
    fprintf(fp, "%s %d\n", unichar, MaxLength);
  }
  fclose(fp);
} // WritePFFMTable

/*--------------------------------------------------------------------------*/
// Reads xheights for various fonts from filename and records them into
// xheights[] keyed by fontinfo_id (xheights[] is assumed to have
// fontinfo_table->size() slots allocated).
void InitXHeights(const char *filename,
                   const UnicityTable<FontInfo> &fontinfo_table,
                   int xheights[]) {
  for (int i = 0; i < fontinfo_table.size(); ++i) xheights[i] = -1;
  if (filename == NULL) return;
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Failed to load font xheights from %s\n", filename);
    return;
  }
  tprintf("Reading x-heights from %s ...\n", filename);
  FontInfo fontinfo;
  fontinfo.properties = 0;  // not used to lookup in the table
  char buffer[1024];
  int xht;
  while (!feof(f)) {
    ASSERT_HOST(fscanf(f, "%1024s %d\n", buffer, &xht) == 2);
    fontinfo.name = buffer;
    if (!fontinfo_table.contains(fontinfo)) continue;
    int fontinfo_id = fontinfo_table.get_id(fontinfo);
    xheights[fontinfo_id] = xht;
  }
}  // InitXHeights

/*--------------------------------------------------------------------------*/
// Reads spacing stats from filename and adds them to fontinfo_table.
void AddSpacingInfo(const char *filename,
                     int fontinfo_id,
                     const UNICHARSET &unicharset,
                     const int xheights[],
                     UnicityTable<FontInfo> *fontinfo_table) {
  FILE* fontinfo_file = fopen(filename, "r");
  if (fontinfo_file == NULL) return;
  tprintf("Reading spacing from %s ...\n", filename);
  int scale = kBlnXHeight / xheights[fontinfo_id];
  int num_unichars;
  char uch[UNICHAR_LEN];
  char kerned_uch[UNICHAR_LEN];
  int x_gap, x_gap_before, x_gap_after, num_kerned;
  ASSERT_HOST(fscanf(fontinfo_file, "%d\n", &num_unichars) == 1);
  FontInfo *fi = fontinfo_table->get_mutable(fontinfo_id);
  fi->init_spacing(unicharset.size());
  FontSpacingInfo *spacing = NULL;
  for (int l = 0; l < num_unichars; ++l) {
    ASSERT_HOST(fscanf(fontinfo_file, "%s %d %d %d",
                       uch, &x_gap_before,
                       &x_gap_after, &num_kerned) == 4);
    bool valid = unicharset.contains_unichar(uch);
    if (valid) {
      spacing = new FontSpacingInfo();
      spacing->x_gap_before = static_cast<inT16>(x_gap_before * scale);
      spacing->x_gap_after = static_cast<inT16>(x_gap_after * scale);
    }
    for (int k = 0; k < num_kerned; ++k) {
      ASSERT_HOST(fscanf(fontinfo_file, "%s %d", kerned_uch, &x_gap) == 2);
      if (!valid || !unicharset.contains_unichar(kerned_uch)) continue;
      spacing->kerned_unichar_ids.push_back(
          unicharset.unichar_to_id(kerned_uch));
      spacing->kerned_x_gaps.push_back(static_cast<inT16>(x_gap * scale));
    }
    if (valid) fi->add_spacing(unicharset.unichar_to_id(uch), spacing);
  }
  fclose(fontinfo_file);
}  // AddSpacingInfo

