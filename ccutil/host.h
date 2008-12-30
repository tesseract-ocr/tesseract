/******************************************************************************
 **  Filename:       Host.h
 **  Purpose:        This is the system independent typedefs and defines
 **  Author:         MN, JG, MD
 **  Version:        5.4.1
 **  History:        11/7/94 MCD received the modification that Lennart made
 **                  to port to 32 bit world and modify this file so that it
 **                  will be shared between platform.
 **                  11/9/94 MCD Make MSW32 subset of MSW. Now MSW means
 **                  MicroSoft Window and MSW32 means the 32 bit worlds
 **                  of MicroSoft Window. Therefore you want the environment
 **                  to be MicroSoft Window and in the 32 bit world -
 **                  __MSW__ and __MSW32__ must be uncommented out.
 **                  11/30/94 MCD Incorporated comments received for more
 **                  readability and the missing typedef for FLOAT.
 **                  12/1/94 MCD Added PFVOID typedef
 **                  5/1/95 MCD. Made many changes based on the inputs.
 **                  Changes:
 **                  1) Rearrange the #ifdef so that there're definitions for
 **                  particular platforms.
 **                  2) Took out the #define for computer and environment
 **                  that developer can uncomment
 **                  3) Added __OLDCODE__ where the defines will be
 **                  obsoleted in the next version and advise not to use.
 **                  4) Added the definitions for the following:
 **                  FILE_HANDLE, MEMORY_HANDLE, BOOL8,
 **                  MAX_INT8, MAX_INT16, MAX_INT32, MAX_UINT8
 **                  MAX_UINT16, MAX_UINT32, MAX_FLOAT32
 **                 06/19/96 MCD. Took out MAX_FLOAT32
 **                 07/15/96 MCD. Fixed the comments error
 **                 Add back BOOL8.
 **
 **  (c) Copyright Hewlett-Packard Company, 1988-1996.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef   __HOST__
#define   __HOST__

/******************************************************************************
 **                                IMPORTANT!!!                                                                                                                 **
 **                                                                                                                                                                              **
 ** Defines either __MSW__, __MSW32__, __MAC__, __UNIX__, __OS2__, __PM__ to
 ** use the specified definitions indicated below in the preprocessor settings.                                                        **
 **                                                                                                                                                                              **
 ** Also define either  __FarProc__ or  __FarData__  and __MOTO__ to use the
 ** specified definitions indicated below in the preprocessor settings.                                                                        **
 **                                                                                                                                                                             **
 ** If a preprocessor settings is not allow in the compiler that is being use,
 ** then it is recommended that a "platform.h" is created with the definition
 ** of the computer and/or operating system.
 ******************************************************************************/

#include "platform.h"
/* __MSW32__ */
#ifdef __MSW32__
#include <windows.h>
#include <winbase.h>             // winbase.h contains windows.h

#define DLLIMPORT __declspec( dllimport)
#define DLLEXPORT __declspec( dllexport)

#else
/********************************************************/
/* __MSW__ */
#ifdef __MSW__
#include <windows.h>             // provides standard definitions (like HANDLE)

#define DLLIMPORT __import
#define DLLEXPORT __export
#endif
#endif

/********************************************************/
/* __MAC__ */
#ifdef __MAC__
#include <Types.h>
/*----------------------------*/
/*----------------------------*/
#define DLLIMPORT
#define DLLEXPORT

#endif
/********************************************************/
#if defined(__UNIX__) || defined( __DOS__ ) || defined(__OS2__) || defined(__PM__)
/*----------------------------*/
/* FarProc and FarData */
/*----------------------------*/
#define DLLIMPORT
#define DLLEXPORT
/*----------------------------*/
#endif
/*****************************************************************************
 **
 **                      Standard GHC Definitions
 **
 *****************************************************************************/

#ifdef __MOTO__
#define __NATIVE__   MOTO
#else
#define __NATIVE__   INTEL
#endif

//typedef HANDLE FD*  PHANDLE;

// definitions of portable data types (numbers and characters)
typedef SIGNED char inT8;
typedef unsigned char uinT8;
typedef short inT16;
typedef unsigned short uinT16;
typedef int inT32;
typedef unsigned int uinT32;
#if (_MSC_VER >= 1200)            //%%% vkr for VC 6.0
typedef INT64 inT64;
typedef UINT64 uinT64;
#else
typedef long long int inT64;
typedef unsigned long long int uinT64;
#endif                           //%%% vkr for VC 6.0
typedef float FLOAT32;
typedef double FLOAT64;
typedef unsigned char BOOL8;

#define INT32FORMAT "%d"
#define INT64FORMAT "%lld"

#define MAX_INT8  0x7f
#define MAX_INT16 0x7fff
#define MAX_INT32 0x7fffffff
#define MAX_UINT8 0xff
#define MAX_UINT16  0xffff
#define MAX_UINT32  0xffffffff
#define MAX_FLOAT32 ((float)3.40282347e+38)

#define MIN_INT8  0x80
#define MIN_INT16 0x8000
#define MIN_INT32 0x80000000
#define MIN_UINT8 0x00
#define MIN_UINT16  0x0000
#define MIN_UINT32  0x00000000
#define MIN_FLOAT32 ((float)1.17549435e-38)

// Defines

#ifndef OKAY
#define OKAY            0
#endif

#ifndef HPERR
#define HPERR           -1
#endif

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef NULL
#define NULL            0L
#endif

// Return true if x is within tolerance of y
template<class T> bool NearlyEqual(T x, T y, T tolerance) {
  T diff = x - y;
  return diff <= tolerance && -diff <= tolerance;
}

#endif
