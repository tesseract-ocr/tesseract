/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

/*
 *  sudoku.h
 *
 *    The L_Sudoku holds all the information of the current state.
 *
 *    The input to sudokuCreate() is a file with any number of lines
 *    starting with '#', followed by 9 lines consisting of 9 numbers
 *    in each line.  These have the known values and use 0 for the unknowns.
 *    Blank lines are ignored.
 *
 *    The @locs array holds the indices of the unknowns, numbered
 *    left-to-right and top-to-bottom from 0 to 80.  The array size
 *    is initialized to @num.  @current is the index into the @locs
 *    array of the current guess: locs[current].
 *
 *    The @state array is used to determine the validity of each guess.
 *    It is of size 81, and is initialized by setting the unknowns to 0
 *    and the knowns to their input values.
 */
struct L_Sudoku
{
    l_int32        num;         /* number of unknowns                     */
    l_int32       *locs;        /* location of unknowns                   */
    l_int32        current;     /* index into @locs of current location   */
    l_int32       *init;        /* initial state, with 0 representing     */
                                /* the unknowns                           */
    l_int32       *state;       /* present state, including inits and     */
                                /* guesses of unknowns up to @current     */
    l_int32        nguess;      /* shows current number of guesses        */
    l_int32        finished;    /* set to 1 when solved                   */
    l_int32        failure;     /* set to 1 if no solution is possible    */
};
typedef struct L_Sudoku  L_SUDOKU;


    /* For printing out array data */
enum {
    L_SUDOKU_INIT = 0,
    L_SUDOKU_STATE = 1
};

#endif /* SUDOKU_H_INCLUDED */


