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
noinst_HEADERS += cluster.h
noinst_HEADERS += clusttool.h
noinst_HEADERS += featdefs.h
noinst_HEADERS += float2int.h
noinst_HEADERS += fpoint.h
noinst_HEADERS += intfeaturedist.cpp
noinst_HEADERS += intfeaturemap.cpp
noinst_HEADERS += intfeaturespace.h
noinst_HEADERS += intfx.h
noinst_HEADERS += intmatcher.h
noinst_HEADERS += intproto.h
noinst_HEADERS += kdtree.h
noinst_HEADERS += mf.h
noinst_HEADERS += mfdefs.h
noinst_HEADERS += mfoutline.h
noinst_HEADERS += mfx.h
noinst_HEADERS += normfeat.h
noinst_HEADERS += normmatch.h
noinst_HEADERS += ocrfeatures.h
noinst_HEADERS += outfeat.h
noinst_HEADERS += picofeat.h
noinst_HEADERS += protos.h
noinst_HEADERS += sampleiterator.h
noinst_HEADERS += shapeclassifier.h
noinst_HEADERS += shapetable.h
noinst_HEADERS += tessclassifier.h
noinst_HEADERS += trainingsample.h
noinst_HEADERS += trainingsampleset.h
endif

noinst_LTLIBRARIES = libtesseract_classify.la

libtesseract_classify_la_SOURCES = classify.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_classify_la_SOURCES += adaptive.cpp
libtesseract_classify_la_SOURCES += adaptmatch.cpp
libtesseract_classify_la_SOURCES += blobclass.cpp
libtesseract_classify_la_SOURCES += cluster.cpp
libtesseract_classify_la_SOURCES += clusttool.cpp
libtesseract_classify_la_SOURCES += cutoffs.cpp
libtesseract_classify_la_SOURCES += featdefs.cpp
libtesseract_classify_la_SOURCES += float2int.cpp
libtesseract_classify_la_SOURCES += fpoint.cpp
libtesseract_classify_la_SOURCES += intfeaturedist.cpp
libtesseract_classify_la_SOURCES += intfeaturemap.cpp
libtesseract_classify_la_SOURCES += intfeaturespace.cpp
libtesseract_classify_la_SOURCES += intfx.cpp
libtesseract_classify_la_SOURCES += intmatcher.cpp
libtesseract_classify_la_SOURCES += intproto.cpp
libtesseract_classify_la_SOURCES += kdtree.cpp
libtesseract_classify_la_SOURCES += mf.cpp
libtesseract_classify_la_SOURCES += mfdefs.cpp
libtesseract_classify_la_SOURCES += mfoutline.cpp
libtesseract_classify_la_SOURCES += mfx.cpp
libtesseract_classify_la_SOURCES += normfeat.cpp
libtesseract_classify_la_SOURCES += normmatch.cpp
libtesseract_classify_la_SOURCES += ocrfeatures.cpp
libtesseract_classify_la_SOURCES += outfeat.cpp
libtesseract_classify_la_SOURCES += picofeat.cpp
libtesseract_classify_la_SOURCES += protos.cpp
libtesseract_classify_la_SOURCES += sampleiterator.cpp
libtesseract_classify_la_SOURCES += shapeclassifier.cpp
libtesseract_classify_la_SOURCES += shapetable.cpp
libtesseract_classify_la_SOURCES += tessclassifier.cpp
libtesseract_classify_la_SOURCES += trainingsample.cpp
libtesseract_classify_la_SOURCES += trainingsampleset.cpp
endif
