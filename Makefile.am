# TODO(luc) Add 'doc' to this list when ready
SUBDIRS = ccstruct ccutil classify cutil dict display image textord viewer wordrec ccmain training tessdata

EXTRA_DIST = tessdata phototest.tif tesseract.dsp tesseract.dsw StdAfx.cpp StdAfx.h tessdll.cpp tessdll.h tessdll.dsp tessdll.dll tessdll.lib
#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

dist-hook:
# Need to remove CVS directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name CVS`
# Also remove extra files not needed in a distribution
	rm -rf `find $(distdir) -name configure.ac`
	rm -rf `find $(distdir) -name acinclude.m4`
	rm -rf `find $(distdir) -name aclocal.m4`
