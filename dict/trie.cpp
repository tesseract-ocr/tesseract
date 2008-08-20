/* -*-C-*-
 ********************************************************************************
 *
 * File:        trie.c  (Formerly trie.c)
 * Description:  Functions to build a trie data structure.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Fri Jul 26 12:18:10 1991 (Mark Seaman) marks@hpgrlt
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
/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include "trie.h"
#include "callcpp.h"

#ifdef __UNIX__
#include <assert.h>
#endif
#include <stdio.h>

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
static inT32 move_counter = 0;
static inT32 new_counter  = 0;
static inT32 edge_counter = 0;

inT32 max_new_attempts = 0;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * add_edge_linkage
 *
 * Add a single edge linkage to between the two nodes.  This function
 * can be used to add either forward or backward links.
 **********************************************************************/
void add_edge_linkage(EDGE_ARRAY dawg,
                      NODE_REF node1,
                      NODE_REF node2,
                      EDGE_RECORD direction,
                      int character,
                      EDGE_RECORD word_end) {
  EDGE_REF edge1 = node1;
  EDGE_REF edge2;
  inT32      num_edges = edges_in_node (dawg, node1);
  inT64      last_one;

  word_end  = (word_end ? WERD_END_FLAG : 0);

  if (num_edges == 0) {          /* No edges yet */
    direction = ((direction == FORWARD_EDGE) ? DIRECTION_FLAG : 0);
    link_edge  (dawg, edge1, node2,  character,
      LAST_FLAG | direction | word_end);
  }
  else {
                                 /* Forward links */
    if (direction == FORWARD_EDGE) {
      last_one = (forward_edge (dawg, edge1) ? 0 : LAST_FLAG);
      if (debug)
        cprintf ("moving edges (nodes = " REFFORMAT ", " REFFORMAT \
                 ", num = %d)\n",
          edge1, edge1+1, num_edges);
      copy_edges (dawg, edge1, edge1+1, num_edges);
      link_edge  (dawg, edge1, node2,  character,
        last_one | DIRECTION_FLAG | word_end);
    }
    else {                       /* Backward links */

      if (forward_edge (dawg, edge1))
        edge_loop(dawg, edge1);

                                 /* Existing back edges */
      if (backward_edge (dawg,edge1)) {
        num_edges = 0;
        edge2 = edge1;
        do { num_edges++; }
        edge_loop(dawg, edge2);

        if (debug)
          cprintf ("moving edges (nodes = " REFFORMAT ", " REFFORMAT \
                   ", num = %d)\n",
            edge1, edge1+1, num_edges);
        copy_edges (dawg, edge1, edge1+1, num_edges);
        link_edge(dawg, edge1, node2, character, word_end);
      }
      else {                     /* First back edge */
        link_edge  (dawg, edge1, node2,  character,
          LAST_FLAG | word_end);
      }
    }
  }
}


/**********************************************************************
 * add_new_edge
 *
 * Add an edge between two nodes in the DAWG.  Link the nodes both ways.
 **********************************************************************/
bool add_new_edge(EDGE_ARRAY dawg,
                  NODE_REF *node1,
                  NODE_REF *node2,
                  int character,
                  EDGE_RECORD word_end,
                  inT32 max_num_edges,
                  inT32 reserved_edges) {
  int direction;

  if (debug)
    cprintf ("add_new_edge (nodes = " REFFORMAT ", " REFFORMAT \
             ", ch = '%c', end = %d)\n",
      *node1, *node2,  character, word_end);
  edge_counter++;

  if (!move_node_if_needed(dawg, node1, max_num_edges, reserved_edges))
    return false;
  if (!move_node_if_needed(dawg, node2, max_num_edges, reserved_edges))
    return false;

  direction = (int) FORWARD_EDGE;
  add_edge_linkage(dawg, *node1, *node2, direction, character, word_end);

  direction = (int) BACKWARD_EDGE;
  add_edge_linkage(dawg, *node2, *node1, direction, character, word_end);
  return true;
}

/**********************************************************************
 * add_word_ending
 *
 * Set the word ending flags in an already existing edge pair.
 * Return true on success.
 **********************************************************************/
