/* -*-C-*-
********************************************************************************
*
* File:         reduce.cpp
* Description:  Functions to reduce a TRIE into a DAWG
* Author:       Mark Seaman, OCR Technology
* Created:      Fri Oct 16 14:37:00 1987
* Modified:     Wed Jun 19 16:51:29 1991 (Mark Seaman) marks@hpgrlt
* Language:     C
* Package:      N/A
* Status:       Reusable Software Component
*
* (c) Copyright 1987, Hewlett-Packard Company, all rights reserved.
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


/*
----------------------------------------------------------------------
                     I n c l u d e s
----------------------------------------------------------------------
*/

#include "reduce.h"

#include "makedawg.h"
#include "cutil.h"

#ifdef __UNIX__
#include <assert.h>
#endif

/*
----------------------------------------------------------------------
                     T y p e s
----------------------------------------------------------------------
*/



/*
----------------------------------------------------------------------
                     V a r i a b l e s
----------------------------------------------------------------------
*/

static inT32 debug_1 = 0;

/*
----------------------------------------------------------------------
                     M a c r o s
----------------------------------------------------------------------
*/


/*
----------------------------------------------------------------------
                     F u n c t i o n s
----------------------------------------------------------------------
*/

/**********************************************************************
* collapse_source_nodes
*
* A pair of edges has been found that can be reduced.  This function
* accomplishes that reduction by collapsing the two nodes into a
* single node.
**********************************************************************/

void collapse_source_nodes (EDGE_ARRAY dawg,
                            NODE_REF   source_node_1,
                            NODE_REF   source_node_2,
                            NODE_REF   dest_node,
                            inT32        max_num_edges,
                            inT32        reserved_edges) {
  inT32      num_links;
  EDGE_REF   edge;
  /*    NODE_REF   new_source_1; */

  num_links = num_forward_edges (dawg, source_node_2);

  /*   if (debug_1)
       printf ("Node = %d, Input 1 = %d, Input 2 = %6d, num_links = %d\n",
       dest_node, source_node_1, source_node_2, num_links);

       if (debug) {
       printf ("Node = %d, Input 1 = %d, Input 2 = %6d, num_links = %d\n",
       dest_node, source_node_1, source_node_2, num_links);
       print_dawg_node (dawg, source_node_1);
       print_dawg_node (dawg, source_node_2);
       new_line ();
       }
  */
  /* Remove forward links in   */
  edge = source_node_2;                       /* source_node_2 - dest_node */
  if (forward_edge (dawg, edge)) {
    do {
      remove_edge_linkage (dawg, dest_node, source_node_2,
                           BACKWARD_EDGE,
                           edge_letter (dawg, edge),
                           end_of_word (dawg, edge));
    } edge_loop (dawg, edge);
  }
  /* Fix backward links */
  edge = source_node_2;                          /* in source_node_2 */
  edge += num_forward_edges (dawg, source_node_2);
  if (backward_edge (dawg, edge)) {
    do {
      move_node_if_needed (dawg, &source_node_1,
                           max_num_edges, reserved_edges);

      add_edge_linkage (dawg, source_node_1, next_node (dawg, edge),
                        BACKWARD_EDGE,
                        edge_letter (dawg, edge),
                        end_of_word (dawg, edge));
      /* Node moved */
      relocate_edge (dawg, next_node (dawg, edge),
                     source_node_2, source_node_1);
    } edge_loop (dawg, edge);
  }
  delete_node (dawg, source_node_2);

  /*   if (debug) {
       printf ("Number of edges  source = %d, dest = %d\n",
       edges_in_node (dawg, new_source_1),
       edges_in_node (dawg, dest_node));
       print_int ("Number of edges = ", edges_in_node (dawg, dest_node));
       print_dawg_node (dawg, new_source_1);

       print_string ("_________________________________________");
       new_line ();
       }
  */
}


/**********************************************************************
* eliminate_redundant_edges
*
* Compare these two edges in this node to see if they point to two
* nodes that could be collapsed.  If they do, then perform the
* reduction and return TRUE.  If not, return FALSE.
**********************************************************************/

