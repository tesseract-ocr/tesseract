##### http://autoconf-archive.cryp.to/ax_create_stdint_h.html
#
# SYNOPSIS
#
#   AX_CREATE_STDINT_H [( HEADER-TO-GENERATE [, HEDERS-TO-CHECK])]
#
# DESCRIPTION
#
#   the "ISO C9X: 7.18 Integer types <stdint.h>" section requires the
#   existence of an include file <stdint.h> that defines a set of
#   typedefs, especially uint8_t,int32_t,uintptr_t. Many older
#   installations will not provide this file, but some will have the
#   very same definitions in <inttypes.h>. In other enviroments we can
#   use the inet-types in <sys/types.h> which would define the typedefs
#   int8_t and u_int8_t respectivly.
#
#   This macros will create a local "_stdint.h" or the headerfile given
#   as an argument. In many cases that file will just "#include
#   <stdint.h>" or "#include <inttypes.h>", while in other environments
#   it will provide the set of basic 'stdint's definitions/typedefs:
#
#     int8_t,uint8_t,int16_t,uint16_t,int32_t,uint32_t,intptr_t,uintptr_t
#     int_least32_t.. int_fast32_t.. intmax_t
#
#   which may or may not rely on the definitions of other files, or
#   using the AC_CHECK_SIZEOF macro to determine the actual sizeof each
#   type.
#
#   if your header files require the stdint-types you will want to
#   create an installable file mylib-int.h that all your other
#   installable header may include. So if you have a library package
#   named "mylib", just use
#
#        AX_CREATE_STDINT_H(mylib-int.h)
#
#   in configure.ac and go to install that very header file in
#   Makefile.am along with the other headers (mylib.h) - and the
#   mylib-specific headers can simply use "#include <mylib-int.h>" to
#   obtain the stdint-types.
#
#   Remember, if the system already had a valid <stdint.h>, the
#   generated file will include it directly. No need for fuzzy
#   HAVE_STDINT_H things... (oops, GCC 4.2.x has deliberatly disabled
#   its stdint.h for non-c99 compilation and the c99-mode is not the
#   default. Therefore this macro will not use the compiler's stdint.h
#   - please complain to the GCC developers).
#
# LAST MODIFICATION
#
#   2006-10-13
#
# COPYLEFT
#
#   Copyright (c) 2006 Guido U. Draheim <guidod@gmx.de>
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation; either version 2 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#   02111-1307, USA.
#
#   As a special exception, the respective Autoconf Macro's copyright
#   owner gives unlimited permission to copy, distribute and modify the
#   configure scripts that are the output of Autoconf when processing
#   the Macro. You need not follow the terms of the GNU General Public
#   License when using or distributing such scripts, even though
#   portions of the text of the Macro appear in them. The GNU General
#   Public License (GPL) does govern all other use of the material that
#   constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the
#   Autoconf Macro released by the Autoconf Macro Archive. When you
#   make and distribute a modified version of the Autoconf Macro, you
#   may extend this special exception to the GPL to apply to your
#   modified version as well.

