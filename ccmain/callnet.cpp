/**********************************************************************
 * File:        callnet.cpp  (Formerly callnet.c)
 * Description: Interface to Neural Net matcher
 * Author:      Phil Cheatle
 * Created:     Wed Nov 18 10:35:00 GMT 1992
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
#include "errcode.h"
//#include "nmatch.h"
#include "globals.h"

#define OUTPUT_NODES 94

const ERRCODE NETINIT = "NN init error";

//extern "C"
//{
//extern char*                          demodir;                                        /* where program lives */

void init_net() {  /* Initialise net */
#ifdef ASPIRIN_INCLUDED
  char wts_filename[256];

  if (nmatch_init_network () != 0) {
    NETINIT.error ("Init_net", EXIT, "Errcode %s", nmatch_error_string ());
  }
  strcpy(wts_filename, demodir);
  strcat (wts_filename, "tessdata/netwts");

  if (nmatch_load_network (wts_filename) != 0) {
    NETINIT.error ("Init_net", EXIT, "Weights failed, Errcode %s",
      nmatch_error_string ());
  }
#endif
}


void callnet(  /* Apply image to net */
             float *input_vector,
             char *top,
             float *top_score,
             char *next,
             float *next_score) {
#ifdef ASPIRIN_INCLUDED
  float *output_vector;
  int i;
  int max_out_i = 0;
  int next_max_out_i = 0;
  float max_out = -9;
  float next_max_out = -9;
  
  nmatch_set_input(input_vector);
  nmatch_propagate_forward();
  output_vector = nmatch_get_output ();
  
  /* Now find top two choices */

  for (i = 0; i < OUTPUT_NODES; i++) {
    if (output_vector[i] > max_out) {
      next_max_out = max_out;
      max_out = output_vector[i];
      next_max_out_i = max_out_i;
      max_out_i = i;
    }
    else {
      if (output_vector[i] > next_max_out) {
        next_max_out = output_vector[i];
        next_max_out_i = i;
      }
    }
  }
  *top = max_out_i + '!';
  *next = next_max_out_i + '!';
  *top_score = max_out;
  *next_score = next_max_out;
#endif
}


//};
