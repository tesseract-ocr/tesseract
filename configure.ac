# -*-Shell-script-*-
#
# Copyright (c) Luc Vincent

# ----------------------------------------
# Initialization
# ----------------------------------------
AC_PREREQ([2.69])
AC_INIT([tesseract],
        [m4_esyscmd_s([git describe --abbrev=4 2>/dev/null || cat VERSION])],
        [https://github.com/tesseract-ocr/tesseract/issues],,
        [https://github.com/tesseract-ocr/tesseract/])

# Store command like options for CXXFLAGS
OLD_CXXFLAGS=$CXXFLAGS
AC_PROG_CXX([g++ clang++])
# reset compiler flags to initial flags
AC_LANG([C++])
AC_LANG_COMPILER_REQUIRE
CXXFLAGS=${CXXFLAGS:-""}
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/tesseract.cpp])
AC_PREFIX_DEFAULT([/usr/local])

# Automake configuration. Do not require README file (we use README.md).
AM_INIT_AUTOMAKE([foreign subdir-objects nostdinc])

# Define date of package, etc. Could be useful in auto-generated
# documentation.
PACKAGE_YEAR=2023
PACKAGE_DATE="07/11"

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

AC_CONFIG_HEADERS([include/config_auto.h:config/config.h.in])

# default conditional
AM_CONDITIONAL([T_WIN], false)
AM_CONDITIONAL([GRAPHICS_DISABLED], false)
AC_SUBST([AM_CPPFLAGS])

# Be less noisy by default.
# Can be overridden with `configure --disable-silent-rules` or with `make V=1`.
AM_SILENT_RULES([yes])

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
        AC_SUBST([AM_LDFLAGS], ['-no-undefined'])
        ;;
    cygwin*)
        AM_CONDITIONAL([ADD_RT], false)
        AC_SUBST([NOUNDEFINED], ['-no-undefined'])
        ;;
    solaris*)
        LIBS="$LIBS -lsocket -lnsl -lrt -lxnet"
        AM_CONDITIONAL([ADD_RT], true)
        ;;
    *darwin*)
        OPENCL_LIBS=""
        OPENCL_INC=""
        AM_CONDITIONAL([ADD_RT], false)
        ;;
    *android*|openbsd*)
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

WERROR=-Werror
# The test code used by AX_CHECK_COMPILE_FLAG uses an empty statement
# and unused macros which must not raise a compiler error, but it must
# be an error if flags like -avx are ignored on ARM and other
# architectures because they are unsupported.
AX_CHECK_COMPILE_FLAG([-Werror=unused-command-line-argument], [WERROR=-Werror=unused-command-line-argument])

## Checks for supported compiler options.

AM_CONDITIONAL([HAVE_AVX], false)
AM_CONDITIONAL([HAVE_AVX2], false)
AM_CONDITIONAL([HAVE_AVX512F], false)
AM_CONDITIONAL([HAVE_FMA], false)
AM_CONDITIONAL([HAVE_SSE4_1], false)
AM_CONDITIONAL([HAVE_NEON], false)

case "${host_cpu}" in

  amd64|*86*)

    AX_CHECK_COMPILE_FLAG([-mavx], [avx=true], [avx=false], [$WERROR])
    AM_CONDITIONAL([HAVE_AVX], ${avx})
    if $avx; then
      AC_DEFINE([HAVE_AVX], [1], [Enable AVX instructions])
    fi

    AX_CHECK_COMPILE_FLAG([-mavx2], [avx2=true], [avx2=false], [$WERROR])
    AM_CONDITIONAL([HAVE_AVX2], $avx2)
    if $avx2; then
      AC_DEFINE([HAVE_AVX2], [1], [Enable AVX2 instructions])
    fi

    AX_CHECK_COMPILE_FLAG([-mavx512f], [avx512f=true], [avx512f=false], [$WERROR])
    AM_CONDITIONAL([HAVE_AVX512F], $avx512f)
    if $avx512f; then
      AC_DEFINE([HAVE_AVX512F], [1], [Enable AVX512F instructions])
    fi

    AX_CHECK_COMPILE_FLAG([-mfma], [fma=true], [fma=false], [$WERROR])
    AM_CONDITIONAL([HAVE_FMA], $fma)
    if $fma; then
      AC_DEFINE([HAVE_FMA], [1], [Enable FMA instructions])
    fi

    AX_CHECK_COMPILE_FLAG([-msse4.1], [sse41=true], [sse41=false], [$WERROR])
    AM_CONDITIONAL([HAVE_SSE4_1], $sse41)
    if $sse41; then
      AC_DEFINE([HAVE_SSE4_1], [1], [Enable SSE 4.1 instructions])
    fi

    ;;

  aarch64*|arm64)

    # ARMv8 always has NEON and does not need special compiler flags.
    AM_CONDITIONAL([HAVE_NEON], true)
    AC_DEFINE([HAVE_NEON], [1], [Enable NEON instructions])
    ;;

  arm*)

    AX_CHECK_COMPILE_FLAG([-mfpu=neon], [neon=true], [neon=false], [$WERROR])
    AM_CONDITIONAL([HAVE_NEON], $neon)
    if $neon; then
      AC_DEFINE([HAVE_NEON], [1], [Enable NEON instructions])
      NEON_CXXFLAGS="-mfpu=neon"
      AC_SUBST([NEON_CXXFLAGS])
      check_for_neon=1
    fi

    ;;

  *)

    AC_MSG_WARN([No compiler options for $host_cpu])

