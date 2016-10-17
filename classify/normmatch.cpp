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
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
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
#include "params.h"

struct NORM_PROTOS
{
  int NumParams;
  PARAM_DESC *ParamDesc;
  LIST* Protos;
  int NumProtos;
};

/*----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------*/
double NormEvidenceOf(register double NormAdj);

void PrintNormMatch(FILE *File,
                    int NumParams,
                    PROTOTYPE *Proto,
                    FEATURE Feature);

NORM_PROTOS *ReadNormProtos(FILE *File);

/*----------------------------------------------------------------------------
        Variables
----------------------------------------------------------------------------*/

/** control knobs used to control the normalization adjustment process */
double_VAR(classify_norm_adj_midpoint, 32.0, "Norm adjust midpoint ...");
double_VAR(classify_norm_adj_curl, 2.0, "Norm adjust curl ...");
/** Weight of width variance against height and vertical position. */
const double kWidthErrorWeighting = 0.125;

/*----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
namespace tesseract {
/**
 * This routine compares Features against each character
 * normalization proto for ClassId and returns the match
 * rating of the best match.
 * @param ClassId id of class to match against
 * @param feature character normalization feature
 * @param DebugMatch controls dump of debug info
 *
 * Globals:
 * #NormProtos character normalization prototypes
 *
 * @return Best match rating for Feature against protos of ClassId.
 * @note Exceptions: none
 * @note History: Wed Dec 19 16:56:12 1990, DSJ, Created.
 */
