ACLOCAL_AMFLAGS = -I m4
SUBDIRS = ccstruct ccutil classify cube cutil dict image neural_networks/runtime textord viewer wordrec ccmain training tessdata testing java api doc
#if USING_GETTEXT
#SUBDIRS += po
#AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
#endif

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes \
      aclocal.m4 config configure.ac runautoconf tesseract.spec contrib

#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .deps`
	rm -rf `find $(distdir) -name Makefile.in`