bool add_word_ending(EDGE_ARRAY dawg,
                     EDGE_REF edge,
                     NODE_REF the_next_node,
                     int ch) {
  EDGE_REF other_edge = the_next_node;
  // Find the backward link from the_next_node back with ch.
  if (forward_edge(dawg, other_edge))
    edge_loop(dawg, other_edge);
  if (backward_edge(dawg, other_edge)) {
    other_edge = edge_char_of(dawg, other_edge, ch, false);
    if (other_edge != NO_EDGE) {
      // Mark both directions as end of word.
      dawg[edge] |= (WERD_END_FLAG << FLAG_START_BIT);
      dawg[other_edge] |= (WERD_END_FLAG << FLAG_START_BIT);
      return true;  // Success.
    }
  }
  return false;  // Failed.
}

/**********************************************************************
 * add_word_to_dawg
 *
 * Add in a word by creating the necessary nodes and edges.
 **********************************************************************/
void add_word_to_dawg(EDGE_ARRAY dawg,
                      const char *string,
                      inT32 max_num_edges,
                      inT32 reserved_edges) {
  EDGE_REF    edge;
  NODE_REF    last_node = 0;
  NODE_REF    the_next_node;
  inT32         i;
  inT32         still_finding_chars = TRUE;
  inT32         word_end = FALSE;
  bool          add_failed = false;

  if (debug) cprintf("Adding word %s\n", string);
  for (i=0; i<strlen(string)-1; i++) {
    unsigned char ch = case_sensative ? string[i] : tolower(string[i]);
    if (still_finding_chars) {
      edge = edge_char_of(dawg, last_node, ch, word_end);
      if (debug) cprintf ("exploring edge = " REFFORMAT "\n", edge);
      if (edge == NO_EDGE)
        still_finding_chars = FALSE;
      else
      if (next_node (dawg, edge) == 0) {
        word_end = TRUE;
        still_finding_chars = FALSE;
        remove_edge (dawg, last_node, 0, ch, word_end);
      }
      else {
        last_node = next_node (dawg, edge);
      }
    }

    if (! still_finding_chars) {
      the_next_node = new_dawg_node (dawg, DEFAULT_NODE_SIZE,
                                     max_num_edges, reserved_edges);
      if (the_next_node == 0) {
        add_failed = true;
        break;
      }
      if (edges_in_node (dawg, last_node) + last_node == the_next_node) {
        //cprintf ("Node collision at %d\n", the_next_node);
        the_next_node = new_dawg_node (dawg, DEFAULT_NODE_SIZE,
                                       max_num_edges, reserved_edges);
        if (the_next_node == 0) {
          add_failed = true;
          break;
        }
      }
      if (!add_new_edge (dawg, &last_node, &the_next_node, ch,
                         word_end, max_num_edges, reserved_edges)) {
        add_failed = true;
        break;
      }
      word_end = FALSE;
      if (debug)
        cprintf ("adding node = %ld\n", the_next_node);
      last_node = the_next_node;
    }
  }

  the_next_node = 0;
  unsigned char ch = case_sensative ? string[i] : tolower(string[i]);
  if (still_finding_chars &&
      (edge = edge_char_of(dawg, last_node, ch, false))!= NO_EDGE &&
      (the_next_node = next_node(dawg, edge)) != 0) {
    // An extension of this word already exists in the trie, so we
    // only have to add the ending flags in both directions.
    if (!add_word_ending(dawg, edge, the_next_node, ch))
      cprintf("Unable to find backward edge for subword ending! %s\n", string);
  } else {
    if (!add_failed &&
        !add_new_edge(dawg, &last_node, &the_next_node, ch,
                      TRUE, max_num_edges, reserved_edges))
      add_failed = true;
  }

  if (edges_in_node (dawg, 0) > reserved_edges) {
    cprintf ("error: Not enough room in root node, %d\n",
      edges_in_node (dawg, 0));
    add_failed = true;
  }
  if (add_failed) {
    cprintf ("Re-initializing document dictionary...\n");
    initialize_dawg(dawg,max_num_edges);
  }
}


/**********************************************************************
 * initialize_dawg
 *
 * Initialize the DAWG data structure for further used.  Reset each of
 * the edge cells to NO_EDGE.
 **********************************************************************/
