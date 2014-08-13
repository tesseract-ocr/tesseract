# -*-Shell-script-*-
#
# Copyright (c) Luc Vincent

# ----------------------------------------
# Initialization
# ----------------------------------------

AC_PREREQ(2.50)
AC_INIT([tesseract], [3.03], [http://code.google.com/p/tesseract-ocr/issues/list])
CXXFLAGS=${CXXFLAGS:-""}
AC_CONFIG_MACRO_DIR([m4])
AC_REVISION([$Revision$])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(api/tesseractmain.cpp)
AC_PREFIX_DEFAULT(/usr/local)

# Define date of package, etc. Could be useful in auto-generated
# documentation.
# TODO(luc) Generate good documentation using doxygen or equivalent
PACKAGE_YEAR=2014
PACKAGE_DATE="01/14"

AC_DEFINE_UNQUOTED(PACKAGE_NAME,["${PACKAGE_NAME}"],[Name of package])
AC_DEFINE_UNQUOTED(PACKAGE_VERSION,["${PACKAGE_VERSION}"],[Version number])
AC_DEFINE_UNQUOTED(PACKAGE_YEAR,"$PACKAGE_YEAR",[Official year for this release])
AC_DEFINE_UNQUOTED(PACKAGE_DATE,"$PACKAGE_DATE",[Official date of release])

AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_YEAR)
AC_SUBST(PACKAGE_DATE)

GENERIC_LIBRARY_NAME=tesseract

# Release versioning
GENERIC_MAJOR_VERSION=3
GENERIC_MINOR_VERSION=3
GENERIC_MICRO_VERSION=0

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
GENERIC_LIBRARY_VERSION=$GENERIC_MAJOR_VERSION:$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_API_VERSION)
AC_SUBST(GENERIC_MAJOR_VERSION)

AC_SUBST(GENERIC_LIBRARY_VERSION)
PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

# ----------------------------------------
# Automake configuration
# ----------------------------------------

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config_auto.h:config/config.h.in)
AM_MAINTAINER_MODE

# default conditional
AM_CONDITIONAL(MINGW, false)
AM_CONDITIONAL(OSX, false)
AM_CONDITIONAL(GRAPHICS_DISABLED, false)

OPENCL_INC="/opt/AMDAPP/include"
OPENCL_LIBS="-lOpenCL"
#############################
#
# Platform specific setup
#
#############################
AC_CANONICAL_HOST
case "${host_os}" in
    mingw32*)
        AC_DEFINE_UNQUOTED(MINGW,1,[This is a MinGW system])
        AM_CONDITIONAL(MINGW, true)
        AM_CONDITIONAL(ADD_RT, false)
        AC_SUBST([AM_LDFLAGS], ['-Wl,-no-undefined -Wl,--as-needed'])
        ;;
    cygwin*)
	AM_CONDITIONAL(ADD_RT, false)
        ;;			
    solaris*)
        LIBS="-lsocket -lnsl -lrt -lxnet"
        AM_CONDITIONAL(ADD_RT, true)
        ;;
    *darwin*)
        OPENCL_LIBS="-framework OpenCL"
        OPENCL_INC=""
        AM_CONDITIONAL(ADD_RT, false)
        ;;
    powerpc-*-darwin*)
        OPENCL_LIBS=""
        ;;
    *)
        # default
        AM_CONDITIONAL(ADD_RT, true)
        ;;
esac

includedir="${includedir}/tesseract"

