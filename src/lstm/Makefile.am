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
AM_CXXFLAGS += -DTESSDATA_PREFIX=@datadir@/
endif

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

include_HEADERS = \
        convolve.h ctc.h fullyconnected.h functions.h input.h \
        lstm.h lstmrecognizer.h lstmtrainer.h maxpool.h \
        networkbuilder.h network.h networkio.h networkscratch.h \
        parallel.h plumbing.h recodebeam.h reconfig.h reversed.h \
        series.h static_shape.h stridemap.h tfnetwork.h weightmatrix.h

noinst_HEADERS = 

noinst_LTLIBRARIES = libtesseract_lstm.la

libtesseract_lstm_la_SOURCES = \
        convolve.cpp ctc.cpp fullyconnected.cpp functions.cpp input.cpp \
        lstm.cpp lstmrecognizer.cpp lstmtrainer.cpp maxpool.cpp \
        networkbuilder.cpp network.cpp networkio.cpp \
        parallel.cpp plumbing.cpp recodebeam.cpp reconfig.cpp reversed.cpp \
        series.cpp stridemap.cpp tfnetwork.cpp weightmatrix.cpp
