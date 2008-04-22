/* -*-C-*-
********************************************************************************
*
* File:         reduce.h
* Description:  Functions to reduce a TRIE into a DAWG
* Author:       Mark Seaman, SW Productivity
* Created:      Fri Oct 16 14:37:00 1987
* Modified:     Wed Jun 19 16:03:08 1991 (Mark Seaman) marks@hpgrlt
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

#ifndef REDUCE_H
#define REDUCE_H

/*
----------------------------------------------------------------------
                     I n c l u d e s
----------------------------------------------------------------------
*/

#include "general.h"
#include "dawg.h"
#include "trie.h"


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

void collapse_source_nodes(EDGE_ARRAY dawg,
                           NODE_REF source_node_1,
                           NODE_REF source_node_2,
                           NODE_REF dest_node,
                           inT32 max_num_edges,
                           inT32 reserved_edges);

inT32 eliminate_redundant_edges(EDGE_ARRAY dawg,
                                NODE_REF node,
                                EDGE_REF edge_1,
                                EDGE_REF edge_2,
                                inT32 max_num_edges,
                                inT32 reserved_edges);

inT32 letter_order(const void* edge1_ptr,
                   const void* edge2_ptr);

void print_n_edges(EDGE_RECORD *edge1,
                   inT32 n);

inT32 reduce_lettered_edges(EDGE_ARRAY dawg,
                            EDGE_REF *edge,
                            NODE_REF node,
                            NODE_MARKER reduced_nodes,
                            inT32 max_num_edges,
                            inT32 reserved_edges);

void reduce_node_input(EDGE_ARRAY dawg,
                       NODE_REF node,
                       NODE_MARKER reduced_nodes,
                       inT32 max_num_edges,
                       inT32 reserved_edges);

inT32 same_output(EDGE_ARRAY dawg,
                  NODE_REF node1,
                  NODE_REF node2);

void trie_to_dawg(EDGE_ARRAY dawg,
                  inT32 max_num_edges,
                  inT32 reserved_edges);


#endif
