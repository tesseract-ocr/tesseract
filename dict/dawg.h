/* -*-C-*-
 ********************************************************************************
 *
 * File:        dawg.h  (Formerly dawg.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jun 19 16:50:24 1991 (Mark Seaman) marks@hpgrlt
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
 *********************************************************************************/

#ifndef DAWG_H
#define DAWG_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include <ctype.h>
#include "general.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
/* #define MAX_WERD_LENGTH        (inT32) 40 */
/* #define MAX_NODE_EDGES_DISPLAY (inT32) 100 */
/* #define LAST_FLAG              (inT32) 1 */
/* #define DIRECTION_FLAG         (inT32) 2 */
/* #define WERD_END_FLAG          (inT32) 4 */

/* #define LETTER_START_BIT       0 */
/* #define FLAG_START_BIT         8 */
/* #define NEXT_EDGE_START_BIT    11 */

/* #define NO_EDGE                (inT32) 0x001fffff */

/* #define NEXT_EDGE_MASK         (inT32) 0xfffff800 */
/* #define FLAGS_MASK             (inT32) 0x00000700 */
/* #define LETTER_MASK            (inT32) 0x000000ff */

/* #define REFFORMAT "%d" */

/* typedef uinT32 EDGE_RECORD; */
/* typedef EDGE_RECORD *EDGE_ARRAY; */
/* typedef inT32 EDGE_REF; */
/* typedef inT32 NODE_REF; */

#define MAX_WERD_LENGTH        (inT64) 40
#define MAX_NODE_EDGES_DISPLAY (inT64) 100
#define LAST_FLAG              (inT64) 1
#define DIRECTION_FLAG         (inT64) 2
#define WERD_END_FLAG          (inT64) 4

#define LETTER_START_BIT       0
#define FLAG_START_BIT         8
#define NEXT_EDGE_START_BIT    11

#ifdef __MSW32__
#define NO_EDGE                (inT64) 0x001fffffffffffffi64
#define NEXT_EDGE_MASK         (inT64) 0xfffffffffffff800i64
#define FLAGS_MASK             (inT64) 0x0000000000000700i64
#define LETTER_MASK            (inT64) 0x00000000000000ffi64
#else
#define NO_EDGE                (inT64) 0x001fffffffffffffll
#define NEXT_EDGE_MASK         (inT64) 0xfffffffffffff800ll
#define FLAGS_MASK             (inT64) 0x0000000000000700ll
#define LETTER_MASK            (inT64) 0x00000000000000ffll
#endif

#define MAX_NUM_EDGES_IN_SQUISHED_DAWG_FILE 3000000

#define REFFORMAT "%lld"

typedef uinT64 EDGE_RECORD;
typedef EDGE_RECORD *EDGE_ARRAY;
typedef inT64 EDGE_REF;
typedef inT64 NODE_REF;

/*---------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
extern inT32 case_sensative;
extern inT32 debug;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
* edge_of
*
* Access the edge that is indexed by the requested edge number.
**********************************************************************/

#define edge_of(edges,e)  \
  ((edges)[e])

/**********************************************************************
* print_edge
*
* Print the contents of a single edge entry in the DAWG.
**********************************************************************/

#define print_edge(dawg,edge)                                     \
  printf ("%7d : next = %7d, char = '%c', %s %s %s\n",            \
          edge, next_node (dawg, edge), edge_letter (dawg, edge), \
          (forward_edge (dawg, edge) ? "FORWARD" : "       "),    \
          (last_edge    (dawg, edge) ? "LAST"    : "    "),       \
          (end_of_word  (dawg, edge) ? "EOW"     : ""))           \

/**********************************************************************
 * next_node
 *
 * The next node visited in the DAWG by following this edge.
 **********************************************************************/

#define next_node(edges,e)  \
(((edges)[e] & NEXT_EDGE_MASK) >> NEXT_EDGE_START_BIT)

/**********************************************************************
 * set_next_edge
 *
 * Set the next node link for this edge in the DAWG.
 **********************************************************************/

#define set_next_edge(edges,e,value)               \
((edges)[e] = ((edges)[e] & (~NEXT_EDGE_MASK)) |\
              ((value << NEXT_EDGE_START_BIT) & NEXT_EDGE_MASK))

/**********************************************************************
* empty_edge_spot
*
* Return TRUE if this edge spot in this location is unoccupied.
**********************************************************************/

#define empty_edge_spot(edges,e)  \
  ((edges)[e] == NEXT_EDGE_MASK)

/**********************************************************************
 * set_empty_edge
 *
 * Return TRUE if this edge spot in this location is unoccupied.
 **********************************************************************/

#define set_empty_edge(edges,e)  \
((edges)[e] = NEXT_EDGE_MASK)

/**********************************************************************
 * clear_all_edges
 *
 * Go through all the edges in the DAWG and clear out each one.
 **********************************************************************/

#define clear_all_edges(dawg,edge,max_num_edges) \
for  (edge=0; edge<max_num_edges; edge++)      \
	set_empty_edge (dawg, edge);

/**********************************************************************
 * edge_occupied
 *
 * Return TRUE if this edge spot in this location is occupied.
 **********************************************************************/

#define edge_occupied(edges,e)  \
((edges)[e] != NEXT_EDGE_MASK)

/**********************************************************************
* edge_flags
*
* The letter choice that corresponds to this edge in the DAWG.
**********************************************************************/

#define edge_flags(edges,e)  \
  (((edges)[e] & FLAGS_MASK) >> FLAG_START_BIT)

