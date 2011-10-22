ACLOCAL_AMFLAGS = -I m4
SUBDIRS = ccutil viewer cutil image ccstruct dict classify wordrec neural_networks/runtime textord cube ccmain api . training java tessdata testing doc
#if USING_GETTEXT
#SUBDIRS += po
#AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
#endif

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes \
      aclocal.m4 config configure.ac autogen.sh tesseract.spec contrib

#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .deps`
	rm -rf `find $(distdir) -name Makefile.in`
