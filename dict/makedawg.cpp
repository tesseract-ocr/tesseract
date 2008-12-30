/* -*-C-*-
********************************************************************************
*
* File:         makedawg.cpp
* Description:  Create a Directed Accyclic Word Graph
* Author:       Mark Seaman, OCR Technology
* Created:      Fri Oct 16 14:37:00 1987
* Modified:     Fri Jul 26 12:18:12 1991 (Mark Seaman) marks@hpgrlt
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
#ifdef __MSW32__
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

#include "makedawg.h"

#include "reduce.h"
#include "cutil.h"
#include "callcpp.h"

#ifdef __UNIX__
#include <assert.h>
#endif
#include <time.h>


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
* build_node_map
*
* Create a node map that will help translate the indices of the DAWG
* into a compacted form.
* Construct in memory a mapping from the memory node values into the
* disk node values.  Return the values in this map as requested.  If
* a new value mapping is requested assign the next sequential number
* to it.
**********************************************************************/

NODE_MAP build_node_map (EDGE_ARRAY dawg,
                         inT32       *num_nodes,
                         inT32       both_links,
                         inT32       max_num_edges,
                         inT32       reserved_edges) {
  EDGE_REF   edge;
  NODE_MAP   node_map;
  inT32       node_counter;
  inT32       num_edges;

  node_map = (NODE_MAP) malloc (sizeof (EDGE_REF) * max_num_edges);

  for (edge=0; edge<max_num_edges; edge++)       /* Init all slots */
    node_map [edge] = -1;

  if (both_links)                                /* Start after reserved */
    node_counter  = reserved_edges;
  else
    node_counter  = num_forward_edges (dawg, 0);

  *num_nodes   = 0;
  for (edge=0; edge<max_num_edges; edge++) {     /* Search all slots */

    if (forward_edge (dawg, edge)) {
      (*num_nodes)++;                          /* Count nodes links */

      node_map [edge] = (edge ? node_counter : 0);

      if (both_links)
        num_edges = edges_in_node     (dawg, edge);
      else
        num_edges = num_forward_edges (dawg, edge);

      if (edge != 0) node_counter += num_edges;
      edge += num_edges;
      if (backward_edge (dawg, edge))  edge_loop (dawg, edge);
      edge--;
    }
  }
  return (node_map);
}


/**********************************************************************
* compact_dawg
*
* Compact the DAWG (array of edges) to leave a large chunk of blank
* space at the end.
**********************************************************************/

void compact_dawg (EDGE_ARRAY dawg,
                   inT32       max_num_edges,
                   inT32       reserved_edges) {
  EDGE_REF   edge;
  inT32      num_edges = 0;
  NODE_REF   next_node_space;
  NODE_REF   node = 0;
  NODE_REF   destination;
  inT32      node_count;
  NODE_MAP   node_map;
  NODE_REF   the_next_node;

  if (max_new_attempts < NUM_PLACEMENT_ATTEMPTS / 10) return;
  max_new_attempts = 0;

  print_string ("Compacting the DAWG");
  node_map = build_node_map (dawg, &node_count, TRUE,
                             max_num_edges, reserved_edges);

  edge = 0;
  next_node_space = reserved_edges;
  while (edge < max_num_edges) {
    /* Found a node ? */
    if (forward_edge (dawg, edge)) {
      node = edge;
      num_edges  = edges_in_node (dawg, node);
      /* Move the edges */
      if (node != 0) {
        destination = next_node_space;
        if (node != next_node_space)
          move_edges (dawg, node, next_node_space, num_edges);
      }
      else {
        destination = 0;
      }
      /* Should be moved */
      if (debug) cprintf ("Compacting node from " REFFORMAT " to " REFFORMAT \
                          " (%d)\n", node, destination, num_edges);

      for (edge = destination;
           edge < destination + num_edges;
           edge++) {

        the_next_node = next_node (dawg, edge);

        assert (the_next_node >= 0            &&
                the_next_node < max_num_edges &&
                node_map [the_next_node] >= 0 &&
                node_map [the_next_node] < max_num_edges);

        /* Map each edge in node */
        if (debug) cprintf ("   " REFFORMAT " --> ", next_node (dawg, edge));
        set_next_edge (dawg, edge, node_map [next_node (dawg, edge)]);
        if (debug)  cprintf (REFFORMAT "\n", next_node (dawg, edge));
      }

      if (destination != 0) next_node_space = edge;
      edge = node + num_edges;
    }
    else {
      edge++;
    }
  }

  cprintf ("Compacting node from " REFFORMAT " to " REFFORMAT "  (%d)\n",
           node, next_node_space, num_edges);
  free (node_map);
}


