SUBDIRS =
AM_CPPFLAGS =

EXTRA_DIST = viewer.vcproj

include_HEADERS = \
    scrollview.h svmnode.h svutil.h svpaint.cpp

lib_LTLIBRARIES = libtesseract_viewer.la
libtesseract_viewer_la_SOURCES = \
    scrollview.cpp svmnode.cpp svutil.cpp

libtesseract_viewer_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