void initialize_dawg(EDGE_ARRAY dawg, inT32 max_num_edges) {
  inT32 x;


   //changed by jetsoft
  // these values were not getting changes.

  move_counter = 0;
  new_counter  = 0;
  edge_counter = 0;
  // end jetsoft

  clear_all_edges(dawg, x, max_num_edges);
}


/**********************************************************************
 * move_node_if_needed
 *
 * Move the node location if there is a need to do it because there is
 * not enough room to add the new edges.
 **********************************************************************/
bool move_node_if_needed(EDGE_ARRAY dawg,
                         NODE_REF* node,
                         inT32 max_num_edges,
                         inT32 reserved_edges) {
  if (room_in_node(dawg, *node))
    return true;

  NODE_REF   this_new_node;
  EDGE_REF   edge;
  inT32      num_edges = edges_in_node (dawg, *node);

  if (debug)
    print_dawg_node(dawg, *node);

  this_new_node = new_dawg_node (dawg, num_edges + EDGE_NUM_MARGIN,
                                 max_num_edges, reserved_edges);
  if (this_new_node == 0)
    return false;

  if (debug)
    cprintf ("move_node  (from = " REFFORMAT ", to = " REFFORMAT
             ", num = %d)\n",
      *node, this_new_node, num_edges);

  move_edges(dawg, *node, this_new_node, num_edges);

  if (debug)
    print_dawg_node(dawg, this_new_node);

  edge = this_new_node;
  if (forward_edge (dawg, edge)) {
    do {
      relocate_edge (dawg, next_node (dawg, edge), *node, this_new_node);
    } edge_loop (dawg, edge);
  }
  if (backward_edge (dawg, edge)) {
    do {
      relocate_edge (dawg, next_node (dawg, edge), *node, this_new_node);
    } edge_loop (dawg, edge);
  }

  move_counter++;
  *node = this_new_node;
  return true;
}


/**********************************************************************
 * new_dawg_node
 *
 * Create a space within the DAWG data structure to house a node that
 * consists of the requested number of edges.
 **********************************************************************/
NODE_REF new_dawg_node(EDGE_ARRAY dawg,
                       inT32 num_edges,
                       inT32 max_num_edges,
                       inT32 reserved_edges) {
  inT32        i;
  inT32        n;
  inT32        edge_index;
  inT32        edge_collision;

  /* Try several times */
  for (i=0; i<NUM_PLACEMENT_ATTEMPTS; i++) {

    edge_index = reserved_edges +
        long_rand (max_num_edges - MAX_WERD_LENGTH - reserved_edges);
    edge_collision = FALSE;
    /* Find N adjacent slots */
    for  (n=0; n<num_edges && !edge_collision; n++) {
      if (edge_occupied (dawg, edge_index + n))
        edge_collision = TRUE;
    }

    if (! edge_collision) {
      new_counter++;
      //?			max_new_attempts = max (max_new_attempts, i);
      return (edge_index);
    }
  }

  cprintf ("DAWG Table is too full, nodes = %d, edges = %d, moves %d\n",
           new_counter, edge_counter, move_counter);
  return 0;
}

/**********************************************************************
* print_dawg_map
*
* Print the contents of one of the nodes in the DAWG.
**********************************************************************/
void print_dawg_map (EDGE_ARRAY dawg, inT32 max_num_edges) {
   EDGE_REF edge = 0;
   inT32 counter = 0;

   do {
      if (edge_occupied (dawg, edge))
        printf (".");
      else
        printf (" ");
      if (counter % 100 == 0) fflush (stdout);
   } while (edge++ < max_num_edges);
}


