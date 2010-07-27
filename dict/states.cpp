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
#include "tordvars.h"
#include "callcpp.h"

/*-------------------------------------------------------------------------
            Variables
--------------------------------------------------------------------------*/
#define STATEBLOCK 100           /* Cells per block */
makestructure (newstate, free_state, printstate, STATE,
freestate, STATEBLOCK, "STATE", statecount);

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
  if (tord_debug_8)
    print_state ("bin_to_pieces = ", state, num_joints);

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
      if (tord_debug_8)
        cprintf ("[%d]=%d ", num_pieces - 1, pieces[num_pieces - 1]);
    }
    /* Next mask value */
    mask = ((mask == 1) ? (1 << 31) : (mask >> 1));
  }
  pieces[num_pieces]++;
  pieces[++num_pieces] = 0;
  ASSERT_HOST (num_pieces < MAX_NUM_CHUNKS + 2);
  if (tord_debug_8)
    new_line();
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


/**
 * compare_states
 *
 * Compare the 2 states at the given blob index. Return 1 if the given
 * blob is a fragment compared to reality, 2 if correct, 4 if a join,
 * and 5 if both a join and a fragment.
 * On return the blob index is set to the corresponding index in the
 * correct string.
 */
int compare_states(STATE *true_state, STATE *this_state, int *blob_index) {
  int blob_count;                //number found
  int true_index;                //index of true blob
  int index;                     //current
  int result = 0;                //return value
  uinT32 mask;

  if (true_state->part1 == this_state->part1
    && true_state->part2 == this_state->part2)
    return 2;
  if (*blob_index == 0) {
    if (bits_in_states > 32) {
      for (mask = 1 << (bits_in_states - 33); mask != 0; mask >>= 1) {
        if (this_state->part1 & mask) {
          if (true_state->part1 & mask)
            return 2;
          else
            return 1;
        }
        else if (true_state->part1 & mask)
          return 4;
      }
      index = 31;
    }
    else
      index = bits_in_states - 1;
    for (mask = 1 << index; mask != 0; mask >>= 1) {
      if (this_state->part2 & mask) {
        if (true_state->part2 & mask)
          return 2;
        else
          return 1;
      }
      else if (true_state->part2 & mask)
        return 4;
    }
    return 2;
  }
  else {
    blob_count = 0;
    true_index = 0;
    if (bits_in_states > 32) {
      for (mask = 1 << (bits_in_states - 33); mask != 0; mask >>= 1) {
        if (true_state->part1 & mask)
          true_index++;
        if (this_state->part1 & mask) {
          blob_count++;
          if (blob_count == *blob_index) {
            if ((true_state->part1 & mask) == 0)
              result = 1;
            break;
          }
        }
      }
      if (blob_count == *blob_index) {
        for (mask >>= 1; mask != 0; mask >>= 1) {
          if (this_state->part1 & mask) {
            if ((true_state->part1 & mask) && result == 0)
              return 2;
            else
              return result | 1;
          }
          else if (true_state->part1 & mask)
            result |= 4;
        }
      }
      index = 31;
    }
    else
      index = bits_in_states - 1;
    mask = 1 << index;
    if (blob_count < *blob_index) {
      for (; mask != 0; mask >>= 1) {
        if (true_state->part2 & mask)
          true_index++;
        if (this_state->part2 & mask) {
          blob_count++;
          if (blob_count == *blob_index) {
            if ((true_state->part2 & mask) == 0)
              result = 1;
            break;
          }
        }
      }
      if (blob_count != *blob_index)
        return 2;
      mask >>= 1;
    }
    *blob_index = true_index;
    for (; mask != 0; mask >>= 1) {
      if (this_state->part2 & mask) {
        if ((true_state->part2 & mask) && result == 0)
          return 2;
        else
          return result | 1;
      }
      else if (true_state->part2 & mask)
        result |= 4;
    }
    return result == 0 ? 2 : result;
  }
}
