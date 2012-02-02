/* -*-C-*-
 ********************************************************************************
 *
 * File:        states.c  (Formerly states.c)
 * Description:  Representations of search states
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 16 15:49:34 1990
 * Modified:     Mon Jun 17 17:54:41 1991 (Mark Seaman) marks@hpgrlt
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
#include "states.h"
#include "structures.h"
#include "callcpp.h"

/*-------------------------------------------------------------------------
            Variables
--------------------------------------------------------------------------*/
makestructure(newstate, free_state, STATE);

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**
 * @name bin_to_chunks
 *
 * Convert a representation of the search state in "STATE" form to one
 * in "SEARCH_STATE" form.  Create the memory required to hold the
 * resultant state value.
 *
 * @param state The state to convert
 */
SEARCH_STATE bin_to_chunks(STATE *state, int num_joints) {
  int x;
  unsigned int mask;
  int depth;
  int pieces = 0;
  SEARCH_STATE s;

  s = memalloc (sizeof (int) * (ones_in_state (state, num_joints) + 1));

  depth = 1;
  mask = 1 << (num_joints - 1 - 32);
  for (x = num_joints; x > 32; x--) {
    if (state->part1 & mask) {
      s[depth++] = pieces;
      pieces = 0;
    }
    else {
      pieces++;
    }
    mask >>= 1;
  }

  if (num_joints > 32)
    mask = 1 << 31;
  else
    mask = 1 << (num_joints - 1);

  while (x--) {
    if (state->part2 & mask) {
      s[depth++] = pieces;
      pieces = 0;
    }
    else {
      pieces++;
    }
    mask >>= 1;
  }
  s[0] = depth - 1;

  return (s);
}


/**
 * bin_to_pieces
 *
 * Convert the binary (bit vector) format of a search state to an array
 * of piece counts. This array has a zero element after the last valid
 * character.
 */
void bin_to_pieces(STATE *state, int num_joints, PIECES_STATE pieces) {
  int x;
  unsigned int mask;             /* Bit mask */
  inT16 num_pieces = 0;
  /* Preset mask */
  mask = ((num_joints > 32) ?
    (1 << (num_joints - 1 - 32)) : (1 << (num_joints - 1)));

  pieces[num_pieces] = 0;

  for (x = num_joints - 1; x >= 0; x--) {
                                 /* Iterate all bits */
    pieces[num_pieces]++;

    if ((x < 32) ?               /* Test for 1 bit */
      ((state->part2 & mask) ? TRUE : FALSE) :
    ((state->part1 & mask) ? TRUE : FALSE)) {
      pieces[++num_pieces] = 0;
    }
    /* Next mask value */
    mask = ((mask == 1) ? (1 << 31) : (mask >> 1));
  }
  pieces[num_pieces]++;
  pieces[++num_pieces] = 0;
  ASSERT_HOST (num_pieces < MAX_NUM_CHUNKS + 2);
}


/**
 * insert_new_chunk
 *
 * Add a new chunk division into this state vector at the location
 * requested.
 */
void insert_new_chunk(register STATE *state,
                      register int index,
                      register int num_joints) {
  register unsigned int mask;
  register unsigned int result;

  index = (num_joints - index);
  if (index < 32) {
    mask = ~0;
    mask <<= index;
    result = (mask & state->part2) << 1;
    result |= (~mask & state->part2);
    state->part1 <<= 1;
    if (state->part2 & 0x80000000)
      state->part1 |= 1;
    state->part2 = result;
  }
  else {
    mask = ~0;
    mask <<= index - 32;
    result = (mask & state->part1) << 1;
    result |= (~mask & state->part1);
    state->part1 = result;
  }
}


/**
 * new_state
 *
 * Create a memory space for a new state variable.  Set its initial
 * value according to the parameters.
 */
STATE *new_state(STATE *oldstate) {
  STATE *this_state;

  this_state = newstate ();
  this_state->part1 = oldstate->part1;
  this_state->part2 = oldstate->part2;
  return (this_state);
}


/**
 * ones_in_state
 *
 * Return the number of ones that are in this state.
 */
int ones_in_state(STATE *state, int num_joints) {
  inT8 num_ones = 0;
  inT8 x;
  unsigned int mask;

  if (num_joints > 32)           /* Preset mask */
    mask = 1 << (num_joints - 1 - 32);
  else
    mask = 1 << (num_joints - 1);

  for (x = num_joints - 1; x >= 0; x--) {
                                 /* Iterate all bits */

    if (x < 32)
      num_ones += ((state->part2 & mask) ? 1 : 0);
    else
      num_ones += ((state->part1 & mask) ? 1 : 0);

    if (mask == 1)               /* Next mask value */
      mask = 1 << 31;
    else
      mask >>= 1;
  }

  return (num_ones);
}


/**
 * print_state
 *
 * Print out the current state variable on a line with a label.
 */
void print_state(const char *label, STATE *state, int num_joints) {
  int x;
  unsigned int mask;             /* Bit mask */

  if (num_joints > 32)           /* Preset mask */
    mask = 1 << (num_joints - 1 - 32);
  else
    mask = 1 << (num_joints - 1);

  cprintf ("%s ", label);

  for (x = num_joints - 1; x >= 0; x--) {
                                 /* Iterate all bits */

    if (x < 32)
      cprintf ("%d", ((state->part2 & mask) ? 1 : 0));
    else
      cprintf ("%d", ((state->part1 & mask) ? 1 : 0));
    if (x % 4 == 0)
      cprintf (" ");

    if (mask == 1)               /* Next mask value */
      mask = 1 << 31;
    else
      mask >>= 1;
  }

  new_line();
}

// Prints out the number of fragments in each segment in a state to
// toappend.
void print_state(STATE *state, int num_joints, STRING *toappend) {
  PIECES_STATE pieces;
  bin_to_pieces(state, num_joints, pieces);
  for (int i = 0; pieces[i] > 0; i++) {
   if (i > 0) {
     toappend->add_str_int(" ", pieces[i]);
   } else {
     toappend->add_str_int("", pieces[i]);
   }
  }
}

/**
 * set_n_ones
 *
 * Set the first n bits in a state.
 */
void set_n_ones(STATE *state, int n) {
  if (n < 32) {
    state->part2 = ~0;
    state->part2 >>= 32 - n;
    state->part1 = 0;
  }
  else {
    state->part2 = ~0;
    state->part1 = ~0;
    state->part1 >>= 64 - n;
  }
}
