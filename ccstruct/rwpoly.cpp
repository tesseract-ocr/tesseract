/**********************************************************************
 * File:        rwpoly.c  (Formerly rw_poly.c)
 * Description: latest version of manual page decomp tool
 * Author:      Sheelagh Lloyd
 * Created:     16:05 24/3/93
 *
 * This version constructs a list of blocks.
 *
 * (C) Copyright 1993, Hewlett-Packard Ltd.
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
 **********************************************************************/

#include "mfcpch.h"
#include          "pageblk.h"
#include          "rwpoly.h"

#include          "hpddef.h"     //must be last (handpd.dll)

#define EXTERN

EXTERN DLLSYM PAGE_BLOCK_LIST *page_block_list;
EXTERN PAGE_BLOCK_IT page_block_it;
EXTERN BOOL_VAR (blocks_read_asc, TRUE, "Read blocks in ascii format");
EXTERN BOOL_VAR (blocks_write_asc, TRUE, "Write blocks in ascii format");

DLLSYM void write_poly_blocks(FILE *blfile, PAGE_BLOCK_LIST *blocks) {

  if (blocks_write_asc)
    blocks->serialise_asc (blfile);
  else
    blocks->serialise (blfile);

  return;
}


DLLSYM PAGE_BLOCK_LIST *read_poly_blocks(                  //read file
                                         const char *name  //file to read
                                        ) {
  FILE *infp;
  int c;
  inT16 number_of_pblocks;
                                 //output list
  PAGE_BLOCK_LIST *pb_list = NULL;
  PAGE_BLOCK *page_block;        //new block for list
  inT32 len;                     /*length to retrive */
  PAGE_BLOCK_IT it;

  if ((infp = fopen (name, "r")) != NULL) {
    if (((c = fgetc (infp)) != EOF) && (ungetc (c, infp) != EOF)) {
      if (blocks_read_asc) {
        pb_list = new PAGE_BLOCK_LIST;

        len = de_serialise_INT32 (infp);
        it.set_to_list (pb_list);
        for (; len > 0; len--) {
          page_block = PAGE_BLOCK::new_de_serialise_asc (infp);
                                 /*put on the list */
          it.add_to_end (page_block);
        }
      }
      else
        pb_list = PAGE_BLOCK_LIST::de_serialise (infp);
      page_block_list = pb_list; //set global for now
    }
    fclose(infp);
  }
  else {
                                 //can't open file
    CANTOPENFILE.error ("read_poly_blocks", TESSLOG, name);
    pb_list = new PAGE_BLOCK_LIST;
    page_block_list = pb_list;   //set global for now
  }
  page_block_it.set_to_list (pb_list);
  number_of_pblocks = pb_list->length ();

  tprintf ("%d page blocks read\n", number_of_pblocks);
  return pb_list;

}
