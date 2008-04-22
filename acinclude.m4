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