/**********************************************************************
 * edge_letter
 *
 * The letter choice that corresponds to this edge in the DAWG.
 **********************************************************************/

#define edge_letter(edges,e)  \
  (static_cast<int>(((edges)[e] & LETTER_MASK) >> LETTER_START_BIT))

/**********************************************************************
* letter_of_edge
*
* The letter choice that corresponds to this edge in the DAWG.
**********************************************************************/

#define letter_of_edge(edge)  \
  (static_cast<int>((edge & LETTER_MASK) >> LETTER_START_BIT))

/**********************************************************************
 * last_edge
 *
 * Return TRUE if this edge is the last edge in the sequence.  This is
 * TRUE for the last one in both the forward and backward part.
 **********************************************************************/

#define last_edge(edges,e)  \
((edges)[e] & (LAST_FLAG << FLAG_START_BIT))

/**********************************************************************
 * end_of_word
 *
 * Return TRUE if this edge marks the end of a word.
 **********************************************************************/

#define end_of_word(edges,e)  \
((edges)[e] & (WERD_END_FLAG << FLAG_START_BIT))

/**********************************************************************
 * forward_edge
 *
 * Return TRUE if this edge is in the forward direction.
 **********************************************************************/

#define forward_edge(edges,e)  \
((edges)[e] & (DIRECTION_FLAG << FLAG_START_BIT) && \
	edge_occupied (edges,e))

/**********************************************************************
 * backward_edge
 *
 * Return TRUE if this edge is in the backward direction.
 **********************************************************************/

#define backward_edge(edges,e)  \
(! ((edges)[e] & (DIRECTION_FLAG << FLAG_START_BIT)) && \
	edge_occupied (edges,e))

/**********************************************************************
 * edge_loop
 *
 * Loop for each of the edges in the forward direction.  This macro
 * can be used in the following way:
 *********************************************************************/

#define edge_loop(edges,e)  \
while (! last_edge (edges,e++))

/**********************************************************************
 * case_is_okay
 *
 * Check the case of this character in the character string to make
 * sure that there is not a problem with the case.
 **********************************************************************/
// TODO(tkielbus) Replace islalpha, islower & isupper by unicode versions.
// However the lengths information is not available at this point in the
// code. We will probably get rid of the dictionaries at some point anyway.
#define case_is_okay(word,i)                                \
(i ?                                                      \
	((isupper(word[i]) && islower(word[i-1])) ?              \
	FALSE :                                                 \
	((islower(word[i]) && isupper(word[i-1]) &&             \
		i>1 && isalpha (word[i-2])) ?                       \
	FALSE :                                                \
	TRUE)) :                                               \
	TRUE)

/**********************************************************************
 * trailing_punc
 *
 * Check for leading punctuation.
 **********************************************************************/

#define trailing_punc(ch) \
((ch == '}'  ) ||       \
	(ch == ':'  ) ||       \
	(ch == ';'  ) ||       \
	(ch == '-'  ) ||       \
	(ch == ']'  ) ||       \
	(ch == '!'  ) ||       \
	(ch == '?'  ) ||       \
	(ch == '`'  ) ||       \
	(ch == ','  ) ||       \
	(ch == '.'  ) ||       \
	(ch == ')'  ) ||       \
	(ch == '\"' ) ||       \
	(ch == '\'' ))

/**********************************************************************
 * leading_punc
 *
 * Check for leading punctuation.
 **********************************************************************/

#define leading_punc(ch)  \
((ch == '\"' ) ||       \
	(ch == '('  ) ||       \
	(ch == '{'  ) ||       \
	(ch == '['  ) ||       \
	(ch == '`'  ) ||       \
	(ch == '\'' ))

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
EDGE_REF edge_char_of(EDGE_ARRAY dawg,
                      NODE_REF node,
                      int character,
                      int word_end);

inT32 edges_in_node(EDGE_ARRAY dawg, NODE_REF node);


inT32 def_letter_is_okay(EDGE_ARRAY dawg,
                     NODE_REF *node,
                     inT32 char_index,
                     char prevchar,
                     const char *word,
                     inT32 word_end);

/*
 * Allow for externally provided letter_is_okay.
 */
typedef inT32 (*LETTER_OK_FUNC)(EDGE_ARRAY, NODE_REF*, inT32, char, const char*,
                                inT32);
extern LETTER_OK_FUNC letter_is_okay;


inT32 num_forward_edges(EDGE_ARRAY dawg, NODE_REF node);

void print_dawg_node(EDGE_ARRAY dawg, NODE_REF node);

EDGE_ARRAY read_squished_dawg(const char *filename);

inT32 verify_trailing_punct(EDGE_ARRAY dawg, char *word, inT32 char_index);

inT32 word_in_dawg(EDGE_ARRAY dawg, const char *string);

/*
#if defined(__STDC__) || defined(__cplusplus) || MAC_OR_DOS
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* dawg.c
EDGE_REF edge_char_of
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node,
  int character,
  int word_end));

inT32 edges_in_node
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node));

inT32 def_letter_is_okay
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF *node,
  inT32 char_index,
  char *word,
  inT32 word_end));

inT32 num_forward_edges
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node));

void print_dawg_node
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node));

void read_squished_dawg
  _ARGS((char *filename,
  EDGE_ARRAY dawg,
  inT32 max_num_edges));

inT32 verify_trailing_punct
  _ARGS((EDGE_ARRAY dawg,
  char *word,
  inT32 char_index));

inT32 word_in_dawg
  _ARGS((EDGE_ARRAY dawg,
  char *string));

#undef _ARGS
*/
#endif
