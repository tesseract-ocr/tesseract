/* -*-C-*-
 ********************************************************************************
 *
 * File:        choicearr.h  (Formerly choicearr.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Mar 19 15:27:49 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 ********************************************************************************
 */

#ifndef CHOICEARR_H
#define CHOICEARR_H

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include "tessarray.h"
#include "choices.h"

/*
----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------
*/

typedef ARRAY CHOICES_LIST;

#define CHOICES_PER_LIST 40

/*
----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------
*/

/**********************************************************************
 * free_choice_list
 *
 * Free a list of choices. Free the array structure but not each of the
 * sublists of choices.
 **********************************************************************/

#define free_choice_list(choice_list)  \
array_free (choice_list)

/**********************************************************************
 * for_each_choice
 *
 * Iterate through each of the possible choices.
 **********************************************************************/

#define for_each_choice(array,index)  \
array_loop (array, index)

/**********************************************************************
 * free_all_choices
 *
 * Free an array of choices (deep free).
 **********************************************************************/

#define free_all_choices(choices,index)                      \
for_each_choice (choices, index) {                         \
	free_choices ((CHOICES) array_value (choices, index));  \
}                                                          \
array_free (choices)                                       \


/**********************************************************************
 * new_choice_list
 *
 * Return a new array structure that is a list of choices. Each set of
 * choices will be of type CHOICES.
 **********************************************************************/

#define new_choice_list()  \
array_new (CHOICES_PER_LIST)
#endif
