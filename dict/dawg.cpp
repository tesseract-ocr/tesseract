/* -*-C-*-
 ********************************************************************************
 *
 * File:        dawg.c  (Formerly dawg.c)
 * Description:  Use a Directed Accyclic Word Graph
 * Author:       Mark Seaman, OCR Technology
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Wed Jul 24 16:59:16 1991 (Mark Seaman) marks@hpgrlt
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
#include "dawg.h"
#include "cutil.h"
#include "callcpp.h"
#include "context.h"
#include "strngs.h"

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
INT32 debug          = 0;
INT32 case_sensative = 0;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
/**********************************************************************
 * edge_char_of
 *
 * Return the edge that corresponds to the letter out of this node.
 **********************************************************************/
EDGE_REF edge_char_of(EDGE_ARRAY dawg,
                      NODE_REF node,
                      int character,
                      int word_end) {
  EDGE_REF   edge = node;

  if (! case_sensative) character = tolower (character);

  if (edge_occupied (dawg, edge)) {
    do {
      if ((edge_letter (dawg, edge) == character) &&
        (! word_end || end_of_word(dawg,edge)))
        return (edge);

    } edge_loop (dawg, edge);
  }

  return (NO_EDGE);
}


/**********************************************************************
 * edges_in_node
 *
 * Count the number of edges in this node in the DAWG. This includes
 * both forward and back links.
 **********************************************************************/
INT32 edges_in_node(EDGE_ARRAY dawg, NODE_REF node) {
  EDGE_REF   edge = node;

  if (edge_occupied (dawg, edge)) {
    edge_loop(dawg, edge);
    if (edge_occupied (dawg, edge) && backward_edge (dawg, edge)) {
      edge_loop(dawg, edge);
      return (edge - node);
    }
    else {
      return (edge - node);
    }
  }
  else {
    return (edge - node);
  }
}


/**********************************************************************
 * letter_is_okay
 *
 * Check this letter in light of the current state.  If everything is
 * still OK then return TRUE;
 **********************************************************************/
INT32 letter_is_okay(EDGE_ARRAY dawg,
                     NODE_REF *node,
                     INT32 char_index,
                     char prevchar,
                     const char *word,
                     INT32 word_end) {
  EDGE_REF     edge;
  STRING dummy_word(word);  // Auto-deleting string fixes memory leak.

  if (*node == NO_EDGE) {        /* Trailing punctuation */
    if (trailing_punc (dummy_word [char_index])
      && (!trailing_punc (prevchar)
      || punctuation_ok(dummy_word.string())>=0))
      return (TRUE);
    else
      return (FALSE);
  }
  else {
    /* Leading punctuation */
    if (*node == 0                           &&
      char_index != 0                      &&
      isalpha (dummy_word [char_index])          &&
      ! leading_punc (dummy_word [char_index-1]) &&
    dummy_word [char_index-1] != '-') {
      return (FALSE);
    }
  }
  /* Handle compund words */
  if (dummy_word [char_index] == '-') {
    if (char_index>0 && !word_end
      && word [char_index-1] == '-'
      && word [char_index+1] == '-')
      return FALSE;              /*not allowed*/
    dummy_word [char_index] = (char) 0;
    if (word_in_dawg (dawg, dummy_word.string())) {
      dummy_word [char_index] = '-';
      *node = 0;
      return (TRUE);
    }
    else {
      dummy_word [char_index] = '-';
      return (FALSE);
    }
  }
  /* Check the DAWG */
  edge = edge_char_of (dawg, *node, dummy_word [char_index], word_end);

  if (edge != NO_EDGE) {         /* Normal edge in DAWG */
    if (case_sensative || case_is_okay (dummy_word, char_index)) {
                                 //next_node (dawg, edge);
      *node = (dawg)[edge] & NO_EDGE;
      return (TRUE);
    }
    else {
      return (FALSE);
    }
  }
  else {
    /* Leading punctuation */
    if (leading_punc (word [char_index]) &&
    (char_index == 0  ||  leading_punc (dummy_word [char_index-1]))) {
      *node = 0;
      if (leading_punc (prevchar) || punctuation_ok (word)>=0)
        return (TRUE);
      else
        return FALSE;
    }
    /* Trailing punctuation */
    if (verify_trailing_punct (dawg, &dummy_word[0], char_index)) {
      *node = NO_EDGE;
      return (TRUE);
    }

    return (FALSE);
  }
}


/**********************************************************************
 * num_forward_edges
 *
 * Count and return the number of forward edges for this node.
 **********************************************************************/
INT32 num_forward_edges(EDGE_ARRAY dawg, NODE_REF node) {
  EDGE_REF   edge = node;
  INT32        num  = 0;

  if (forward_edge (dawg, edge)) {
    do {
      num++;
    } edge_loop (dawg, edge);
  }

  return (num);
}


/**********************************************************************
 * print_dawg_node
 *
 * Print the contents of one of the nodes in the DAWG.
 **********************************************************************/
