/* -*-C-*-
********************************************************************************
*
* File:         makedawg.h
* Description:  Create a Directed Accyclic Word Graph
* Author:       Mark Seaman, SW Productivity
* Created:      Fri Oct 16 14:37:00 1987
* Modified:     Wed Jul 17 17:18:49 1991 (Mark Seaman) marks@hpgrlt
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

#ifndef MAKEDAWG_H
#define MAKEDAWG_H

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

NODE_MAP build_node_map(EDGE_ARRAY dawg,
                        inT32 *num_nodes,
                        inT32 both_links,
                        inT32 max_num_edges,
                        inT32 reserved_edges);

void compact_dawg(EDGE_ARRAY dawg,
                  inT32 max_num_edges,
                  inT32 reserved_edges);

void delete_node(EDGE_ARRAY dawg,
                 NODE_REF node);

void write_squished_dawg(const char *filename,
                         EDGE_ARRAY dawg,
                         inT32 max_num_edges,
                         inT32 reserved_edges);

#endif
