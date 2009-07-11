/******************************************************************************
 **	Filename:    normmatch.c
 **	Purpose:     Simple matcher based on character normalization features.
 **	Author:      Dan Johnson
 **	History:     Wed Dec 19 16:18:06 1990, DSJ, Created.
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
#include "normmatch.h"

#include <stdio.h>
#include <math.h>

#include "classify.h"
#include "clusttool.h"
#include "const.h"
#include "efio.h"
#include "emalloc.h"
#include "globals.h"
#include "helpers.h"
#include "normfeat.h"
#include "scanutils.h"
#include "unicharset.h"
#include "varable.h"

struct NORM_PROTOS
{
  int NumParams;
  PARAM_DESC *ParamDesc;
  LIST* Protos;
  int NumProtos;
};

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
FLOAT32 NormEvidenceOf(register FLOAT32 NormAdj);

void PrintNormMatch(FILE *File,
                    int NumParams,
                    PROTOTYPE *Proto,
                    FEATURE Feature);

NORM_PROTOS *ReadNormProtos(FILE *File);

/**----------------------------------------------------------------------------
        Variables
----------------------------------------------------------------------------**/

/* control knobs used to control the normalization adjustment process */
double_VAR(classify_norm_adj_midpoint, 32.0, "Norm adjust midpoint ...");
double_VAR(classify_norm_adj_curl, 2.0, "Norm adjust curl ...");

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
namespace tesseract {
FLOAT32 Classify::ComputeNormMatch(CLASS_ID ClassId, FEATURE Feature,
                                   BOOL8 DebugMatch) {
/*
 **	Parameters:
 **		ClassId		id of class to match against
 **		Feature		character normalization feature
 **		DebugMatch	controls dump of debug info
 **	Globals:
 **		NormProtos	character normalization prototypes
 **	Operation: This routine compares Features against each character
 **		normalization proto for ClassId and returns the match
 **		rating of the best match.
 **	Return: Best match rating for Feature against protos of ClassId.
 **	Exceptions: none
 **	History: Wed Dec 19 16:56:12 1990, DSJ, Created.
 */
  LIST Protos;
  FLOAT32 BestMatch;
  FLOAT32 Match;
  FLOAT32 Delta;
  PROTOTYPE *Proto;
  int ProtoId;

  /* handle requests for classification as noise */
  if (ClassId == NO_CLASS) {
    /* kludge - clean up constants and make into control knobs later */
    Match = (Feature->Params[CharNormLength] *
      Feature->Params[CharNormLength] * 500.0 +
      Feature->Params[CharNormRx] *
      Feature->Params[CharNormRx] * 8000.0 +
      Feature->Params[CharNormRy] *
      Feature->Params[CharNormRy] * 8000.0);
    return (1.0 - NormEvidenceOf (Match));
  }

  BestMatch = MAX_FLOAT32;
  Protos = NormProtos->Protos[ClassId];

  if (DebugMatch) {
    cprintf ("\nFeature = ");
    WriteFeature(stdout, Feature);
  }

  ProtoId = 0;
  iterate(Protos) {
    Proto = (PROTOTYPE *) first_node (Protos);
    Delta = Feature->Params[CharNormY] - Proto->Mean[CharNormY];
    Match = Delta * Delta * Proto->Weight.Elliptical[CharNormY];
    Delta = Feature->Params[CharNormRx] - Proto->Mean[CharNormRx];
    Match += Delta * Delta * Proto->Weight.Elliptical[CharNormRx];

    if (Match < BestMatch)
      BestMatch = Match;

    if (DebugMatch) {
      cprintf ("Proto %1d = ", ProtoId);
      WriteNFloats (stdout, NormProtos->NumParams, Proto->Mean);
      cprintf ("      var = ");
      WriteNFloats (stdout, NormProtos->NumParams,
        Proto->Variance.Elliptical);
      cprintf ("    match = ");
      PrintNormMatch (stdout, NormProtos->NumParams, Proto, Feature);
    }
    ProtoId++;
  }
  return (1.0 - NormEvidenceOf (BestMatch));
}                                /* ComputeNormMatch */

void Classify::FreeNormProtos() {
  if (NormProtos != NULL) {
    for (int i = 0; i < NormProtos->NumProtos; i++)
      FreeProtoList(&NormProtos->Protos[i]);
    Efree(NormProtos->Protos);
    Efree(NormProtos->ParamDesc);
    Efree(NormProtos);
    NormProtos = NULL;
  }
}
}  // namespace tesseract

