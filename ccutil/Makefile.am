SUBDIRS =
AM_CXXFLAGS =

if !NO_TESSDATA_PREFIX
AM_CXXFLAGS += -DTESSDATA_PREFIX=@datadir@/
endif

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

EXTRA_DIST = mfcpch.cpp

include_HEADERS = \
	basedir.h errcode.h fileerr.h genericvector.h helpers.h host.h memry.h \
	ndminx.h params.h platform.h serialis.h strngs.h tesscallback.h \
	unichar.h unicharmap.h unicharset.h
     
noinst_HEADERS = \
    ambigs.h bits16.h bitvector.h ccutil.h clst.h elst2.h \
    elst.h globaloc.h hashfn.h hosthplb.h indexmapbidi.h lsterr.h \
    mfcpch.h notdll.h nwmain.h ocrclass.h qrsequence.h secname.h \
    sorthelper.h stderr.h tessdatamanager.h tprintf.h unicity_table.h \
    unicodes.h 

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

if MINGW
AM_CPPFLAGS += -I$(top_srcdir)/vs2008/port -DWINDLLNAME=\"lib@GENERIC_LIBRARY_NAME@\"
noinst_HEADERS += ../vs2008/port/strtok_r.h
libtesseract_ccutil_la_SOURCES += ../vs2008/port/strtok_r.cpp
endif