/**********************************************************************
* read_full_dawg
*
* Read this DAWG in from a file.  The nodes in this DAWG contain both
* forward and backward edges.  There are gaps in between the nodes.
**********************************************************************/
void read_full_dawg (const char *filename,
                     EDGE_ARRAY dawg,
                     inT32 max_num_edges) {
   FILE       *file;
   EDGE_REF   node_index;
   inT32      num_edges;
   inT32      node_count;
   inT32      error_occured = FALSE;

   if (debug) print_string ("read_dawg");

   clear_all_edges (dawg, node_index, max_num_edges);

   file = open_file (filename, "rb");

   fread (&node_count, sizeof (inT32), 1, file);

   while (node_count-- > 0) {

      fread (&node_index, sizeof (EDGE_REF), 1, file);
      fread (&num_edges,  sizeof (inT32),    1, file);

      assert (node_index + num_edges < max_num_edges);
      fread (&dawg[node_index], sizeof (EDGE_RECORD), num_edges, file);

      if (debug > 2) {
         print_dawg_node (dawg, node_index);
         new_line ();
      }

   }
   fclose (file);
   if (error_occured) exit (1);
}


/**********************************************************************
 * read_word_list
 *
 * Read the requested file (containing a list of words) and add all
 * the words to the DAWG.
 **********************************************************************/
void read_word_list(const char *filename,
                    EDGE_ARRAY dawg,
                    inT32 max_num_edges,
                    inT32 reserved_edges) {
  FILE *word_file;
  char string [CHARS_PER_LINE];
  int  word_count = 0;
  int old_debug = debug;
  if (debug > 0 && debug < 3)
    debug = 0;

  word_file = open_file (filename, "r");

  initialize_dawg(dawg, max_num_edges);

  while (fgets (string, CHARS_PER_LINE, word_file) != NULL) {
    string [strlen (string) - 1] = (char) 0;
    ++word_count;
    if (debug && word_count % 10000 == 0)
      cprintf("Read %d words so far\n", word_count);
    if (string[0] != '\0' /* strlen (string) */) {
      if (!word_in_dawg(dawg, string)) {
        add_word_to_dawg(dawg, string, max_num_edges, reserved_edges);
        if (!word_in_dawg(dawg, string)) {
          cprintf("error: word not in DAWG after adding it '%s'\n", string);
          return;
        }
      }
    }
  }
  debug = old_debug;
  if (debug)
    cprintf("Read %d words total.\n", word_count);
  fclose(word_file);
}


/**********************************************************************
 * relocate_edge
 *
 * The location of a node has moved, so an edge entry in another node
 * must be changed.  Change the value of this edge entry to match the
 * new location of the node.
 **********************************************************************/
void relocate_edge(EDGE_ARRAY dawg,
                   NODE_REF node,
                   NODE_REF old_node,
                   NODE_REF new_node) {
  EDGE_REF   edge;

  if (debug) cprintf ("relocate (" REFFORMAT ", " REFFORMAT " ==> " \
                      REFFORMAT ")\n", node, old_node, new_node);

  edge = node;
  if (forward_edge (dawg, edge)) {
    do {
      if (next_node (dawg, edge) == old_node) {
        if (debug)
          cprintf ("forward assign (" REFFORMAT ", " REFFORMAT " ==> " \
                   REFFORMAT ")\n", edge, old_node, new_node);

        set_next_edge(dawg, edge, new_node);
      }
    } edge_loop (dawg, edge);
  }

  if (backward_edge (dawg, edge)) {
    do {
      if (next_node (dawg, edge) == old_node) {
        if (debug)
          cprintf ("backward assign (" REFFORMAT ", " REFFORMAT " ==> " \
                   REFFORMAT ")\n", edge, old_node, new_node);

        set_next_edge(dawg, edge, new_node);
      }
    }
    edge_loop(dawg, edge);
  }
}


/**********************************************************************
 * remove_edge
 *
 * Add a single edge linkage to between the two nodes.  This function
 * can be used to add either forward or backward links.
 **********************************************************************/
void remove_edge(EDGE_ARRAY dawg,
                 NODE_REF node1,
                 NODE_REF node2,
                 int character,
                 EDGE_RECORD word_end) {
  remove_edge_linkage(dawg, node1, node2, FORWARD_EDGE, character, word_end);

  remove_edge_linkage(dawg, node2, node1, BACKWARD_EDGE, character, word_end);
}


/**********************************************************************
 * remove_edge_linkage
 *
 * Remove this edge reference from this node.  Move the edge entries in
 * this node to fill the gap.
 **********************************************************************/
