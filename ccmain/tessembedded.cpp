/**********************************************************************
 * File:        tessembedded.cpp
 * Description: Main program for merge of tess and editor.
 * Author:          Marius Renn
 * Created:         Sun Oct 21 2006
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
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
#include "applybox.h"
#include "control.h"
#include "tessvars.h"
#include "tessedit.h"
#include "pageres.h"
#include "imgs.h"
#include "varabled.h"
#include "tprintf.h"
#include "tessembedded.h"
#include "stderr.h"
#include "notdll.h"
#include "mainblk.h"
#include "globals.h"
#include "tfacep.h"
#include "callnet.h"

void tessembedded_read_file(STRING &name,
                            BLOCK_LIST *blocks) {
  int c;                      //input character
  FILE *infp;                 //input file
  BLOCK_IT block_it(blocks);  //iterator
  BLOCK *block;               //current block

  ICOORD page_tr;             //topright of page

  char *filename_extension;

  block_it.move_to_last ();

                                 // ptr to last dot
  filename_extension = strrchr (name.string (), '.');
  #ifdef __UNIX__
  if (strcmp (filename_extension, ".pb") == 0) {
    tprintf ("Converting from .pb file format.\n");
                                 //construct blocks
    read_and_textord (name.string (), blocks);
  }
  else
  #endif
                                 //xiaofan, a hack here
  if (strcmp (filename_extension, ".tif") == 0) {
    //              tprintf( "Interpreting .bl file format.\n" );
                                 //construct blocks
    edges_and_textord (name.string (), blocks);
  }
  else {
    if ((strcmp (filename_extension, ".pg") == 0) ||
      // read a .pg file
                                 // or a .sp file
    (strcmp (filename_extension, ".sp") == 0)) {
      tprintf ("Reading %s file format.\n", filename_extension);
      infp = fopen (name.string (), "r");
      if (infp == NULL)
        CANTOPENFILE.error ("pgeditor_read_file", EXIT, name.string ());
      //can't open file

      while (((c = fgetc (infp)) != EOF) && (ungetc (c, infp) != EOF)) {
                                 //get one
        block = BLOCK::de_serialise (infp);
                                 //add to list
        block_it.add_after_then_move (block);
      }
      fclose(infp); 
    }
  }
}

int init_tessembedded(const char *arg0,
                      const char *textbase,
                      const char *configfile,
                      int configc,
                      const char *const *configv) {
  main_setup(arg0, textbase, configc, configv); 

  debug_window_on.set_value (FALSE);
  
  static char c_path[MAX_PATH];  //path for c code
  strcpy (c_path, datadir.string ());
  c_path[strlen (c_path) - strlen (m_data_sub_dir.string ())] = '\0';
  demodir = c_path;
  start_recog(configfile, textbase); 
  
  init_net(); 
  
  return 0;
}

void end_tessembedded() {
  end_recog();
}