AC_DEFUN([AX_CHECK_DATA_MODEL],[
   AC_CHECK_SIZEOF(char)
   AC_CHECK_SIZEOF(short)
   AC_CHECK_SIZEOF(int)
   AC_CHECK_SIZEOF(long)
   AC_CHECK_SIZEOF(void*)
   ac_cv_char_data_model=""
   ac_cv_char_data_model="$ac_cv_char_data_model$ac_cv_sizeof_char"
   ac_cv_char_data_model="$ac_cv_char_data_model$ac_cv_sizeof_short"
   ac_cv_char_data_model="$ac_cv_char_data_model$ac_cv_sizeof_int"
   ac_cv_long_data_model=""
   ac_cv_long_data_model="$ac_cv_long_data_model$ac_cv_sizeof_int"
   ac_cv_long_data_model="$ac_cv_long_data_model$ac_cv_sizeof_long"
   ac_cv_long_data_model="$ac_cv_long_data_model$ac_cv_sizeof_voidp"
   AC_MSG_CHECKING([data model])
   case "$ac_cv_char_data_model/$ac_cv_long_data_model" in
    122/242)     ac_cv_data_model="IP16"  ; n="standard 16bit machine" ;;
    122/244)     ac_cv_data_model="LP32"  ; n="standard 32bit machine" ;;
    122/*)       ac_cv_data_model="i16"   ; n="unusual int16 model" ;;
    124/444)     ac_cv_data_model="ILP32" ; n="standard 32bit unixish" ;;
    124/488)     ac_cv_data_model="LP64"  ; n="standard 64bit unixish" ;;
    124/448)     ac_cv_data_model="LLP64" ; n="unusual 64bit unixish" ;;
    124/*)       ac_cv_data_model="i32"   ; n="unusual int32 model" ;;
    128/888)     ac_cv_data_model="ILP64" ; n="unusual 64bit numeric" ;;
    128/*)       ac_cv_data_model="i64"   ; n="unusual int64 model" ;;
    222/*2)      ac_cv_data_model="DSP16" ; n="strict 16bit dsptype" ;;
    333/*3)      ac_cv_data_model="DSP24" ; n="strict 24bit dsptype" ;;
    444/*4)      ac_cv_data_model="DSP32" ; n="strict 32bit dsptype" ;;
    666/*6)      ac_cv_data_model="DSP48" ; n="strict 48bit dsptype" ;;
    888/*8)      ac_cv_data_model="DSP64" ; n="strict 64bit dsptype" ;;
    222/*|333/*|444/*|666/*|888/*) :
                 ac_cv_data_model="iDSP"  ; n="unusual dsptype" ;;
     *)          ac_cv_data_model="none"  ; n="very unusual model" ;;
   esac
   AC_MSG_RESULT([$ac_cv_data_model ($ac_cv_long_data_model, $n)])
])

dnl AX_CHECK_HEADER_STDINT_X([HEADERLIST][,ACTION-IF])
AC_DEFUN([AX_CHECK_HEADER_STDINT_X],[
AC_CACHE_CHECK([for stdint uintptr_t], [ac_cv_header_stdint_x],[
 ac_cv_header_stdint_x="" # the 1997 typedefs (inttypes.h)
  AC_MSG_RESULT([(..)])
  for i in m4_ifval([$1],[$1],[stdint.h inttypes.h sys/inttypes.h sys/types.h])
  do
   unset ac_cv_type_uintptr_t
   unset ac_cv_type_uint64_t
   AC_CHECK_TYPE(uintptr_t,[ac_cv_header_stdint_x=$i],continue,[#include <$i>])
   AC_CHECK_TYPE(uint64_t,[and64="/uint64_t"],[and64=""],[#include<$i>])
   m4_ifvaln([$1],[$1]) break
  done
  AC_MSG_CHECKING([for stdint uintptr_t])
 ])
])

AC_DEFUN([AX_CHECK_HEADER_STDINT_O],[
AC_CACHE_CHECK([for stdint uint32_t], [ac_cv_header_stdint_o],[
 ac_cv_header_stdint_o="" # the 1995 typedefs (sys/inttypes.h)
  AC_MSG_RESULT([(..)])
  for i in m4_ifval([$1],[$1],[inttypes.h sys/inttypes.h sys/types.h stdint.h])
  do
   unset ac_cv_type_uint32_t
   unset ac_cv_type_uint64_t
   AC_CHECK_TYPE(uint32_t,[ac_cv_header_stdint_o=$i],continue,[#include <$i>])
   AC_CHECK_TYPE(uint64_t,[and64="/uint64_t"],[and64=""],[#include<$i>])
   m4_ifvaln([$1],[$1]) break
   break;
  done
  AC_MSG_CHECKING([for stdint uint32_t])
 ])
])

AC_DEFUN([AX_CHECK_HEADER_STDINT_U],[
AC_CACHE_CHECK([for stdint u_int32_t], [ac_cv_header_stdint_u],[
 ac_cv_header_stdint_u="" # the BSD typedefs (sys/types.h)
  AC_MSG_RESULT([(..)])
  for i in m4_ifval([$1],[$1],[sys/types.h inttypes.h sys/inttypes.h]) ; do
   unset ac_cv_type_u_int32_t
   unset ac_cv_type_u_int64_t
   AC_CHECK_TYPE(u_int32_t,[ac_cv_header_stdint_u=$i],continue,[#include <$i>])
   AC_CHECK_TYPE(u_int64_t,[and64="/u_int64_t"],[and64=""],[#include<$i>])
   m4_ifvaln([$1],[$1]) break
   break;
  done
  AC_MSG_CHECKING([for stdint u_int32_t])
 ])
])

AC_DEFUN([AX_CREATE_STDINT_H],
[# ------ AX CREATE STDINT H -------------------------------------
AC_MSG_CHECKING([for stdint types])
ac_stdint_h=`echo ifelse($1, , _stdint.h, $1)`
# try to shortcircuit - if the default include path of the compiler
# can find a "stdint.h" header then we assume that all compilers can.
AC_CACHE_VAL([ac_cv_header_stdint_t],[
old_CXXFLAGS="$CXXFLAGS" ; CXXFLAGS=""
old_CPPFLAGS="$CPPFLAGS" ; CPPFLAGS=""
old_CFLAGS="$CFLAGS"     ; CFLAGS=""
AC_TRY_COMPILE([#include <stdint.h>],[int_least32_t v = 0;],
[ac_cv_stdint_result="(assuming C99 compatible system)"
 ac_cv_header_stdint_t="stdint.h"; ],
[ac_cv_header_stdint_t=""])
if test "$GCC" = "yes" && test ".$ac_cv_header_stdint_t" = "."; then
CFLAGS="-std=c99"
AC_TRY_COMPILE([#include <stdint.h>],[int_least32_t v = 0;],
[AC_MSG_WARN(your GCC compiler has a defunct stdint.h for its default-mode)])
fi
CXXFLAGS="$old_CXXFLAGS"
CPPFLAGS="$old_CPPFLAGS"
CFLAGS="$old_CFLAGS" ])

v="... $ac_cv_header_stdint_h"
if test "$ac_stdint_h" = "stdint.h" ; then
 AC_MSG_RESULT([(are you sure you want them in ./stdint.h?)])
elif test "$ac_stdint_h" = "inttypes.h" ; then
 AC_MSG_RESULT([(are you sure you want them in ./inttypes.h?)])
elif test "_$ac_cv_header_stdint_t" = "_" ; then
 AC_MSG_RESULT([(putting them into $ac_stdint_h)$v])
else
 ac_cv_header_stdint="$ac_cv_header_stdint_t"
 AC_MSG_RESULT([$ac_cv_header_stdint (shortcircuit)])
fi

if test "_$ac_cv_header_stdint_t" = "_" ; then # can not shortcircuit..

dnl .....intro message done, now do a few system checks.....
dnl btw, all old CHECK_TYPE macros do automatically "DEFINE" a type,
dnl therefore we use the autoconf implementation detail CHECK_TYPE_NEW
dnl instead that is triggered with 3 or more arguments (see types.m4)

inttype_headers=`echo $2 | sed -e 's/,/ /g'`

ac_cv_stdint_result="(no helpful system typedefs seen)"
AX_CHECK_HEADER_STDINT_X(dnl
   stdint.h inttypes.h sys/inttypes.h $inttype_headers,
   ac_cv_stdint_result="(seen uintptr_t$and64 in $i)")

if test "_$ac_cv_header_stdint_x" = "_" ; then
AX_CHECK_HEADER_STDINT_O(dnl,
   inttypes.h sys/inttypes.h stdint.h $inttype_headers,
   ac_cv_stdint_result="(seen uint32_t$and64 in $i)")
fi

if test "_$ac_cv_header_stdint_x" = "_" ; then
if test "_$ac_cv_header_stdint_o" = "_" ; then
AX_CHECK_HEADER_STDINT_U(dnl,
   sys/types.h inttypes.h sys/inttypes.h $inttype_headers,
   ac_cv_stdint_result="(seen u_int32_t$and64 in $i)")
fi fi

dnl if there was no good C99 header file, do some typedef checks...
if test "_$ac_cv_header_stdint_x" = "_" ; then
   AC_MSG_CHECKING([for stdint datatype model])
   AC_MSG_RESULT([(..)])
   AX_CHECK_DATA_MODEL
fi

if test "_$ac_cv_header_stdint_x" != "_" ; then
   ac_cv_header_stdint="$ac_cv_header_stdint_x"
elif  test "_$ac_cv_header_stdint_o" != "_" ; then
   ac_cv_header_stdint="$ac_cv_header_stdint_o"
elif  test "_$ac_cv_header_stdint_u" != "_" ; then
   ac_cv_header_stdint="$ac_cv_header_stdint_u"
else
   ac_cv_header_stdint="stddef.h"
fi

AC_MSG_CHECKING([for extra inttypes in chosen header])
AC_MSG_RESULT([($ac_cv_header_stdint)])
dnl see if int_least and int_fast types are present in _this_ header.
unset ac_cv_type_int_least32_t
unset ac_cv_type_int_fast32_t
AC_CHECK_TYPE(int_least32_t,,,[#include <$ac_cv_header_stdint>])
AC_CHECK_TYPE(int_fast32_t,,,[#include<$ac_cv_header_stdint>])
AC_CHECK_TYPE(intmax_t,,,[#include <$ac_cv_header_stdint>])

fi # shortcircut to system "stdint.h"
# ------------------ PREPARE VARIABLES ------------------------------
if test "$GCC" = "yes" ; then
ac_cv_stdint_message="using gnu compiler "`$CC --version | head -1`
else
ac_cv_stdint_message="using $CC"
fi

AC_MSG_RESULT([make use of $ac_cv_header_stdint in $ac_stdint_h dnl
$ac_cv_stdint_result])

dnl -----------------------------------------------------------------
# ----------------- DONE inttypes.h checks START header -------------
AC_CONFIG_COMMANDS([$ac_stdint_h],[
AC_MSG_NOTICE(creating $ac_stdint_h : $_ac_stdint_h)
ac_stdint=$tmp/_stdint.h

echo "#ifndef" $_ac_stdint_h >$ac_stdint
echo "#define" $_ac_stdint_h "1" >>$ac_stdint
echo "#ifndef" _GENERATED_STDINT_H >>$ac_stdint
echo "#define" _GENERATED_STDINT_H '"'$PACKAGE $VERSION'"' >>$ac_stdint
echo "/* generated $ac_cv_stdint_message */" >>$ac_stdint
if test "_$ac_cv_header_stdint_t" != "_" ; then
echo "#define _STDINT_HAVE_STDINT_H" "1" >>$ac_stdint
echo "#include <stdint.h>" >>$ac_stdint
echo "#endif" >>$ac_stdint
echo "#endif" >>$ac_stdint
else

