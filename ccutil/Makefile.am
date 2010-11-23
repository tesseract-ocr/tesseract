SUBDIRS =
AM_CXXFLAGS = -DTESSDATA_PREFIX=@datadir@/

EXTRA_DIST = mfcpch.cpp scanutils.cpp scanutils.h

include_HEADERS = \
    ambigs.h basedir.h bits16.h boxread.h \
    tesscallback.h ccutil.h clst.h \
    debugwin.h elst2.h elst.h errcode.h \
    fileerr.h genericvector.h globaloc.h \
    hashfn.h helpers.h host.h hosthplb.h lsterr.h \
    memblk.h memry.h memryerr.h mfcpch.h \
    ndminx.h notdll.h nwmain.h \
    ocrclass.h platform.h qrsequence.h \
    secname.h serialis.h sorthelper.h stderr.h strngs.h \
    tessdatamanager.h tprintf.h \
    unichar.h unicharmap.h unicharset.h unicity_table.h \
    params.h

lib_LTLIBRARIES = libtesseract_ccutil.la
libtesseract_ccutil_la_SOURCES = \
    ambigs.cpp basedir.cpp bits16.cpp boxread.cpp \
    ccutil.cpp clst.cpp debugwin.cpp \
    elst2.cpp elst.cpp errcode.cpp \
    globaloc.cpp hashfn.cpp \
    mainblk.cpp memblk.cpp memry.cpp \
    serialis.cpp strngs.cpp \
    tessdatamanager.cpp tprintf.cpp \
    unichar.cpp unicharmap.cpp unicharset.cpp \
    params.cpp

libtesseract_ccutil_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
