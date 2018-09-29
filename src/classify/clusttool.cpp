/******************************************************************************
 ** Filename: clustertool.c
 ** Purpose:  Misc. tools for use with the clustering routines
 ** Author:   Dan Johnson
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
 *****************************************************************************/

//--------------------------Include Files----------------------------------
#include "clusttool.h"
#include "emalloc.h"
#include <cstdio>
#include <cmath>

using tesseract::TFile;

//---------------Global Data Definitions and Declarations--------------------
#define TOKENSIZE 80         //< max size of tokens read from an input file
#define QUOTED_TOKENSIZE "79"
#define MAXSAMPLESIZE 65535  //< max num of dimensions in feature space
//#define MAXBLOCKSIZE  65535   //< max num of samples in a character (block
// size)

/**
 * This routine reads a single integer from the specified
 * file and checks to ensure that it is between 0 and
 * MAXSAMPLESIZE.
 * @param fp open text file to read sample size from
 * @return Sample size
 * @note Globals: None
 */
uint16_t ReadSampleSize(TFile *fp) {
  int SampleSize = 0;

  const int kMaxLineSize = 100;
  char line[kMaxLineSize];
  ASSERT_HOST(fp->FGets(line, kMaxLineSize) != nullptr);
  ASSERT_HOST(sscanf(line, "%d", &SampleSize) == 1);
  ASSERT_HOST(SampleSize >= 0 && SampleSize <= MAXSAMPLESIZE);
  return SampleSize;
}

/**
 * This routine reads textual descriptions of sets of parameters
 * which describe the characteristics of feature dimensions.
 *
 * @param fp open text file to read N parameter descriptions from
 * @param N number of parameter descriptions to read
 * @return Pointer to an array of parameter descriptors.
 * @note Globals: None
 */
PARAM_DESC *ReadParamDesc(TFile *fp, uint16_t N) {
  PARAM_DESC *ParamDesc;
  char linear_token[TOKENSIZE], essential_token[TOKENSIZE];

  ParamDesc = (PARAM_DESC *) Emalloc (N * sizeof (PARAM_DESC));
  for (int i = 0; i < N; i++) {
    const int kMaxLineSize = TOKENSIZE * 4;
    char line[kMaxLineSize];
    ASSERT_HOST(fp->FGets(line, kMaxLineSize) != nullptr);
    ASSERT_HOST(sscanf(line,
                "%" QUOTED_TOKENSIZE "s %" QUOTED_TOKENSIZE "s %f %f",
                linear_token, essential_token, &ParamDesc[i].Min,
                &ParamDesc[i].Max) == 4);
    if (linear_token[0] == 'c')
      ParamDesc[i].Circular = TRUE;
    else
      ParamDesc[i].Circular = FALSE;

    if (linear_token[0] == 'e')
      ParamDesc[i].NonEssential = FALSE;
    else
      ParamDesc[i].NonEssential = TRUE;
    ParamDesc[i].Range = ParamDesc[i].Max - ParamDesc[i].Min;
    ParamDesc[i].HalfRange = ParamDesc[i].Range / 2;
    ParamDesc[i].MidRange = (ParamDesc[i].Max + ParamDesc[i].Min) / 2;
  }
  return (ParamDesc);
}

/**
 * This routine reads a textual description of a prototype from
 * the specified file.
 *
 * @param fp open text file to read prototype from
 * @param N number of dimensions used in prototype
 * @return List of prototypes
 * @note Globals: None
 */
