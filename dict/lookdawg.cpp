/* -*-C-*-
********************************************************************************
*
* File:         lookdawg.cpp
* Description:  Look up words in a Directed Accyclic Word Graph
* Author:       Mark Seaman, OCR Technology
* Created:      Fri Oct 16 14:37:00 1987
* Modified:     Thu Jul 25 17:09:55 1991 (Mark Seaman) marks@hpgrlt
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

#include "lookdawg.h"

#include "cutil.h"
#include "trie.h"
#ifdef __UNIX__
#include <assert.h>
#endif

/*
----------------------------------------------------------------------
                     V a r i a b l e s
----------------------------------------------------------------------
*/

/*
----------------------------------------------------------------------
                     F u n c t i o n s
----------------------------------------------------------------------
*/

/**********************************************************************
* check_for_words
*
* Check the DAWG for the words that are listed in the requested file.
* A file name of NULL will cause the words to be read from stdin.
**********************************************************************/

void check_for_words (EDGE_ARRAY dawg,
                      char       *filename) {
  FILE       *word_file;
  char       string [CHARS_PER_LINE];
  int misses = 0;

  word_file = open_file (filename, "r");

  if (filename == NULL) {
    printf ("? ");
    fflush (stdout);
  }

  while (fgets (string, CHARS_PER_LINE, word_file) != NULL) {
    string [strlen (string) - 1] = (char) 0;

    if (strlen (string)) {
      if (debug) {
        debug=0;
        if (! word_in_dawg (dawg, string)) {
          puts (string);
          if (filename == NULL) {
            debug = 1;
            word_in_dawg (dawg, string);
          }
        }
        debug = 1;
      }
      else {
        if (!match_words (dawg, string, 0, 0))
          ++misses;
      }
    }

    if (filename == NULL) {
      printf ("? ");
      fflush (stdout);
    }
  }
  fclose (word_file);
  // Make sure the user sees this with fprintf instead of tprintf.
  fprintf(stderr, "Number of lost words=%d\n", misses);
}

#if 0
/**********************************************************************
* main
*
* Test the DAWG functions.
**********************************************************************/

int main (argc, argv)
   int  argc;
   char **argv;
{
   inT32       max_num_edges  = 700000;
   EDGE_ARRAY  dawg;
   int         argnum = 1;
   int         show_nodes = FALSE;

   dawg = (EDGE_ARRAY) malloc (sizeof (EDGE_RECORD) * max_num_edges);
   if (dawg == NULL) {
      printf ("error: Could not allocate enough memory for DAWG  ");
      printf ("(%ld,%03ld bytes needed)\n",
	      sizeof (EDGE_RECORD) * max_num_edges / 1000,
	      sizeof (EDGE_RECORD) * max_num_edges % 1000);
      exit (1);
   }

   if (! strcmp (argv[argnum], "-v")) {
      show_nodes = TRUE;
      argnum++;
   }

   if (strcmp  (argv[argnum], "-f")) {
      read_squished_dawg  (argv[argnum++], dawg, max_num_edges);
   }
   else {
      argnum++;
      read_full_dawg  (argv[argnum++], dawg, max_num_edges);
   }

   printf ("argc = %d\n", argc);
   print_int ("argnum", argnum);
   print_string (argv[argnum]);

   if (argc < argnum + 1) {
      printf ("Type in words to search for:          (use * for wildcard)\n");
      debug = show_nodes;
      check_for_words (dawg, NULL);
      new_line ();
   }
   else {
      print_lost_words (dawg, argv[argnum]);
   }
}
#endif

/**********************************************************************
* match_words
*
* Match all of the words that are specified with this string.  The *'s
* in this string are wildcards.
**********************************************************************/

bool match_words (EDGE_ARRAY  dawg,
                  char        *string,
                  inT32         index,
                  NODE_REF    node) {
  EDGE_REF   edge;
  inT32        word_end;

  if (string[index] == '*') {
    bool any_matched = false;
    edge = node;
    do {
      string[index] = edge_letter (dawg, edge);
      if (match_words (dawg, string, index, node))
        any_matched = true;
    } edge_loop (dawg, edge);
    string[index] = '*';
    return any_matched;
  }
  else {
    word_end = (string[index+1] == (char) 0);
    edge = edge_char_of(dawg, node,
                        static_cast<unsigned char>(string[index]), word_end);
    if (edge != NO_EDGE) {                         /* Normal edge in DAWG */
      node = next_node (dawg, edge);
      if (word_end) {
        printf ("%s\n", string);
        return true;
      }
      else if (node != 0) {
        return match_words (dawg, string, index+1, node);
      }
    }
  }
  return false;
}


/**********************************************************************
* print_lost_words
*
* Check the DAWG for the words that are listed in the requested file.
* A file name of NULL will cause the words to be read from stdin. Print
* each of the words that can not be found in the DAWG.
**********************************************************************/

void print_lost_words (EDGE_ARRAY dawg,
                       char       *filename) {
  FILE       *word_file;
  char       string [CHARS_PER_LINE];

  word_file = open_file (filename, "r");

  while (fgets (string, CHARS_PER_LINE, word_file) != NULL) {
    string [strlen (string) - 1] = (char) 0;

    if (strlen (string)) {
      if (! word_in_dawg (dawg, string)) {
        puts (string);
      }
    }
  }
  fclose (word_file);
}
