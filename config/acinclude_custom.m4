# Copyright (c) 2003  Luc Vincent
#
# This M4 script includes a number of non-standard
# autoconf macros that are typically available
# at the GNU Autoconf Macro Archive:
#
#   http://www.gnu.org/software/ac-archive/
#
# Specially developed macros are placed a the
# end of this file. Some of them were originally written
# by L. Bottou and Y. LeCun



dnl -------------------------------------------------------
dnl @synopsis AC_PROG_DOXYGEN_VERSION(VERSION,
dnl               [ACTION-IF-TRUE], [ACTION-IF-FALSE])
dnl
dnl Check if doxygen is present and if version on machine
dnl is at least version number supplied as argument
dnl
dnl Makes sure that doxygen supports the version indicated. If true the
dnl shell commands in ACTION-IF-TRUE are executed. If not the shell
dnl commands in ACTION-IF-FALSE are run. Note if $DOXYGEN is not set (for
dnl example by running AC_CHECK_PROG or AC_PATH_PROG),
dnl AC_CHECK_PROG(DOXYGEN, doxygen, doxygen) will be run.
dnl
dnl Example:
dnl 
dnl  AC_PROG_DOXYGEN_VERSION(1.3.2)
dnl
dnl This will check to make sure that the version of doxygen you have
dnl is at least version 1.3.2
dnl -------------------------------------------------------

AC_DEFUN([AC_PROG_DOXYGEN_VERSION],[dnl
# Make sure we have doxygen
if test -z "$DOXYGEN"; then
#  AC_CHECK_PROG(DOXYGEN,doxygen,doxygen)
  AC_PATH_PROG(DOXYGEN,doxygen)
fi

# Check if version of Doxygen is sufficient
ac_doxygen_version=$1

if test "x$DOXYGEN" != "x"; then
  AC_MSG_CHECKING(for doxygen version at least equal to $ac_doxygen_version)
  # NB: It would be nice to log the error if there is one, but we cannot rely
  # on autoconf internals...
  # This is the actual version of doxygen
  doxy_version=`$DOXYGEN --version`
  # Need to turn these floating point numbers into integers:
  AC_DEFINE_VERSIONLEVEL(DOXYGEN_VERSION_REQUIRED, $ac_doxygen_version)
  AC_DEFINE_VERSIONLEVEL(DOXYGEN_VERSION_ACTUAL, $doxy_version)

  if test $DOXYGEN_VERSION_ACTUAL -lt $DOXYGEN_VERSION_REQUIRED; then
    AC_MSG_RESULT(no);
    $3
  else
    AC_MSG_RESULT(ok);
    $2
  fi
else
  AC_MSG_WARN(could not find doxygen)
fi
])dnl


dnl -------------------------------------------------------
dnl @synopsis AC_DEFINE_INSTALL_PATHS
dnl Define various installation paths
dnl -------------------------------------------------------
AC_DEFUN([AC_DEFINE_INSTALL_PATHS],[
  save_prefix="${prefix}"
  save_exec_prefix="${exec_prefix}"
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
  DIR_PREFIX="`eval echo \"$prefix\"`"
  AC_DEFINE_UNQUOTED(DIR_PREFIX,["${DIR_PREFIX}"],[directory "prefix"])
  DIR_EXEC_PREFIX="`eval echo \"$exec_prefix\"`"
  AC_DEFINE_UNQUOTED(DIR_EXEC_PREFIX,["${DIR_EXEC_PREFIX}"],[directory "exec_prefix"])
  DIR_BINDIR="`eval echo \"$bindir\"`"
  AC_DEFINE_UNQUOTED(DIR_BINDIR,["${DIR_BINDIR}"],[directory "bindir"])
  DIR_LIBDIR="`eval echo \"$libdir\"`"
  AC_DEFINE_UNQUOTED(DIR_LIBDIR,["${DIR_LIBDIR}"],[directory "libdir"])
  DIR_DATADIR="`eval echo \"$datadir\"`"
  AC_DEFINE_UNQUOTED(DIR_DATADIR,["${DIR_DATADIR}"],[directory "datadir"])
  DIR_MANDIR="`eval echo \"$mandir\"`"
  AC_DEFINE_UNQUOTED(DIR_MANDIR,["${DIR_MANDIR}"],[directory "mandir"])
  prefix="${save_prefix}"
  exec_prefix="${save_exec_prefix}"
])


