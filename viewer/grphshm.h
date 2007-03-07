/**********************************************************************
 * File:        grphshm.h  (Formerly graphshm.h)
 * Description: Header for graphics shared memory functions.
 * Author:      Ray Smith
 * Created:     Thu May 24 15:29:28 BST 1990
 *
 * (C) Copyright 1990, Hewlett-Packard Ltd.
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

#ifndef           GRPHSHM_H
#define           GRPHSHM_H

#include          "sbgtypes.h"
#include          "grphics.h"

#define SHMSIZE       (65536*8)  /*shm segment size */
#define PIPESIZE      8192       /*pipe bufer size */
#define USER_RW       0600       /*permission flags */
                                 /*starbase environ */
#define SBADDR        "SB_DISPLAY_ADDR"
#define WMSHM       "WMSHMSPC"   /*shared mem size */
#define WMSHMDEFAULT    0x200000 /*default value */
#define SBDEFAULT     0x0b00000  /*default ADDR */
#define SHMOFFSET     0x200000   /*offset before sbaddr */
#define MAXDATA       0x1000000  /*default data seg size */

#define SBDAEMON      "sbdaemon"
#define REMSH       "remsh"      /*command for remote use */
#define DISP        "DISPLAY"    /*environ var */
#define LOCAL1        "local"    /*possible values */
#define LOCAL2        "unix"     /*of DISPLAY */

#define FLUSH_OUT     0          /*kick_daemon commands */
#define FLUSH_IN      1
#define AWAIT_BUFFER    2        /*wait for free buffer */
#define COUNT_READS     3        /*count a queue clear */
#define MAX_PENDING     255      /*max pending reads */

extern SHMINFO shminfo;          /*shared memory */
extern WINFD sbfds[MAXWINDOWS];  /*file descriptors */
extern INT16 maxsbfd;
#ifdef __MSW32__
extern DWORD event_id;           //event thread id
#endif

void start_sbdaemon();  /*start the daemon */
void cleanup_sbdaemon();  /*forget about the daemon */
BOOL8 remote_display(            //check for remote
                     char *name  //name of host
                    );
DLLSYM void *getshm(            /*get memory */
                    INT32 size  /*required size */
                   );
void kick_daemon(           /*empty queue */
                 INT8 mode  /*control mode */
                );
#ifdef __MSW32__
int two_way_pipe (               //do one file
const char *file,                //program to run
const char *argv[],              //args to execvp
HANDLE fds[]                     //output fds
);
#endif
#endif
