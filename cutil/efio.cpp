/******************************************************************************
 **	Filename:	efio.c
 **	Purpose:	Utility I/O routines
 **	Author:		Dan Johnson
 **	History:	5/21/89, DSJ, Created.
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
#include "efio.h"
#include "danerror.h"
#include <stdio.h>
#include <string.h>

#define MAXERRORMESSAGE   256

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
FILE *Efopen(const char *Name, const char *Mode) { 
/*
 **	Parameters:
 **		Name	name of file to be opened
 **		Mode	mode to be used to open file
 **	Globals:
 **		None
 **	Operation:
 **		This routine attempts to open the specified file in the
 **		specified mode.  If the file can be opened, a pointer to
 **		the open file is returned.  If the file cannot be opened,
 **		an error is trapped.
 **	Return:
 **		Pointer to open file.
 **	Exceptions:
 **		FOPENERROR		unable to open specified file
 **	History:
 **		5/21/89, DSJ, Created.
 */
  FILE *File;
  char ErrorMessage[MAXERRORMESSAGE];

  File = fopen (Name, Mode);
  if (File == NULL) {
    sprintf (ErrorMessage, "Unable to open %s", Name);
    DoError(FOPENERROR, ErrorMessage); 
    return (NULL);
  }
  else
    return (File);
}                                /* Efopen */