inT32 eliminate_redundant_edges (EDGE_ARRAY dawg,
                                 NODE_REF   node,
                                 EDGE_REF   edge_1,
                                 EDGE_REF   edge_2,
                                 inT32        max_num_edges,
                                 inT32        reserved_edges) {
  static inT32 elim_count = 0;
  static inT32 keep_count = 0;

  if (same_output (dawg,
                   next_node (dawg, edge_1),
                   next_node (dawg, edge_2))) {
    elim_count++;

    collapse_source_nodes (dawg,
                           next_node (dawg, edge_1),
                           next_node (dawg, edge_2),
                           node,
                           max_num_edges, reserved_edges);
    /*      if (debug_1) {
            printf ("Collapsing node %d\n", node);
            print_dawg_node (dawg, node);
            printf ("Candidate edges = %d, %d\n", edge_1, edge_2);
            printf ("Candidate nodes = %d, %d\n\n",
            next_node (dawg, edge_1), next_node (dawg, edge_2));
            new_line ();
            }
    */
    return (TRUE);
  }
  else {
    keep_count++;
  }
  return (FALSE);
}


/**********************************************************************
* letter_order
*
* Compare two edges to see which one of the letters is larger.
**********************************************************************/

inT32 letter_order (const void* edge1_ptr,
                    const void* edge2_ptr) {

  if (letter_of_edge(*((EDGE_RECORD*) edge1_ptr)) <
      letter_of_edge(*((EDGE_RECORD*) edge2_ptr)))
    return (-1);

  if (letter_of_edge(*((EDGE_RECORD*) edge1_ptr)) >
      letter_of_edge(*((EDGE_RECORD*) edge2_ptr)))
    return (1);

  return (0);
}


/*
  printf ("%c%c   %c%c  ",
  edge1.letter, (edge1.flags & WORD_END_FLAG ? '*' : ' '),
  edge2.letter, (edge2.flags & WORD_END_FLAG ? '*' : ' '));
  printf ("\n");
  printf ("+\n");
  printf ("-\n");
*/

void print_n_edges (EDGE_RECORD *edge1,
                    inT32         n) {
  EDGE_RECORD *edge;

  edge = edge1;
  while (n-- > 0) {
    printf ("%c ", letter_of_edge(edge[0]));
    edge++;
  }

  new_line ();
}

/**********************************************************************
* reduce_lettered_edges
*
* The edge parameter is pointing to the first edge in a group of edges
* in this node with a particular letter value.  Look through these edges
* to see if any of them can be collapsed.  If so do it.  When all edges
* with this letter have been reduced then return to the caller.
* If further reduction is possible with this same letter then the
* edge parameter is not incremented.  When no further reduction is
* possible then FALSE is returned.
**********************************************************************/

inT32 reduce_lettered_edges (EDGE_ARRAY  dawg,
                             EDGE_REF    *edge,
                             NODE_REF    node,
                             NODE_MARKER reduced_nodes,
                             inT32        max_num_edges,
                             inT32        reserved_edges) {
  EDGE_REF    edge_1;
  EDGE_REF    edge_2;
  inT32         fixed_one;
  inT32         did_something = FALSE;

  if (debug_1)
    printf ("reduce_lettered_edges (edge=" REFFORMAT ")\n", *edge);

  /* Loop for each back edge */
  edge_1 = *edge;
  while ((! last_edge (dawg, edge_1)) &&
         edge_letter (dawg, edge_1) == edge_letter (dawg, *edge)) {

    edge_2 = edge_1 + 1;                        /* Compare all back edges */
    do {

      if (edge_letter (dawg, edge_1) <  edge_letter (dawg, edge_2))
        break;

      if (debug_1) {
        printf (REFFORMAT " (%c), " REFFORMAT " (%c) ",
                edge_1, edge_letter (dawg, edge_1),
                edge_2, edge_letter (dawg, edge_2));
      }

      if (edge_2 != edge_1 &&
          edge_letter (dawg, edge_2) == edge_letter (dawg, edge_1) &&
          end_of_word (dawg, edge_2) == end_of_word (dawg, edge_1) &&
          eliminate_redundant_edges (dawg, node, edge_1, edge_2,
                                     max_num_edges, reserved_edges)) {
        reduced_nodes [next_node (dawg, edge_1)] = 0;
        fixed_one     = TRUE;
        did_something = TRUE;
      }
      else {
        if (debug_1) printf ("   .");
        fixed_one = FALSE;
      }
      if (debug_1) printf ("\n");

    } while (fixed_one ?
             edge_occupied (dawg, edge_2) :
             (! last_edge (dawg, edge_2++)));
    edge_1++;
  }

  if (! did_something) {
    if (last_edge (dawg, edge_1))
      return (FALSE);
    else
      *edge = edge_1;
  }
  return (TRUE);
}


