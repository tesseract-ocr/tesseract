AM_CPPFLAGS += -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer -DUSE_STD_NAMESPACE

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    bitvec.h callcpp.h const.h cutil.h cutil_class.h danerror.h efio.h \
    emalloc.h globals.h \
    oldlist.h structures.h

noinst_LTLIBRARIES = libtesseract_cutil.la

libtesseract_cutil_la_SOURCES = \
    bitvec.cpp callcpp.cpp cutil.cpp cutil_class.cpp danerror.cpp efio.cpp \
    emalloc.cpp \
    oldlist.cpp structures.cpp
