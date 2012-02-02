/******************************************************************************
 **	Filename:	clustertool.c
 **	Purpose:	Misc. tools for use with the clustering routines
 **	Author:		Dan Johnson
 **	History:	6/6/89, DSJ, Created.
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

//--------------------------Include Files----------------------------------
#include "clusttool.h"
#include "const.h"
#include "danerror.h"
#include "emalloc.h"
#include "scanutils.h"
#include <stdio.h>
#include <math.h>

//---------------Global Data Definitions and Declarations--------------------
#define TOKENSIZE 80             //max size of tokens read from an input file
#define MAXSAMPLESIZE 65535      //max num of dimensions in feature space
//#define MAXBLOCKSIZE  65535   //max num of samples in a character (block size)

/*---------------------------------------------------------------------------
          Public Code
-----------------------------------------------------------------------------*/
/** ReadSampleSize ***********************************************************
Parameters:	File	open text file to read sample size from
Globals:	None
Operation:	This routine reads a single integer from the specified
      file and checks to ensure that it is between 0 and
      MAXSAMPLESIZE.
Return:		Sample size
Exceptions:	ILLEGALSAMPLESIZE	illegal format or range
History:	6/6/89, DSJ, Created.
******************************************************************************/
uinT16 ReadSampleSize(FILE *File) {
  int SampleSize;

  if ((fscanf (File, "%d", &SampleSize) != 1) ||
    (SampleSize < 0) || (SampleSize > MAXSAMPLESIZE))
    DoError (ILLEGALSAMPLESIZE, "Illegal sample size");
  return (SampleSize);
}                                // ReadSampleSize


/** ReadParamDesc *************************************************************
Parameters:	File	open text file to read N parameter descriptions from
      N	number of parameter descriptions to read
Globals:	None
Operation:	This routine reads textual descriptions of sets of parameters
      which describe the characteristics of feature dimensions.
Return:		Pointer to an array of parameter descriptors.
Exceptions:	ILLEGALCIRCULARSPEC
      ILLEGALESSENTIALSPEC
      ILLEGALMINMAXSPEC
History:	6/6/89, DSJ, Created.
******************************************************************************/
PARAM_DESC *ReadParamDesc(FILE *File, uinT16 N) {
  int i;
  PARAM_DESC *ParamDesc;
  char Token[TOKENSIZE];

  ParamDesc = (PARAM_DESC *) Emalloc (N * sizeof (PARAM_DESC));
  for (i = 0; i < N; i++) {
    if (fscanf (File, "%s", Token) != 1)
      DoError (ILLEGALCIRCULARSPEC,
        "Illegal circular/linear specification");
    if (Token[0] == 'c')
      ParamDesc[i].Circular = TRUE;
    else
      ParamDesc[i].Circular = FALSE;

    if (fscanf (File, "%s", Token) != 1)
      DoError (ILLEGALESSENTIALSPEC,
        "Illegal essential/non-essential spec");
    if (Token[0] == 'e')
      ParamDesc[i].NonEssential = FALSE;
    else
      ParamDesc[i].NonEssential = TRUE;
    if (fscanf (File, "%f%f", &(ParamDesc[i].Min), &(ParamDesc[i].Max)) !=
      2)
      DoError (ILLEGALMINMAXSPEC, "Illegal min or max specification");
    ParamDesc[i].Range = ParamDesc[i].Max - ParamDesc[i].Min;
    ParamDesc[i].HalfRange = ParamDesc[i].Range / 2;
    ParamDesc[i].MidRange = (ParamDesc[i].Max + ParamDesc[i].Min) / 2;
  }
  return (ParamDesc);
}                                // ReadParamDesc


