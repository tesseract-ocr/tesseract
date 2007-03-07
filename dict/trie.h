/* -*-C-*-
 ********************************************************************************
 *
 * File:        trie.h  (Formerly trie.h)
 * Description:  Functions to build a trie data structure.
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Jul 26 11:26:34 1991 (Mark Seaman) marks@hpgrlt
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
#ifndef TRIE_H
#define TRIE_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "dawg.h"
#include "cutil.h"

/*----------------------------------------------------------------------
              T y p e s
----------------------------------------------------------------------*/
#define NUM_PLACEMENT_ATTEMPTS (INT32) 100
#define EDGE_NUM_MARGIN        (INT32) 2
#define DEFAULT_NODE_SIZE      (INT32) 2
#define FORWARD_EDGE           (INT32) 0
#define BACKWARD_EDGE          (INT32) 1

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
//?extern INT32 max_new_attempts;

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * link_edge
 *
 * Set up this edge record to the requested values.
 **********************************************************************/

#define link_edge(edges,e,nxt,ch,flgs)                \
(edges[e] = ((INT32) (nxt)                        | \
					((INT32) (ch)   << LETTER_START_BIT) | \
					((INT32) (flgs) << FLAG_START_BIT)))

/**********************************************************************
 * set_last_flag
 *
 * Set up this edge record to be the last one in a sequence of edges.
 **********************************************************************/

#define set_last_flag(edges,e)   \
(edges[e] |= (LAST_FLAG << FLAG_START_BIT))

/**********************************************************************
 * copy_edge
 *
 * Move the contents of a single of edge from one place in the dawg to
 * another.
 **********************************************************************/

#define copy_edge(dawg,from,to)              \
	dawg[to] = dawg[from]

/**********************************************************************
 * move_edges
 *
 * Move the location of a set of edges from one place in the dawg to
 * another.  There can be no overlap between 'from' and 'to'.
 **********************************************************************/

#define move_edges(dawg,from,to,num)              \
{                                               \
	INT32 i;                                     \
	for (i=0; i<num; i++) {                      \
		copy_edge(dawg,from+i,to+i);              \
		dawg[from+i] = NO_EDGE;                   \
	}                                            \
}                                               \


/**********************************************************************
 * copy_edges
 *
 * Copy the location of a set of edges from one place in the dawg to
 * another.  The copy is carried out so that the 'from' and 'to' spaces
 * can overlap, as long as:
 *     from < to
 **********************************************************************/

#define copy_edges(dawg,from,to,num)            \
{                                             \
	INT32 i;                                   \
	for (i=num-1; i>=0; i--) {                 \
		copy_edge(dawg,from+i,to+i);            \
	}                                          \
}                                             \


/**********************************************************************
 * move_node_if_needed
 *
 * Move the node location if there is a need to do it because there is
 * not enough room to add the new edges.
 **********************************************************************/

#define move_node_if_needed(dawg,node,max_num_edges,reserved_edges)     \
if (! room_in_node (dawg, node)) {                                    \
	node = move_node (dawg, node, max_num_edges, reserved_edges);      \
}                                                                     \


/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
void add_edge_linkage(EDGE_ARRAY dawg,
                      NODE_REF node1,
                      NODE_REF node2,
                      INT32 direction,
                      char character,
                      INT32 word_end);

void add_new_edge(EDGE_ARRAY dawg,
                  NODE_REF *node1,
                  NODE_REF *node2,
                  char character,
                  INT32 word_end,
                  INT32 max_num_edges,
                  INT32 reserved_edges);

void add_word_to_dawg(EDGE_ARRAY dawg,
                      char *string,
                      INT32 max_num_edges,
                      INT32 reserved_edges);

void initialize_dawg(EDGE_ARRAY dawg, INT32 max_num_edges); 

NODE_REF move_node(EDGE_ARRAY dawg,
                   NODE_REF node,
                   INT32 max_num_edges,
                   INT32 reserved_edges);

NODE_REF new_dawg_node(EDGE_ARRAY dawg,
                       INT32 num_edges,
                       INT32 max_num_edges,
                       INT32 reserved_edges);

void read_word_list(char *filename,
                    EDGE_ARRAY dawg,
                    INT32 max_num_edges,
                    INT32 reserved_edges);

void relocate_edge(EDGE_ARRAY dawg,
                   NODE_REF node,
                   NODE_REF old_node,
                   NODE_REF new_node);

void remove_edge(EDGE_ARRAY dawg,
                 NODE_REF node1,
                 NODE_REF node2,
                 char character,
                 INT32 word_end);

void remove_edge_linkage(EDGE_ARRAY dawg,
                         NODE_REF node,
                         NODE_REF next,
                         INT32 direction,
                         char character,
                         INT32 word_end);

INT32 room_in_node(EDGE_ARRAY dawg, NODE_REF node); 

/*
#if defined(__STDC__) || defined(__cplusplus)
# define	_ARGS(s) s
#else
# define	_ARGS(s) ()
#endif*/

/* trie.c *
void add_edge_linkage
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node1,
  NODE_REF node2,
  INT32 direction,
  int character,
  INT32 word_end));

void add_new_edge
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF *node1,
  NODE_REF *node2,
  int character,
  INT32 word_end,
  INT32 max_num_edges,
  INT32 reserved_edges));

void add_word_to_dawg
  _ARGS((EDGE_ARRAY dawg,
  char *string,
  INT32 max_num_edges,
  INT32 reserved_edges));

void initialize_dawg
  _ARGS((EDGE_ARRAY dawg,
  INT32 max_num_edges));

NODE_REF move_node
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node,
  INT32 max_num_edges,
  INT32 reserved_edges));

NODE_REF new_dawg_node
  _ARGS((EDGE_ARRAY dawg,
  INT32 num_edges,
  INT32 max_num_edges,
  INT32 reserved_edges));

void read_word_list
  _ARGS((char *filename,
  EDGE_ARRAY dawg,
  INT32 max_num_edges,
  INT32 reserved_edges));

void relocate_edge
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node,
  NODE_REF old_node,
  NODE_REF new_node));

void remove_edge
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node1,
  NODE_REF node2,
  int character,
  INT32 word_end));

void remove_edge_linkage
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node,
  NODE_REF next,
  INT32 direction,
  int character,
  INT32 word_end));

INT32 room_in_node
  _ARGS((EDGE_ARRAY dawg,
  NODE_REF node));

#undef _ARGS
*/
#endif