void print_dawg_node(EDGE_ARRAY dawg, NODE_REF node) {
  EDGE_REF   edge = node;
  const char       *forward_string  = "FORWARD";
  const char       *backward_string = "       ";

  const char       *last_string     = "LAST";
  const char       *not_last_string = "    ";

  const char       *eow_string      = "EOW";
  const char       *not_eow_string  = "   ";

  const char       *direction;
  const char       *is_last;
  const char       *eow;

  char       ch;

  if (edge_occupied (dawg, edge)) {
    do {
      if (forward_edge (dawg, edge)) direction = forward_string;
      else                           direction = backward_string;

      if (last_edge    (dawg, edge)) is_last   = last_string;
      else                           is_last   = not_last_string;

      if (end_of_word  (dawg, edge)) eow       = eow_string;
      else                           eow       = not_eow_string;

      ch = edge_letter (dawg, edge);
      cprintf ("%7d : next = %7d, char = '%c', %s %s %s\n",
        (int) edge, (int) next_node (dawg, edge), ch,
        direction, is_last, eow);

      if (edge - node > MAX_NODE_EDGES) return;
    } edge_loop (dawg, edge);

    if (edge_occupied (dawg, edge) && backward_edge (dawg, edge)) {
      do {
        if (forward_edge (dawg, edge)) direction = forward_string;
        else                           direction = backward_string;

        if (last_edge    (dawg, edge)) is_last   = last_string;
        else                           is_last   = not_last_string;

        if (end_of_word  (dawg, edge)) eow       = eow_string;
        else                           eow       = not_eow_string;

        ch = edge_letter (dawg, edge);
        cprintf ("%7d : next = %7d, char = '%c', %s %s %s\n",
          (int) edge, (int) next_node (dawg, edge), ch,
          direction, is_last, eow);

        if (edge - node > MAX_NODE_EDGES) return;
      } edge_loop (dawg, edge);
    }
  }
  else {
    cprintf ("%5d : no edges in this node\n", node);
  }
  new_line();
}


/**********************************************************************
 * read_squished_dawg
 *
 * Write the DAWG out to a file
 **********************************************************************/
void read_squished_dawg(char *filename, EDGE_ARRAY dawg, INT32 max_num_edges) {
  FILE       *file;
  EDGE_REF   edge;
  INT32      num_edges;
  INT32      node_count = 0;

  if (debug) print_string ("read_debug");

  clear_all_edges(dawg, edge, max_num_edges);

  #ifdef __UNIX__
  file = open_file (filename, "r");
  #else
  file = open_file (filename, "rb");
  #endif
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);
  fread (&num_edges,  sizeof (int), 1, file);
  // Auto-detect relative endianness of file and OS as future DAWG
  // files may be little-endian.
  long diff1 = sizeof(EDGE_RECORD)*num_edges + sizeof(int) - fsize;
  reverse32(&num_edges);
  long diff2 = sizeof(EDGE_RECORD)*num_edges + sizeof(int) - fsize;
  reverse32(&num_edges);
  // One of diff1 and diff2 should now be 0, but find the smallest
  // just in case.
  if (diff1 < 0) diff1 = -diff1;
  if (diff2 < 0) diff2 = -diff2;
  bool swap = diff2 < diff1;
  if (swap)
    reverse32(&num_edges);
  fread (&dawg[0], sizeof (EDGE_RECORD), num_edges, file);
  fclose(file);
  if (swap)
    for (edge=0;edge<num_edges;edge++)
      reverse32(&dawg[edge]);

  for  (edge=0; edge<max_num_edges; edge++)
    if (last_edge (dawg, edge)) node_count++;
}


/**********************************************************************
 * verify_trailing_punct
 *
 * Make sure that there is a valid transition from the word core to a
 * string of trailing puntuation.  TRUE is returned if everything is
 * OK.
 **********************************************************************/
INT32 verify_trailing_punct(EDGE_ARRAY dawg, char *word, INT32 char_index) {
  char       last_char;
  char       *first_char;

  if (trailing_punc (word [char_index])) {

    last_char = word [char_index];
    word [char_index] = (char) 0;

    for (first_char = word; leading_punc (first_char[0]); first_char++);

    if (word_in_dawg (dawg, first_char)) {
      word [char_index] = last_char;
      return (TRUE);
    }
    word [char_index] = last_char;
  }
  return (FALSE);
}


/**********************************************************************
 * word_in_dawg
 *
 * Test to see if the word can be found in the DAWG.
 **********************************************************************/
INT32 word_in_dawg(EDGE_ARRAY dawg, const char *string) {
  NODE_REF   node = 0;
  INT32        i;
  INT32         length;

  length=strlen(string);
  if (length==0)
    return FALSE;
  for (i=0; i<length; i++) {
    if (debug) {
      print_dawg_node(dawg, node);
      new_line();
    }

    if (! letter_is_okay (dawg, &node, i,'\0', string, (string[i+1]==0))) {
      return (FALSE);
    }
  }

  return (TRUE);
}
