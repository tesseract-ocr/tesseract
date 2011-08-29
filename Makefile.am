ACLOCAL_AMFLAGS = -I m4
SUBDIRS = ccstruct ccutil classify cube cutil dict image neural_networks/runtime textord viewer wordrec ccmain training tessdata testing java api doc
#if USING_GETTEXT
#SUBDIRS += po
#AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
#endif

EXTRA_DIST = eurotext.tif phototest.tif ReleaseNotes \
      aclocal.m4 config configure.ac runautoconf tesseract.spec contrib

#EXTRA_DIST = doc/html doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.pdf doc/@PACKAGE_NAME@_@PACKAGE_VERSION@.ps.gz

lib_LTLIBRARIES = libtesseract.la
libtesseract_la_LIBADD = \
    api/libtesseract_api.la \
    ccmain/libtesseract_main.la \
    cube/libtesseract_cube.la \
    neural_networks/runtime/libtesseract_neural.la \
    textord/libtesseract_textord.la \
    wordrec/libtesseract_wordrec.la \
    classify/libtesseract_classify.la \
    dict/libtesseract_dict.la \
    ccstruct/libtesseract_ccstruct.la \
    image/libtesseract_image.la \
    cutil/libtesseract_cutil.la \
    viewer/libtesseract_viewer.la \
    ccutil/libtesseract_ccutil.la

libtesseract_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .svn`
	rm -rf `find $(distdir) -name .deps`
	rm -rf `find $(distdir) -name Makefile.in`
