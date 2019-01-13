SUBDIRS =
AM_CXXFLAGS =
AM_CPPFLAGS =

if !NO_TESSDATA_PREFIX
AM_CXXFLAGS += -DTESSDATA_PREFIX=@datadir@
endif

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

pkginclude_HEADERS = \
    genericvector.h helpers.h host.h \
    ocrclass.h platform.h serialis.h strngs.h \
    tesscallback.h unichar.h

noinst_HEADERS = \
    ambigs.h basedir.h bits16.h bitvector.h ccutil.h clst.h doubleptr.h elst2.h \
    elst.h errcode.h fileerr.h fileio.h \
    genericheap.h globaloc.h \
    indexmapbidi.h kdpair.h lsterr.h \
    object_cache.h params.h qrsequence.h sorthelper.h \
    scanutils.h tessdatamanager.h tprintf.h \
    unicharcompress.h unicharmap.h unicharset.h unicity_table.h unicodes.h \
    universalambigs.h

noinst_LTLIBRARIES = libtesseract_ccutil.la

libtesseract_ccutil_la_SOURCES = \
    ambigs.cpp basedir.cpp bitvector.cpp \
    ccutil.cpp clst.cpp \
    elst2.cpp elst.cpp errcode.cpp \
    fileio.cpp \
    globaloc.cpp indexmapbidi.cpp \
    mainblk.cpp \
    serialis.cpp strngs.cpp scanutils.cpp \
    tessdatamanager.cpp tprintf.cpp \
    unichar.cpp unicharcompress.cpp unicharmap.cpp unicharset.cpp unicodes.cpp \
    params.cpp universalambigs.cpp

if T_WIN
AM_CPPFLAGS += -DWINDLLNAME=\"lib@GENERIC_LIBRARY_NAME@\"
endif