PROTOTYPE *ReadPrototype(TFile *fp, uint16_t N) {
  char sig_token[TOKENSIZE], shape_token[TOKENSIZE];
  PROTOTYPE *Proto;
  int SampleCount;
  int i;

  const int kMaxLineSize = TOKENSIZE * 4;
  char line[kMaxLineSize];
  if (fp->FGets(line, kMaxLineSize) == nullptr ||
      sscanf(line, "%" QUOTED_TOKENSIZE "s %" QUOTED_TOKENSIZE "s %d",
             sig_token, shape_token, &SampleCount) != 3) {
    tprintf("Invalid prototype: %s\n", line);
    return nullptr;
  }
  Proto = (PROTOTYPE *)Emalloc(sizeof(PROTOTYPE));
  Proto->Cluster = nullptr;
  if (sig_token[0] == 's')
    Proto->Significant = TRUE;
  else
    Proto->Significant = FALSE;

  switch (shape_token[0]) {
    case 's':
      Proto->Style = spherical;
      break;
    case 'e':
      Proto->Style = elliptical;
      break;
    case 'a':
      Proto->Style = automatic;
      break;
    default:
      tprintf("Invalid prototype style specification:%s\n", shape_token);
      Proto->Style = elliptical;
  }

  ASSERT_HOST(SampleCount >= 0);
  Proto->NumSamples = SampleCount;

  Proto->Mean = ReadNFloats(fp, N, nullptr);
  ASSERT_HOST(Proto->Mean != nullptr);

  switch (Proto->Style) {
    case spherical:
      ASSERT_HOST(ReadNFloats(fp, 1, &(Proto->Variance.Spherical)) != nullptr);
      Proto->Magnitude.Spherical =
          1.0 / sqrt(2.0 * M_PI * Proto->Variance.Spherical);
      Proto->TotalMagnitude = pow(Proto->Magnitude.Spherical, (float)N);
      Proto->LogMagnitude = log((double)Proto->TotalMagnitude);
      Proto->Weight.Spherical = 1.0 / Proto->Variance.Spherical;
      Proto->Distrib = nullptr;
      break;
    case elliptical:
      Proto->Variance.Elliptical = ReadNFloats(fp, N, nullptr);
      ASSERT_HOST(Proto->Variance.Elliptical != nullptr);
      Proto->Magnitude.Elliptical = (float *)Emalloc(N * sizeof(float));
      Proto->Weight.Elliptical = (float *)Emalloc(N * sizeof(float));
      Proto->TotalMagnitude = 1.0;
      for (i = 0; i < N; i++) {
        Proto->Magnitude.Elliptical[i] =
            1.0 / sqrt(2.0 * M_PI * Proto->Variance.Elliptical[i]);
        Proto->Weight.Elliptical[i] = 1.0 / Proto->Variance.Elliptical[i];
        Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
      }
      Proto->LogMagnitude = log((double)Proto->TotalMagnitude);
      Proto->Distrib = nullptr;
      break;
    default:
      Efree(Proto);
      tprintf("Invalid prototype style\n");
      return nullptr;
  }
  return Proto;
}

/**
 * This routine reads N floats from the specified text file
 * and places them into Buffer.  If Buffer is nullptr, a buffer
 * is created and passed back to the caller.  If EOF is
 * encountered before any floats can be read, nullptr is
 * returned.
 * @param fp open text file to read floats from
 * @param N number of floats to read
 * @param Buffer pointer to buffer to place floats into
 * @return Pointer to buffer holding floats or nullptr if EOF
 * @note Globals: None
 */
float *ReadNFloats(TFile *fp, uint16_t N, float Buffer[]) {
  const int kMaxLineSize = 1024;
  char line[kMaxLineSize];
  if (fp->FGets(line, kMaxLineSize) == nullptr) {
    tprintf("Hit EOF in ReadNFloats!\n");
    return nullptr;
  }
  bool needs_free = false;

  if (Buffer == nullptr) {
    Buffer = static_cast<float *>(Emalloc(N * sizeof(float)));
    needs_free = true;
  }

  char *startptr = line;
  for (int i = 0; i < N; i++) {
    char *endptr;
    Buffer[i] = strtof(startptr, &endptr);
    if (endptr == startptr) {
      tprintf("Read of %d floats failed!\n", N);
      if (needs_free) Efree(Buffer);
      return nullptr;
    }
    startptr = endptr;
  }
  return Buffer;
}

/**
 * This routine writes an array of dimension descriptors to
 * the specified text file.
 * @param File open text file to write param descriptors to
 * @param N number of param descriptors to write
 * @param ParamDesc array of param descriptors to write
 * @return None
 * @note Globals: None
 */
