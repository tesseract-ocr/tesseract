SUBDIRS =
AM_CPPFLAGS =

include_HEADERS = \
    scrollview.h svmnode.h svutil.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_viewer.la
else
lib_LTLIBRARIES = libtesseract_viewer.la
libtesseract_viewer_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
endif

libtesseract_viewer_la_SOURCES = \
    scrollview.cpp svmnode.cpp svutil.cpp svpaint.cpp


