/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifndef  LEPTONICA_ENVIRON_H
#define  LEPTONICA_ENVIRON_H

/*------------------------------------------------------------------------*
 *  Defines and includes differ for Unix and Windows.  Also for Windows,  *
 *  differentiate between conditionals based on platform and compiler.    *
 *      For platforms:                                                    *
 *          _WIN32       =>     Windows, 32- or 64-bit                    *
 *          _WIN64       =>     Windows, 64-bit only                      *
 *          __CYGWIN__   =>     Cygwin                                    *
 *      For compilers:                                                    *
 *          __GNUC__     =>     gcc                                       *
 *          _MSC_VER     =>     msvc                                      *
 *------------------------------------------------------------------------*/

/* MS VC++ does not provide stdint.h, so define the missing types here */

#ifndef _MSC_VER
#include <stdint.h>

#else
/* Note that _WIN32 is defined for both 32 and 64 bit applications,
   whereas _WIN64 is defined only for the latter */

#ifdef _WIN64
typedef __int64 intptr_t;
typedef unsigned __int64 uintptr_t;
#else
typedef int intptr_t;
typedef unsigned int uintptr_t;
#endif

/* VC++6 doesn't seem to have powf, expf. */
#if (_MSC_VER < 1400)
#define powf(x, y) (float)pow((double)(x), (double)(y))
#define expf(x) (float)exp((double)(x))
#endif

#endif /* _MSC_VER */

/* Windows specifics */

#ifdef _WIN32

/* DLL EXPORTS and IMPORTS:
 * Important: LEPTONLIB_* is deprected.  It is retained here only for
 * compatibility with tesseract 3.00.  In your project files, use
 * LIBLEPT_EXPORTS and LIBLEPT_IMPORTS  */
#if defined(LIBLEPT_EXPORTS) || defined(LEPTONLIB_EXPORTS)
#define LEPT_DLL __declspec(dllexport)
#elif defined(LIBLEPT_IMPORTS) || defined(LEPTONLIB_IMPORTS)
#define LEPT_DLL __declspec(dllimport)
#else
#define LEPT_DLL
#endif

#else  /* non-WINDOWS-SPECIFICS */
#include <stdint.h>
#define LEPT_DLL
#endif  /* _WIN32 */

typedef intptr_t l_intptr_t;
typedef uintptr_t l_uintptr_t;
typedef void *L_TIMER;


/*--------------------------------------------------------------------*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *                          USER CONFIGURABLE                         *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *                 Environ variables with I/O libraries               *
 *               Manual Configuration Only: NOT AUTO_CONF             *
 *--------------------------------------------------------------------*/
/*
 *  Leptonica provides interfaces to link to five external image I/O
 *  libraries, plus zlib.  Setting any of these to 0 here causes
 *  non-functioning stubs to be linked.
 */
#ifndef HAVE_CONFIG_H
#define  HAVE_LIBJPEG     1
#define  HAVE_LIBTIFF     1
#define  HAVE_LIBPNG      1
#define  HAVE_LIBZ        1
#define  HAVE_LIBGIF      1
#define  HAVE_LIBUNGIF    0
#define  HAVE_LIBWEBP     0
#endif  /* ~HAVE_CONFIG_H */

/*
 * On linux systems, you can do I/O between Pix and memory.  Specifically,
 * you can compress (write compressed data to memory from a Pix) and
 * uncompress (read from compressed data in memory to a Pix).
 * For jpeg, png, pnm and bmp, these use the non-posix GNU functions
 * fmemopen() and open_memstream().  These functions are not
 * available on other systems.  To use these functions in linux,
 * you must define HAVE_FMEMOPEN to be 1 here.
 */
#ifndef HAVE_CONFIG_H
#define  HAVE_FMEMOPEN    0
#endif  /* ~HAVE_CONFIG_H */


/*--------------------------------------------------------------------*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *                          USER CONFIGURABLE                         *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*
 *       Environ variables for uncompressed formatted image I/O       *
 *--------------------------------------------------------------------*/
/*
 *  Leptonica supplies image I/O for pnm, bmp, ps, and pdf.
 *  Setting any of these to 0 causes non-functioning stubs to be linked.
 */
#define  USE_BMPIO        1
#define  USE_PNMIO        1
#define  USE_PSIO         1
#define  USE_PDFIO        1


/*--------------------------------------------------------------------*
 *                          Built-in types                            *
 *--------------------------------------------------------------------*/
typedef signed char             l_int8;
typedef unsigned char           l_uint8;
typedef short                   l_int16;
typedef unsigned short          l_uint16;
typedef int                     l_int32;
typedef unsigned int            l_uint32;
typedef float                   l_float32;
typedef double                  l_float64;


/*------------------------------------------------------------------------*
 *                            Standard macros                             *
 *------------------------------------------------------------------------*/
#ifndef L_MIN
#define L_MIN(x,y)   (((x) < (y)) ? (x) : (y))
#endif

#ifndef L_MAX
#define L_MAX(x,y)   (((x) > (y)) ? (x) : (y))
#endif

#ifndef L_ABS
#define L_ABS(x)     (((x) < 0) ? (-1 * (x)) : (x))
#endif

