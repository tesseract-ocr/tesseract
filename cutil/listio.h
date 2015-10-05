/* -*-C-*-
################################################################################
#
# File:         listio.h
# Description:  List I/O processing procedures.
# Author:       Mark Seaman, Software Productivity
# Created:      Thu Jul 23 13:24:09 1987
# Modified:     Mon Oct 16 11:38:52 1989 (Mark Seaman) marks@hpgrlt
# Language:     C
# Package:      N/A
# Status:       Reusable Software Component
#
# (c) Copyright 1987, Hewlett-Packard Company.
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
#
################################################################################
 * Revision 1.5  89/06/27  11:56:00  11:56:00  marks (Mark Seaman)
 * Fixed MAC_OR_DOS bug
 *

  This file contains the interface definitions to a set of general purpose
  list I/O routines.

***********************************************************************/
#ifndef LISTIO_H
#define LISTIO_H

#include <stdio.h>
#include "oldlist.h"

/*----------------------------------------------------------------------------
        Public Function Prototypes
--------------------------------------------------------------------------*/
LIST read_list(const char *filename);
#endif
