# -*-Shell-script-*-
#
# Copyright (c) Luc Vincent

# ----------------------------------------
# Initialization
# ----------------------------------------

AC_PREREQ(2.50)
AC_CONFIG_MACRO_DIR([m4])
AC_INIT(tesseract, 3.01, theraysmith@gmail.com)
AC_REVISION($Id: configure.ac,v 1.4 2007/02/02 22:38:17 theraysmith Exp $)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(api/tesseractmain.cpp)
AC_PREFIX_DEFAULT(/usr/local)
AC_CANONICAL_HOST

# Define date of package, etc. Could be useful in auto-generated
# documentation.
# TODO(luc) Generate good documentation using doxygen or equivalent
PACKAGE_YEAR=2011
PACKAGE_DATE="05/24"

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
GENERIC_MINOR_VERSION=1
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
    [  --enable-graphics         Enable graphics (ScrollView) (default).],
    [enable_graphics=$enableval],
    [enable_graphics="yes"])
AC_MSG_RESULT($enable_graphics)
if test "$enable_graphics" = "no"; then
  AC_DEFINE([DISABLE_GRAPHICS], [], [Disable graphics])
fi

localedir='${prefix}/share/locale'

# Not used yet, so disable
#AC_ARG_ENABLE([gettext],
#    [  --enable-gettext         Enable gettext (default).],
#    [enable_gettext=$enableval],
#    [enable_gettext="yes"])
#AC_MSG_RESULT($enable_gettext)
#if test "$enable_gettext" = "yes"; then
#  AM_GNU_GETTEXT_VERSION([0.17])
#  AM_GNU_GETTEXT([external])  
#  AC_SUBST(localedir)
#else
#  AC_DEFINE([NO_GETTEXT], [], [Disable Gettext])
#fi
#AM_CONDITIONAL([USING_GETTEXT], [test "$enable_gettext" = "yes"])

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
AC_PROG_CXX(cl.exe g++)
# Not needed
# AC_PROG_CC

AC_PROG_LIBTOOL

# Automake configuration
# ----------------------------------------

# Note: may need to configure automake to use ZIP as a distribution
# format because of an apparent bug with GZIP, which results in bogus
# archives.
# TODO(luc) Resolve this issue.
#AM_INIT_AUTOMAKE(dist-zip)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config_auto.h:config/config.h.in)
#AM_PROG_CC_C_O
AM_MAINTAINER_MODE
# Need to tell automake if Visual C++ is being used:
AM_CONDITIONAL(USING_CL, test x$CC = xcl.exe)

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
# AC_PROG_LN_S
# AC_PATH_PROG(MV, mv)
# AC_PATH_PROG(CP, cp)
# AC_PATH_PROG(RM, rm)
# AC_PATH_PROG(AR, ar)
# AC_PATH_PROG(TOUCH, touch)
# AC_PATH_PROG(SED, sed)
# AC_PATH_PROG(BASH, bash, ,[$PATH:/usr/bin:/util/tools/bin])
# # To use substitution in makefiles, use something like:
# AC_SUBST(BASH)

# TODO(luc) Handle documentation. None of the following
# is really needed until then
#
# AC_PROG_DOXYGEN_VERSION(1.3.2,[DOXYGEN_OK=1])
# AC_PATH_PROG(DOT, dot)
# AC_PATH_PROG(LATEX, latex)
# AC_PATH_PROG(DVIPS, dvips)
# AC_PATH_PROG(MAKEINDEX, makeindex)
# AC_PATH_PROG(PDFLATEX, pdflatex)
# AC_PATH_PROG(GZIP, gzip)
#
# if test -z "$DOXYGEN_OK" -o -z "$DOT"; then
#   AC_MSG_WARN([------------------------------------
# *** Disabling automatic documentation generation for this
# *** package. Please check that you have 'doxygen' (version
# *** $ac_doxygen_version or later) and 'graphviz' (aka, 'dot')
# *** installed on your system. In addition, to generate
# *** PostScript and PDF documentation, you will need to have
# *** LaTeX and PdfLaTeX respectively. Re-run this configuration
# *** script after you have updated your environment.
# --------------------------------------------------------])
#
# # We have appropriate version of doxygen and dot, so we
# # can generate documentation. It remains to be seen whether
# # we can generate PDF and PostScript documentation..
# else
#   GENERATE_DOCUMENTATION="true"
#
# # Determine if PostScript documentation is generated:
#   if test -z "$LATEX" -o -z "$DVIPS" -o -z "$MAKEINDEX"; then
#     AC_MSG_WARN([Disabling generation of PostScript documentation])
#   else
#     GENERATE_PS_DOCUMENTATION="true"
#   fi
#
#   # Determine if PDF documentation is generated:
#   if test -z "$PDFLATEX" -o -z "$MAKEINDEX"; then
#     AC_MSG_WARN([Disabling generation of PDF documentation])
#   else
#     GENERATE_PDF_DOCUMENTATION="true"
#   fi
# fi
#
# # These substitutions could be inside the 'else'
# # conditionals above, but it is not necessary and would
# # only cause some confusion...
# AC_SUBST(DOXYGEN)
# AC_SUBST(DOT)
# AC_SUBST(LATEX)
# AC_SUBST(DVIPS)
# AC_SUBST(MAKEINDEX)
# AC_SUBST(PDFLATEX)
# AC_SUBST(GZIP)
#
# # Adjust makefiles based on the kind of documentation that
# # is being generated,
# AM_CONDITIONAL(GENERATE_DOCUMENTATION, test -n "$GENERATE_DOCUMENTATION")
# AM_CONDITIONAL(GENERATE_PS_DOCUMENTATION, test -n "$GENERATE_PS_DOCUMENTATION")
# AM_CONDITIONAL(GENERATE_PDF_DOCUMENTATION, test -n "$GENERATE_PDF_DOCUMENTATION")