esac

# check whether feenableexcept is supported. some C libraries (e.g. uclibc) don't.
AC_CHECK_FUNCS([feenableexcept])

# additional checks for NEON targets
if test x$check_for_neon = x1; then
  AC_MSG_NOTICE([checking how to detect NEON availability])
  AC_CHECK_FUNCS([getauxval elf_aux_info android_getCpuFamily])

  if test $ac_cv_func_getauxval = no && test $ac_cv_func_elf_aux_info = no && test $ac_cv_func_android_getCpuFamily = no; then
      AC_MSG_WARN([NEON is available, but we don't know how to check for it.  Will not be able to use NEON.])
  fi
fi

AX_CHECK_COMPILE_FLAG([-fopenmp-simd], [openmp_simd=true], [openmp_simd=false], [$WERROR])
AM_CONDITIONAL([OPENMP_SIMD], $openmp_simd)

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

AC_MSG_CHECKING([--enable-float32 argument])
AC_ARG_ENABLE([float32],
	      AS_HELP_STRING([--disable-float32], [disable float and enable double for LSTM]))
AC_MSG_RESULT([$enable_float32])
if test "$enable_float32" != "no"; then
  AC_DEFINE([FAST_FLOAT], [1], [Enable float for LSTM])
fi

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
if test "$enable_legacy" = "no"; then
  AC_DEFINE([DISABLED_LEGACY_ENGINE], [1], [Disable legacy OCR engine])
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

# Configure arguments which allow disabling some optional libraries.
AC_ARG_WITH([archive],
            AS_HELP_STRING([--with-archive],
                           [Build with libarchive which supports compressed model files @<:@default=check@:>@]),
            [], [with_archive=check])
AC_ARG_WITH([curl],
            AS_HELP_STRING([--with-curl],
                           [Build with libcurl which supports processing an image URL @<:@default=check@:>@]),
            [], [with_curl=check])
AC_ARG_WITH([tensorflow],
  AS_HELP_STRING([--with-tensorflow],
                 [support TensorFlow @<:@default=check@:>@]),
  [], [with_tensorflow=check])

# Check whether to build with support for TensorFlow.
AM_CONDITIONAL([TENSORFLOW], false)
TENSORFLOW_LIBS=
AS_IF([test "x$with_tensorflow" != xno],
  [AC_CHECK_HEADERS([tensorflow/core/framework/graph.pb.h],
    [AC_SUBST([TENSORFLOW_LIBS], ["-lprotobuf -ltensorflow_cc"])
      AM_CONDITIONAL([TENSORFLOW], true)
    ],
    [if test "x$with_tensorflow" != xcheck; then
      AC_MSG_FAILURE(
        [--with-tensorflow was given, but test for libtensorflow-dev failed])
     fi
    ])
  ])

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
            [Define if you have the $1 framework])
     AS_TR_CPP([FRAMEWORK_$1])="-framework $1"
     AC_SUBST(AS_TR_CPP([FRAMEWORK_$1]))
   fi]
)

have_opencl_lib=false
OPENCL_CPPFLAGS=''
OPENCL_LDFLAGS=''
case "${host_os}" in
  *darwin* | *-macos10*)
    MY_CHECK_FRAMEWORK([Accelerate])
    if test $my_cv_framework_Accelerate = yes; then
      AM_CPPFLAGS="-DHAVE_FRAMEWORK_ACCELERATE $AM_CPPFLAGS"
      AM_LDFLAGS="$AM_LDFLAGS -framework Accelerate"
    fi
    MY_CHECK_FRAMEWORK([OpenCL])
    if test "$enable_opencl" = "yes"; then
      if test $my_cv_framework_OpenCL = no; then
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
AM_CONDITIONAL([OPENCL], [test "$enable_opencl" = "yes"])
AC_SUBST([OPENCL_CPPFLAGS])
AC_SUBST([OPENCL_LDFLAGS])

# check whether to build tesseract with -fvisibility=hidden -fvisibility-inlines-hidden
# http://gcc.gnu.org/wiki/Visibility
# https://groups.google.com/g/tesseract-dev/c/l2ZFrpgYkSc/m/_cdYSRDSXuUJ
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