AC_ARG_WITH(extra-includes,
            AC_HELP_STRING([--with-extra-includes=DIR],
                       [Define an additional directory for include files]),
        [ if test -d "$withval" ; then
            CFLAGS="$CFLAGS -I$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_ARG_WITH(extra-libraries,
            AC_HELP_STRING([--with-extra-libraries=DIR],
                       [Define an additional directory for library files]),
        [ if test -d "$withval" ; then
           LDFLAGS="$LDFLAGS -L$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_MSG_CHECKING(--enable-graphics argument)
AC_ARG_ENABLE([graphics],
 [AC_HELP_STRING([--enable-graphics],[enable graphics (ScrollView) (default)])
AC_HELP_STRING([--disable-graphics],[disable graphics (ScrollView)])],
    [enable_graphics=$enableval],
    [enable_graphics="yes"])
AC_MSG_RESULT($enable_graphics)
if test "$enable_graphics" = "no"; then
  AC_DEFINE([GRAPHICS_DISABLED], [], [Disable graphics])
  AM_CONDITIONAL(GRAPHICS_DISABLED, true)
fi

# check whether to build embedded version
AC_MSG_CHECKING(--enable-embedded argument)
AC_ARG_ENABLE([embedded],
    [  --enable-embedded       enable embedded build (default=no)],
    [enable_embedded=$enableval],
    [enable_embedded="no"])
AC_MSG_RESULT($enable_embedded)
AM_CONDITIONAL([EMBEDDED], [test "$enable_embedded" = "yes"])
if test "$enable_embedded" = "yes"; then
  AC_SUBST([AM_CPPFLAGS], [-DEMBEDDED])
fi

# check whether to build opencl version
AC_MSG_CHECKING(--enable-opencl argument)
AC_ARG_ENABLE([opencl],
    [  --enable-opencl         enable opencl build (default=no)],
    [enable_opencl=$enableval],
    [enable_opencl="no"])
AC_MSG_RESULT($enable_opencl)
AM_CONDITIONAL([USE_OPENCL], [test "$enable_opencl" = "yes"])
if test "$enable_opencl" = "yes"; then
  AC_SUBST([AM_CPPFLAGS], [-DUSE_OPENCL])
  AC_SUBST([OPENCL_HDR_PATH],[$OPENCL_INC])
  AC_SUBST([OPENCL_LIB],[$OPENCL_LIBS])
fi

# check whether to build tesseract with -fvisibility=hidden -fvisibility-inlines-hidden
# http://gcc.gnu.org/wiki/Visibility
# http://groups.google.com/group/tesseract-dev/browse_thread/thread/976645ae98189127
AC_MSG_CHECKING(--enable-visibility argument)
AC_ARG_ENABLE([visibility],
    [AC_HELP_STRING([--enable-visibility],[enable experimental build with fvisibility (default=no)])],
    [enable_visibility=$enableval],
    [enable_visibility="no"])
AC_MSG_RESULT($enable_visibility)
AM_CONDITIONAL([VISIBILITY], [test "$enable_visibility" = "yes"])

# check whether to build multiple libraries
AC_MSG_CHECKING(--enable-multiple-libraries argument)
AC_ARG_ENABLE([multiple-libraries],
    [AC_HELP_STRING([--enable-multiple-libraries],[enable multiple libraries (default=no)])],
    [enable_mlibs=$enableval],
    [enable_mlibs="no"])
AC_MSG_RESULT($enable_mlibs)
AM_CONDITIONAL([USING_MULTIPLELIBS], [test "$enable_mlibs" = "yes"])

# Check if tessdata-prefix is disabled
AC_MSG_CHECKING(whether to use tessdata-prefix)
AC_ARG_ENABLE(tessdata-prefix,
    [AC_HELP_STRING([--disable-tessdata-prefix],
            [dont set TESSDATA-PREFIX during compile])],
    [tessdata_prefix="no"], [tessdata_prefix="yes"])
AC_MSG_RESULT($tessdata_prefix)
AM_CONDITIONAL([NO_TESSDATA_PREFIX], [test "$tessdata_prefix" = "no"])

# Check whether enable debuging
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--enable-debug],
        [turn on debugging (default=no)])],
    [debug=$enableval],
    [debug="no"])
AC_MSG_RESULT($debug)
if test x"$debug" = x"yes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -Wno-uninitialized -O0 -DDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -g -Wall -Wno-uninitialized -O0 -DDEBUG"
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

# Special cases
case "$host" in
  *-darwin* | *-macos10*)
     if test -d /opt/local ; then
       CPPFLAGS="$CPPFLAGS -I/opt/local/include"
       LDFLAGS="$LDFLAGS -L/opt/local/lib"
     elif test -d /sw ; then
       CPPFLAGS="$CPPFLAGS -I/sw/include"
       LDFLAGS="$LDFLAGS -L/sw/lib"
     fi
  ;;
esac

# ----------------------------------------
# Check Compiler Characteristics and
# configure automake. The two appear to
# be intimately linked...
# ----------------------------------------

# Define order of compilers
AC_PROG_CXX(g++)

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
AC_MSG_CHECKING(whether compiler supports C++11)
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
AC_CHECK_FUNCS(snprintf,, [snprintfworks=yes])
CXXFLAGS="$OLD_CXXFLAGS"

# ----------------------------------------
# Check for libraries
# ----------------------------------------

AC_SEARCH_LIBS(sem_init,pthread rt)


# ----------------------------------------
# Checks for header files.
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(sys/ipc.h sys/shm.h)
AC_CHECK_HEADERS(limits.h malloc.h)
# Enable use of system-defined bool type if available:
AC_HEADER_STDBOOL

# Misc
AC_SYS_INTERPRETER
AC_SYS_LARGEFILE

AC_CHECK_FUNCS([getline])

# ----------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ----------------------------------------

