# Absolute path of directory 'tessdata' with traineddata files
# (must be on same level as top source directory).
TESSDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/tessdata

# Absolute path of directory 'testing' with test images and ground truth texts
# (must be directly below top source directory).
TESTING_DIR=$(shell cd $(top_srcdir) && pwd)/testing

AM_CPPFLAGS += -DTESSDATA_DIR="\"$(TESSDATA_DIR)\""
AM_CPPFLAGS += -DTESTING_DIR="\"$(TESTING_DIR)\""
AM_CPPFLAGS += -DPANGO_ENABLE_ENGINE
AM_CPPFLAGS += -I$(top_builddir)/src/api
AM_CPPFLAGS += -I$(top_srcdir)/src/api
AM_CPPFLAGS += -I$(top_srcdir)/src/arch
AM_CPPFLAGS += -I$(top_srcdir)/src/ccmain
AM_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
AM_CPPFLAGS += -I$(top_srcdir)/src/ccutil
AM_CPPFLAGS += -I$(top_srcdir)/src/classify
AM_CPPFLAGS += -I$(top_srcdir)/src/cutil
AM_CPPFLAGS += -I$(top_srcdir)/src/dict
AM_CPPFLAGS += -I$(top_srcdir)/src/display
AM_CPPFLAGS += -I$(top_srcdir)/src/lstm
AM_CPPFLAGS += -I$(top_srcdir)/src/textord
AM_CPPFLAGS += -I$(top_srcdir)/src/viewer
AM_CPPFLAGS += -I$(top_srcdir)/src/wordrec

# Build examples
TESS_LIBS = $(top_builddir)/src/api/libtesseract.la
AM_CPPFLAGS +=   -isystem 

#List of source files needed to build the executable:

check_PROGRAMS = \
api_example \
osd_example

TESTS = $(check_PROGRAMS)

api_example_SOURCES = api_example.cc
api_example_LDADD =  $(TESS_LIBS) $(LEPTONICA_LIBS)

osd_example_SOURCES = osd_example.cc
osd_example_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

# for windows
if T_WIN
api_example_LDADD += -lws2_32
osd_example_LDADD += -lws2_32
endif

