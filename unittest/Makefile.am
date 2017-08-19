AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS += \
    -DUSE_STD_NAMESPACE -DPANGO_ENABLE_ENGINE \
    -I$(top_srcdir)/ccmain -I$(top_srcdir)/api \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/lstm -I$(top_srcdir)/arch \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/textord -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/display \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil
	
# Build googletest:
check_LTLIBRARIES = libgtest.la libgtest_main.la
libgtest_la_SOURCES = ../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -pthread 
libgtest_main_la_SOURCES = ../googletest/googletest/src/gtest_main.cc
## libgtest_main_la_LIBADD = libgtest.la

# Build unittests
GTEST_LIBS =  libgtest.la libgtest_main.la
AM_CPPFLAGS +=   -isystem $(top_srcdir)/googletest/googletest/include  

check_PROGRAMS = \
  apiexample_test \
  tesseracttests \
  matrix_test

TESTS = $(check_PROGRAMS)

#List of source files needed to build the executable:

tesseracttests_SOURCES = ../tests/tesseracttests.cpp
tesseracttests_LDADD = $(GTEST_LIBS)

matrix_test_SOURCES = matrix_test.cc
matrix_test_LDADD = $(GTEST_LIBS)
	
apiexample_test_SOURCES = apiexample_test.cc
#apiexample_test_LDFLAGS = -static
apiexample_test_LDFLAGS = $(OPENCL_LDFLAGS)

if USING_MULTIPLELIBS
apiexample_test_LDADD = \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la
else
apiexample_test_LDADD = \
    $(top_srcdir)/api/libtesseract.la
endif

apiexample_test_LDADD += $(LEPTONICA_LIBS)
apiexample_test_LDADD += $(GTEST_LIBS)

# for windows
if T_WIN
apiexample_test_LDADD += -lws2_32
matrix_test_LDADD += -lws2_32
tesseracttests_LDADD  += -lws2_32

AM_CPPFLAGS += -I$(top_srcdir)/vs2010/port
endif