void WriteParamDesc(FILE *File, uint16_t N, const PARAM_DESC ParamDesc[]) {
  int i;

  for (i = 0; i < N; i++) {
    if (ParamDesc[i].Circular)
      fprintf (File, "circular ");
    else
      fprintf (File, "linear   ");

    if (ParamDesc[i].NonEssential)
      fprintf (File, "non-essential ");
    else
      fprintf (File, "essential     ");

    fprintf (File, "%10.6f %10.6f\n", ParamDesc[i].Min, ParamDesc[i].Max);
  }
}

/**
 * This routine writes a textual description of a prototype
 * to the specified text file.
 * @param File open text file to write prototype to
 * @param N number of dimensions in feature space
 * @param Proto prototype to write out
 * @return None
 * @note Globals: None
 */
void WritePrototype(FILE *File, uint16_t N, PROTOTYPE *Proto) {
  int i;

  if (Proto->Significant)
    fprintf (File, "significant   ");
  else
    fprintf (File, "insignificant ");
  WriteProtoStyle (File, (PROTOSTYLE) Proto->Style);
  fprintf (File, "%6d\n\t", Proto->NumSamples);
  WriteNFloats (File, N, Proto->Mean);
  fprintf (File, "\t");

  switch (Proto->Style) {
    case spherical:
      WriteNFloats (File, 1, &(Proto->Variance.Spherical));
      break;
    case elliptical:
      WriteNFloats (File, N, Proto->Variance.Elliptical);
      break;
    case mixed:
      for (i = 0; i < N; i++)
      switch (Proto->Distrib[i]) {
        case normal:
          fprintf (File, " %9s", "normal");
          break;
        case uniform:
          fprintf (File, " %9s", "uniform");
          break;
        case D_random:
          fprintf (File, " %9s", "random");
          break;
        case DISTRIBUTION_COUNT:
          ASSERT_HOST(!"Distribution count not allowed!");
      }
      fprintf (File, "\n\t");
      WriteNFloats (File, N, Proto->Variance.Elliptical);
  }
}

/**
 * This routine writes a text representation of N floats from
 * an array to a file.  All of the floats are placed on one line.
 * @param File open text file to write N floats to
 * @param N number of floats to write
 * @param Array array of floats to write
 * @return None
 * @note Globals: None
 */
void WriteNFloats(FILE * File, uint16_t N, float Array[]) {
  for (int i = 0; i < N; i++)
    fprintf(File, " %9.6f", Array[i]);
  fprintf(File, "\n");
}

/**
 * This routine writes to the specified text file a word
 * which represents the ProtoStyle.  It does not append
 * a carriage return to the end.
 * @param File open text file to write prototype style to
 * @param ProtoStyle prototype style to write
 * @return None
 * @note Globals: None
 */
void WriteProtoStyle(FILE *File, PROTOSTYLE ProtoStyle) {
  switch (ProtoStyle) {
    case spherical:
      fprintf (File, "spherical");
      break;
    case elliptical:
      fprintf (File, "elliptical");
      break;
    case mixed:
      fprintf (File, "mixed");
      break;
    case automatic:
      fprintf (File, "automatic");
      break;
  }
}

/**
 * This routine writes a textual description of each prototype
 * in the prototype list to the specified file.  It also
 * writes a file header which includes the number of dimensions
 * in feature space and the descriptions for each dimension.
 * @param File open text file to write prototypes to
 * @param N number of dimensions in feature space
 * @param ParamDesc descriptions for each dimension
 * @param ProtoList list of prototypes to be written
 * @param WriteSigProtos TRUE to write out significant prototypes
 * @param WriteInsigProtos TRUE to write out insignificants
 * @note Globals: None
 * @return None
 */

void WriteProtoList(FILE* File, uint16_t N, PARAM_DESC* ParamDesc,
                    LIST ProtoList, bool WriteSigProtos,
                    bool WriteInsigProtos) {
  PROTOTYPE *Proto;

  /* write file header */
  fprintf(File,"%0d\n",N);
  WriteParamDesc(File,N,ParamDesc);

  /* write prototypes */
  iterate(ProtoList)
    {
      Proto = (PROTOTYPE *) first_node (ProtoList);
      if ((Proto->Significant && WriteSigProtos) ||
          (!Proto->Significant && WriteInsigProtos))
        WritePrototype(File, N, Proto);
    }
}
