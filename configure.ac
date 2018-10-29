# -*-Shell-script-*-
#
# Copyright (c) Luc Vincent

# ----------------------------------------
# Initialization
# ----------------------------------------
AC_PREREQ([2.63])
AC_INIT([tesseract],
        [m4_esyscmd_s([test -d .git && git describe --abbrev=4 || cat VERSION])],
        [https://github.com/tesseract-ocr/tesseract/issues],,
        [https://github.com/tesseract-ocr/tesseract/])
AC_PROG_CXX([g++ clang++])
AC_LANG([C++])
AC_LANG_COMPILER_REQUIRE
CXXFLAGS=${CXXFLAGS:-""}
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/api/tesseractmain.cpp])
AC_PREFIX_DEFAULT([/usr/local])

# Automake configuration. Do not require README file (we use README.md).
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Define date of package, etc. Could be useful in auto-generated
# documentation.
PACKAGE_YEAR=2018
PACKAGE_DATE="10/29"

abs_top_srcdir=`AS_DIRNAME([$0])`

AC_DEFINE_UNQUOTED([PACKAGE_NAME], ["${PACKAGE_NAME}"], [Name of package])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION], ["${PACKAGE_VERSION}"], [Version number])
AC_DEFINE_UNQUOTED([PACKAGE_YEAR], ["$PACKAGE_YEAR"], [Official year for this release])
AC_DEFINE_UNQUOTED([PACKAGE_DATE], ["$PACKAGE_DATE"], [Official date of release])

AC_SUBST([PACKAGE_NAME])
AC_SUBST([PACKAGE_VERSION])
AC_SUBST([PACKAGE_YEAR])
AC_SUBST([PACKAGE_DATE])

GENERIC_LIBRARY_NAME=tesseract

# Release versioning. Get versions from PACKAGE_VERSION.
AX_SPLIT_VERSION
GENERIC_MAJOR_VERSION=$(echo "$AX_MAJOR_VERSION" | $SED 's/^[[^0-9]]*//')
GENERIC_MINOR_VERSION=$AX_MINOR_VERSION
GENERIC_MICRO_VERSION=`echo "$AX_POINT_VERSION" | $SED 's/^\([[0-9]][[0-9]]*\).*/\1/'`

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
GENERIC_LIBRARY_VERSION=$GENERIC_MAJOR_VERSION:$GENERIC_MINOR_VERSION
AC_SUBST([GENERIC_API_VERSION])
AC_SUBST([GENERIC_MAJOR_VERSION])
AC_SUBST([GENERIC_MINOR_VERSION])
AC_SUBST([GENERIC_MICRO_VERSION])

AC_SUBST([GENERIC_LIBRARY_VERSION])
PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST([GENERIC_LIBRARY_NAME])

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST([GENERIC_RELEASE])
AC_SUBST([GENERIC_VERSION])

AC_CONFIG_HEADERS([config_auto.h:config/config.h.in])

# default conditional
AM_CONDITIONAL([T_WIN], false)
AM_CONDITIONAL([OSX], false)
AM_CONDITIONAL([GRAPHICS_DISABLED], false)
AC_SUBST([AM_CPPFLAGS])

OPENCL_INC="/opt/AMDAPP/include"
OPENCL_LIBS="-lOpenCL"
#############################
#
# Platform specific setup
#
#############################
AC_CANONICAL_HOST
case "${host_os}" in
    mingw*)
        AM_CONDITIONAL([T_WIN], true)
        AM_CONDITIONAL([ADD_RT], false)
        AC_SUBST([AM_LDFLAGS], ['-Wl,-no-undefined -Wl,--as-needed'])
        ;;
    cygwin*)
        AM_CONDITIONAL([ADD_RT], false)
        AC_SUBST([NOUNDEFINED], ['-no-undefined'])
        ;;
    solaris*)
        LIBS="-lsocket -lnsl -lrt -lxnet"
        AM_CONDITIONAL([ADD_RT], true)
        ;;
    *darwin*)
        OPENCL_LIBS=""
        OPENCL_INC=""
        AM_CONDITIONAL([ADD_RT], false)
        ;;
    powerpc-*-darwin*)
        OPENCL_LIBS=""
        ;;
    *)
        # default
        AM_CONDITIONAL([ADD_RT], true)
        ;;
