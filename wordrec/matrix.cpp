/* -*-C-*-
 ********************************************************************************
 *
 * File:        matrix.c  (Formerly matrix.c)
 * Description:  Ratings matrix code. (Used by associator)
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 13:18:47 1990
 * Modified:     Wed Mar 20 09:44:47 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "matrix.h"
#include "cutil.h"
#include "freelist.h"
#include "callcpp.h"

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * create_matrix
 *
 * Allocate a piece of memory to hold a matrix of choice list pointers.
 * initialize all the elements of the matrix to NULL.
 **********************************************************************/
MATRIX create_matrix(int dimension) {
  MATRIX m;
  int x;
  int y;

  m = (MATRIX) memalloc ((dimension * dimension + 1) * sizeof (LIST));
  m[0] = (LIST) dimension;
  for (x = 0; x < dimension; x++)
    for (y = 0; y < dimension; y++)
      matrix_put(m, x, y, NOT_CLASSIFIED);
  return (m);
}


/**********************************************************************
 * free_matrix
 *
 * Deallocate the memory taken up by a matrix of match ratings.
 *********************************************************************/
void free_matrix(MATRIX matrix) {
  int x;
  int y;
  int dimension = matrix_dimension (matrix);
  CHOICES matrix_cell;

  for (x = 0; x < dimension; x++) {
    for (y = 0; y < dimension; y++) {
      matrix_cell = matrix_get (matrix, x, y);
      if (matrix_cell != NOT_CLASSIFIED)
        free_choices(matrix_cell);
    }
  }
  memfree(matrix);
}


/**********************************************************************
 * print_matrix
 *
 * Print the best guesses out of the match rating matrix.
 **********************************************************************/
void print_matrix(MATRIX rating_matrix) {
  int x;
  int dimension;
  int spread;
  CHOICES rating;

  cprintf ("Ratings Matrix (top choices)\n");

  dimension = matrix_dimension (rating_matrix);
  /* Do each diagonal */
  for (spread = 0; spread < dimension; spread++) {
    /* For each spot */
    for (x = 0; x < dimension - spread; x++) {
      /* Process one square */
      rating = matrix_get (rating_matrix, x, x + spread);

      if (rating != NOT_CLASSIFIED) {
        cprintf ("\t[%d,%d] : ", x, x + spread);
        if (first_node (rating))
          cprintf ("%-10s%4.0f\t|\t",
            class_string (first_node (rating)),
            class_probability (first_node (rating)));
        if (second_node (rating))
          cprintf ("%-10s%4.0f\t|\t",
            class_string (second_node (rating)),
            class_probability (second_node (rating)));
        if (third (rating))
          cprintf ("%-10s%4.0f\n",
            class_string (third (rating)),
            class_probability (third (rating)));
        else
          new_line();
      }
    }
  }
}
