AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/viewer

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    bitvec.h callcpp.h \
    emalloc.h \
    oldlist.h

noinst_LTLIBRARIES = libtesseract_cutil.la

libtesseract_cutil_la_SOURCES = \
    callcpp.cpp \
    emalloc.cpp \
    oldlist.cpp
