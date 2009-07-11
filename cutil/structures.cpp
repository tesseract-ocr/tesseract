/* -*-C-*-
 ********************************************************************************
 *
 * File:        structures.c  (Formerly structures.c)
 * Description:  Allocate all the different types of structures.
 * Author:       Mark Seaman, OCR Technology
 * Created:      Wed May 30 10:27:26 1990
 * Modified:     Mon Jul 15 10:39:18 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Experimental (Do Not Distribute)
 *
 * (c) Copyright 1990, Hewlett-Packard Company.
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
#include "structures.h"
#include "callcpp.h"

#include <stdio.h>

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
#define BLOBBLOCK          64    /*no allocated together */
#define OUTLINEBLOCK       300   /*of each type */
#define NODEBLOCK          36    /*blocks all about 1K bytes */
#define EDGEPTBLOCK        50
#define WERDBLOCK          42
#define LISTBLOCK          300

/*----------------------------------------------------------------------
              V a r i a b l e s
----------------------------------------------------------------------*/
int structblockcount = 0;
void_void memory_print_functions[NUM_DATA_TYPES];
int max_data_types = 0;

/*----------------------------------------------------------------------
              F u n c t i o n s
----------------------------------------------------------------------*/
makestructure (newword, oldword, printword, TWERD,
freeword, WERDBLOCK, "TWERD", wordcount)
makestructure (newoutline, oldoutline, printol, TESSLINE,
freeoutline, OUTLINEBLOCK, "TESSLINE", outlinecount);

makestructure (new_cell, free_cell, printcell, list_rec,
freelist, LISTBLOCK, "LIST", listcount);

newstructure (newblob, TBLOB, freeblob, BLOBBLOCK, "newblob", blobcount);
oldstructure (oldblob, TBLOB, freeblob, "BLOB", blobcount);

newstructure (newedgept, EDGEPT, freeedgept, EDGEPTBLOCK, "newedgept",
edgeptcount);
oldstructure (oldedgept, EDGEPT, freeedgept, "EDGEPT", edgeptcount);
