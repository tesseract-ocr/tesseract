/**********************************************************************
 * File:        callcpp.cpp
 * Description: extern C interface calling C++ from C.
 * Author:		Ray Smith
 * Created:		Sun Feb 04 20:39:23 MST 1996
 *
 * (C) Copyright 1996, Hewlett-Packard Co.
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
#include          "errcode.h"
#ifdef __UNIX__
#include          <assert.h>
#include <stdarg.h>
#endif
#include          <time.h>
#include          "memry.h"
#include          "grphics.h"
#include          "evnts.h"
#include          "varable.h"
#include          "callcpp.h"
#include          "tprintf.h"
//#include                                      "strace.h"
#include          "host.h"

//extern "C" {

INT_VAR (tess_cp_mapping0, 0, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping1, 1, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping2, 2, "Mappings for class pruner distance");
INT_VAR (tess_cp_mapping3, 3, "Mappings for class pruner distance");
INT_VAR (stopper_numbers_on, 0, "Allow numbers to be acceptable choices");
INT_VAR (config_pruner_enabled, 0, "Turn on config pruner");
INT_VAR (feature_prune_percentile, 0, "Percent of features to use");
INT_VAR (newcp_ratings_on, 0, "Use new class pruner normalisation");
INT_VAR (record_matcher_output, 0, "Record detailed matcher info");
INT_VAR (il1_adaption_test, 0, "Dont adapt to i/I at beginning of word");
double_VAR (permuter_pending_threshold, 0.0,
"Worst conf for using pending dictionary");
double_VAR (newcp_duff_rating, 0.30, "Worst rating for calling real matcher");
double_VAR (newcp_prune_threshold, 1.2, "Ratio of best to prune");
double_VAR (tessedit_cp_ratio, 0.0, "Ratio from best to prune");
//Global matcher info from the class pruner.
INT32 cp_classes;
INT32 cp_bestindex;
INT32 cp_bestrating;
INT32 cp_bestconf;
char cp_chars[2];
INT32 cp_ratings[2];
INT32 cp_confs[2];
INT32 cp_maps[4];
//Global info to control writes of matcher info
INT32 blob_type;                 //write control
char blob_answer;                //correct char
char *word_answer;               //correct word
INT32 matcher_pass;              //pass in chopper.c
INT32 bits_in_states;            //no of bits in states

#ifndef __UNIX__
/**********************************************************************
 * assert
 *
 * A version of assert for C on NT.
 **********************************************************************/

void assert(             //recog one owrd
            int testing  //assert fail if false
           ) {
  ASSERT_HOST(testing); 
}
#endif

void setup_cp_maps() { 
  cp_maps[0] = tess_cp_mapping0;
  cp_maps[1] = tess_cp_mapping1;
  cp_maps[2] = tess_cp_mapping2;
  cp_maps[3] = tess_cp_mapping3;
}


void trace_stack() {  //Trace current stack
}


void
cprintf (                        //Trace printf
const char *format, ...          //special message
) {
  va_list args;                  //variable args
  char msg[1000];

  va_start(args, format);  //variable list
  vsprintf(msg, format, args);  //Format into msg
  va_end(args); 

  tprintf ("%s", msg);
}


char *c_alloc_string(             //allocate string
                     INT32 count  //no of chars required
                    ) {
  return alloc_string (count);
}


void c_free_string(              //free a string
                   char *string  //string to free
                  ) {
  free_string(string); 
}


void *c_alloc_struct(                  //allocate memory
                     INT32 count,      //no of chars required
                     const char *name  //class name
                    ) {
  return alloc_struct (count, name);
}


void c_free_struct(                   //free a structure
                   void *deadstruct,  //structure to free
                   INT32 count,       //no of bytes
                   const char *name   //class name
                  ) {
  free_struct(deadstruct, count, name); 
}


void *c_alloc_mem_p(             //allocate permanent space
                    INT32 count  //block size to allocate
                   ) {
  return alloc_mem_p (count);
}


void *c_alloc_mem(             //get some memory
                  INT32 count  //no of bytes to get
                 ) {
  return alloc_mem (count);
}


void c_free_mem(                //free mem from alloc_mem
                void *oldchunk  //chunk to free
               ) {
  free_mem(oldchunk); 
}


void c_check_mem(                     //check consistency
                 const char *string,  //context message
                 INT8 level           //level of check
                ) {
  check_mem(string, level); 
}

#ifndef GRAPHICS_DISABLED
void *c_create_window(                   /*create a window */
                      const char *name,  /*name/title of window */
                      INT16 xpos,        /*coords of window */
                      INT16 ypos,        /*coords of window */
                      INT16 xsize,       /*size of window */
                      INT16 ysize,       /*size of window */
                      double xmin,       /*scrolling limits */
                      double xmax,       /*to stop users */
                      double ymin,       /*getting lost in */
                      double ymax        /*empty space */
                     ) {
  return create_window (name, SCROLLINGWIN, xpos, ypos, xsize, ysize,
    xmin, xmax, ymin, ymax, TRUE, FALSE, FALSE, TRUE);
}


void c_line_color_index(  /*set color */
                        void *win,
                        C_COL index) {
  WINDOW window = (WINDOW) win;

  //      ASSERT_HOST(index>=0 && index<=48);
  if (index < 0 || index > 48)
    index = (C_COL) 1;
  window->Line_color_index ((COLOUR) index);
}


void c_move(  /*move pen */
            void *win,
            double x,
            double y) {
  WINDOW window = (WINDOW) win;

  window->Move2d (x, y);
}


void c_draw(  /*move pen */
            void *win,
            double x,
            double y) {
  WINDOW window = (WINDOW) win;

  window->Draw2d (x, y);
}


void c_make_current(  /*move pen */
                    void *win) {
  WINDOW window = (WINDOW) win;

  window->Make_picture_current ();
}


void c_clear_window(  /*move pen */
                    void *win) {
  WINDOW window = (WINDOW) win;

  window->Clear_view_surface ();
}


char window_wait(  /*move pen */
                 void *win) {
  WINDOW window = (WINDOW) win;
  GRAPHICS_EVENT event;

  await_event(window, TRUE, ANY_EVENT, &event); 
  if (event.type == KEYPRESS_EVENT)
    return event.key;
  else
    return '\0';
}
#endif

void reverse32(void *ptr) { 
  char tmp;
  char *cptr = (char *) ptr;

  tmp = *cptr;
  *cptr = *(cptr + 3);
  *(cptr + 3) = tmp;
  tmp = *(cptr + 1);
  *(cptr + 1) = *(cptr + 2);
  *(cptr + 2) = tmp;
}


void reverse16(void *ptr) { 
  char tmp;
  char *cptr = (char *) ptr;

  tmp = *cptr;
  *cptr = *(cptr + 1);
  *(cptr + 1) = tmp;
}


//};
