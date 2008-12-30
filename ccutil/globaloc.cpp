/**********************************************************************
 * File:        errcode.c  (Formerly error.c)
 * Description: Generic error handler function
 * Author:      Ray Smith
 * Created:     Tue May  1 16:28:39 BST 1990
 *
 * (C) Copyright 1989, Hewlett-Packard Ltd.
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

#include          "mfcpch.h"
#include          <signal.h>
#include          "errcode.h"
#include          "tprintf.h"

/*inT16 global_loc_code = LOC_INIT;//location code
inT16 global_subloc_code = SUBLOC_NORM;
                                 //pass2 subloc code
inT16 global_subsubloc_code = SUBSUBLOC_OTHER;
                                 //location code
inT16 global_abort_code = NO_ABORT_CODE;
                                 //Prog abort code
*/
void signal_exit(                 //
                 int signal_code  //Signal which
                ) {
  /*int exit_status;

  if ((global_loc_code == LOC_PASS2) || (global_loc_code == LOC_FUZZY_SPACE))
    global_loc_code += global_subloc_code + global_subsubloc_code;

  if (signal_code < 0) {
    exit_status = global_loc_code * 8 + global_abort_code * 2 + 1;
    tprintf ("Signal_exit %d ABORT. LocCode: %d  AbortCode: %d\n",
      exit_status, global_loc_code, global_abort_code);
  }
  else {
    exit_status = global_loc_code * 8 + signal_code * 2;
    tprintf ("Signal_exit %d SIGNAL ABORT. LocCode: %d  SignalCode: %d\n",
      exit_status, global_loc_code, signal_code);
  }

  exit(exit_status);*/
  exit(signal_code);
}


/*************************************************************************
 * err_exit()
 * All program exits should go through this point. It allows a meaningful status
 * code to be generated for the real exit() call. The status code is made up
 * as follows:
 *  Bit  0    : 1 = Program Abort   0 = System Abort
 *	Bits 1,2  : IF bit 0 = 1 THEN ERRCODE::abort_code
 *				ELSE    0 = Bus Err or Seg Vi
 *								1 = Floating point exception
 *							2 = TimeOut (Signal 15 from command timer)
 *							3 = Any other signal
 *  Bits 3..7 : Location code NEVER 0 !
 *************************************************************************/

//extern "C" {

void err_exit() {
  signal_exit (-1);
}


void signal_termination_handler(  //The real signal
                                int sig) {
  tprintf ("Signal_termination_handler called with signal %d\n", sig);
  switch (sig) {
    case SIGABRT:
      signal_exit (-1);          //use abort code
      //         case SIGBUS:
    case SIGSEGV:
      signal_exit (0);
    case SIGFPE:
      signal_exit (1);           //floating point
    case SIGTERM:
      signal_exit (2);           //timeout by cmdtimer
    default:
      signal_exit (3);           //Anything else
  }
}


//};                                                                                                            //end extern "C"


void set_global_loc_code(int loc_code) {
  // global_loc_code = loc_code;

}


void set_global_subloc_code(int loc_code) {
  // global_subloc_code = loc_code;

}


void set_global_subsubloc_code(int loc_code) {
  // global_subsubloc_code = loc_code;

}