#ifndef L_SIGN
#define L_SIGN(x)    (((x) < 0) ? -1 : 1)
#endif

#ifndef UNDEF
#define UNDEF        -1
#endif

#ifndef NULL
#define NULL          0
#endif

#ifndef TRUE
#define TRUE          1
#endif

#ifndef FALSE
#define FALSE         0
#endif


/*--------------------------------------------------------------------*
 *         Environ variables used within compiler invocation          *
 *--------------------------------------------------------------------*/
/*
 *  To control conditional compilation, one of two variables
 *
 *       L_LITTLE_ENDIAN  (e.g., for Intel X86)
 *       L_BIG_ENDIAN     (e.g., for Sun SPARC, Mac Power PC)
 *
 *  is defined when the GCC compiler is invoked.
 *  All code should compile properly for both hardware architectures.
 */


/*------------------------------------------------------------------------*
 *                   Simple search state variables                        *
 *------------------------------------------------------------------------*/
enum {
    L_NOT_FOUND = 0,
    L_FOUND = 1
};


/*------------------------------------------------------------------------*
 *                      Standard memory allocation                        *
 *
 *  These specify the memory management functions that are used
 *  on all heap data except for Pix.  Memory management for Pix
 *  also defaults to malloc and free.  See pix1.c for details.
 *------------------------------------------------------------------------*/
#define MALLOC(blocksize)           malloc(blocksize)
#define CALLOC(numelem, elemsize)   calloc(numelem, elemsize)
#define REALLOC(ptr, blocksize)     realloc(ptr, blocksize)
#define FREE(ptr)                   free(ptr)


/*------------------------------------------------------------------------*
 *         Control printing of error, warning, and info messages         *
 *                                                                        *
 *      (Use -DNO_CONSOLE_IO on compiler line to prevent text output)     *
 *------------------------------------------------------------------------*/
#ifdef  NO_CONSOLE_IO

#define PROCNAME(name)
#define ERROR_PTR(a,b,c)            ((void *)(c))
#define ERROR_INT(a,b,c)            ((l_int32)(c))
#define ERROR_FLOAT(a,b,c)          ((l_float32)(c))
#define L_ERROR(a,b)
#define L_ERROR_STRING(a,b,c)
#define L_ERROR_INT(a,b,c)
#define L_ERROR_FLOAT(a,b,c)
#define L_WARNING(a,b)
#define L_WARNING_STRING(a,b,c)
#define L_WARNING_INT(a,b,c)
#define L_WARNING_INT2(a,b,c,d)
#define L_WARNING_FLOAT(a,b,c)
#define L_WARNING_FLOAT2(a,b,c,d)
#define L_INFO(a,b)
#define L_INFO_STRING(a,b,c)
#define L_INFO_INT(a,b,c)
#define L_INFO_INT2(a,b,c,d)
#define L_INFO_FLOAT(a,b,c)
#define L_INFO_FLOAT2(a,b,c,d)

#else

#define PROCNAME(name)              static const char procName[] = name
#define ERROR_PTR(a,b,c)            returnErrorPtr((a),(b),(c))
#define ERROR_INT(a,b,c)            returnErrorInt((a),(b),(c))
#define ERROR_FLOAT(a,b,c)          returnErrorFloat((a),(b),(c))
#define L_ERROR(a,b)                l_error((a),(b))
#define L_ERROR_STRING(a,b,c)       l_errorString((a),(b),(c))
#define L_ERROR_INT(a,b,c)          l_errorInt((a),(b),(c))
#define L_ERROR_FLOAT(a,b,c)        l_errorFloat((a),(b),(c))
#define L_WARNING(a,b)              l_warning((a),(b))
#define L_WARNING_STRING(a,b,c)     l_warningString((a),(b),(c))
#define L_WARNING_INT(a,b,c)        l_warningInt((a),(b),(c))
#define L_WARNING_INT2(a,b,c,d)     l_warningInt2((a),(b),(c),(d))
#define L_WARNING_FLOAT(a,b,c)      l_warningFloat((a),(b),(c))
#define L_WARNING_FLOAT2(a,b,c,d)   l_warningFloat2((a),(b),(c),(d))
#define L_INFO(a,b)                 l_info((a),(b))
#define L_INFO_STRING(a,b,c)        l_infoString((a),(b),(c))
#define L_INFO_INT(a,b,c)           l_infoInt((a),(b),(c))
#define L_INFO_INT2(a,b,c,d)        l_infoInt2((a),(b),(c),(d))
#define L_INFO_FLOAT(a,b,c)         l_infoFloat((a),(b),(c))
#define L_INFO_FLOAT2(a,b,c,d)      l_infoFloat2((a),(b),(c),(d))

#endif  /* NO_CONSOLE_IO */


/*------------------------------------------------------------------------*
 *                        snprintf() renamed in MSVC                      *
 *------------------------------------------------------------------------*/
#ifdef _MSC_VER
#define snprintf(buf, size, ...)  _snprintf_s(buf, size, _TRUNCATE, __VA_ARGS__)
#endif


#endif /* LEPTONICA_ENVIRON_H */