cat >>$ac_stdint <<STDINT_EOF

/* ................... shortcircuit part ........................... */

#if defined HAVE_STDINT_H || defined _STDINT_HAVE_STDINT_H
#include <stdint.h>
#else
#include <stddef.h>

/* .................... configured part ............................ */

STDINT_EOF

echo "/* whether we have a C99 compatible stdint header file */" >>$ac_stdint
if test "_$ac_cv_header_stdint_x" != "_" ; then
  ac_header="$ac_cv_header_stdint_x"
  echo "#define _STDINT_HEADER_INTPTR" '"'"$ac_header"'"' >>$ac_stdint
else
  echo "/* #undef _STDINT_HEADER_INTPTR */" >>$ac_stdint
fi

echo "/* whether we have a C96 compatible inttypes header file */" >>$ac_stdint
if  test "_$ac_cv_header_stdint_o" != "_" ; then
  ac_header="$ac_cv_header_stdint_o"
  echo "#define _STDINT_HEADER_UINT32" '"'"$ac_header"'"' >>$ac_stdint
else
  echo "/* #undef _STDINT_HEADER_UINT32 */" >>$ac_stdint
fi

echo "/* whether we have a BSD compatible inet types header */" >>$ac_stdint
if  test "_$ac_cv_header_stdint_u" != "_" ; then
  ac_header="$ac_cv_header_stdint_u"
  echo "#define _STDINT_HEADER_U_INT32" '"'"$ac_header"'"' >>$ac_stdint