/**********************************************************************
* delete_node
*
* Remove all the edges that are currently used within this node in the
* DAWG.
**********************************************************************/

void delete_node (EDGE_ARRAY dawg,
                  NODE_REF   node) {
  EDGE_REF   edge = node;
  inT32       counter = edges_in_node (dawg, node);

  /*
    printf ("node deleted = %d (%d)\n", node, counter);
  */
  while (counter--)
    set_empty_edge (dawg, edge++);
}


/**********************************************************************
* write_squished_dawg
*
* Write the DAWG out to a file
**********************************************************************/

void write_squished_dawg (const char *filename,
                          EDGE_ARRAY dawg,
                          inT32      max_num_edges,
                          inT32      reserved_edges) {
  FILE       *file;
  EDGE_REF    edge;
  inT32       num_edges;
  inT32       node_count = 0;
  NODE_MAP    node_map;
  EDGE_REF    old_index;
  uinT32      temp_record_32;

  if (debug) print_string ("write_squished_dawg");

  node_map = build_node_map (dawg, &node_count, FALSE, max_num_edges,
                             reserved_edges);

#ifdef WIN32
  file = open_file(filename, "wb");
#else
  file = open_file(filename, "w");
#endif

  num_edges = 0;                                 /* Count number of edges */
  for (edge=0; edge<max_num_edges; edge++)
    if (forward_edge (dawg, edge))
      num_edges++;

  num_edges = htonl(num_edges);
  fwrite (&num_edges, sizeof (inT32), 1, file);  /* Write edge count to file */
  num_edges = ntohl(num_edges);

  printf ("%d nodes in DAWG\n", node_count);
  printf ("%d edges in DAWG\n", num_edges);

  if (num_edges > MAX_NUM_EDGES_IN_SQUISHED_DAWG_FILE) {
    cprintf("Error: squished DAWG is too big to be written (%d edges > %d).\n",
            num_edges, MAX_NUM_EDGES_IN_SQUISHED_DAWG_FILE);
    exit(1);
  }

  for (edge=0; edge<max_num_edges; edge++) {
    /* Write forward edges */
    if (forward_edge (dawg, edge)) {
      do {
        old_index = next_node (dawg,edge);
        set_next_edge (dawg, edge, node_map [next_node (dawg, edge)]);
        temp_record_32 = htonl((uinT32) edge_of (dawg,edge));
        fwrite (&temp_record_32, sizeof (uinT32), 1, file);
        set_next_edge (dawg, edge, old_index);
      } edge_loop (dawg, edge);

      if (backward_edge (dawg, edge))          /* Skip back links */
        edge_loop (dawg, edge);

      edge--;
    }
  }

  free   (node_map);
  fclose (file);
}

#if 0
/**********************************************************************
* main
*
* Test the DAWG functions.
**********************************************************************/