AC_CHECK_TYPES(wchar_t)
AC_CHECK_TYPES(long long int)
AC_CHECK_TYPES(mbstate_t,,,[#include "wchar.h"])

# ----------------------------------------
# Test auxiliary packages
# ----------------------------------------

# Check location of leptonica/liblept headers.
AC_MSG_CHECKING(for leptonica)
AC_ARG_VAR(LIBLEPT_HEADERSDIR,[Leptonica headers directory])

have_lept=no
if test "$LIBLEPT_HEADERSDIR" = "" ; then
  LIBLEPT_HEADERSDIR="/usr/local/include /usr/include"
fi
for incd in $LIBLEPT_HEADERSDIR
do
  for lept in . leptonica liblept
  do
    if test -r "$incd/$lept/allheaders.h" ; then
      CPPFLAGS="$CPPFLAGS -I$incd/$lept"
      have_lept=yes
    fi
  done
done
if test "$have_lept" = yes ; then
  AC_MSG_RESULT(yes)
  AC_CHECK_LIB(lept,pixCreate,[], AC_MSG_ERROR([leptonica library missing]))
else
  AC_MSG_ERROR([leptonica not found])
fi

AC_MSG_CHECKING([leptonica version >= 1.70])
AC_PREPROC_IFELSE(
    [AC_LANG_PROGRAM([#include "allheaders.h"],
[#if (LIBLEPT_MAJOR_VERSION >= 1) && (LIBLEPT_MINOR_VERSION >= 70)
int i = 0;
#else
#error You need to upgrade your leptonica library!
#endif])],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_FAILURE([leptonica 1.70 or higher is required])])

# Check location of icu headers
have_icu=false
AC_CHECK_HEADERS(unicode/uchar.h, have_icu=true, have_icu=false)
if !($have_icu); then
        AC_MSG_WARN(Training tools WILL NOT be built because of missing icu library.)
        AC_MSG_WARN(Try to install libicu-devel package.)
fi
AM_CONDITIONAL(ENABLE_TRAINING, $have_icu)

# Check location of pango headers
have_pango=false
AC_CHECK_HEADERS(pango-1.0/pango/pango-features.h, have_pango=true, have_pango=false)
if !($have_pango); then
        AC_MSG_WARN(Training tools WILL NOT be built because of missing pango library.)
        AC_MSG_WARN(Try to install libpango1.0-dev package.)
else
      CPPFLAGS="$CPPFLAGS $(pkg-config --cflags pango)"
fi
AM_CONDITIONAL(ENABLE_TRAINING, $have_pango)

# Check location of cairo headers
have_cairo=false
AC_CHECK_HEADERS(cairo/cairo-version.h, have_cairo=true, have_cairo=false)
if !($have_cairo); then
        AC_MSG_WARN(Training tools WILL NOT be built because of missing cairo library.)
        AC_MSG_WARN(Try to install libcairo-dev?? package.)
else
      CPPFLAGS="$CPPFLAGS $(pkg-config --cflags cairo)"
fi
AM_CONDITIONAL(ENABLE_TRAINING, $have_cairo)

# set c++11 support based on platform/compiler
if test "x$has_cpp11" = "xyes"; then
  case "$host" in
    cygwin*)
      if test "x$snprintfworks" = "xno"; then
        # cygwin workaround
        # http://stackoverflow.com/questions/20149633/how-to-use-snprintf-in-g-std-c11-version-4-8-2/20149792#20149792
        CXXFLAGS="$CXXFLAGS -std=gnu++11"
      fi
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
  AC_MSG_WARN(Training tools WILL NOT be built because of missing c++11 support.)
  AM_CONDITIONAL(ENABLE_TRAINING, false)
fi

# ----------------------------------------
# Final Tasks and Output
# ----------------------------------------

# Output files
AC_CONFIG_FILES([Makefile tesseract.pc])
AC_CONFIG_FILES(api/Makefile)
AC_CONFIG_FILES(ccmain/Makefile)
AC_CONFIG_FILES(opencl/Makefile)
AC_CONFIG_FILES(ccstruct/Makefile)
AC_CONFIG_FILES(ccutil/Makefile)
AC_CONFIG_FILES(classify/Makefile)
AC_CONFIG_FILES(cube/Makefile)
AC_CONFIG_FILES(cutil/Makefile)
AC_CONFIG_FILES(dict/Makefile)
AC_CONFIG_FILES(neural_networks/runtime/Makefile)
AC_CONFIG_FILES(textord/Makefile)
AC_CONFIG_FILES(viewer/Makefile)
AC_CONFIG_FILES(wordrec/Makefile)
AC_CONFIG_FILES(tessdata/Makefile)
AC_CONFIG_FILES(tessdata/configs/Makefile)
AC_CONFIG_FILES(tessdata/tessconfigs/Makefile)
AC_CONFIG_FILES(testing/Makefile)
AC_CONFIG_FILES(java/Makefile)
AC_CONFIG_FILES(java/com/Makefile)
AC_CONFIG_FILES(java/com/google/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/events/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/ui/Makefile)
AC_CONFIG_FILES(doc/Makefile)
AM_COND_IF([ENABLE_TRAINING], AC_CONFIG_FILES(training/Makefile))
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
  echo ""
  echo "Training tools can be build and installed (after building of $PACKAGE_NAME) with:"
  echo ""
  echo "$ make training"
  echo "$ sudo make training-install"
  echo ""
,
  echo ""
  echo "You can not build training tools because of missing dependency."
  echo "Check configure output for details."
  echo ""
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