else
  echo "/* #undef _STDINT_HEADER_U_INT32 */" >>$ac_stdint
fi

echo "" >>$ac_stdint

if test "_$ac_header" != "_" ; then if test "$ac_header" != "stddef.h" ; then
  echo "#include <$ac_header>" >>$ac_stdint
  echo "" >>$ac_stdint
fi fi

echo "/* which 64bit typedef has been found */" >>$ac_stdint
if test "$ac_cv_type_uint64_t" = "yes" ; then
echo "#define   _STDINT_HAVE_UINT64_T" "1"  >>$ac_stdint
else
echo "/* #undef _STDINT_HAVE_UINT64_T */" >>$ac_stdint
fi
if test "$ac_cv_type_u_int64_t" = "yes" ; then
echo "#define   _STDINT_HAVE_U_INT64_T" "1"  >>$ac_stdint
else
echo "/* #undef _STDINT_HAVE_U_INT64_T */" >>$ac_stdint
fi
echo "" >>$ac_stdint

echo "/* which type model has been detected */" >>$ac_stdint
if test "_$ac_cv_char_data_model" != "_" ; then
echo "#define   _STDINT_CHAR_MODEL" "$ac_cv_char_data_model" >>$ac_stdint
echo "#define   _STDINT_LONG_MODEL" "$ac_cv_long_data_model" >>$ac_stdint
else
echo "/* #undef _STDINT_CHAR_MODEL // skipped */" >>$ac_stdint
echo "/* #undef _STDINT_LONG_MODEL // skipped */" >>$ac_stdint
fi
echo "" >>$ac_stdint

