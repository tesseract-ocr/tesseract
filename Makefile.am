# TODO(luc) Add 'doc' to this list when ready
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = ccstruct ccutil classify cutil dict image textord viewer wordrec ccmain training tessdata testing java api vs2008
#if USING_GETTEXT
#SUBDIRS += po
#AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
#endif

EXTRA_DIST = config/config.rpath  eurotext.tif phototest.tif ReleaseNotes \
      acinclude.m4 config configure.ac runautoconf tesseract.spec doc contrib

#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

dist-hook:
# Need to remove CVS directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .deps`
# Also remove extra files not needed in a distribution
	rm -rf `find $(distdir) -name configure.ac`
	rm -rf `find $(distdir) -name acinclude.m4`
	rm -rf `find $(distdir) -name aclocal.m4`