esac

## Checks for supported compiler options.
AM_CONDITIONAL([AVX_OPT], false)
AM_CONDITIONAL([AVX2_OPT], false)
AM_CONDITIONAL([SSE41_OPT], false)

AX_CHECK_COMPILE_FLAG([-mavx], [avx=true], [avx=false])
if $avx; then
    AM_CONDITIONAL([AVX_OPT], true)
fi

AX_CHECK_COMPILE_FLAG([-mavx2], [avx2=true], [avx2=false])
if $avx2; then
    AM_CONDITIONAL([AVX2_OPT], true)
fi

AX_CHECK_COMPILE_FLAG([-msse4.1], [sse41=true], [sse41=false])
if $sse41; then
    AM_CONDITIONAL([SSE41_OPT], true)
fi

AC_ARG_WITH([extra-includes],
            [AS_HELP_STRING([--with-extra-includes=DIR],
                       [Define an additional directory for include files])],
            [if test -d "$withval" ; then
               CFLAGS="$CFLAGS -I$withval"
             else
               AC_MSG_ERROR([Cannot stat directory $withval])
             fi])

AC_ARG_WITH([extra-libraries],
            [AS_HELP_STRING([--with-extra-libraries=DIR],
                       [Define an additional directory for library files])],
            [if test -d "$withval" ; then
              LDFLAGS="$LDFLAGS -L$withval"
             else
               AC_MSG_ERROR([Cannot stat directory $withval])
             fi])

AC_MSG_CHECKING([--enable-graphics argument])
AC_ARG_ENABLE([graphics],
  AS_HELP_STRING([--disable-graphics], [disable graphics (ScrollView)]))
AC_MSG_RESULT([$enable_graphics])
if test "$enable_graphics" = "no"; then
  AC_DEFINE([GRAPHICS_DISABLED], [], [Disable graphics])
  AM_CONDITIONAL([GRAPHICS_DISABLED], true)
fi

AC_MSG_CHECKING([--enable-legacy argument])
AC_ARG_ENABLE([legacy],
  AS_HELP_STRING([--disable-legacy], [disable the legacy OCR engine]))
AC_MSG_RESULT([$enable_legacy])
AM_CONDITIONAL([DISABLED_LEGACY_ENGINE], test "$enable_legacy" = "no")

# check whether to build embedded version
AC_MSG_CHECKING([--enable-embedded argument])
AC_ARG_ENABLE([embedded],
    AS_HELP_STRING([--enable-embedded], [enable embedded build [default=no]]))
AC_MSG_RESULT([$enable_embedded])
AM_CONDITIONAL([EMBEDDED], [test "$enable_embedded" = "yes"])
if test "$enable_embedded" = "yes"; then
  AM_CPPFLAGS="-DEMBEDDED $AM_CPPFLAGS"
fi

# check whether to build OpenMP support
AC_OPENMP

have_tiff=false
# Note that the first usage of AC_CHECK_HEADERS must be unconditional.
AC_CHECK_HEADERS([tiffio.h], [have_tiff=true], [have_tiff=false])

# check whether to build opencl version
AC_MSG_CHECKING([--enable-opencl argument])
AC_ARG_ENABLE([opencl],
    AS_HELP_STRING([--enable-opencl], [enable opencl build [default=no]]))
AC_MSG_RESULT([$enable_opencl])
# check for opencl header
have_opencl=false
if test "$enable_opencl" = "yes"; then
  AC_CHECK_HEADERS([CL/cl.h], [have_opencl=true], [
    AC_CHECK_HEADERS(OpenCL/cl.h, [have_opencl=true], [have_opencl=false])
  ])
fi