/**********************************************************************
* reduce_node_input
*
* Eliminate any redundant edges from this node in the DAWG.
**********************************************************************/

void reduce_node_input (EDGE_ARRAY dawg,
                        NODE_REF   node,
                        NODE_MARKER reduced_nodes,
                        inT32        max_num_edges,
                        inT32        reserved_edges) {
  EDGE_REF   edge_1;
  inT32        forward_edges  = num_forward_edges (dawg, node);
  inT32        backward_edges = edges_in_node (dawg, node) - forward_edges;

  static inT32 num_nodes_reduced = 0;

  if (debug_1) {
    printf ("reduce_node_input (node=" REFFORMAT ")\n", node);
    print_dawg_node (dawg, node);
  }

  if (++num_nodes_reduced % 100 == 0) {
    printf ("%d nodes reduced\n", num_nodes_reduced);
    if (debug_1 && num_nodes_reduced % 1000 == 0) {
      write_full_dawg ("temp-save", dawg, max_num_edges);
    }
  }

  qsort ((void *) &edge_of (dawg, node + forward_edges),
         backward_edges,
         sizeof (EDGE_RECORD),
         letter_order);

  /*   if (debug_1) {
       printf ("__________________________\n");
       print_dawg_node (dawg, node);
       }
  */
  edge_1 = node + forward_edges;
  while (reduce_lettered_edges (dawg, &edge_1, node, reduced_nodes,
                                max_num_edges, reserved_edges));

  reduced_nodes [node] = 1;                      /* Mark as reduced */

  if (debug_1) {
    printf ("After reduction:\n");
    print_dawg_node (dawg, node);
  }

  edge_1 = node + num_forward_edges (dawg, node); /* Reduce next level */
  if (backward_edge (dawg, edge_1)) {
    do {
      if (next_node (dawg, edge_1) &&
          reduced_nodes [next_node (dawg, edge_1)] == 0)
        reduce_node_input (dawg, next_node (dawg, edge_1), reduced_nodes,
                           max_num_edges, reserved_edges);
    } edge_loop (dawg, edge_1);
  }
}


/**********************************************************************
* same_output
*
* Check to see if these two nodes have identical output.  If so then
* they can be collapsed into a single node.
**********************************************************************/

inT32 same_output (EDGE_ARRAY dawg,
                   NODE_REF   node1,
                   NODE_REF   node2) {
  if (debug_1) printf ("Edge nodes = " REFFORMAT " , " \
                     REFFORMAT " \n", node1, node2);

  if (num_forward_edges (dawg, node1) == 1  &&
      num_forward_edges (dawg, node2) == 1) {
    if (debug_1) printf ("   * ");
    return (TRUE);
  }
  else {
    if (debug_1) {
      printf ("   %d,%d \n",
              num_forward_edges (dawg, node1),
              num_forward_edges (dawg, node2));
      print_dawg_node (dawg, node1);
      print_dawg_node (dawg, node2);
    }

    return (FALSE);
  }
}


/**********************************************************************
* trie_to_dawg
*
* Change a Trie data structure into a DAWG by eliminating the redund
**********************************************************************/

void trie_to_dawg (EDGE_ARRAY dawg,
                   inT32        max_num_edges,
                   inT32        reserved_edges) {
  NODE_MARKER reduced_nodes;
  inT32         x;

  max_new_attempts = 100000;
  compact_dawg (dawg, max_num_edges, reserved_edges);

  reduced_nodes = (NODE_MARKER) malloc (max_num_edges);
  for (x=0; x<max_num_edges; x++) reduced_nodes [x] = 0;

  reduce_node_input (dawg, 0, reduced_nodes, max_num_edges, reserved_edges);

  free (reduced_nodes);
}
