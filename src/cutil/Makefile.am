AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/viewer

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = callcpp.h cutil_class.h 

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += bitvec.h
noinst_HEADERS += emalloc.h
noinst_HEADERS += oldlist.h
endif

noinst_LTLIBRARIES = libtesseract_cutil.la

libtesseract_cutil_la_SOURCES = \
    cutil_class.cpp \
    callcpp.cpp 

if !DISABLED_LEGACY_ENGINE
libtesseract_cutil_la_SOURCES += emalloc.cpp
libtesseract_cutil_la_SOURCES += oldlist.cpp
endif