# https://lists.apple.com/archives/unix-porting/2009/Jan/msg00026.html
m4_define([MY_CHECK_FRAMEWORK],
  [AC_CACHE_CHECK([if -framework $1 works],[my_cv_framework_$1],
     [save_LIBS="$LIBS"
     LIBS="$LIBS -framework $1"
     AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])],
             [my_cv_framework_$1=yes],
            [my_cv_framework_$1=no])
     LIBS="$save_LIBS"
    ])
   if test "$my_cv_framework_$1"="yes"; then
     AC_DEFINE(AS_TR_CPP([HAVE_FRAMEWORK_$1]), 1,
            [Define if you have the  $1 framework])
     AS_TR_CPP([FRAMEWORK_$1])="-framework $1"
     AC_SUBST(AS_TR_CPP([FRAMEWORK_$1]))
   fi]
)

have_opencl_lib=false
OPENCL_CPPFLAGS=''
OPENCL_LDFLAGS=''
case "${host_os}" in
  *darwin* | *-macos10*)
    echo "checking for OpenCL framework"
    MY_CHECK_FRAMEWORK([OpenCL])
    if test $my_cv_framework_OpenCL = yes; then
       have_opencl_lib=true
    fi
    if test "$enable_opencl" = "yes"; then
      if !($have_opencl_lib); then
        AC_MSG_ERROR([Required OpenCL library not found!])
      fi
      AM_CPPFLAGS="-DUSE_OPENCL $AM_CPPFLAGS"
      OPENCL_CPPFLAGS=""
      OPENCL_LDFLAGS="-framework OpenCL"
    fi
    ;;
  *)
    # default
    if test "$enable_opencl" = "yes"; then
        AC_CHECK_LIB([OpenCL], [clGetPlatformIDs],
                     [have_opencl_lib=true], [have_opencl_lib=false])
        if !($have_opencl); then
            AC_MSG_ERROR([Required OpenCL headers not found!])
        fi
        if !($have_opencl_lib); then
            AC_MSG_ERROR([Required OpenCL library not found!])
        fi
        AM_CPPFLAGS="-DUSE_OPENCL $AM_CPPFLAGS"
        OPENCL_CPPFLAGS="-I${OPENCL_INC}"
        OPENCL_LDFLAGS="${OPENCL_LIBS}"
    fi
    ;;
esac
AM_CONDITIONAL([USE_OPENCL], [test "$enable_opencl" = "yes"])
AC_SUBST([OPENCL_CPPFLAGS])
AC_SUBST([OPENCL_LDFLAGS])

# check whether to build tesseract with -fvisibility=hidden -fvisibility-inlines-hidden
# http://gcc.gnu.org/wiki/Visibility
# http://groups.google.com/group/tesseract-dev/browse_thread/thread/976645ae98189127
AC_MSG_CHECKING([--enable-visibility argument])
AC_ARG_ENABLE([visibility],
  AS_HELP_STRING([--enable-visibility],
                 [enable experimental build with -fvisibility [default=no]]))
AC_MSG_RESULT([$enable_visibility])
AM_CONDITIONAL([VISIBILITY], [test "$enable_visibility" = "yes"])

# Check if tessdata-prefix is disabled
AC_MSG_CHECKING([whether to use tessdata-prefix])
AC_ARG_ENABLE([tessdata-prefix],
    [AS_HELP_STRING([--disable-tessdata-prefix],
            [don't set TESSDATA-PREFIX during compile])],
    [tessdata_prefix="no"], [tessdata_prefix="yes"])
AC_MSG_RESULT([$tessdata_prefix])
AM_CONDITIONAL([NO_TESSDATA_PREFIX], [test "$tessdata_prefix" = "no"])

# Check whether to enable debugging
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [turn on debugging [default=no]]))
AC_MSG_RESULT([$enable_debug])
if test x"$enable_debug" = x"yes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -O0 -DDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -g -Wall -DDEBUG"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2 -DNDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -DNDEBUG"
fi

#localedir='${prefix}/share/locale'

# Always look into a "gnu" directory.
curwd=`pwd`
if test -d $curwd/gnu/include ; then
   CPPFLAGS="$CPPFLAGS -I$curwd/gnu/include"
fi
if test -d $curwd/gnu/lib ; then
   LDFLAGS="$LDFLAGS -L$curwd/gnu/lib"
fi

# ----------------------------------------
# Additional checking of compiler characteristics
# ----------------------------------------

# Check Endianness. If Big Endian, this will define WORDS_BIGENDIAN
AC_C_BIGENDIAN


# ----------------------------------------
# Init libtool
# ----------------------------------------

LT_INIT


# ----------------------------------------
# C++ related options
# ----------------------------------------

AC_MSG_CHECKING([if compiling with clang])
AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([], [[
#ifndef __clang__
       not clang
#endif
]])],
[CLANG=yes], [CLANG=no])

