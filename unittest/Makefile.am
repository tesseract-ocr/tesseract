AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS += -DUSE_STD_NAMESPACE -DPANGO_ENABLE_ENGINE
AM_CPPFLAGS += -I$(top_srcdir)/api
AM_CPPFLAGS += -I$(top_srcdir)/arch 
AM_CPPFLAGS += -I$(top_srcdir)/ccmain
AM_CPPFLAGS += -I$(top_srcdir)/ccstruct 
AM_CPPFLAGS += -I$(top_srcdir)/ccutil 
AM_CPPFLAGS += -I$(top_srcdir)/classify 
AM_CPPFLAGS += -I$(top_srcdir)/cutil
AM_CPPFLAGS += -I$(top_srcdir)/dict 
AM_CPPFLAGS += -I$(top_srcdir)/display 
AM_CPPFLAGS += -I$(top_srcdir)/lstm 
AM_CPPFLAGS += -I$(top_srcdir)/textord 
AM_CPPFLAGS += -I$(top_srcdir)/viewer 
AM_CPPFLAGS += -I$(top_srcdir)/wordrec
	
# Build googletest:
check_LTLIBRARIES = libgtest.la libgtest_main.la
libgtest_la_SOURCES = ../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -pthread 
libgtest_main_la_SOURCES = ../googletest/googletest/src/gtest_main.cc
## libgtest_main_la_LIBADD = libgtest.la

# Build unittests
GTEST_LIBS =  libgtest.la libgtest_main.la
TESS_LIBS = $(top_builddir)/api/libtesseract.la
AM_CPPFLAGS +=   -isystem $(top_srcdir)/googletest/googletest/include  

check_PROGRAMS = \
  apiexample_test \
  intsimdmatrix_test \
  tesseracttests \
  matrix_test

TESTS = $(check_PROGRAMS)

#List of source files needed to build the executable:
	
apiexample_test_SOURCES = apiexample_test.cc
apiexample_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
apiexample_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

intsimdmatrix_test_SOURCES = intsimdmatrix_test.cc
intsimdmatrix_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS)

matrix_test_SOURCES = matrix_test.cc
matrix_test_LDADD = $(GTEST_LIBS) $(TESS_LIBS)

tesseracttests_SOURCES = ../tests/tesseracttests.cpp
tesseracttests_LDADD = $(GTEST_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

# for windows
if T_WIN
apiexample_test_LDADD += -lws2_32
intsimdmatrix_test_LDADD += -lws2_32
matrix_test_LDADD += -lws2_32
tesseracttests_LDADD  += -lws2_32

AM_CPPFLAGS += -I$(top_srcdir)/vs2010/port
endif

EXTRA_apiexample_test_DEPENDENCIES = $(abs_top_builddir)/testing/phototest.tif
EXTRA_apiexample_test_DEPENDENCIES += $(abs_top_builddir)/testing/phototest.txt

$(abs_top_builddir)/testing/phototest.tif:
	ln -s $(top_srcdir)/testing/phototest.tif $(top_builddir)/testing/phototest.tif

$(abs_top_builddir)/testing/phototest.txt:
	ln -s $(top_srcdir)/testing/phototest.txt $(top_builddir)/testing/phototest.txt
