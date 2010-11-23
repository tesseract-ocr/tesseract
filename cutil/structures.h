/* -*-C-*-
 ********************************************************************************
 *
 * File:        structures.h  (Formerly structures.h)
 * Description:  Allocate all the different types of structures.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 30 10:12:12 1990
 * Modified:     Tue May 21 11:07:47 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 *********************************************************************************/
#ifndef STRUCTURES_H
#define STRUCTURES_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "oldlist.h"
#include "freelist.h"
#include "danerror.h"

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * makestructure
 *
 * Allocate a chunk of memory for a particular data type.  This macro
 * defines an allocation, deallocation, and status printing function
 * for each new data type.
 **********************************************************************/

#define makestructure(newfunc, old, type)                \
type *newfunc()                                                                  \
{                                                                            \
	return new type; \
}                                                                            \
																									\
																									\
																									\
void old(type* deadelement)                                                       \
{                                                                            \
	delete deadelement; \
}                                                                            \

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
extern LIST new_cell();
extern void free_cell(LIST);
#endif
