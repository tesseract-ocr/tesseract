AM_CPPFLAGS += \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/training \
    -I$(top_srcdir)/src/viewer

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = classify.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += adaptive.h
noinst_HEADERS += blobclass.h
noinst_HEADERS += cluster.h \
    clusttool.h \
    featdefs.h \
    float2int.h \
    fpoint.h \
    intfeaturespace.h \
    intfx.h intmatcher.h \
    intproto.h kdtree.h \
    mf.h \
    mfdefs.h \
    mfoutline.h \
    mfx.h \
    normfeat.h \
    normmatch.h \
    ocrfeatures.h \
    outfeat.h \
    picofeat.h \
    protos.h \
    shapeclassifier.h \
    shapetable.h \
    tessclassifier.h \
    trainingsample.h
endif

noinst_LTLIBRARIES = libtesseract_classify.la

libtesseract_classify_la_SOURCES = classify.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_classify_la_SOURCES += adaptive.cpp
libtesseract_classify_la_SOURCES += adaptmatch.cpp
libtesseract_classify_la_SOURCES += blobclass.cpp
libtesseract_classify_la_SOURCES += cluster.cpp \
    clusttool.cpp \
    cutoffs.cpp \
    featdefs.cpp \
    float2int.cpp \
    fpoint.cpp \
    intfeaturespace.cpp \
    intfx.cpp \
    intmatcher.cpp \
    intproto.cpp \
    kdtree.cpp \
    mf.cpp mfdefs.cpp \
    mfoutline.cpp \
    mfx.cpp \
    normfeat.cpp \
    normmatch.cpp \
    ocrfeatures.cpp \
    outfeat.cpp \
    picofeat.cpp \
    protos.cpp \
    shapeclassifier.cpp \
    shapetable.cpp \
    tessclassifier.cpp \
    trainingsample.cpp
endif