echo "/* whether int_least types were detected */" >>$ac_stdint
if test "$ac_cv_type_int_least32_t" = "yes"; then
echo "#define   _STDINT_HAVE_INT_LEAST32_T" "1"  >>$ac_stdint
else
echo "/* #undef _STDINT_HAVE_INT_LEAST32_T */" >>$ac_stdint
fi
echo "/* whether int_fast types were detected */" >>$ac_stdint
if test "$ac_cv_type_int_fast32_t" = "yes"; then
echo "#define   _STDINT_HAVE_INT_FAST32_T" "1" >>$ac_stdint
else
echo "/* #undef _STDINT_HAVE_INT_FAST32_T */" >>$ac_stdint
fi
echo "/* whether intmax_t type was detected */" >>$ac_stdint
if test "$ac_cv_type_intmax_t" = "yes"; then
echo "#define   _STDINT_HAVE_INTMAX_T" "1" >>$ac_stdint
else
echo "/* #undef _STDINT_HAVE_INTMAX_T */" >>$ac_stdint
fi
echo "" >>$ac_stdint

  cat >>$ac_stdint <<STDINT_EOF
/* .................... detections part ............................ */

/* whether we need to define bitspecific types from compiler base types */
#ifndef _STDINT_HEADER_INTPTR
#ifndef _STDINT_HEADER_UINT32
#ifndef _STDINT_HEADER_U_INT32
#define _STDINT_NEED_INT_MODEL_T
#else
#define _STDINT_HAVE_U_INT_TYPES
#endif
#endif
#endif

#ifdef _STDINT_HAVE_U_INT_TYPES
#undef _STDINT_NEED_INT_MODEL_T
#endif

#ifdef  _STDINT_CHAR_MODEL
#if     _STDINT_CHAR_MODEL+0 == 122 || _STDINT_CHAR_MODEL+0 == 124
#ifndef _STDINT_BYTE_MODEL
#define _STDINT_BYTE_MODEL 12
#endif
#endif
#endif

