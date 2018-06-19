# -*-Shell-script-*-
#
# Copyright (c) Luc Vincent

# ----------------------------------------
# Initialization
# ----------------------------------------
AC_PREREQ([2.59])
AC_INIT([tesseract], [3.05.02], [https://github.com/tesseract-ocr/tesseract/issues])
AC_PROG_CXX([g++ clang++])
AC_LANG([C++])
AC_LANG_COMPILER_REQUIRE
CXXFLAGS=${CXXFLAGS:-""}
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([api/tesseractmain.cpp])
AC_PREFIX_DEFAULT([/usr/local])

# Define date of package, etc. Could be useful in auto-generated
# documentation.
PACKAGE_YEAR=2018
PACKAGE_DATE="06/19"

abs_top_srcdir=`AS_DIRNAME([$0])`
gitrev="`git --git-dir=${abs_top_srcdir}/.git --work-tree=${abs_top_srcdir} describe --always --tags`"
if test -n "${gitrev}" ; then
	AC_REVISION(["${gitrev}"])
	AC_DEFINE_UNQUOTED([GIT_REV], ["${gitrev}"], [Define to be the git revision])
	echo "Using git revision: ${gitrev}"
fi

AC_DEFINE_UNQUOTED([PACKAGE_NAME], ["${PACKAGE_NAME}"], [Name of package])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION], ["${PACKAGE_VERSION}"], [Version number])
AC_DEFINE_UNQUOTED([PACKAGE_YEAR], ["$PACKAGE_YEAR"], [Official year for this release])
AC_DEFINE_UNQUOTED([PACKAGE_DATE], ["$PACKAGE_DATE"], [Official date of release])

AC_SUBST([PACKAGE_NAME])
AC_SUBST([PACKAGE_VERSION])
AC_SUBST([PACKAGE_YEAR])
AC_SUBST([PACKAGE_DATE])

GENERIC_LIBRARY_NAME=tesseract

# Release versioning
GENERIC_MAJOR_VERSION=3
GENERIC_MINOR_VERSION=5
GENERIC_MICRO_VERSION=2

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
GENERIC_LIBRARY_VERSION=$GENERIC_MAJOR_VERSION:$GENERIC_MINOR_VERSION
AC_SUBST([GENERIC_API_VERSION])
AC_SUBST([GENERIC_MAJOR_VERSION])

AC_SUBST([GENERIC_LIBRARY_VERSION])
PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST([GENERIC_LIBRARY_NAME])

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST([GENERIC_RELEASE])
AC_SUBST([GENERIC_VERSION])

# ----------------------------------------
# Automake configuration
# ----------------------------------------

# Do not require README file (we use README.md)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config_auto.h:config/config.h.in])
AM_MAINTAINER_MODE

# default conditional
AM_CONDITIONAL([T_WIN], false)
AM_CONDITIONAL([MINGW], false)
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
        AC_DEFINE_UNQUOTED([MINGW], 1, [This is a MinGW system])
        AM_CONDITIONAL([T_WIN], true)
        AM_CONDITIONAL([MINGW], true)
        AM_CONDITIONAL([ADD_RT], false)
        AC_SUBST([AM_LDFLAGS], ['-Wl,-no-undefined -Wl,--as-needed'])
        ;;
    cygwin*)
        AM_CONDITIONAL([ADD_RT], false)
        AC_SUBST([AM_LDFLAGS], ['-Wl,-no-undefined -Wl,--as-needed'])
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

includedir="${includedir}/tesseract"

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
 [AS_HELP_STRING([--enable-graphics],[enable graphics (ScrollView) (default)])
AS_HELP_STRING([--disable-graphics],[disable graphics (ScrollView)])],
    [enable_graphics=$enableval],
    [enable_graphics="yes"])
AC_MSG_RESULT([$enable_graphics])
if test "$enable_graphics" = "no"; then
  AC_DEFINE([GRAPHICS_DISABLED], [], [Disable graphics])
  AM_CONDITIONAL([GRAPHICS_DISABLED], true)
fi