FLOAT32 Classify::ComputeNormMatch(CLASS_ID ClassId,
                                   const FEATURE_STRUCT& feature,
                                   BOOL8 DebugMatch) {
  LIST Protos;
  FLOAT32 BestMatch;
  FLOAT32 Match;
  FLOAT32 Delta;
  PROTOTYPE *Proto;
  int ProtoId;

  if (ClassId >= NormProtos->NumProtos) {
    ClassId = NO_CLASS;
  }

  /* handle requests for classification as noise */
  if (ClassId == NO_CLASS) {
    /* kludge - clean up constants and make into control knobs later */
    Match = (feature.Params[CharNormLength] *
      feature.Params[CharNormLength] * 500.0 +
      feature.Params[CharNormRx] *
      feature.Params[CharNormRx] * 8000.0 +
      feature.Params[CharNormRy] *
      feature.Params[CharNormRy] * 8000.0);
    return (1.0 - NormEvidenceOf (Match));
  }

  BestMatch = MAX_FLOAT32;
  Protos = NormProtos->Protos[ClassId];

  if (DebugMatch) {
    tprintf("\nChar norm for class %s\n", unicharset.id_to_unichar(ClassId));
  }

  ProtoId = 0;
  iterate(Protos) {
    Proto = (PROTOTYPE *) first_node (Protos);
    Delta = feature.Params[CharNormY] - Proto->Mean[CharNormY];
    Match = Delta * Delta * Proto->Weight.Elliptical[CharNormY];
    if (DebugMatch) {
      tprintf("YMiddle: Proto=%g, Delta=%g, Var=%g, Dist=%g\n",
              Proto->Mean[CharNormY], Delta,
              Proto->Weight.Elliptical[CharNormY], Match);
    }
    Delta = feature.Params[CharNormRx] - Proto->Mean[CharNormRx];
    Match += Delta * Delta * Proto->Weight.Elliptical[CharNormRx];
    if (DebugMatch) {
      tprintf("Height: Proto=%g, Delta=%g, Var=%g, Dist=%g\n",
              Proto->Mean[CharNormRx], Delta,
              Proto->Weight.Elliptical[CharNormRx], Match);
    }
    // Ry is width! See intfx.cpp.
    Delta = feature.Params[CharNormRy] - Proto->Mean[CharNormRy];
    if (DebugMatch) {
      tprintf("Width: Proto=%g, Delta=%g, Var=%g\n",
              Proto->Mean[CharNormRy], Delta,
              Proto->Weight.Elliptical[CharNormRy]);
    }
    Delta = Delta * Delta * Proto->Weight.Elliptical[CharNormRy];
    Delta *= kWidthErrorWeighting;
    Match += Delta;
    if (DebugMatch) {
      tprintf("Total Dist=%g, scaled=%g, sigmoid=%g, penalty=%g\n",
              Match, Match / classify_norm_adj_midpoint,
              NormEvidenceOf(Match), 256 * (1 - NormEvidenceOf(Match)));
    }

    if (Match < BestMatch)
      BestMatch = Match;

    ProtoId++;
  }
  return 1.0 - NormEvidenceOf(BestMatch);
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

/*----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------*/
/**
 * @name NormEvidenceOf
 *
 * Return the new type of evidence number corresponding to this
 * normalization adjustment.  The equation that represents the transform is:
 *       1 / (1 + (NormAdj / midpoint) ^ curl)
 */
double NormEvidenceOf(register double NormAdj) {
  NormAdj /= classify_norm_adj_midpoint;

  if (classify_norm_adj_curl == 3)
    NormAdj = NormAdj * NormAdj * NormAdj;
  else if (classify_norm_adj_curl == 2)
    NormAdj = NormAdj * NormAdj;
  else
    NormAdj = pow (NormAdj, classify_norm_adj_curl);
  return (1.0 / (1.0 + NormAdj));
}


/*---------------------------------------------------------------------------*/
/**
 * This routine dumps out detailed normalization match info.
 * @param File		open text file to dump match debug info to
 * @param NumParams	# of parameters in proto and feature
 * @param Proto[]		array of prototype parameters
 * @param Feature[]	array of feature parameters
 * Globals: none
 * @return  none
 * @note Exceptions: none
 * @note History: Wed Jan  2 09:49:35 1991, DSJ, Created.
 */
void PrintNormMatch(FILE *File,
                    int NumParams,
                    PROTOTYPE *Proto,
                    FEATURE Feature) {
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
/**
 * This routine allocates a new data structure to hold
 * a set of character normalization protos.  It then fills in
 * the data structure by reading from the specified File.
 * @param File open text file to read normalization protos from
 * @param end_offset
 * Globals: none
 * @return Character normalization protos.
 * @note Exceptions: none
 * @note History: Wed Dec 19 16:38:49 1990, DSJ, Created.
 */
NORM_PROTOS *Classify::ReadNormProtos(FILE *File, inT64 end_offset) {
  NORM_PROTOS *NormProtos;
  int i;
  char unichar[2 * UNICHAR_LEN + 1];
  UNICHAR_ID unichar_id;
  LIST Protos;
  int NumProtos;

  /* allocate and initialization data structure */
  NormProtos = (NORM_PROTOS *) Emalloc (sizeof (NORM_PROTOS));
  NormProtos->NumProtos = unicharset.size();
  NormProtos->Protos = (LIST *) Emalloc (NormProtos->NumProtos * sizeof(LIST));
  for (i = 0; i < NormProtos->NumProtos; i++)
    NormProtos->Protos[i] = NIL_LIST;

  /* read file header and save in data structure */
  NormProtos->NumParams = ReadSampleSize (File);
  NormProtos->ParamDesc = ReadParamDesc (File, NormProtos->NumParams);

  /* read protos for each class into a separate list */
  while ((end_offset < 0 || ftell(File) < end_offset) &&
         tfscanf(File, "%s %d", unichar, &NumProtos) == 2) {
    if (unicharset.contains_unichar(unichar)) {
      unichar_id = unicharset.unichar_to_id(unichar);
      Protos = NormProtos->Protos[unichar_id];
      for (i = 0; i < NumProtos; i++)
        Protos =
            push_last (Protos, ReadPrototype (File, NormProtos->NumParams));
      NormProtos->Protos[unichar_id] = Protos;
    } else {
      cprintf("Error: unichar %s in normproto file is not in unichar set.\n",
              unichar);
      for (i = 0; i < NumProtos; i++)
        FreePrototype(ReadPrototype (File, NormProtos->NumParams));
    }
    SkipNewline(File);
  }
  return (NormProtos);
}                                /* ReadNormProtos */
}  // namespace tesseract