#ifndef _STDINT_HAVE_INT_LEAST32_T
#define _STDINT_NEED_INT_LEAST_T
#endif

#ifndef _STDINT_HAVE_INT_FAST32_T
#define _STDINT_NEED_INT_FAST_T
#endif

#ifndef _STDINT_HEADER_INTPTR
#define _STDINT_NEED_INTPTR_T
#ifndef _STDINT_HAVE_INTMAX_T
#define _STDINT_NEED_INTMAX_T
#endif
#endif


/* .................... definition part ............................ */

/* some system headers have good uint64_t */
#ifndef _HAVE_UINT64_T
#if     defined _STDINT_HAVE_UINT64_T  || defined HAVE_UINT64_T
#define _HAVE_UINT64_T
#elif   defined _STDINT_HAVE_U_INT64_T || defined HAVE_U_INT64_T
#define _HAVE_UINT64_T
typedef u_int64_t uint64_t;
#endif
#endif

#ifndef _HAVE_UINT64_T
/* .. here are some common heuristics using compiler runtime specifics */
#if defined __STDC_VERSION__ && defined __STDC_VERSION__ >= 199901L
#define _HAVE_UINT64_T
#define _HAVE_LONGLONG_UINT64_T
typedef long long int64_t;
typedef unsigned long long uint64_t;

#elif !defined __STRICT_ANSI__
#if defined _MSC_VER || defined __WATCOMC__ || defined __BORLANDC__
#define _HAVE_UINT64_T
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#elif defined __GNUC__ || defined __MWERKS__ || defined __ELF__
/* note: all ELF-systems seem to have loff-support which needs 64-bit */
#if !defined _NO_LONGLONG
#define _HAVE_UINT64_T
#define _HAVE_LONGLONG_UINT64_T
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

#elif defined __alpha || (defined __mips && defined _ABIN32)
#if !defined _NO_LONGLONG
typedef long int64_t;
typedef unsigned long uint64_t;
#endif
  /* compiler/cpu type to define int64_t */
#endif
#endif
#endif

#if defined _STDINT_HAVE_U_INT_TYPES
/* int8_t int16_t int32_t defined by inet code, redeclare the u_intXX types */
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;

/* glibc compatibility */
#ifndef __int8_t_defined
#define __int8_t_defined
#endif
#endif

#ifdef _STDINT_NEED_INT_MODEL_T
/* we must guess all the basic types. Apart from byte-adressable system, */
/* there a few 32-bit-only dsp-systems that we guard with BYTE_MODEL 8-} */
/* (btw, those nibble-addressable systems are way off, or so we assume) */

dnl   /* have a look at "64bit and data size neutrality" at */
dnl   /* http://unix.org/version2/whatsnew/login_64bit.html */
dnl   /* (the shorthand "ILP" types always have a "P" part) */

#if defined _STDINT_BYTE_MODEL
#if _STDINT_LONG_MODEL+0 == 242
/* 2:4:2 =  IP16 = a normal 16-bit system                */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned long   uint32_t;
#ifndef __int8_t_defined
#define __int8_t_defined
typedef          char    int8_t;
typedef          short   int16_t;
typedef          long    int32_t;
#endif
#elif _STDINT_LONG_MODEL+0 == 244 || _STDINT_LONG_MODEL == 444
/* 2:4:4 =  LP32 = a 32-bit system derived from a 16-bit */
/* 4:4:4 = ILP32 = a normal 32-bit system                */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
#ifndef __int8_t_defined
#define __int8_t_defined
typedef          char    int8_t;
typedef          short   int16_t;
typedef          int     int32_t;
#endif
#elif _STDINT_LONG_MODEL+0 == 484 || _STDINT_LONG_MODEL+0 == 488
/* 4:8:4 =  IP32 = a 32-bit system prepared for 64-bit    */
/* 4:8:8 =  LP64 = a normal 64-bit system                 */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
#ifndef __int8_t_defined
#define __int8_t_defined
typedef          char    int8_t;
typedef          short   int16_t;
typedef          int     int32_t;
#endif
/* this system has a "long" of 64bit */
#ifndef _HAVE_UINT64_T
#define _HAVE_UINT64_T
typedef unsigned long   uint64_t;
typedef          long    int64_t;
#endif
#elif _STDINT_LONG_MODEL+0 == 448
/*      LLP64   a 64-bit system derived from a 32-bit system */
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
#ifndef __int8_t_defined
#define __int8_t_defined
typedef          char    int8_t;
typedef          short   int16_t;
typedef          int     int32_t;
#endif
/* assuming the system has a "long long" */
#ifndef _HAVE_UINT64_T
#define _HAVE_UINT64_T
#define _HAVE_LONGLONG_UINT64_T
typedef unsigned long long uint64_t;
typedef          long long  int64_t;
#endif
#else
#define _STDINT_NO_INT32_T
#endif
#else
#define _STDINT_NO_INT8_T
#define _STDINT_NO_INT32_T
#endif
#endif