/** ReadPrototype *************************************************************
Parameters:	File	open text file to read prototype from
      N	number of dimensions used in prototype
Globals:	None
Operation:	This routine reads a textual description of a prototype from
      the specified file.
Return:		List of prototypes
Exceptions:	ILLEGALSIGNIFICANCESPEC
      ILLEGALSAMPLECOUNT
      ILLEGALMEANSPEC
      ILLEGALVARIANCESPEC
      ILLEGALDISTRIBUTION
History:	6/6/89, DSJ, Created.
******************************************************************************/
PROTOTYPE *ReadPrototype(FILE *File, uinT16 N) {
  char Token[TOKENSIZE];
  int Status;
  PROTOTYPE *Proto;
  int SampleCount;
  int i;

  if ((Status = fscanf (File, "%s", Token)) == 1) {
    Proto = (PROTOTYPE *) Emalloc (sizeof (PROTOTYPE));
    Proto->Cluster = NULL;
    if (Token[0] == 's')
      Proto->Significant = TRUE;
    else
      Proto->Significant = FALSE;

    Proto->Style = ReadProtoStyle (File);

    if ((fscanf (File, "%d", &SampleCount) != 1) || (SampleCount < 0))
      DoError (ILLEGALSAMPLECOUNT, "Illegal sample count");
    Proto->NumSamples = SampleCount;

    Proto->Mean = ReadNFloats (File, N, NULL);
    if (Proto->Mean == NULL)
      DoError (ILLEGALMEANSPEC, "Illegal prototype mean");

    switch (Proto->Style) {
      case spherical:
        if (ReadNFloats (File, 1, &(Proto->Variance.Spherical)) == NULL)
          DoError (ILLEGALVARIANCESPEC, "Illegal prototype variance");
        Proto->Magnitude.Spherical =
          1.0 / sqrt ((double) (2.0 * PI * Proto->Variance.Spherical));
        Proto->TotalMagnitude =
          pow (Proto->Magnitude.Spherical, (float) N);
        Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);
        Proto->Weight.Spherical = 1.0 / Proto->Variance.Spherical;
        Proto->Distrib = NULL;
        break;
      case elliptical:
        Proto->Variance.Elliptical = ReadNFloats (File, N, NULL);
        if (Proto->Variance.Elliptical == NULL)
          DoError (ILLEGALVARIANCESPEC, "Illegal prototype variance");
        Proto->Magnitude.Elliptical =
          (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
        Proto->Weight.Elliptical =
          (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
        Proto->TotalMagnitude = 1.0;
        for (i = 0; i < N; i++) {
          Proto->Magnitude.Elliptical[i] =
            1.0 /
            sqrt ((double) (2.0 * PI * Proto->Variance.Elliptical[i]));
          Proto->Weight.Elliptical[i] =
            1.0 / Proto->Variance.Elliptical[i];
          Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
        }
        Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);
        Proto->Distrib = NULL;
        break;
      case mixed:
        Proto->Distrib =
          (DISTRIBUTION *) Emalloc (N * sizeof (DISTRIBUTION));
        for (i = 0; i < N; i++) {
          if (fscanf (File, "%s", Token) != 1)
            DoError (ILLEGALDISTRIBUTION,
              "Illegal prototype distribution");
          switch (Token[0]) {
            case 'n':
              Proto->Distrib[i] = normal;
              break;
            case 'u':
              Proto->Distrib[i] = uniform;
              break;
            case 'r':
              Proto->Distrib[i] = D_random;
              break;
            default:
              DoError (ILLEGALDISTRIBUTION,
                "Illegal prototype distribution");
          }
        }
        Proto->Variance.Elliptical = ReadNFloats (File, N, NULL);
        if (Proto->Variance.Elliptical == NULL)
          DoError (ILLEGALVARIANCESPEC, "Illegal prototype variance");
        Proto->Magnitude.Elliptical =
          (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
        Proto->Weight.Elliptical =
          (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));
        Proto->TotalMagnitude = 1.0;
        for (i = 0; i < N; i++) {
          switch (Proto->Distrib[i]) {
            case normal:
              Proto->Magnitude.Elliptical[i] = 1.0 /
                sqrt ((double)
                (2.0 * PI * Proto->Variance.Elliptical[i]));
              Proto->Weight.Elliptical[i] =
                1.0 / Proto->Variance.Elliptical[i];
              break;
            case uniform:
            case D_random:
              Proto->Magnitude.Elliptical[i] = 1.0 /
                (2.0 * Proto->Variance.Elliptical[i]);
              break;
            case DISTRIBUTION_COUNT:
              ASSERT_HOST(!"Distribution count not allowed!");
          }
          Proto->TotalMagnitude *= Proto->Magnitude.Elliptical[i];
        }
        Proto->LogMagnitude = log ((double) Proto->TotalMagnitude);
        break;
    }
    return (Proto);
  }
  else if (Status == EOF)
    return (NULL);
  else {
    DoError (ILLEGALSIGNIFICANCESPEC, "Illegal significance specification");
    return (NULL);
  }
}                                // ReadPrototype


/* ReadProtoStyle *************************************************************
Parameters:	File	open text file to read prototype style from
Globals:	None
Operation:	This routine reads an single token from the specified
      text file and interprets it as a prototype specification.
Return:		Prototype style read from text file
Exceptions:	ILLEGALSTYLESPEC	illegal prototype style specification
History:	6/8/89, DSJ, Created.
*******************************************************************************/
PROTOSTYLE ReadProtoStyle(FILE *File) {
  char Token[TOKENSIZE];
  PROTOSTYLE Style;

  if (fscanf (File, "%s", Token) != 1)
    DoError (ILLEGALSTYLESPEC, "Illegal prototype style specification");
  switch (Token[0]) {
    case 's':
      Style = spherical;
      break;
    case 'e':
      Style = elliptical;
      break;
    case 'm':
      Style = mixed;
      break;
    case 'a':
      Style = automatic;
      break;
    default:
      Style = elliptical;
      DoError (ILLEGALSTYLESPEC, "Illegal prototype style specification");
  }
  return (Style);
}                                // ReadProtoStyle


