# TODO(luc) Add 'doc' to this list when ready
SUBDIRS = ccstruct ccutil classify cutil dict pageseg image textord viewer wordrec ccmain training tessdata testing dlltest java

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes tesseract.spec config \
      tesseract.dsp tesseract.dsw tesseract.vcproj tesseract.sln \
      StdAfx.cpp StdAfx.h tessdll.cpp tessdll.h tessdll.dsp tessdll.vcproj

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