/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/**********************************************************************
 * NormEvidenceOf
 *
 * Return the new type of evidence number corresponding to this
 * normalization adjustment.  The equation that represents the transform is:
 *       1 / (1 + (NormAdj / midpoint) ^ curl)
 **********************************************************************/
FLOAT32 NormEvidenceOf(register FLOAT32 NormAdj) {
  NormAdj /= classify_norm_adj_midpoint;

  if (classify_norm_adj_curl == 3)
    NormAdj = NormAdj * NormAdj * NormAdj;
  else if (classify_norm_adj_curl == 2)
    NormAdj = NormAdj * NormAdj;
  else
    NormAdj = pow(static_cast<double>(NormAdj), classify_norm_adj_curl);
  return (1.0 / (1.0 + NormAdj));
}


/*---------------------------------------------------------------------------*/
void PrintNormMatch(FILE *File,
                    int NumParams,
                    PROTOTYPE *Proto,
                    FEATURE Feature) {
/*
 **	Parameters:
 **		File		open text file to dump match debug info to
 **		NumParams	# of parameters in proto and feature
 **		Proto[]		array of prototype parameters
 **		Feature[]	array of feature parameters
 **	Globals: none
 **	Operation: This routine dumps out detailed normalization match info.
 **	Return: none
 **	Exceptions: none
 **	History: Wed Jan  2 09:49:35 1991, DSJ, Created.
 */
  int i;
  FLOAT32 ParamMatch;
  FLOAT32 TotalMatch;

  for (i = 0, TotalMatch = 0.0; i < NumParams; i++) {
    ParamMatch = (Feature->Params[i] - Mean(Proto, i)) /
      StandardDeviation(Proto, i);

    fprintf (File, " %6.1f", ParamMatch);

    if (i == CharNormY || i == CharNormRx)
      TotalMatch += ParamMatch * ParamMatch;
  }
  fprintf (File, " --> %6.1f (%4.2f)\n",
    TotalMatch, NormEvidenceOf (TotalMatch));

}                                /* PrintNormMatch */


/*---------------------------------------------------------------------------*/
namespace tesseract {
NORM_PROTOS *Classify::ReadNormProtos(FILE *File, inT64 end_offset) {
/*
 **	Parameters:
 **		File	open text file to read normalization protos from
 **	Globals: none
 **	Operation: This routine allocates a new data structure to hold
 **		a set of character normalization protos.  It then fills in
 **		the data structure by reading from the specified File.
 **	Return: Character normalization protos.
 **	Exceptions: none
 **	History: Wed Dec 19 16:38:49 1990, DSJ, Created.
 */
  NORM_PROTOS *NormProtos;
  int i;
  char unichar[UNICHAR_LEN + 1];
  UNICHAR_ID unichar_id;
  LIST Protos;
  int NumProtos;

  /* allocate and initialization data structure */
  NormProtos = (NORM_PROTOS *) Emalloc (sizeof (NORM_PROTOS));
  NormProtos->NumProtos = unicharset.size();
  NormProtos->Protos = (LIST *) Emalloc (NormProtos->NumProtos * sizeof(LIST));
  for (i = 0; i < NormProtos->NumProtos; i++)
    NormProtos->Protos[i] = NIL;

  /* read file header and save in data structure */
  NormProtos->NumParams = ReadSampleSize (File);
  NormProtos->ParamDesc = ReadParamDesc (File, NormProtos->NumParams);

  /* read protos for each class into a separate list */
  while ((end_offset < 0 || ftell(File) < end_offset) &&
         fscanf(File, "%s %d", unichar, &NumProtos) == 2) {
    if (unicharset.contains_unichar(unichar)) {
      unichar_id = unicharset.unichar_to_id(unichar);
      Protos = NormProtos->Protos[unichar_id];
      for (i = 0; i < NumProtos; i++)
        Protos =
            push_last (Protos, ReadPrototype (File, NormProtos->NumParams));
      NormProtos->Protos[unichar_id] = Protos;
    } else
      cprintf("Error: unichar %s in normproto file is not in unichar set.\n");
    SkipNewline(File);
  }
  return (NormProtos);
}                                /* ReadNormProtos */
}  // namespace tesseract