dnl -------------------------------------------------------
dnl @synopsis AC_CHECK_CXX_OPT(OPTION,
dnl               ACTION-IF-OKAY,ACTION-IF-NOT-OKAY)
dnl Check if compiler accepts option OPTION.
dnl -------------------------------------------------------
AC_DEFUN(AC_CHECK_CXX_OPT,[
 opt="$1"
 AC_MSG_CHECKING([if $CXX accepts $opt])
 echo 'void f(){}' > conftest.cc
 if test -z "`${CXX} ${CXXFLAGS} ${OPTS} $opt -c conftest.cc 2>&1`"; then
    AC_MSG_RESULT(yes)
    rm conftest.* 
    $2
 else
    AC_MSG_RESULT(no)
    rm conftest.*
    $3
 fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CC_OPTIMIZE
dnl Setup option --enable-debug
dnl Collects optimization/debug option in variable CFLAGS,
dnl filtering options already in CFLAGS. Also define
dnl DEBUG_MODE if appropriate.
dnl Adapted from AC_CXX_OPTIMIZE in djvulibre
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_OPTIMIZE],[
   AC_REQUIRE([AC_CANONICAL_HOST])
   AC_ARG_ENABLE(debug,
        AC_HELP_STRING([--enable-debug],
                       [Compile with debugging options (default: no)]),
        [ac_debug=$enableval],[ac_debug=no])
   OPTS=
   AC_SUBST(OPTS)
   dnl VCOPTS_COMMON="/nologo /G5 /Zp1 /W3 /Za /Op- /GX"
   # Note: the /Za option might be nice to have, but it is
   # incompatible with <windows.h>, which some packages
   # (like the tiff library) require
   VCOPTS_COMMON="/nologo /G5 /Zp1 /W3"
   dnl AC_SUBST(OPTS)
   saved_CXXFLAGS="$CXXFLAGS"
   saved_CFLAGS="$CFLAGS"
   CXXFLAGS=
   CFLAGS=
   for opt in $saved_CXXFLAGS ; do
     case $opt in
       -g*) test $ac_debug != no && OPTS="$OPTS $opt" ;;
       -O*) ;;
       *) CXXFLAGS="$CXXFLAGS $opt" ;;
     esac
   done
   for opt in $saved_CFLAGS ; do
     case $opt in
       -O*|-g*) ;;
       *) CFLAGS="$CFLAGS $opt" ;;
     esac
   done
   if test x$ac_debug = xno ; then
     OPTS=-DNDEBUG
     if test x$CXX = xcl.exe ; then
	OPTS="$OPTS $VCOPTS_COMMON /Ow /O2"
     else
        AC_CHECK_CXX_OPT([-O3],[OPTS="$OPTS -O3"],
           [ AC_CHECK_CXX_OPT([-O2], [OPTS="$OPTS -O2"] ) ] )
        dnl This triggers compiler bugs with gcc-3.2.2 - comment out for now
	dnl AC_CHECK_CXX_OPT([-funroll-loops], [OPTS="$OPTS -funroll-loops"])
        cpu=`uname -m 2>/dev/null`
        test -z "$cpu" && cpu=${host_cpu}
        case "${host_cpu}" in
           i?86)
              opt="-mcpu=${host_cpu}"
              AC_CHECK_CXX_OPT([$opt], [OPTS="$OPTS $opt"])
              ;;
        esac
     fi
   else
     if test x$CXX = xcl.exe ; then
	OPTS="$OPTS $VCOPTS_COMMON /Od /Z7"
     fi
     AC_DEFINE(DEBUG_MODE,1,[Define when compiling in debug mode])
   fi
   if test x$CXX != xcl.exe ; then
      AC_CHECK_CXX_OPT([-Wall],[OPTS="$OPTS -Wall"])
   fi
   case x"$ac_debug" in
changequote(<<, >>)dnl
     x[0-9])  OPTS="$OPTS -DDEBUGLVL=$ac_debug" ;;
     xr*)   OPTS="$OPTS -DRUNTIME_DEBUG_ONLY" ;;
