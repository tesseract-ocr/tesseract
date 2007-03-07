/******************************************************************************
 **	Filename:    mfvars.c
 **	Purpose:     Hooks global microfeature variables into the wo system.
 **	Author:      Dan Johnson
 **	History:     Fri Jan 12 12:47:20 1990, DSJ, Created.
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
#include "blobclass.h"
#include "extract.h"
#include "adaptmatch.h"

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void mfeature_variables() { 
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: Install global variables into the wiseowl variable system.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Jan 12 13:17:07 1990, DSJ, Created.
 */
  InitBlobClassifierVars(); 
  InitExtractorVars(); 
}                                /* mfeature_variables */


/*---------------------------------------------------------------------------*/
void mfeature_init() { 
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: none
 **	Return: none
 **	Exceptions: none
 **	History: Fri Jan 12 13:22:41 1990, DSJ, Created.
 */
  InitAdaptiveClassifier(); 
}                                /* mfeature_init */
