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
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "general.h"
#include "danerror.h"
#include "callcpp.h"
#include "globaloc.h"
#ifdef __UNIX__
#include "assert.h"
#endif

#include <stdio.h>
#include <setjmp.h>

#define MAXTRAPDEPTH    100

#define ERRORTRAPDEPTH    1000

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
static jmp_buf ErrorTrapStack[MAXTRAPDEPTH];
static VOID_PROC ProcTrapStack[MAXTRAPDEPTH];
static inT32 CurrentTrapDepth = 0;

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ReleaseErrorTrap() {
/*
 **	Parameters:
 **		None
 **	Globals:
 **		CurrentTrapDepth	number of traps on the stack
 **	Operation:
 **		This routine removes the current error trap from the
 **		error trap stack, thus returning control to the previous
 **		error trap.  If the error trap stack is empty, nothing is
 **		done.
 **	Return:
 **		None
 **	Exceptions:
 **		None
 **	History:
 **		4/3/89, DSJ, Created.
 */
  if (CurrentTrapDepth > 0) {
    CurrentTrapDepth--;
  }
}                                /* ReleaseErrorTrap */


/*---------------------------------------------------------------------------*/
void DoError(int Error, const char *Message) {
/*
 **	Parameters:
 **		Error	error number which is to be trapped
 **		Message	pointer to a string to be printed as an error message
 **	Globals:
 **		ErrorTrapStack		stack of error traps
 **		CurrentTrapDepth	number of traps on the stack
 **	Operation:
 **		This routine prints the specified error message to stderr.
 **		It then jumps to the current error trap.  If the error trap
 **		stack is empty, the calling program is terminated with a
 **		fatal error message.
 **	Return:
 **		None - this routine does not return.
 **	Exceptions:
 **		Empty error trap stack terminates the calling program.
 **	History:
 **		4/3/89, DSJ, Created.
 */
  if (Message != NULL) {
    cprintf ("\nError: %s!\n", Message);
  }

  if (CurrentTrapDepth <= 0) {
    cprintf ("\nFatal error: No error trap defined!\n");

    /* SPC 20/4/94
       There used to be a call to abort() here. I've changed it to call into the
       C++ error code to generate a meaningful status code
     */
    signal_termination_handler(Error);
  }

  if (ProcTrapStack[CurrentTrapDepth - 1] != DO_NOTHING)
    (*ProcTrapStack[CurrentTrapDepth - 1]) ();

  longjmp (ErrorTrapStack[CurrentTrapDepth - 1], 1);
}                                /* DoError */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
jmp_buf &PushErrorTrap(VOID_PROC Procedure) {
/*
 **	Parameters:
 **		Procedure		trap procedure to execute
 **	Globals:
 **		ErrorTrapStack		stack of error traps
 **		CurrentTrapDepth	number of traps on the stack
 **	Operation:
 **		This routine pushes a new error trap onto the top of
 **		the error trap stack.  This new error trap can then be
 **		used in a call to setjmp.  This trap is then in effect
 **		until ReleaseErrorTrap is called.  WARNING: a procedure
 **		that calls PushErrorTrap should never exit before calling
 **		ReleaseErrorTrap.
 **	Return:
 **		Pointer to a new error trap buffer
 **	Exceptions:
 **		Traps an error if the error trap stack is already full
 **	History:
 **		3/17/89, DSJ, Created.
 **		9/12/90, DSJ, Added trap procedure parameter.
 */
  if (CurrentTrapDepth >= MAXTRAPDEPTH)
    DoError (ERRORTRAPDEPTH, "Error trap depth exceeded");
  ProcTrapStack[CurrentTrapDepth] = Procedure;
  return ErrorTrapStack[CurrentTrapDepth++];

}                                /* PushErrorTrap */
