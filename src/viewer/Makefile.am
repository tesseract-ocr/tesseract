AM_CPPFLAGS += -I$(top_srcdir)/src/ccutil

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    scrollview.h svmnode.h svutil.h

noinst_LTLIBRARIES = libtesseract_viewer.la

libtesseract_viewer_la_SOURCES = \
    scrollview.cpp svmnode.cpp svutil.cpp

# TODO: Add rule to generate svpaint from svpaint.cpp.