main (argc, argv)
   int argc;
   char **argv;
{
   extern int  optind;
   extern char *optarg;
   int         option;
   time_t      start_time;
   time_t      end_time;
   FILE        *word_file;
   char        string [CHARS_PER_LINE];
   inT32       word_count = 0;
   inT32       max_num_edges  = 700000;
   inT32       reserved_edges = 50000;
   EDGE_ARRAY  dawg;
   char        *wordfile = "WORDS";
   char        *dawgfile = "DAWG";
   char        filename [CHARS_PER_LINE];
   int         baselength;

   start_time = time (&start_time);

   dawg = (EDGE_ARRAY) malloc (sizeof (EDGE_RECORD) * max_num_edges);
   if (dawg == NULL) {
   		printf ("error: Could not allocate enough memory for DAWG  ");
   		printf ("(%ld,%03ld bytes needed)\n",
   				sizeof (EDGE_RECORD) * max_num_edges / 1000,
   				sizeof (EDGE_RECORD) * max_num_edges % 1000);
   		exit (1);
   }

   if (argc > 1) {
      strcpy (filename, argv[1]);
   }
   else {
      strcpy (filename, "WORDS");
   }

   baselength = strlen (filename);

/*   strcpy (filename+baselength, ".ful");
   read_full_dawg (filename, dawg, max_num_edges);
*/
   strcpy (filename+baselength, ".lst");
   printf ("Building Dawg from word list in file, '%s'\n", filename);
   read_word_list (filename, dawg, max_num_edges, reserved_edges);

   strcpy (filename+baselength, ".ful");
   printf ("Writing full Trie file, '%s'\n", filename);
   write_full_dawg (filename, dawg, max_num_edges);

   strcpy (filename+baselength, ".opt");
   trie_to_dawg (dawg, max_num_edges, reserved_edges);
   printf ("Writing full DAWG file, '%s'\n", filename);
   write_full_dawg (filename, dawg, max_num_edges);

   strcpy (filename+baselength, ".squ");
   printf ("Writing squished file, '%s'\n", filename);
   write_squished_dawg (filename, dawg, max_num_edges, reserved_edges);

   end_time = time (&end_time);
   printf ("Seconds Elapsed = %4.1lf\n",
   			difftime (end_time, start_time));


   while ((option = getopt (argc, argv, "e:c:d:n:s:t:v")) != EOF)
     switch (option) {

      case 'c' :  {
           printf ("makedawg -c %s %s\n", optarg, argv[optind]);

           printf ("Reading Dawg file, '%s'\n", optarg);
           read_dawg  (optarg, dawg, max_num_edges);

           max_new_attempts = 1000;
           compact_dawg (dawg, max_num_edges, reserved_edges);

           printf ("Writing full file, '%s'\n", argv[optind]);
           write_full_dawg (argv[optind++], dawg, max_num_edges);

           break;
        }

      case 'd' :  {
	 printf ("makedawg -d %s %s\n", optarg, argv[optind]);

	 printf ("Reading Dawg file, '%s'\n", optarg);
	 read_dawg  (optarg, dawg, max_num_edges);
	 trie_to_dawg (dawg, max_num_edges, reserved_edges);

	 printf ("Writing full file, '%s'\n", argv[optind]);
	 write_full_dawg (argv[optind++], dawg, max_num_edges);

	 break;
      }

      case 'n' :  {
	 printf ("makedawg -n %s %s\n", optarg, argv[optind]);

	 printf ("Building Dawg from word list in file, '%s'\n", optarg);
	 read_word_list (optarg, dawg, max_num_edges, reserved_edges);

	 printf ("Writing full Dawg file, '%s'\n", argv[optind]);
	 write_full_dawg (argv[optind++], dawg, max_num_edges);

	 break;
      }

      case 's' :  {
	 printf ("makedawg -s %s %s\n", optarg, argv[optind]);

	 printf ("Reading Dawg file, '%s'\n", optarg);
	 read_dawg  (optarg, dawg, max_num_edges);

	 printf ("Writing squished file, '%s'\n", argv[optind]);
	 write_squished_dawg (argv[optind++], dawg, max_num_edges, reserved_edges);

	 break;
      }

      case 'v' :  {
	 debug = 1;
	 break;
      }

      case 't' :  {
	 read_squished_dawg  (optarg, dawg, max_num_edges);

	 if (optind < argc)
	   check_for_words (dawg, argv[optind++]);
	 else
	   check_for_words (dawg, NULL);

	 break;
      }

      case 'e' :  {
	 read_dawg  (optarg, dawg, max_num_edges);

	 if (optind < argc)
	   check_for_words (dawg, argv[optind++]);
	 else
	   check_for_words (dawg, NULL);

	 break;
      }

      default : {
	 printf ("usage: makedawg -c <old-dawg> <new-dawg>\n");
	 printf ("                -d <old-dawg> <new-dawg>\n");
	 printf ("                -n <words>    <dawg>    \n");
	 printf ("                -s <old-dawg> <new-dawg>\n");
	 printf ("                -e <dawg>     <words>   \n");
	 printf ("                -t <dawg>     <words>   \n");
	 printf ("                -v                      \n");
      }
   }
}

#endif
