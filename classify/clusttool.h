/******************************************************************************
 **	Filename:	clusttool.h
 **	Purpose:	Definition of clustering utility tools
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
#ifndef   __CLUSTERTOOL__
#define   __CLUSTERTOOL__

//--------------------------Include Files---------------------------------------
#include "host.h"
#include "cluster.h"
#include <stdio.h>

/*-------------------------------------------------------------------------
        Public Funtion Prototype
--------------------------------------------------------------------------*/
uinT16 ReadSampleSize(FILE *File);

PARAM_DESC *ReadParamDesc(FILE *File, uinT16 N);

PROTOTYPE *ReadPrototype(FILE *File, uinT16 N);

PROTOSTYLE ReadProtoStyle(FILE *File);

FLOAT32 *ReadNFloats (FILE * File, uinT16 N, FLOAT32 Buffer[]);

void WriteParamDesc (FILE * File, uinT16 N, PARAM_DESC ParamDesc[]);

void WritePrototype(FILE *File, uinT16 N, PROTOTYPE *Proto);

void WriteNFloats (FILE * File, uinT16 N, FLOAT32 Array[]);

void WriteProtoStyle(FILE *File, PROTOSTYLE ProtoStyle);

void WriteProtoList(
     FILE	*File,
     uinT16	N,
     PARAM_DESC	ParamDesc[],
     LIST	ProtoList,
     BOOL8	WriteSigProtos,
     BOOL8	WriteInsigProtos);

FLOAT32 UniformRandomNumber(FLOAT32 MMin, FLOAT32 MMax);

//--------------Global Data Definitions and Declarations---------------------
// define errors that can be trapped
#define ILLEGALSAMPLESIZE 5000
#define ILLEGALCIRCULARSPEC 5001
#define ILLEGALMINMAXSPEC 5002
#define ILLEGALSIGNIFICANCESPEC 5003
#define ILLEGALSTYLESPEC  5004
#define ILLEGALSAMPLECOUNT  5005
#define ILLEGALMEANSPEC 5006
#define ILLEGALVARIANCESPEC 5007
#define ILLEGALDISTRIBUTION 5008
#define ILLEGALFLOAT  5009
#define ILLEGALESSENTIALSPEC  5013
#endif
