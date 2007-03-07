/* -*-C-*-
 ********************************************************************************
 *
 * File:        matrix.h  (Formerly matrix.h)
 * Description:  Ratings matrix code. (Used by associator)
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 13:22:06 1990
 * Modified:     Tue Mar 19 16:00:20 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef MATRIX_H
#define MATRIX_H

#include "oldlist.h"
#include "choices.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
typedef LIST *MATRIX;            /* Matrix of LIST */
#define NOT_CLASSIFIED (CHOICES) -1

/*----------------------------------------------------------------------
            Macros
----------------------------------------------------------------------*/
/**********************************************************************
 * matrix_dimension
 *
 * Provide the dimension of this square matrix.
 **********************************************************************/

#define matrix_dimension(matrix)  \
((long) matrix [0])

/**********************************************************************
 * matrix_index
 *
 * Expression to select a specific location in the matrix.
 **********************************************************************/

#define matrix_index(matrix,column,row)  \
((row) * matrix_dimension(matrix) + (column) + 1)

/**********************************************************************
 * matrix_put
 *
 * Put a list element into the matrix at a specific location.
 **********************************************************************/

#define matrix_put(matrix,column,row,thing)  \
((matrix) [matrix_index ((matrix), (column), (row))] = (thing))

/**********************************************************************
 * matrix_get
 *
 * Get the item at a specified location from the matrix.
 **********************************************************************/

#define matrix_get(matrix,column,row)  \
((matrix) [matrix_index ((matrix), (column), (row))])

/*---------------------------------------------------------------------
          Public Function Prototypes
----------------------------------------------------------------------*/
MATRIX create_matrix(int dimension); 

void free_matrix(MATRIX matrix); 

void print_matrix(MATRIX rating_matrix); 

/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* matrix.c
MATRIX create_matrix
    _ARGS((int dimension));

MATRIX free_matrix
    _ARGS((MATRIX matrix));

void print_matrix
    _ARGS((MATRIX rating_matrix));

#undef _ARGS
*/
#endif