# Detect Clang compiler
AC_MSG_CHECKING([if compiling with clang])
AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([], [[
#ifndef __clang__
       not clang
#endif
]])],
[CLANG=yes], [CLANG=no])
AC_MSG_RESULT([$CLANG])

# Check whether to enable debugging
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE([debug],
  AS_HELP_STRING([--enable-debug], [turn on debugging [default=no]]))
AC_MSG_RESULT([$enable_debug])
if test x"$enable_debug" = x"yes"; then
    CXXFLAGS=${CXXFLAGS:-"-O2"}
    AM_CPPFLAGS="$AM_CPPFLAGS -g -Wall -DDEBUG -pedantic"
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -DDEBUG -pedantic"
    if test "x$CLANG" = "xyes"; then
        # https://clang.llvm.org/docs/CommandGuide/clang.html
        # clang treats -Og as -O1
        AM_CPPFLAGS="$AM_CPPFLAGS -O0"
        AM_CXXFLAGS="$AM_CXXFLAGS -O0"
    else
        AM_CPPFLAGS="$AM_CPPFLAGS -Og"
        AM_CXXFLAGS="$AM_CXXFLAGS -Og"
    fi
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2 -DNDEBUG"
    AM_CPPFLAGS="$AM_CPPFLAGS -O2 -DNDEBUG"
fi

# ----------------------------------------
# Init libtool
# ----------------------------------------

LT_INIT


# ----------------------------------------
# C++ related options
# ----------------------------------------
dnl **********************
dnl Turn on C++17 or newer
dnl **********************

CPLUSPLUS=
AX_CHECK_COMPILE_FLAG([-std=c++17], [CPLUSPLUS=17], [], [$WERROR])
AX_CHECK_COMPILE_FLAG([-std=c++20], [CPLUSPLUS=20], [], [$WERROR])

if test -z "$CPLUSPLUS"; then
  AC_MSG_ERROR([Your compiler does not have the necessary C++17 support! Cannot proceed.])
fi

# Set C++17 or newer support based on platform/compiler
case "${host_os}" in
  cygwin*)
    CXXFLAGS="$CXXFLAGS -std=gnu++$CPLUSPLUS"
    ;;
  *-darwin* | *-macos10*)
    CXXFLAGS="$CXXFLAGS -std=c++$CPLUSPLUS"
    if test "x$CLANG" = "xyes"; then
      LDFLAGS="$LDFLAGS -stdlib=libc++"
    fi
    ;;
  *)
    # default
    CXXFLAGS="$CXXFLAGS -std=c++$CPLUSPLUS"
    ;;
esac


# ----------------------------------------
# Check for libraries
# ----------------------------------------

AC_SEARCH_LIBS([pthread_create], [pthread])

# Set PKG_CONFIG_PATH for macOS with Homebrew unless it is already set.
AC_CHECK_PROG([have_brew], brew, true, false)
if $have_brew; then
  brew_prefix=$(brew --prefix)
  if test -z "$PKG_CONFIG_PATH"; then
    PKG_CONFIG_PATH=$brew_prefix/opt/icu4c/lib/pkgconfig:$brew_prefix/opt/libarchive/lib/pkgconfig
    export PKG_CONFIG_PATH
  fi
fi

# ----------------------------------------
# Check for programs needed to build documentation.
# ----------------------------------------

AM_CONDITIONAL([ASCIIDOC], false)
AM_CONDITIONAL([HAVE_XML_CATALOG_FILES], false)
AC_ARG_ENABLE([doc],
              AS_HELP_STRING([--disable-doc], [disable build of documentation])
              [],
              [: m4_divert_text([DEFAULTS], [enable_doc=check])])
AS_IF([test "$enable_doc" != "no"], [
  AC_CHECK_PROG([have_asciidoc], asciidoc, true, false)
  AC_CHECK_PROG([have_xsltproc], xsltproc, true, false)
  # macOS with Homebrew requires the environment variable
  # XML_CATALOG_FILES for xsltproc.
  if $have_asciidoc && $have_xsltproc; then
    AM_CONDITIONAL([ASCIIDOC], true)
    XML_CATALOG_FILES=
    if $have_brew; then
      catalog_file=$brew_prefix/etc/xml/catalog
      if test -f $catalog_file; then
        AM_CONDITIONAL([HAVE_XML_CATALOG_FILES], true)
        XML_CATALOG_FILES=file:$catalog_file
      else
        AC_MSG_WARN([Missing file $catalog_file.])
      fi
    fi
    AC_SUBST([XML_CATALOG_FILES])
  else
    AS_IF([test "x$enable_doc" != xcheck], [
      AC_MSG_FAILURE(
        [--enable-doc was given, but test for asciidoc and xsltproc failed])
    ])
  fi
])

