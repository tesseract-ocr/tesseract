SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/ccutil

EXTRA_DIST = cutil.vcproj

include_HEADERS = \
    bitvec.h callcpp.h const.h cutil.h cutil_class.h danerror.h efio.h \
    emalloc.h freelist.h funcdefs.h general.h globals.h listio.h \
    oldheap.h oldlist.h structures.h tessarray.h

lib_LTLIBRARIES = libtesseract_cutil.la
libtesseract_cutil_la_SOURCES = \
    bitvec.cpp cutil.cpp cutil_class.cpp danerror.cpp efio.cpp \
    emalloc.cpp freelist.cpp globals.cpp listio.cpp oldheap.cpp \
    oldlist.cpp structures.cpp tessarray.cpp

libtesseract_cutil_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