/** ReadNFloats *************************************************************
Parameters:	File	open text file to read floats from
      N	number of floats to read
      Buffer	pointer to buffer to place floats into
Globals:	None
Operation:	This routine reads N floats from the specified text file
      and places them into Buffer.  If Buffer is NULL, a buffer
      is created and passed back to the caller.  If EOF is
      encountered before any floats can be read, NULL is
      returned.
Return:		Pointer to buffer holding floats or NULL if EOF
Exceptions:	ILLEGALFLOAT
History:	6/6/89, DSJ, Created.
******************************************************************************/
FLOAT32 *
ReadNFloats (FILE * File, uinT16 N, FLOAT32 Buffer[]) {
  int i;
  int NumFloatsRead;

  if (Buffer == NULL)
    Buffer = (FLOAT32 *) Emalloc (N * sizeof (FLOAT32));

  for (i = 0; i < N; i++) {
    NumFloatsRead = fscanf (File, "%f", &(Buffer[i]));
    if (NumFloatsRead != 1) {
      if ((NumFloatsRead == EOF) && (i == 0))
        return (NULL);
      else
        DoError (ILLEGALFLOAT, "Illegal float specification");
    }
  }
  return (Buffer);
}                                // ReadNFloats


/** WriteParamDesc ************************************************************
Parameters:	File	open text file to write param descriptors to
      N		number of param descriptors to write
      ParamDesc	array of param descriptors to write
Globals:	None
Operation:	This routine writes an array of dimension descriptors to
      the specified text file.
Return:		None
Exceptions:	None
History:	6/6/89, DSJ, Created.
******************************************************************************/
void
WriteParamDesc (FILE * File, uinT16 N, PARAM_DESC ParamDesc[]) {
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
}                                // WriteParamDesc


/** WritePrototype ************************************************************
Parameters:	File	open text file to write prototype to
      N		number of dimensions in feature space
      Proto	prototype to write out
Globals:	None
Operation:	This routine writes a textual description of a prototype
      to the specified text file.
Return:		None
Exceptions:	None
History:	6/12/89, DSJ, Created.
*******************************************************************************/
void WritePrototype(FILE *File, uinT16 N, PROTOTYPE *Proto) {
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
}                                // WritePrototype


/** WriteNFloats ***********************************************************
Parameters:	File	open text file to write N floats to
      N		number of floats to write
      Array	array of floats to write
Globals:	None
Operation:	This routine writes a text representation of N floats from
      an array to a file.  All of the floats are placed on one line.
Return:		None
Exceptions:	None
History:	6/6/89, DSJ, Created.
****************************************************************************/
void WriteNFloats(FILE * File, uinT16 N, FLOAT32 Array[]) {
  for (int i = 0; i < N; i++)
    fprintf(File, " %9.6f", Array[i]);
  fprintf(File, "\n");
}                                // WriteNFloats


/** WriteProtoSyle **********************************************************
Parameters:	File	open text file to write prototype style to
      ProtoStyle	prototype style to write
Globals:	None
Operation:	This routine writes to the specified text file a word
      which represents the ProtoStyle.  It does not append
      a carriage return to the end.
Return:		None
Exceptions:	None
History:	6/8/89, DSJ, Created.
****************************************************************************/
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
}                                // WriteProtoStyle

/*---------------------------------------------------------------------------*/
void WriteProtoList(
     FILE	*File,
     uinT16	N,
     PARAM_DESC	ParamDesc[],
     LIST	ProtoList,
     BOOL8	WriteSigProtos,
     BOOL8	WriteInsigProtos)

/*
**	Parameters:
**		File		open text file to write prototypes to
**		N		number of dimensions in feature space
**		ParamDesc	descriptions for each dimension
**		ProtoList	list of prototypes to be written
**		WriteSigProtos	TRUE to write out significant prototypes
**		WriteInsigProtos	TRUE to write out insignificants
**	Globals:
**		None
**	Operation:
**		This routine writes a textual description of each prototype
**		in the prototype list to the specified file.  It also
**		writes a file header which includes the number of dimensions
**		in feature space and the descriptions for each dimension.
**	Return:
**		None
**	Exceptions:
**		None
**	History:
**		6/12/89, DSJ, Created.
*/

{
  PROTOTYPE	*Proto;

  /* write file header */
  fprintf(File,"%0d\n",N);
  WriteParamDesc(File,N,ParamDesc);

  /* write prototypes */
  iterate(ProtoList)
    {
      Proto = (PROTOTYPE *) first_node ( ProtoList );
      if (( Proto->Significant && WriteSigProtos )	||
	  ( ! Proto->Significant && WriteInsigProtos ) )
	WritePrototype( File, N, Proto );
    }
}	/* WriteProtoList */