# ----------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ----------------------------------------

AC_CHECK_TYPES([wchar_t],,, [#include "wchar.h"])
AC_CHECK_TYPES([long long int])

# ----------------------------------------
# Test auxiliary packages
# ----------------------------------------

AM_CONDITIONAL([HAVE_LIBCURL], false)
AS_IF([test "x$with_curl" != xno], [
  PKG_CHECK_MODULES([libcurl], [libcurl], [have_libcurl=true], [have_libcurl=false])
  AM_CONDITIONAL([HAVE_LIBCURL], $have_libcurl)
  if $have_libcurl; then
    AC_DEFINE([HAVE_LIBCURL], [1], [Enable libcurl])
  else
    AS_IF([test "x$with_curl" != xcheck], [
      AC_MSG_FAILURE(
        [--with-curl was given, but test for libcurl failed])
    ])
  fi
])

PKG_CHECK_MODULES([LEPTONICA], [lept >= 1.74], [have_lept=true], [have_lept=false])
if $have_lept; then
  CPPFLAGS="$CPPFLAGS $LEPTONICA_CFLAGS"
else
  AC_MSG_ERROR([Leptonica 1.74 or higher is required. Try to install libleptonica-dev package.])
fi

AM_CONDITIONAL([HAVE_LIBARCHIVE], false)
AS_IF([test "x$with_archive" != xno], [
  PKG_CHECK_MODULES([libarchive], [libarchive], [have_libarchive=true], [have_libarchive=false])
  AM_CONDITIONAL([HAVE_LIBARCHIVE], [$have_libarchive])
  if $have_libarchive; then
    AC_DEFINE([HAVE_LIBARCHIVE], [1], [Enable libarchive])
    CPPFLAGS="$CPPFLAGS $libarchive_CFLAGS"
  else
    AS_IF([test "x$with_archive" != xcheck], [
      AC_MSG_FAILURE(
        [--with-archive was given, but test for libarchive failed])
    ])
  fi
])

AM_CONDITIONAL([ENABLE_TRAINING], true)

# Check availability of ICU packages.
PKG_CHECK_MODULES([ICU_UC], [icu-uc >= 52.1], [have_icu_uc=true], [have_icu_uc=false])
PKG_CHECK_MODULES([ICU_I18N], [icu-i18n >= 52.1], [have_icu_i18n=true], [have_icu_i18n=false])
if !($have_icu_uc && $have_icu_i18n); then
  AC_MSG_WARN([icu 52.1 or higher is required, but was not found.])
  AC_MSG_WARN([Training tools WILL NOT be built.])
  AC_MSG_WARN([Try to install libicu-dev package.])
  AM_CONDITIONAL([ENABLE_TRAINING], false)
fi

# Check location of pango headers
PKG_CHECK_MODULES([pango], [pango >= 1.38.0], [have_pango=true], [have_pango=false])
if !($have_pango); then
        AC_MSG_WARN([pango 1.38.0 or higher is required, but was not found.])
        AC_MSG_WARN([Training tools WILL NOT be built.])
        AC_MSG_WARN([Try to install libpango1.0-dev package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
fi

# Check location of cairo headers
PKG_CHECK_MODULES([cairo], [cairo], [have_cairo=true], [have_cairo=false])
if !($have_cairo); then
        AC_MSG_WARN([Training tools WILL NOT be built because of missing cairo library.])
        AC_MSG_WARN([Try to install libcairo-dev?? package.])
        AM_CONDITIONAL([ENABLE_TRAINING], false)
fi

PKG_CHECK_MODULES([pangocairo], [pangocairo], [], [false])
PKG_CHECK_MODULES([pangoft2], [pangoft2], [], [false])

# ----------------------------------------
# Final Tasks and Output
# ----------------------------------------

# Output files
AC_CONFIG_FILES([include/tesseract/version.h])
AC_CONFIG_FILES([Makefile tesseract.pc])
AC_CONFIG_FILES([tessdata/Makefile])
AC_CONFIG_FILES([tessdata/configs/Makefile])
AC_CONFIG_FILES([tessdata/tessconfigs/Makefile])
AC_CONFIG_FILES([java/Makefile])
AC_CONFIG_FILES([java/com/Makefile])
AC_CONFIG_FILES([java/com/google/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/events/Makefile])
AC_CONFIG_FILES([java/com/google/scrollview/ui/Makefile])
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

AM_COND_IF([ASCIIDOC], [
  echo "This will also build the documentation."
], [
  AS_IF([test "$enable_doc" = "no"], [
    echo "Documentation will not be built because it was disabled."
  ], [
    echo "Documentation will not be built because asciidoc or xsltproc is missing."
  ])
])

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
   echo "You cannot build training tools because of missing dependency."
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