void remove_edge_linkage(EDGE_ARRAY dawg,
                         NODE_REF node,
                         NODE_REF next,
                         EDGE_RECORD direction,
                         int character,
                         EDGE_RECORD word_end) {
  inT32      forward_edges;
  inT32      num_edges;
  NODE_REF   e = node;
  inT32      last_flag;

  forward_edges = num_forward_edges (dawg, node);
  num_edges = edges_in_node (dawg, node);

  for (e=node; e<node+num_edges; e++) {
    /* Is this the edge*/
    if ((edge_letter (dawg, e) == character) &&
      ((direction == FORWARD_EDGE) ?
      forward_edge(dawg,e) : backward_edge(dawg,e)) &&
      (next_node (dawg, e) == next) &&
    (word_end ? end_of_word(dawg,e) : (! end_of_word(dawg,e)))) {

      if (debug)
        cprintf ("remove (edge = " REFFORMAT ", character is '%c')\n",
          e, edge_letter (dawg, e));

      /* Delete the slot */
      last_flag = last_edge (dawg, e);
      set_empty_edge(dawg, e);
      move_edges (dawg, e+1, e, num_edges+node-e-1);
      /* Restore 'last' flag */
      if (direction == FORWARD_EDGE) {
        if ((forward_edges - 1) &&
        (forward_edges - 1 == e - node)) {
          set_last_flag (dawg, e - 1);
        }
      }
      else {
        if ((num_edges - forward_edges - 1) &&
        (num_edges - 1 == e - node)) {
          set_last_flag (dawg, e - 1);
        }
      }
      if (debug)
        print_dawg_node(dawg, node);
      return;
    }
  }
  cprintf ("error: Could not find the edge to remove, %d char = '%c'\n",
           next, character);
  print_dawg_node(dawg, node);
  exit (1);
}


/**********************************************************************
 * room_in_node
 *
 * Check to see if there is enough room left in this node for one more
 * edge link.  This may be a forward or backward link.
 **********************************************************************/
inT32 room_in_node(EDGE_ARRAY dawg, NODE_REF node) {
  EDGE_REF   edge = node;

  if (edge_occupied (dawg, edge + edges_in_node (dawg, node))) {
    return (FALSE);
  }
  else {
    return (TRUE);
  }
}


/**********************************************************************
* write_full_dawg
*
* Write the DAWG out to a file
**********************************************************************/
void write_full_dawg (const char *filename, EDGE_ARRAY dawg,
                      inT32 max_num_edges) {
   FILE       *file;
   EDGE_REF   edge;
   inT32      num_edges;
   inT32      node_count = 0;
   NODE_REF   node;

   if (debug) print_string ("write_full_dawg");

   for (edge=0; edge<max_num_edges; edge++) {
                                                  /* Count nodes links */
      if (forward_edge (dawg, edge)) {
         node_count++;
         if (forward_edge  (dawg, edge))  edge_loop (dawg, edge);
         if (backward_edge (dawg, edge))  edge_loop (dawg, edge);
         edge--;
      }
   }

   file = open_file (filename, "wb");
   fwrite (&node_count, sizeof (inT32), 1, file);

   node_count = 0;
   for (edge=0; edge<max_num_edges; edge++) {
                                                  /* Write all edges */
      if (forward_edge (dawg, edge)) {
         node =  edge;
         num_edges = edges_in_node (dawg, edge);

         assert ((node + num_edges < max_num_edges) && (num_edges > 0));

         fwrite (&edge,                sizeof (EDGE_REF),    1,         file);
	 fwrite (&num_edges,           sizeof (inT32),       1,         file);
	 fwrite (&edge_of (dawg,edge), sizeof (EDGE_RECORD), num_edges, file);

         node_count++;

         if (debug) {
           printf ("Writing node index=" REFFORMAT
                   ", node_count=%d, edges=%d\n",
                   node, node_count, num_edges);

           for (edge = node; edge < node + num_edges; edge++)  {
              printf ("Writing index=" REFFORMAT ", old_node=" REFFORMAT \
                      ", letter=%c, flags=%d\n",
                      node, next_node (dawg,edge),
                      (char) edge_letter(dawg,edge),
                      (char) edge_flags (dawg,edge));
           }
         }

         edge = node + num_edges - 1;
      }
   }

   fclose (file);
}