# Test for GNUWIN32 tools (only useful under windows)
# AC_PATH_GNUWIN32

# ----------------------------------------
# C++ related options
# ----------------------------------------

AC_LANG_CPLUSPLUS

# Enable --enable-debug or --disable-debug and set
# compile options accordingly. We are supposed to be either
# in debug mode or in optimize mode. Note that in debug mode,
# DEBUG_MODE will be set by this macro
# AC_CXX_OPTIMIZE
# AC_CXX_BOOL
# AC_CXX_TYPENAME
# AC_CXX_STDINCLUDES
# AC_CXX_RPO

# ----------------------------------------
# Check for libraries
# ----------------------------------------

# This option seems to always add -lm to the link line,
# which causes unnecessary warnings with Visual C++.
# Comment it out for now.
#AC_CHECK_LIB(m,sqrt)
AC_CHECK_LIB(z,deflate)
AC_CHECK_LIB(png,png_read_png)
AC_CHECK_LIB(jpeg,jpeg_read_scanlines)
AC_CHECK_LIB(pthread,sem_init)

# ----------------------------------------
# Checks for header files.
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(sys/ipc.h sys/shm.h)
AC_CHECK_HEADERS(limits.h malloc.h)
AC_CHECK_HEADERS(allheaders.h)
# Enable use of system-defined bool type if available:
AC_HEADER_STDBOOL

# Misc
AC_SYS_INTERPRETER
AC_SYS_LARGEFILE


# ----------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ----------------------------------------

AC_CHECK_TYPES(wchar_t)
AC_CHECK_TYPES(long long int)
AC_CHECK_TYPES(mbstate_t,,,[#include "wchar.h"])

#AC_TYPE_MODE_T
#AC_TYPE_OFF_T
AC_TYPE_SIZE_T
#AC_TYPE_PID_T


# ----------------------------------------
# Checks for library functions.
# ----------------------------------------

AC_FUNC_MMAP
AC_FUNC_FORK
AC_CHECK_FUNCS(strerror vsnprintf)
AC_CHECK_FUNCS(gethostname)
AC_CHECK_FUNCS(strchr memcpy)
AC_CHECK_FUNCS(acos asin)

# ----------------------------------------
# Test auxilliary packages
# ----------------------------------------

# Search JPEG library - not needed at the moment
# AC_PATH_JPEG(,
# [ no_jpeg=yes
#   AC_MSG_WARN([JPEG support is disabled]) ])

# Search LIBTIFF library
AC_PATH_LIBTIFF(,
[ no_libtiff=yes
  AC_MSG_WARN([TIFF support is disabled]) ])

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

# ----------------------------------------
# Final Tasks and Output
# ----------------------------------------

# Define installation paths
# AC_DEFINE_INSTALL_PATHS
# Redundant with PACKAGE_VERSION - comment out
# AC_DEFINE_UNQUOTED(TESSERACT_VERSION,["${PACKAGE_VERSION}"],[version string])

# Output files
AC_CONFIG_FILES(Makefile)
#if test "$enable_gettext" = "yes"; then
#AC_CONFIG_FILES(po/Makefile.in)
#fi
AC_CONFIG_FILES(api/Makefile)
AC_CONFIG_FILES(ccmain/Makefile)
AC_CONFIG_FILES(ccstruct/Makefile)
AC_CONFIG_FILES(ccutil/Makefile)
AC_CONFIG_FILES(classify/Makefile)
AC_CONFIG_FILES(cube/Makefile)
AC_CONFIG_FILES(cutil/Makefile)
AC_CONFIG_FILES(dict/Makefile)
AC_CONFIG_FILES(image/Makefile)
AC_CONFIG_FILES(neural_networks/runtime/Makefile)
AC_CONFIG_FILES(textord/Makefile)
AC_CONFIG_FILES(viewer/Makefile)
AC_CONFIG_FILES(wordrec/Makefile)
AC_CONFIG_FILES(training/Makefile)
AC_CONFIG_FILES(tessdata/Makefile)
AC_CONFIG_FILES(tessdata/configs/Makefile)
AC_CONFIG_FILES(tessdata/tessconfigs/Makefile)
AC_CONFIG_FILES(testing/Makefile)
if test "$enable_graphics" = "yes"; then
AC_CONFIG_FILES(java/Makefile)
AC_CONFIG_FILES(java/com/Makefile)
AC_CONFIG_FILES(java/com/google/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/events/Makefile)
AC_CONFIG_FILES(java/com/google/scrollview/ui/Makefile)
fi
# AC_CONFIG_FILES(doc/Doxyfile)
# AC_CONFIG_FILES(doc/header.html)
# AC_CONFIG_FILES(doc/footer.html)
# AC_CONFIG_FILES(doc/header.tex)
# AC_CONFIG_FILES(doc/RTF_ExtensionFile)
# AC_CONFIG_FILES(doc/Makefile)
AC_OUTPUT

# Final message
echo ""
echo "Configuration is done."
echo "You can now build $PACKAGE_NAME by running:"
# test x$GXX = xyes && \
# echo "% make depend  [optional]"
echo ""
echo "% make"

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

#ifdef DISABLE_GRAPHICS
#define GRAPHICS_DISABLED
#endif

/* Not used yet
#ifndef NO_GETTEXT
#define USING_GETTEXT
#endif
*/

/* config_auto.h: end */
#endif
])