AC_MSG_RESULT([$CLANG])

dnl ********************
dnl turn on c++11
dnl ********************

OLD_CXXFLAGS=$CXXFLAGS
AC_MSG_CHECKING([whether compiler supports C++11])
CXXFLAGS="$CXXFLAGS -std=c++11"
snprintfworks=no
AC_COMPILE_IFELSE(
[
  AC_LANG_SOURCE([[
    #if (__cplusplus < 201103L)
    #error C++ 11 is unsupported
    #endif
  ]])
], [
     AC_MSG_RESULT(yes)
   ],
   [
     AC_MSG_RESULT(no)
     AC_MSG_ERROR([Your compiler does not have the necessary c++11 support! Cannot proceed.])
   ])
AC_CHECK_FUNCS([snprintf],, [snprintfworks=yes])
CXXFLAGS="$OLD_CXXFLAGS"


# set c++11 support based on platform/compiler
case "${host_os}" in
  cygwin*)
    CXXFLAGS="$CXXFLAGS -std=gnu++11"
    ;;
  *-darwin* | *-macos10*)
     if test "x$CLANG" = "xyes"; then
       CXXFLAGS="$CXXFLAGS -std=c++11 "
       LDFLAGS="$LDFLAGS -stdlib=libc++"
     else
       CXXFLAGS="$CXXFLAGS -std=c++11"
     fi
    ;;
  *)
    # default
    CXXFLAGS="$CXXFLAGS -std=c++11"
    ;;
esac


# ----------------------------------------
# Check for libraries
# ----------------------------------------

AC_SEARCH_LIBS([sem_init], [pthread rt])


# ----------------------------------------
# Checks for header files.
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([sys/ipc.h sys/shm.h])
AC_CHECK_HEADERS([limits.h malloc.h])
# Enable use of system-defined bool type if available:
AC_HEADER_STDBOOL

# ----------------------------------------
# Check for programs needed to build documentation.
# ----------------------------------------

AC_CHECK_PROG([have_asciidoc], asciidoc, true, false)
if $have_asciidoc; then
  AM_CONDITIONAL([ASCIIDOC], true)
else
  AM_CONDITIONAL([ASCIIDOC], false)
fi

# ----------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ----------------------------------------