# Check if cube should be disabled
AC_MSG_CHECKING([whether to disable cube])
AC_ARG_ENABLE([cube],
  [AS_HELP_STRING([--disable-cube], [don't build cube support (experimental)])],
    [disable_cube="yes"], [disable_cube="no"])
AC_MSG_RESULT([$disable_cube])
AM_CONDITIONAL([NO_CUBE_BUILD], [test "$disable_cube" = "yes"])
if test "$disable_cube" = "yes"; then
  AM_CPPFLAGS="-DNO_CUBE_BUILD $AM_CPPFLAGS"
fi

# check whether to build embedded version
AC_MSG_CHECKING([--enable-embedded argument])
AC_ARG_ENABLE([embedded],
    [  --enable-embedded       enable embedded build (default=no)],
    [enable_embedded=$enableval],
    [enable_embedded="no"])
AC_MSG_RESULT([$enable_embedded])
AM_CONDITIONAL([EMBEDDED], [test "$enable_embedded" = "yes"])
if test "$enable_embedded" = "yes"; then
  AM_CPPFLAGS="-DEMBEDDED $AM_CPPFLAGS"
fi

# check whether to build opencl version
AC_MSG_CHECKING([--enable-opencl argument])
AC_ARG_ENABLE([opencl],
    [  --enable-opencl         enable opencl build (default=no)],
    [enable_opencl=$enableval],
    [enable_opencl="no"])
AC_MSG_RESULT([$enable_opencl])
# check for opencl header
have_opencl=false
AC_CHECK_HEADERS([CL/cl.h], [have_opencl=true], [
    AC_CHECK_HEADERS(OpenCL/cl.h, have_opencl=true, have_opencl=false)
])

have_tiff=false
AC_CHECK_HEADERS([tiffio.h], [have_tiff=true], [have_tiff=false])

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
    AC_CHECK_LIB([OpenCL], [clGetPlatformIDs],
                 [have_opencl_lib=true], [have_opencl_lib=false])
    if test "$enable_opencl" = "yes"; then
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
    [AS_HELP_STRING([--enable-visibility],[enable experimental build with fvisibility (default=no)])],
    [enable_visibility=$enableval],
    [enable_visibility="no"])
AC_MSG_RESULT([$enable_visibility])
AM_CONDITIONAL([VISIBILITY], [test "$enable_visibility" = "yes"])

# check whether to build multiple libraries
AC_MSG_CHECKING([--enable-multiple-libraries argument])
AC_ARG_ENABLE([multiple-libraries],
    [AS_HELP_STRING([--enable-multiple-libraries],[enable multiple libraries (default=no)])],
    [enable_mlibs=$enableval],
    [enable_mlibs="no"])
AC_MSG_RESULT([$enable_mlibs])
AM_CONDITIONAL([USING_MULTIPLELIBS], [test "$enable_mlibs" = "yes"])

# Check if tessdata-prefix is disabled
AC_MSG_CHECKING([whether to use tessdata-prefix])
AC_ARG_ENABLE([tessdata-prefix],
    [AS_HELP_STRING([--disable-tessdata-prefix],
            [don't set TESSDATA-PREFIX during compile])],
    [tessdata_prefix="no"], [tessdata_prefix="yes"])
AC_MSG_RESULT([$tessdata_prefix])
AM_CONDITIONAL([NO_TESSDATA_PREFIX], [test "$tessdata_prefix" = "no"])

# Check whether enable debuging
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [turn on debugging (default=no)])],
    [debug=$enableval],
    [debug="no"])
AC_MSG_RESULT([$debug])
if test x"$debug" = x"yes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -O0 -DDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -g -Wall -O0 -DDEBUG"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2 -DNDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -O2 -DNDEBUG"
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
# Check Compiler Characteristics and
# configure automake. The two appear to
# be intimately linked...
# ----------------------------------------

AC_PROG_LIBTOOL

# ----------------------------------------
# Additional checking of compiler characteristics
# ----------------------------------------

# Check Endianness. If Big Endian, this will define WORDS_BIGENDIAN
# See also at end of this file, where we define INTEL_BYTE_ORDER
# or MOTOROLA_BYTE_ORDER.
AC_C_BIGENDIAN


# ----------------------------------------
# Check for programs we need
# ----------------------------------------

# Check where all the following programs are and set
# variables accordingly:
LT_INIT


# ----------------------------------------
# C++ related options
# ----------------------------------------

AC_LANG_CPLUSPLUS

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
     has_cpp11=yes
   ],
   [
     AC_MSG_RESULT(no)
     has_cpp11=no
   ])
