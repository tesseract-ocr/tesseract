SUBDIRS =
AM_CXXFLAGS = -DTESSDATA_PREFIX=@datadir@/

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

EXTRA_DIST = mfcpch.cpp

include_HEADERS = \
    ambigs.h basedir.h bits16.h bitvector.h \
    tesscallback.h ccutil.h clst.h \
    elst2.h elst.h errcode.h \
    fileerr.h genericvector.h globaloc.h \
    hashfn.h helpers.h host.h hosthplb.h indexmapbidi.h lsterr.h \
    memry.h mfcpch.h \
    ndminx.h notdll.h nwmain.h \
    ocrclass.h platform.h qrsequence.h \
    secname.h serialis.h sorthelper.h stderr.h strngs.h \
    tessdatamanager.h tprintf.h \
    unichar.h unicharmap.h unicharset.h unicity_table.h unicodes.h \
    params.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_ccutil.la
else
lib_LTLIBRARIES = libtesseract_ccutil.la
libtesseract_ccutil_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
endif

libtesseract_ccutil_la_SOURCES = \
    ambigs.cpp basedir.cpp bits16.cpp bitvector.cpp \
    ccutil.cpp clst.cpp \
    elst2.cpp elst.cpp errcode.cpp \
    globaloc.cpp hashfn.cpp indexmapbidi.cpp \
    mainblk.cpp memry.cpp \
    serialis.cpp strngs.cpp \
    tessdatamanager.cpp tprintf.cpp \
    unichar.cpp unicharmap.cpp unicharset.cpp unicodes.cpp \
    params.cpp

if EMBEDDED
include_HEADERS += scanutils.h
libtesseract_ccutil_la_SOURCES += scanutils.cpp
endif
