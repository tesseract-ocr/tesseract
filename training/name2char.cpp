/******************************************************************************
**	Filename:    name2char.c
**	Purpose:     Routines to convert between classes and class names.
**	Author:      Dan Johnson
**	History:     Fri Feb 23 08:03:09 1990, DSJ, Created.
**
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
-----------------------------------------------------------------------------*/
#include "name2char.h"
#include "matchdefs.h"
#include "danerror.h"
#include <string.h>

#define ILLEGALCHARNAME		6001

/*----------------------------------------------------------------------------
		  		Global Data Definitions and Declarations
-----------------------------------------------------------------------------*/
/** character ID (ascii code) to character name mapping */
static const char	*NameList[]={
	"!bang",
	  "\"doubleq",
	  "#hash",
	  "$dollar",
	  "%percent",
	  "&and",
	  "'quote",
	  "(lround",
	  ")rround",
	  "*asterisk",
	  "+plus",
	  ",comma",
	  "-minus",
	  ".dot",
	  "/slash",
	  ":colon",
	  ";semic",
	  "<less",
	  "=equal",
	  ">greater",
	  "?question",
	  "@at",
	  "[lsquare",
	  "\\backsl",
	  "]rsquare",
	  "^uparr",
	  "_unders",
	  "`grave",
	  "{lbrace",
	  "|bar",
	  "}rbrace",
	  "~tilde",
	  "AcA",
	  "BcB",
	  "CcC",
	  "DcD",
	  "EcE",
	  "FcF",
	  "GcG",
	  "HcH",
	  "IcI",
	  "JcJ",
	  "KcK",
	  "LcL",
	  "McM",
	  "NcN",
	  "OcO",
	  "PcP",
	  "QcQ",
	  "RcR",
	  "ScS",
	  "TcT",
	  "UcU",
	  "VcV",
	  "WcW",
	  "XcX",
	  "YcY",
	  "ZcZ",
	  NULL
  };


/*----------------------------------------------------------------------------
							Public Code
-----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 * This routine converts the specified character name to
 * an ascii character.
 *
 * @param CharName	character name to convert to a character
 *
 * Globals:
 * - NameList	lookup table for name to char mapping
 *
 * @return Ascii character that corresponds to the character name.
 * @note Exceptions: ILLEGALCHARNAME
 * @note History: Sat Aug 26 12:26:54 1989, DSJ, Created.
 */
CLASS_ID NameToChar (char CharName[])
{
	int	i;

	// look for name in table and return character if found
	for ( i = 0; NameList[i] != NULL; i++ )
		if ( strcmp (CharName, &NameList[i][1]) == 0)
			return (NameList[i][0]);
	if ( strlen (CharName) == 1 )
		return (CharName[0]);	//name is not in table but is a single character
	else	//illegal character
	{
		DoError (ILLEGALCHARNAME, "Illegal character name");
		return 0;
	}
}	/* NameToChar */

/*---------------------------------------------------------------------------*/
void CharToName (
     CLASS_ID	Char,
     char	CharName[])

/*
**	Parameters:
**		Char		character to map to a character name
**		CharName	string to copy character name into
**	Globals:
**		NameList	lookup table for char to name mapping
**	Operation:
**		This routine converts the specified ascii character to a
**		character name.  This is convenient for representing
**		characters which might have special meaning to operating
**		system shells or other programs (e.g. "*?&><" etc.).
**	Return: none
**	Exceptions: none
**	History: Sat Aug 26 12:51:02 1989, DSJ, Created.
*/

{
	int	i;

	/* look for character in table and return a copy of its name if found */
	for ( i = 0; NameList[i] != NULL; i++ )
		if ( Char == NameList[i][0] )
		{
			strcpy ( CharName, &NameList[i][1] );
			return;
		}

		/* if the character is not in the table, then use it as the name */
		CharName[0] = Char;
		CharName[1] = 0;

}	/* CharToName */
