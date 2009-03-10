/* -*-C-*-
 ********************************************************************************
 *
 * File:        hideedge.h  (Formerly hideedge.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Apr 30 12:49:57 1991 (Mark Seaman) marks@hpgrlt
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
 ********************************************************************************
 */

#ifndef HIDEEDGE_H
#define HIDEEDGE_H

/*
----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------
*/

#include "general.h"

/*
----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------
*/

/**********************************************************************
 * is_hidden_edge
 *
 * Check to see if this edge is a hidden edge.  This will prohibit
 * feature extraction and display functions on this edge.  The
 * argument should be of type (EDGEPT*).
 **********************************************************************/

#define is_hidden_edge(edge) \
/*(hidden_edges &&*/ (edge->flags[0])     /*) */

/**********************************************************************
 * hide_edge
 *
 * Make this edge a hidden edge.  This will prohibit feature extraction
 * and display functions on this edge.  The argument should be of type
 * (EDGEPT*).
 **********************************************************************/

#define hide_edge(edge)  \
/*if (hidden_edges)*/ edge->flags[0] = TRUE

/**********************************************************************
 * reveal_edge
 *
 * Make this edge a unhidden edge.  This will prohibit feature extraction
 * and display functions on this edge.  The argument should be of type
 * (EDGEPT*).
 **********************************************************************/

#define reveal_edge(edge)  \
/*if (hidden_edges)*/ edge->flags[0] = FALSE
#endif
