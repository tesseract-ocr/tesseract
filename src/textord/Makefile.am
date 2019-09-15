AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/ccmain \
    -I$(top_srcdir)/src/wordrec \
    -I$(top_srcdir)/src/api \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/classify \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/opencl

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

noinst_HEADERS = \
    alignedblob.h baselinedetect.h bbgrid.h blkocc.h blobgrid.h \
    ccnontextdetect.h cjkpitch.h colfind.h colpartition.h colpartitionset.h \
    colpartitiongrid.h \
    devanagari_processing.h drawtord.h edgblob.h edgloop.h \
    fpchop.h gap_map.h imagefind.h linefind.h makerow.h oldbasel.h \
    pithsync.h pitsync1.h scanedg.h sortflts.h strokewidth.h \
    tabfind.h tablefind.h tabvector.h \
    tablerecog.h textlineprojection.h textord.h \
    topitch.h tordmain.h tovars.h \
    underlin.h wordseg.h workingpartset.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += equationdetectbase.h
endif

noinst_LTLIBRARIES = libtesseract_textord.la

libtesseract_textord_la_SOURCES = \
    alignedblob.cpp baselinedetect.cpp bbgrid.cpp blkocc.cpp blobgrid.cpp \
    ccnontextdetect.cpp cjkpitch.cpp colfind.cpp colpartition.cpp colpartitionset.cpp \
    colpartitiongrid.cpp devanagari_processing.cpp \
    drawtord.cpp edgblob.cpp edgloop.cpp \
    fpchop.cpp gap_map.cpp imagefind.cpp linefind.cpp makerow.cpp oldbasel.cpp \
    pithsync.cpp pitsync1.cpp scanedg.cpp sortflts.cpp strokewidth.cpp \
    tabfind.cpp tablefind.cpp tabvector.cpp \
    tablerecog.cpp textlineprojection.cpp textord.cpp \
    topitch.cpp tordmain.cpp tospace.cpp tovars.cpp \
    underlin.cpp wordseg.cpp workingpartset.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_textord_la_SOURCES += equationdetectbase.cpp
endif
