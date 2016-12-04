AM_CPPFLAGS += \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/cutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/arch -I$(top_srcdir)/viewer -I$(top_srcdir)/classify \
    -I$(top_srcdir)/dict -I$(top_srcdir)/lstm
AUTOMAKE_OPTIONS = subdir-objects
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

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_lstm.la
else
lib_LTLIBRARIES = libtesseract_lstm.la
libtesseract_lstm_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
endif

libtesseract_lstm_la_SOURCES = \
	convolve.cpp ctc.cpp fullyconnected.cpp functions.cpp input.cpp \
        lstm.cpp lstmrecognizer.cpp lstmtrainer.cpp maxpool.cpp \
        networkbuilder.cpp network.cpp networkio.cpp \
	parallel.cpp plumbing.cpp recodebeam.cpp reconfig.cpp reversed.cpp \
        series.cpp stridemap.cpp tfnetwork.cpp weightmatrix.cpp