changequote([, ])dnl 
   esac
   CXXFLAGS="$CXXFLAGS $OPTS"
   CFLAGS="$CFLAGS $OPTS"
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_MEMBER_TEMPLATES
dnl If the compiler supports member templates, 
dnl define HAVE_MEMBER_TEMPLATES.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_MEMBER_TEMPLATES],
[AC_CACHE_CHECK(whether the compiler supports member templates,
ac_cv_cxx_member_templates,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
template<class T, int N> class A
{ public:
  template<int N2> A<T,N> operator=(const A<T,N2>& z) { return A<T,N>(); }
};],[A<double,4> x; A<double,7> y; x = y; return 0;],
 ac_cv_cxx_member_templates=yes, ac_cv_cxx_member_templates=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_member_templates" = yes; then
  AC_DEFINE(HAVE_MEMBER_TEMPLATES,1,
        [define if the compiler supports member templates])
fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_NAMESPACES
dnl Define HAVE_NAMESPACES if the compiler supports
dnl namespaces.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[ AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                 [using namespace Outer::Inner; return i;],
                 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
  AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes && test "$ac_debug" = no; then
  AC_DEFINE(HAVE_NAMESPACES,1,
             [define if the compiler implements namespaces])
fi
])



dnl -------------------------------------------------------
dnl @synopsis AC_CXX_TYPENAME
dnl Define HAVE_TYPENAME if the compiler recognizes 
dnl keyword typename.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_TYPENAME],
[AC_CACHE_CHECK(whether the compiler recognizes typename,
ac_cv_cxx_typename,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([template<typename T>class X {public:X(){}};],
[X<float> z; return 0;],
 ac_cv_cxx_typename=yes, ac_cv_cxx_typename=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_typename" = yes; then
  AC_DEFINE(HAVE_TYPENAME,1,[define if the compiler recognizes typename])
fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_STDINCLUDES
dnl Define HAVE_STDINCLUDES if the compiler has the
dnl new style include files (without the .h)
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_STDINCLUDES],
[AC_CACHE_CHECK(whether the compiler comes with standard includes,
ac_cv_cxx_stdincludes,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <new>
struct X { int a; X(int a):a(a){}; };
X* foo(void *x) { return new(x) X(2); } ],[],
 ac_cv_cxx_stdincludes=yes, ac_cv_cxx_stdincludes=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_stdincludes" = yes; then
  AC_DEFINE(HAVE_STDINCLUDES,1,
    [define if the compiler comes with standard includes])
fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_BOOL
dnl If the compiler recognizes bool as a separate built-in type,
dnl define HAVE_BOOL. Note that a typedef is not a separate
dnl type since you cannot overload a function such that it 
dnl accepts either the basic type or the typedef.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_BOOL],
[AC_CACHE_CHECK(whether the compiler recognizes bool as a built-in type,
ac_cv_cxx_bool,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(char x){return 1;}
int f(bool x){return 1;}
],[bool b = true; return f(b);],
 ac_cv_cxx_bool=yes, ac_cv_cxx_bool=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_bool" = yes; then
  AC_DEFINE(HAVE_BOOL,1,[define if bool is a built-in type])
fi
])

dnl -------------------------------------------------------
dnl @synopsis AC_CXX_EXCEPTIONS
dnl If the C++ compiler supports exceptions handling (try,
dnl throw and catch), define HAVE_EXCEPTIONS.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_EXCEPTIONS],
[AC_CACHE_CHECK(whether the compiler supports exceptions,
ac_cv_cxx_exceptions,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[try { throw  1; } catch (int i) { return i; }],
 ac_cv_cxx_exceptions=yes, ac_cv_cxx_exceptions=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_exceptions" = yes; then
  AC_DEFINE(HAVE_EXCEPTIONS,1,[define if the compiler supports exceptions])
fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_RPO
dnl Defines option --enable-rpo and searches program RPO.
dnl Set output variables CXXRPOFLAGS and RPO. 
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_RPO],
[ CXXRPOFLAGS=
  RPO_YES='#'
  RPO_NO=''
  if test x$GXX = xyes ; then
    AC_ARG_ENABLE([rpo],
      AC_HELP_STRING([--enable-rpo],
                     [Enable compilation with option -frepo]),
      [ac_rpo=$enableval], [ac_rpo=no] )
    if test x$ac_rpo != xno ; then
      CXXRPOFLAGS='-frepo -fno-rtti'
      RPO_YES=''
      RPO_NO='#'
    fi
  fi
  AC_SUBST(CXXRPOFLAGS)
  AC_SUBST(RPO_YES)
  AC_SUBST(RPO_NO)
])


dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_GNUWIN32([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process option --with-gnuwin32.
dnl Try to determine where GNUWIN32 is located
dnl Define GNUWIN32 accordingly (either blank or PATH)
dnl Also update the LDFLAGS and CFLAGS accordingly
dnl ------------------------------------------------------------------
AC_DEFUN([AC_PATH_GNUWIN32],
[
  # Test whether we are running on Windows. This test is becoming
  # useless because other environments exist under Windows, such
  # as mingw, whihc gives host i686-pc-mingw32. Just remove this test
  # for now by setting variable to yes all the time.
#   AC_REQUIRE([AC_CANONICAL_HOST])[]dnl
#   case $host_os in
#     *cygwin* ) USING_CYGWIN_OR_MINGW=yes;;
#      *mingw* ) USING_CYGWIN_OR_MINGW=yes;;
#            * ) USING_CYGWIN_OR_MINGW=no;;
#   esac
  USING_CYGWIN_OR_MINGW=yes
  # If running on Windows, do some tests
  if test x$USING_CYGWIN_OR_MINGW = xyes ; then
    AC_ARG_VAR(GNUWIN32_DIR,[Base directory of GnuWin32 packages])
    ac_gnuwin32=no
    AC_ARG_WITH(gnuwin32,
       AC_HELP_STRING([--with-gnuwin32=DIR],
                      [where the GnuWin32 packages are installed]),
       [ac_gnuwin32=$withval], [ac_gnuwin32=yes] )
    # Process specification
    AC_MSG_CHECKING([for GnuWin32 directory])
    if test x$ac_gnuwin32 = xyes ; then
       # GNUWIN32_BASE could have been set on the command line
       if test x$GNUWIN32_BASE != x ; then
          if test -d "$GNUWIN32_BASE" ; then
             AC_MSG_RESULT([verified at $GNUWIN32_BASE])
          else
             # AC_MSG_RESULT([no GnuWin32 at $GNUWIN32_BASE. Looking elsewhere])
             GNUWIN32_BASE=
          fi
       # Otherwise, GNUWIN32 is an environment variable that the user can set
       elif test x$GNUWIN32 != x ; then
          if test -d "$GNUWIN32" ; then
             GNUWIN32_BASE=$GNUWIN32
             AC_MSG_RESULT([verified at $GNUWIN32_BASE])
          else
             # AC_MSG_RESULT([no GnuWin32 at $GNUWIN32. Looking elsewhere])
             GNUWIN32_BASE=
          fi
       fi
       # Look in default locations if needed:
       if test x$GNUWIN32_BASE = x ; then
          if test -d "C:/Program Files/GnuWin32" ; then
             GNUWIN32_BASE="C:/Program Files/GnuWin32"
             AC_MSG_RESULT([$GNUWIN32_BASE])
          elif test -d "C:/pckg/GnuWin32" ; then
	     GNUWIN32_BASE="C:/pckg/GnuWin32"
             AC_MSG_RESULT([$GNUWIN32_BASE])
          else
	     AC_MSG_RESULT([not found])
          fi
       fi
    # If directory location specified on command line
    elif test x$ac_gnuwin32 != xno ; then
       if test -d "$ac_gnuwin32" ; then
	  GNUWIN32_BASE="$ac_gnuwin32"
          AC_MSG_RESULT([verified at $GNUWIN32_BASE])
       else
          GNUWIN32_BASE=
          AC_MSG_RESULT([not found])
       fi
    fi
    # Now we can update LDFLAGS, CFLAGS and CXXFLAGS
    # Do it in such a way that GnuWin32 has precedence
    # over system includes and libraries
    if test x$GNUWIN32_BASE != x ; then
       CFLAGS="-I$GNUWIN32_BASE/include $CFLAGS"
       CXXFLAGS="-I$GNUWIN32_BASE/include $CXXFLAGS"
       if test x$CXX != xcl.exe ; then
          LDFLAGS="-L$GNUWIN32_BASE/lib $LDFLAGS"
       fi
    fi
  # If not running on cygwin, GNUWIN32 is useless
  else
    GNUWIN32_BASE=
  fi
  # Finally, sets automake conditional
  AM_CONDITIONAL(HAVE_GNUWIN32, test x$GNUWIN32_BASE != x)
])



dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_JPEG([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process option --with-jpeg.
dnl Search JPEG along the extra
dnl Define HAVE_JPEG.
dnl Set output variable JPEG_CFLAGS and JPEG_LIBS
dnl Designed by Leon. Unused at the moment.
dnl ------------------------------------------------------------------

AC_DEFUN([AC_PATH_JPEG],
[
  AC_ARG_VAR(JPEG_LIBS)
  AC_ARG_VAR(JPEG_CFLAGS)
  ac_jpeg=no
  AC_ARG_WITH(jpeg,
     AC_HELP_STRING([--with-jpeg=DIR],
                    [where the IJG jpeg library is located]),
     [ac_jpeg=$withval], [ac_jpeg=yes] )
  # Process specification
  if test x$ac_jpeg = xyes ; then
     test x${JPEG_LIBS+set} != xset && JPEG_LIBS="-ljpeg"
  elif test x$ac_jpeg != xno ; then
     test x${JPEG_LIBS+set} != xset && JPEG_LIBS="-L$ac_jpeg -ljpeg"
     test x${JPEG_CFLAGS+set} != xset && JPEG_CFLAGS="-I$ac_jpeg"
  fi
  # Try linking
  if test x$ac_jpeg != xno ; then
     AC_MSG_CHECKING([for jpeg library])
     save_CFLAGS="$CFLAGS"
     save_CXXFLAGS="$CXXFLAGS"
     save_LIBS="$LIBS"
     CFLAGS="$CFLAGS $JPEG_CFLAGS"
     CXXFLAGS="$CXXFLAGS $JPEG_CFLAGS"
     LIBS="$LIBS $JPEG_LIBS"
     AC_TRY_LINK([
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h> 
#include <jpeglib.h>
#ifdef __cplusplus
}
#endif ],[
jpeg_CreateDecompress(0,0,0);],
       [ac_jpeg=yes], [ac_jpeg=no] )
     CFLAGS="$save_CFLAGS"
     CXXFLAGS="$save_CXXFLAGS"
     LIBS="$save_LIBS"
     AC_MSG_RESULT($ac_jpeg)
   fi
   # Finish
   if test x$ac_jpeg = xno; then
      JPEG_CFLAGS= ; JPEG_LIBS=
      ifelse([$2],,:,[$2])
   else
      AC_DEFINE(HAVE_JPEG,1,[Define if you have the IJG JPEG library.])
      AC_MSG_RESULT([setting JPEG_CFLAGS=$JPEG_CFLAGS])
      AC_MSG_RESULT([setting JPEG_LIBS=$JPEG_LIBS])
      ifelse([$1],,:,[$1])
   fi
])


dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_LIBTIFF([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process option --with-libtiff.
dnl Search LIBTIFF along the extra
dnl Define HAVE_LIBTIFF.
dnl Set output variable LIBTIFF_CFLAGS and LIBTIFF_LIBS
dnl -- inspired from previous AC_PATH_JPEG
dnl ------------------------------------------------------------------
AC_DEFUN([AC_PATH_LIBTIFF],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case $host in
    *msdos* | *go32* | *mingw32* | *cygwin* | *windows*)
      USING_WIN=yes
      ;;
    *)
      USING_WIN=no
  esac
  AC_REQUIRE([AC_PATH_GNUWIN32])
  AC_ARG_VAR(LIBTIFF_LIBS,[Tiff library to link against])
  AC_ARG_VAR(LIBTIFF_CFLAGS,[Compile flags needed for TIFF support])
  ac_libtiff=no
  AC_ARG_WITH(libtiff,
     AC_HELP_STRING([--with-libtiff=DIR],
                    [where the www.libtiff.org libtiff library is located]),
     [ac_libtiff=$withval], [ac_libtiff=yes] )
  AC_MSG_CHECKING([for Leffler libtiff library])
  # Need to define TOP_SRCDIR for the cases where the LIBTIFF library
  # is checked in with the code, at the top-level
  TOP_SRCDIR=`cd $srcdir; pwd`
  LOCAL_LIBTIFFDIR=$TOP_SRCDIR/libtiff
  # First of all, deal with the case when 'cl.exe' is used as compiler
  # indeed, when this is the case, we can only rely on finding
  # the right library and includes, but we can't try compiling
  # and linking. In addition, library needs to be specifically
  # supplied on the link line and special flags are needed...
  if test "x$CXX" = "xcl.exe" ; then
    if test "x$ac_libtiff" = "xyes" ; then
      # If LIBTIFF_LIBS has been set on configure command line
      # or as environment variable, just use it if it exists
      if test "x$LIBTIFF_LIBS" != "x" ; then
        AC_MSG_RESULT(user specified as $LIBTIFF_LIBS)
        if test "x$LIBTIFF_CFLAGS" = "x" ; then
          AC_MSG_WARN(LIBTIFF_CFLAGS is empty)
        fi
      fi
      # Otherwise, test if libtiff is at top level directory, under libtiff
      if test "x$LIBTIFF_LIBS" = "x" && test -r "$LOCAL_LIBTIFFDIR/lib/libtiff.a" && test -r "$LOCAL_LIBTIFFDIR/lib/libjpeg.a" && test -r "$LOCAL_LIBTIFFDIR/lib/libz.a" ; then
        AC_MSG_RESULT($LOCAL_LIBTIFFDIR/lib/libtiff.a)
        LIBTIFF_LIBS="$LOCAL_LIBTIFFDIR/lib/libtiff.a $LOCAL_LIBTIFFDIR/lib/libjpeg.a $LOCAL_LIBTIFFDIR/lib/libz.a user32.lib"
        LIBTIFF_CFLAGS="-I$LOCAL_LIBTIFFDIR/include"
      fi
      # Otherwise, if GnuWin32 was previously located
      if test "x$LIBTIFF_LIBS" = "x" && test -r "$GNUWIN32_BASE/lib/libtiff.a" && test -r "$GNUWIN32_BASE/lib/libjpeg.a" && test -r "$GNUWIN32_BASE/lib/libz.a" ; then
        AC_MSG_RESULT($GNUWIN32_BASE/lib/libtiff.a)
        AC_MSG_WARN($GNUWIN32_BASE/lib/libtiff.a version 3.5.7 works. Some versions like 3.6.1 do not!)
        LIBTIFF_LIBS="$GNUWIN32_BASE/lib/libtiff.a $GNUWIN32_BASE/lib/libjpeg.a $GNUWIN32_BASE/lib/libz.a user32.lib"
      fi
      if test "x$LIBTIFF_LIBS" = "x" ; then
        AC_MSG_RESULT(not found or incomplete)
        ac_libtiff=no
      fi
    elif test "x$ac_libtiff" != "xno" ; then
      test x${LIBTIFF_LIBS+set} != xset && LIBTIFF_LIBS="$ac_libtiff/libtiff"
      test x${LIBTIFF_CFLAGS+set} != xset && LIBTIFF_CFLAGS="-I$ac_libtiff"
    fi
  # If we are not using CL, that is we are either using gcc/cygwin
  # or we are not running under Windows:
  else
    # Process specification
    if test "x$ac_libtiff" = "xyes" ; then
       if test "x$LIBTIFF_LIBS" = "x" ; then
          # If local libtiff exists at top level, and we are running windows, use it first:
          if test "x$USING_WIN" = "xyes" && test -r "$LOCAL_LIBTIFFDIR/lib/libtiff.a" && test -r "$LOCAL_LIBTIFFDIR/lib/libjpeg.a" && test -r "$LOCAL_LIBTIFFDIR/lib/libz.a" ; then
             AC_MSG_RESULT($LOCAL_LIBTIFFDIR/lib/libtiff.a)
             LIBTIFF_LIBS="$LOCAL_LIBTIFFDIR/lib/libtiff.a $LOCAL_LIBTIFFDIR/lib/libjpeg.a $LOCAL_LIBTIFFDIR/lib/libz.a -lole32 -luuid -lwsock32"
             LIBTIFF_CFLAGS="-I$LOCAL_LIBTIFFDIR/include"
          fi
          # If GNUWIN32_BASE is defined, it means we are running under
          # Windows and we should use these builds if available because
          # they are the best bet
          if test "x$LIBTIFF_LIBS" = "x" && test "x$GNUWIN32_BASE" != "x" ; then
             # With version 3.5.7 of gnuwin32 libtiff, we do not need to link
             # against "$GNUWIN32_BASE/lib/libgw32c.a", so no need to test
             if test -r "$GNUWIN32_BASE/lib/libtiff.a" && test -r "$GNUWIN32_BASE/lib/libjpeg.a" && test -r "$GNUWIN32_BASE/lib/libz.a" ; then
                AC_MSG_RESULT($GNUWIN32_BASE/lib/libtiff.a)
                # With 3.5.7 version of libtiff, no need to add
                # $GNUWIN32_BASE/lib/libgw32c.a after  $GNUWIN32_BASE/lib/libz.a
                LIBTIFF_LIBS="$GNUWIN32_BASE/lib/libtiff.a $GNUWIN32_BASE/lib/libjpeg.a $GNUWIN32_BASE/lib/libz.a -lole32 -luuid -lwsock32"
             else
                AC_MSG_RESULT([GNUWIN32 not found or incomplete. Trying something else])
             fi
          fi
          # If LIBTIFF_LIBS is still not defined after potentially going
          # the GNUWIN32 route, then try using home built versions
          if test "x$LIBTIFF_LIBS" = "x" ; then
             TIFF_PACKAGE="$HOME_UNIX/packages/libtiff/libtiff"
             if test -d "$TIFF_PACKAGE" ; then
                LIBTIFF_LIBS="$TIFF_PACKAGE/libtiff.a"
                LIBTIFF_CFLAGS="-I$TIFF_PACKAGE"
             else
	        LIBTIFF_LIBS="-ltiff"
             fi
          fi
       fi
    elif test "x$ac_libtiff" != "xno" ; then
       test "x${LIBTIFF_LIBS+set}" != "xset" && LIBTIFF_LIBS="$ac_libtiff/libtiff.a"
       test "x${LIBTIFF_CFLAGS+set}" != "xset" && LIBTIFF_CFLAGS="-I$ac_libtiff"
    fi
    # Try linking - recall that -Wall is typically on, and any warning
    # will cause this test to fail... This, by the way, is one of the
    # reasons we cannot run this test when using the Microsoft compiler (cl),
    # which outputs tons of "garbage" on stdout:
    if test "x$ac_libtiff" != "xno" ; then
       AC_MSG_CHECKING([linking with $LIBTIFF_LIBS])
       save_CFLAGS="$CFLAGS"
       save_CXXFLAGS="$CXXFLAGS"
       save_LIBS="$LIBS"
       CFLAGS="$CFLAGS $LIBTIFF_CFLAGS"
       CXXFLAGS="$CXXFLAGS $LIBTIFF_CFLAGS"
       LIBS="$LIBS $LIBTIFF_LIBS -lm"
       AC_TRY_LINK([
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h> 
#include <tiffio.h>
#ifdef __cplusplus
}
#endif ],[
TIFFClose((TIFF *) 0);],
       [ac_libtiff=ok],[ac_libtiff=no])
       CFLAGS="$save_CFLAGS"
       CXXFLAGS="$save_CXXFLAGS"
       LIBS="$save_LIBS"
       AC_MSG_RESULT($ac_libtiff)
     fi
  fi
  # Finish
  if test "x$ac_libtiff" = "xno"; then
     LIBTIFF_CFLAGS= ; LIBTIFF_LIBS=
     ifelse([$2],,:,[$2])
  else
     AC_DEFINE(HAVE_LIBTIFF,1,[Define if you have the www.libtiff.org LIBTIFF library.])
     AC_MSG_RESULT([setting LIBTIFF_CFLAGS=$LIBTIFF_CFLAGS])
     AC_MSG_RESULT([setting LIBTIFF_LIBS=$LIBTIFF_LIBS])
     CFLAGS="$LIBTIFF_CFLAGS $CFLAGS"
     CXXFLAGS="$LIBTIFF_CFLAGS $CXXFLAGS"
     if test "x$CXX" = "xcl.exe" ; then
       LIBS="$LIBTIFF_LIBS $LIBS"
     else
       LIBS="$LIBTIFF_LIBS $LIBS -lm"
     fi
     ifelse([$1],,:,[$1])
  fi
  AM_CONDITIONAL(HAVE_LIBTIFF, test x$ac_libtiff != xno)
])
