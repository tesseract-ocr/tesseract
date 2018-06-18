AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/arch \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/classify \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/lstm

SUBDIRS =
AM_CXXFLAGS = $(OPENMP_CXXFLAGS)

if !NO_TESSDATA_PREFIX
AM_CXXFLAGS += -DTESSDATA_PREFIX=@datadir@
endif

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

pkginclude_HEADERS = \
        network.h networkio.h networkscratch.h \
        static_shape.h stridemap.h weightmatrix.h

noinst_HEADERS = convolve.h ctc.h
noinst_HEADERS += fullyconnected.h functions.h input.h
noinst_HEADERS += lstm.h lstmrecognizer.h lstmtrainer.h maxpool.h
noinst_HEADERS += networkbuilder.h
noinst_HEADERS += parallel.h plumbing.h recodebeam.h reconfig.h reversed.h
noinst_HEADERS += series.h tfnetwork.h

noinst_LTLIBRARIES = libtesseract_lstm.la

libtesseract_lstm_la_SOURCES = \
        convolve.cpp ctc.cpp fullyconnected.cpp functions.cpp input.cpp \
        lstm.cpp lstmrecognizer.cpp lstmtrainer.cpp maxpool.cpp \
        networkbuilder.cpp network.cpp networkio.cpp \
        parallel.cpp plumbing.cpp recodebeam.cpp reconfig.cpp reversed.cpp \
        series.cpp stridemap.cpp tfnetwork.cpp weightmatrix.cpp