AC_CHECK_FUNCS([snprintf],, [snprintfworks=yes])
CXXFLAGS="$OLD_CXXFLAGS"

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

# Misc
AC_SYS_INTERPRETER
AC_SYS_LARGEFILE

AC_CHECK_FUNCS([getline])

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
PKG_CHECK_MODULES([ICU_UC], [icu-uc], [have_icu_uc=true], [have_icu_uc=false])
PKG_CHECK_MODULES([ICU_I18N], [icu-i18n], [have_icu_i18n=true], [have_icu_i18n=false])
if !($have_icu_uc && $have_icu_i18n); then
  AC_CHECK_HEADERS([unicode/uchar.h], [have_icu=true], [have_icu=false])
  if !($have_icu); then
        AC_MSG_WARN([Training tools WILL NOT be built because of missing icu library.])
        AC_MSG_WARN([Try to install libicu-devel package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
  else
    ICU_UC_LIBS="-licui18n -licuuc"
  fi
fi

# Check location of pango headers
PKG_CHECK_MODULES([pango], [pango], [have_pango=true], [have_pango=false])
if !($have_pango); then
        AC_MSG_WARN([Training tools WILL NOT be built because of missing pango library.])
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

# set c++11 support based on platform/compiler
if test "x$has_cpp11" = "xyes"; then
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
else
  AC_MSG_WARN([Training tools WILL NOT be built because of missing c++11 support.])
  AM_CONDITIONAL([ENABLE_TRAINING], false)
fi

# ----------------------------------------
# Final Tasks and Output
# ----------------------------------------

# Output files
AC_CONFIG_FILES([Makefile tesseract.pc])
AC_CONFIG_FILES([api/Makefile])
AC_CONFIG_FILES([ccmain/Makefile])
AC_CONFIG_FILES([opencl/Makefile])
AC_CONFIG_FILES([ccstruct/Makefile])
AC_CONFIG_FILES([ccutil/Makefile])
AC_CONFIG_FILES([classify/Makefile])
AC_CONFIG_FILES([cube/Makefile])
AC_CONFIG_FILES([cutil/Makefile])
AC_CONFIG_FILES([dict/Makefile])
AC_CONFIG_FILES([neural_networks/runtime/Makefile])
AC_CONFIG_FILES([textord/Makefile])
AC_CONFIG_FILES([viewer/Makefile])
AC_CONFIG_FILES([wordrec/Makefile])
AC_CONFIG_FILES([tessdata/Makefile])
AC_CONFIG_FILES([tessdata/configs/Makefile])
AC_CONFIG_FILES([tessdata/tessconfigs/Makefile])
AC_CONFIG_FILES([testing/Makefile])
AC_CONFIG_FILES([java/Makefile])
AC_CONFIG_FILES([java/com/Makefile])
AC_CONFIG_FILES([java/com/google/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/events/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/ui/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AM_COND_IF([ENABLE_TRAINING], [AC_CONFIG_FILES(training/Makefile)])
AC_OUTPUT

# Final message
echo ""
echo "Configuration is done."
echo "You can now build and install $PACKAGE_NAME by running:"
echo ""
echo "$ make"
echo "$ sudo make install"
# echo "$ sudo make install LANGS=\"eng ara deu\""
# echo "  Or:"
# echo "$ sudo make install-langs"

AM_COND_IF([ENABLE_TRAINING],
  [echo ""
   echo "Training tools can be build and installed (after building of $PACKAGE_NAME) with:"
   echo ""
   echo "$ make training"
   echo "$ sudo make training-install"
   echo ""],
  [echo ""
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