/*
 * quote from SunOS-5.8 sys/inttypes.h:
 * Use at your own risk.  As of February 1996, the committee is squarely
 * behind the fixed sized types; the "least" and "fast" types are still being
 * discussed.  The probability that the "fast" types may be removed before
 * the standard is finalized is high enough that they are not currently
 * implemented.
 */

#if defined _STDINT_NEED_INT_LEAST_T
typedef  int8_t    int_least8_t;
typedef  int16_t   int_least16_t;
typedef  int32_t   int_least32_t;
#ifdef _HAVE_UINT64_T
typedef  int64_t   int_least64_t;
#endif

typedef uint8_t   uint_least8_t;
typedef uint16_t  uint_least16_t;
typedef uint32_t  uint_least32_t;
#ifdef _HAVE_UINT64_T
typedef uint64_t  uint_least64_t;
#endif
  /* least types */
#endif

#if defined _STDINT_NEED_INT_FAST_T
typedef  int8_t    int_fast8_t;
typedef  int       int_fast16_t;
typedef  int32_t   int_fast32_t;
#ifdef _HAVE_UINT64_T
typedef  int64_t   int_fast64_t;
#endif

typedef uint8_t   uint_fast8_t;
typedef unsigned  uint_fast16_t;
typedef uint32_t  uint_fast32_t;
#ifdef _HAVE_UINT64_T
typedef uint64_t  uint_fast64_t;
#endif
  /* fast types */
#endif

#ifdef _STDINT_NEED_INTMAX_T
#ifdef _HAVE_UINT64_T
typedef  int64_t       intmax_t;
typedef uint64_t      uintmax_t;
#else
typedef          long  intmax_t;
typedef unsigned long uintmax_t;
#endif
#endif

#ifdef _STDINT_NEED_INTPTR_T
#ifndef __intptr_t_defined
#define __intptr_t_defined
/* we encourage using "long" to store pointer values, never use "int" ! */
#if   _STDINT_LONG_MODEL+0 == 242 || _STDINT_LONG_MODEL+0 == 484
typedef  unsigned int   uintptr_t;
typedef           int    intptr_t;
#elif _STDINT_LONG_MODEL+0 == 244 || _STDINT_LONG_MODEL+0 == 444
typedef  unsigned long  uintptr_t;
typedef           long   intptr_t;
#elif _STDINT_LONG_MODEL+0 == 448 && defined _HAVE_UINT64_T
typedef        uint64_t uintptr_t;
typedef         int64_t  intptr_t;
#else /* matches typical system types ILP32 and LP64 - but not IP16 or LLP64 */
typedef  unsigned long  uintptr_t;
typedef           long   intptr_t;
#endif
#endif
#endif

/* The ISO C99 standard specifies that in C++ implementations these
   should only be defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_CONSTANT_MACROS
#ifndef UINT32_C

/* Signed.  */
# define INT8_C(c)      c
# define INT16_C(c)     c
# define INT32_C(c)     c
# ifdef _HAVE_LONGLONG_UINT64_T
#  define INT64_C(c)    c ## L
# else
#  define INT64_C(c)    c ## LL
# endif

