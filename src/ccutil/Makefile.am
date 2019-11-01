AM_CXXFLAGS =
AM_CPPFLAGS =

if !NO_TESSDATA_PREFIX
AM_CXXFLAGS += -DTESSDATA_PREFIX=@datadir@
endif

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

pkginclude_HEADERS = \
    genericvector.h helpers.h \
    ocrclass.h platform.h serialis.h strngs.h \
    tesscallback.h unichar.h

noinst_HEADERS = \
    bits16.h ccutil.h clst.h doubleptr.h elst2.h \
    elst.h errcode.h fileerr.h \
    genericheap.h globaloc.h host.h \
    kdpair.h lsterr.h \
    object_cache.h params.h qrsequence.h sorthelper.h \
    scanutils.h \
    tessdatamanager.h tprintf.h \
    unicharcompress.h unicharmap.h unicharset.h unicity_table.h unicodes.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += ambigs.h
noinst_HEADERS += bitvector.h
noinst_HEADERS += indexmapbidi.h
noinst_HEADERS += universalambigs.h
endif

noinst_LTLIBRARIES = libtesseract_ccutil.la

libtesseract_ccutil_la_SOURCES = \
    ccutil.cpp clst.cpp \
    elst2.cpp elst.cpp errcode.cpp \
    globaloc.cpp \
    mainblk.cpp \
    serialis.cpp strngs.cpp scanutils.cpp \
    tessdatamanager.cpp tprintf.cpp \
    unichar.cpp unicharcompress.cpp unicharmap.cpp unicharset.cpp unicodes.cpp \
    params.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_ccutil_la_SOURCES += ambigs.cpp
libtesseract_ccutil_la_SOURCES += bitvector.cpp
libtesseract_ccutil_la_SOURCES += indexmapbidi.cpp
libtesseract_ccutil_la_SOURCES += universalambigs.cpp
endif

AM_CPPFLAGS += $(libarchive_CFLAGS)
