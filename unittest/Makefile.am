# Absolute path of directory 'tessdata' with traineddata files
# (must be on same level as top source directory).
TESSDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/tessdata

# Absolute path of directory 'testing' with test images and ground truth texts
# (using submodule test).
TESTING_DIR=$(shell cd $(top_srcdir) && pwd)/test/testing

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

# Build googletest:
check_LTLIBRARIES = libgtest.la libgtest_main.la libgmock.la libgmock_main.la
libgtest_la_SOURCES = ../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -pthread 
libgtest_main_la_SOURCES = ../googletest/googletest/src/gtest_main.cc
## libgtest_main_la_LIBADD = libgtest.la

GMOCK_INCLUDES = -I$(top_srcdir)/googletest/googlemock/include \
                 -I$(top_srcdir)/googletest/googlemock \
                 -I$(top_srcdir)/googletest/googletest/include \
                 -I$(top_srcdir)/googletest/googletest

libgmock_la_SOURCES = ../googletest/googlemock/src/gmock-all.cc
libgmock_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                       -pthread
libgmock_main_la_SOURCES = ../googletest/googlemock/src/gmock_main.cc
libgmock_main_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                            -pthread

# Build unittests
GTEST_LIBS =  libgtest.la libgtest_main.la
GMOCK_LIBS =  libgmock.la libgmock_main.la
TESS_LIBS = $(top_builddir)/src/api/libtesseract.la
AM_CPPFLAGS +=   -isystem $(top_srcdir)/googletest/googletest/include \
                 -isystem $(top_srcdir)/googletest/googlemock/include

check_PROGRAMS = \
  apiexample_test \
  progress_test \
  intsimdmatrix_test \
  matrix_test \
  osd_test \
  loadlang_test \
  tesseracttests 


TESTS = $(check_PROGRAMS)

#List of source files needed to build the executable:

apiexample_test_SOURCES = apiexample_test.cc
apiexample_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
apiexample_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

progress_test_SOURCES = progress_test.cc
progress_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
progress_test_LDADD = $(GTEST_LIBS) $(GMOCK_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

intsimdmatrix_test_SOURCES = intsimdmatrix_test.cc
intsimdmatrix_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS)

matrix_test_SOURCES = matrix_test.cc
matrix_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS)

osd_test_SOURCES = osd_test.cc
osd_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

loadlang_test_SOURCES = loadlang_test.cc
loadlang_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

tesseracttests_SOURCES = ../tests/tesseracttests.cpp
tesseracttests_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

# for windows
if T_WIN
apiexample_test_LDADD += -lws2_32
intsimdmatrix_test_LDADD += -lws2_32
matrix_test_LDADD += -lws2_32
osd_test_LDADD += -lws2_32
loadlang_test_LDADD += -lws2_32
tesseracttests_LDADD  += -lws2_32
endif

EXTRA_apiexample_test_DEPENDENCIES = $(abs_top_builddir)/test/testing/phototest.tif
EXTRA_apiexample_test_DEPENDENCIES += $(abs_top_builddir)/test/testing/phototest.txt

$(abs_top_builddir)/test/testing/phototest.tif:
	mkdir -p $(top_builddir)/test/testing
	ln -s $(TESTING_DIR)/phototest.tif $(top_builddir)/test/testing/phototest.tif

$(abs_top_builddir)/test/testing/phototest.txt:
	mkdir -p $(top_builddir)/test/testing
	ln -s $(TESTING_DIR)/phototest.txt $(top_builddir)/test/testing/phototest.txt
