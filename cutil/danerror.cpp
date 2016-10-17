/******************************************************************************
 **	Filename:	danerror.c
 **	Purpose:	Routines for managing error trapping
 **	Author:		Dan Johnson
 **	History:	3/17/89, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
/*----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------*/
#include "host.h"
#include "danerror.h"
#include "tprintf.h"
#include "globaloc.h"
#ifdef __UNIX__
#include "assert.h"
#endif

#include <stdio.h>

/**
 * This routine prints the specified error message to stderr.
 * It then jumps to the current error trap.  If the error trap
 * stack is empty, the calling program is terminated with a
 * fatal error message.
 *
 * @param Error error number which is to be trapped
 * @param Message pointer to a string to be printed as an error message
 * @return None - this routine does not return.
 * @note History: 4/3/89, DSJ, Created.
 */
void DoError(int Error, const char *Message) {
  if (Message != NULL) {
    tprintf("\nError: %s!\n", Message);
  }

  err_exit();
}                                /* DoError */
