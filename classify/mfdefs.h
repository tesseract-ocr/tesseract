/******************************************************************************
 **	Filename:    mfdefs.h
 **	Purpose:     Definition of micro-features
 **	Author:      Dan Johnson
 **	History:     Mon Jan 22 08:42:13 1990, DSJ, Created.
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
#ifndef   MFDEFS_H
#define   MFDEFS_H

/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "oldlist.h"
#include "matchdefs.h"
#include "xform2d.h"

/* maximum size of a bulge for length=1 is sqrt(2)/3 */
#define BULGENORMALIZER   0.942809041

/* definition of a list of micro-features */
typedef LIST MICROFEATURES;

/* definition of structure of micro-features */
#define MFSIZE        6
typedef FLOAT32 MFBLOCK[MFSIZE];
typedef FLOAT32 *MICROFEATURE;

/* definitions of individual micro-feature parameters */
#define XPOSITION     0
#define YPOSITION     1
#define MFLENGTH      2
#define ORIENTATION     3
#define FIRSTBULGE      4
#define SECONDBULGE     5

/**----------------------------------------------------------------------------
            Macros
----------------------------------------------------------------------------**/

/* macros for accessing micro-feature lists */
#define NextFeatureOf(L)  ( (MICROFEATURE) first_node ( L ) )

/**----------------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------------**/
MICROFEATURE NewMicroFeature();

void FreeMicroFeatures(MICROFEATURES MicroFeatures);
#endif
