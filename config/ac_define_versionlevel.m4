dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/ac_define_versionlevel.html
dnl
AC_DEFUN([AC_DEFINE_VERSIONLEVEL],
[
ac_versionlevel_strdf=`echo ifelse($2, , $VERSION, $2) | sed -e 's:[[A-Z-]]*:.:' -e 's:[[^0-9.]]::g' -e 's:^[[.]]*::'`
dnl commented out by Luc, we do not need any output for the
dnl way this M4 macro is called at the moment.
dnl AC_MSG_CHECKING(versionlevel $ac_versionlevel_strdf)
case $ac_versionlevel_strdf in
 *.*.*.|*.*.*.*) :
 ac_versionlevel_major=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_major | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_patch=`echo $ac_versionlevel_minor | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_major=`echo $ac_versionlevel_major | sed -e 's/[[.]].*//'`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=`echo $ac_versionlevel_patch | sed -e 's/[[.]].*//'`
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
        + $ac_versionlevel_patch \
	+ 1` ;;
 *.*.*) :
 ac_versionlevel_major=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_major | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_patch=`echo $ac_versionlevel_minor | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_major=`echo $ac_versionlevel_major | sed -e 's/[[.]].*//'`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=`echo $ac_versionlevel_patch | sed -e 's/[[.]].*//'`
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
        + $ac_versionlevel_patch`               ;;
 *.*.) :
 ac_versionlevel_major=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_major | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_major=`echo $ac_versionlevel_major | sed -e 's/[[.]].*//'`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=0
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
	+ 1000 \
        + $ac_versionlevel_patch`               ;;
 *.*) :
 ac_versionlevel_major=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_major | sed -e 's/[[^.]]*[[.]]//'`
 ac_versionlevel_major=`echo $ac_versionlevel_major | sed -e 's/[[.]].*//'`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=0
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
        + $ac_versionlevel_patch`               ;;
 *.) :
 ac_versionlevel_major=0
 ac_versionlevel_minor=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=0
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
	+ 1000 \
        + $ac_versionlevel_patch`               ;;
 *) :
 ac_versionlevel_major=0
 ac_versionlevel_minor=`echo $ac_versionlevel_strdf`
 ac_versionlevel_minor=`echo $ac_versionlevel_minor | sed -e 's/[[.]].*//'`
 ac_versionlevel_patch=0
 $1=`expr $ac_versionlevel_major '*' 1000000 \
        + $ac_versionlevel_minor '*'   10000 \
        + $ac_versionlevel_patch`               ;;
esac
dnl Also commented out by Luc for now
dnl AC_MSG_RESULT($[$1])
AC_DEFINE_UNQUOTED( $1, $[$1], ifelse( $3, , $PACKAGE versionlevel, $3))
])