AC_CHECK_TYPES([wchar_t],,, [#include "wchar.h"])
AC_CHECK_TYPES([long long int])
AC_CHECK_TYPES([off_t],,, [#include "sys/types.h"])
AC_CHECK_TYPES([mbstate_t],,, [#include "wchar.h"])

# ----------------------------------------
# Test auxiliary packages
# ----------------------------------------

PKG_CHECK_MODULES([LEPTONICA], [lept >= 1.74], [have_lept=true], [have_lept=false])
if $have_lept; then
  CPPFLAGS="$CPPFLAGS $LEPTONICA_CFLAGS"
else
  AC_MSG_ERROR([Leptonica 1.74 or higher is required. Try to install libleptonica-dev package.])
fi

AM_CONDITIONAL([ENABLE_TRAINING], true)

# Check location of icu headers
PKG_CHECK_MODULES([ICU_UC], [icu-uc >= 52.1], [have_icu_uc=true], [have_icu_uc=false])
PKG_CHECK_MODULES([ICU_I18N], [icu-i18n >= 52.1], [have_icu_i18n=true], [have_icu_i18n=false])
if !($have_icu_uc && $have_icu_i18n); then
  AC_CHECK_HEADERS([unicode/uchar.h], [have_icu=true], [have_icu=false])
  if !($have_icu); then
        AC_MSG_WARN([icu 52.1 or higher is required, but was not found.])
        AC_MSG_WARN([Training tools WILL NOT be built.])
        AC_MSG_WARN([Try to install libicu-devel package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
  else
    ICU_UC_LIBS="-licui18n -licuuc"
  fi
fi

# Check location of pango headers
PKG_CHECK_MODULES([pango], [pango >= 1.22.0], [have_pango=true], [have_pango=false])
if !($have_pango); then
        AC_MSG_WARN([pango 1.22.0 or higher is required, but was not found.])
        AC_MSG_WARN([Training tools WILL NOT be built.])
        AC_MSG_WARN([Try to install libpango1.0-dev package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
else
      CPPFLAGS="$CPPFLAGS $pango_CFLAGS"
fi

# Check location of cairo headers
PKG_CHECK_MODULES([cairo], [cairo], [have_cairo=true], [have_cairo=false])
if !($have_cairo); then
        AC_MSG_WARN([Training tools WILL NOT be built because of missing cairo library.])
        AC_MSG_WARN([Try to install libcairo-dev?? package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
else
      CPPFLAGS="$CPPFLAGS $cairo_CFLAGS"
fi


# ----------------------------------------
# Final Tasks and Output
# ----------------------------------------

# Output files
AC_CONFIG_FILES([Makefile tesseract.pc])
AC_CONFIG_FILES([langtests/Makefile])
AC_CONFIG_FILES([src/api/Makefile])
AC_CONFIG_FILES([src/api/tess_version.h])
AC_CONFIG_FILES([src/arch/Makefile])
AC_CONFIG_FILES([src/ccmain/Makefile])
AC_CONFIG_FILES([src/opencl/Makefile])
AC_CONFIG_FILES([src/ccstruct/Makefile])
AC_CONFIG_FILES([src/ccutil/Makefile])
AC_CONFIG_FILES([src/classify/Makefile])
AC_CONFIG_FILES([src/cutil/Makefile])
AC_CONFIG_FILES([src/dict/Makefile])
AC_CONFIG_FILES([src/lstm/Makefile])
AC_CONFIG_FILES([src/textord/Makefile])
AC_CONFIG_FILES([src/viewer/Makefile])
AC_CONFIG_FILES([src/wordrec/Makefile])
AC_CONFIG_FILES([tessdata/Makefile])
AC_CONFIG_FILES([tessdata/configs/Makefile])
AC_CONFIG_FILES([tessdata/tessconfigs/Makefile])
AC_CONFIG_FILES([unlvtests/Makefile])
AC_CONFIG_FILES([unittest/Makefile])
AC_CONFIG_FILES([java/Makefile])
AC_CONFIG_FILES([java/com/Makefile])
AC_CONFIG_FILES([java/com/google/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/events/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/ui/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AM_COND_IF([ENABLE_TRAINING], [AC_CONFIG_FILES(src/training/Makefile)])
AC_OUTPUT

# Final message
echo ""
echo "Configuration is done."
echo "You can now build and install $PACKAGE_NAME by running:"
echo ""
echo "$ make"
echo "$ sudo make install"
echo "$ sudo ldconfig"
echo ""

AM_COND_IF([ASCIIDOC],
  [
   echo "This will also build the documentation."
  ], [
   echo "Documentation will not be built because asciidoc is missing."
  ]
)

# echo "$ sudo make install LANGS=\"eng ara deu\""
# echo "  Or:"
# echo "$ sudo make install-langs"
echo ""

AM_COND_IF([ENABLE_TRAINING],
  [
   echo "Training tools can be built and installed with:"
   echo ""
   echo "$ make training"
   echo "$ sudo make training-install"
   echo ""],
  [
   echo "You can not build training tools because of missing dependency."
   echo "Check configure output for details."
   echo ""]
)

# ----------------------------------------
# CONFIG Template
# ----------------------------------------

# Fence added in configuration file
AH_TOP([
#ifndef CONFIG_AUTO_H
#define CONFIG_AUTO_H
/* config_auto.h: begin */
])

# Stuff added at bottom of file
AH_BOTTOM([

/* Miscellaneous defines */
#define AUTOCONF 1

/* Not used yet
#ifndef NO_GETTEXT
#define USING_GETTEXT
#endif
*/

/* config_auto.h: end */
#endif
])