/* Unsigned.  */
# define UINT8_C(c)     c ## U
# define UINT16_C(c)    c ## U
# define UINT32_C(c)    c ## U
# ifdef _HAVE_LONGLONG_UINT64_T
#  define UINT64_C(c)   c ## UL
# else
#  define UINT64_C(c)   c ## ULL
# endif

/* Maximal type.  */
# ifdef _HAVE_LONGLONG_UINT64_T
#  define INTMAX_C(c)   c ## L
#  define UINTMAX_C(c)  c ## UL
# else
#  define INTMAX_C(c)   c ## LL
#  define UINTMAX_C(c)  c ## ULL
# endif

  /* literalnumbers */
#endif
#endif

/* These limits are merily those of a two complement byte-oriented system */

/* Minimum of signed integral types.  */
# define INT8_MIN               (-128)
# define INT16_MIN              (-32767-1)
# define INT32_MIN              (-2147483647-1)
# define INT64_MIN              (-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types.  */
# define INT8_MAX               (127)
# define INT16_MAX              (32767)
# define INT32_MAX              (2147483647)
# define INT64_MAX              (__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
# define UINT8_MAX              (255)
# define UINT16_MAX             (65535)
# define UINT32_MAX             (4294967295U)
# define UINT64_MAX             (__UINT64_C(18446744073709551615))

/* Minimum of signed integral types having a minimum size.  */
# define INT_LEAST8_MIN         INT8_MIN
# define INT_LEAST16_MIN        INT16_MIN
# define INT_LEAST32_MIN        INT32_MIN
# define INT_LEAST64_MIN        INT64_MIN
/* Maximum of signed integral types having a minimum size.  */
# define INT_LEAST8_MAX         INT8_MAX
# define INT_LEAST16_MAX        INT16_MAX
# define INT_LEAST32_MAX        INT32_MAX
# define INT_LEAST64_MAX        INT64_MAX

/* Maximum of unsigned integral types having a minimum size.  */
# define UINT_LEAST8_MAX        UINT8_MAX
# define UINT_LEAST16_MAX       UINT16_MAX
# define UINT_LEAST32_MAX       UINT32_MAX
# define UINT_LEAST64_MAX       UINT64_MAX

  /* shortcircuit*/
#endif
  /* once */
#endif
#endif
STDINT_EOF
fi
    if cmp -s $ac_stdint_h $ac_stdint 2>/dev/null; then
      AC_MSG_NOTICE([$ac_stdint_h is unchanged])
    else
      ac_dir=`AS_DIRNAME(["$ac_stdint_h"])`
      AS_MKDIR_P(["$ac_dir"])
      rm -f $ac_stdint_h
      mv $ac_stdint $ac_stdint_h
    fi
],[# variables for create stdint.h replacement
PACKAGE="$PACKAGE"
VERSION="$VERSION"
ac_stdint_h="$ac_stdint_h"
_ac_stdint_h=AS_TR_CPP(_$PACKAGE-$ac_stdint_h)
ac_cv_stdint_message="$ac_cv_stdint_message"
ac_cv_header_stdint_t="$ac_cv_header_stdint_t"
ac_cv_header_stdint_x="$ac_cv_header_stdint_x"
ac_cv_header_stdint_o="$ac_cv_header_stdint_o"
ac_cv_header_stdint_u="$ac_cv_header_stdint_u"
ac_cv_type_uint64_t="$ac_cv_type_uint64_t"
ac_cv_type_u_int64_t="$ac_cv_type_u_int64_t"
ac_cv_char_data_model="$ac_cv_char_data_model"
ac_cv_long_data_model="$ac_cv_long_data_model"
ac_cv_type_int_least32_t="$ac_cv_type_int_least32_t"
ac_cv_type_int_fast32_t="$ac_cv_type_int_fast32_t"
ac_cv_type_intmax_t="$ac_cv_type_intmax_t"
])
])
