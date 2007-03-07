/******************************************************************************
 **	Filename:    mfdefs.c
 **	Purpose:     Basic routines for manipulating micro-features
 **	Author:      Dan Johnson
 **	History:     Mon Jan 22 08:48:58 1990, DSJ, Created.
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
#include "mfdefs.h"
#include "emalloc.h"
#include <math.h>

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
MICROFEATURE NewMicroFeature() { 
/*
 **	Parameters: none
 **	Globals: none
 **	Operation:
 **		This routine allocates and returns a new micro-feature
 **		data structure.
 **	Return: New micro-feature.
 **	Exceptions: none
 **	History: 7/27/89, DSJ, Created.
 */
  return ((MICROFEATURE) Emalloc (sizeof (MFBLOCK)));
}                                /* NewMicroFeature */


/*---------------------------------------------------------------------------*/
void FreeMicroFeatures(MICROFEATURES MicroFeatures) { 
/*
 **	Parameters:
 **		MicroFeatures	list of micro-features to be freed
 **	Globals: none
 **	Operation:
 **		This routine deallocates all of the memory consumed by
 **		a list of micro-features.
 **	Return: none
 **	Exceptions: none
 **	History: 7/27/89, DSJ, Created.
 */
  destroy_nodes(MicroFeatures, Efree); 
}                                /* FreeMicroFeatures */
