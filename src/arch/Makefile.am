AM_CPPFLAGS += -I$(top_srcdir)/src/ccstruct -I$(top_srcdir)/src/ccutil -I$(top_srcdir)/src/viewer

SUBDIRS =
AM_CXXFLAGS =

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

pkginclude_HEADERS =

noinst_HEADERS = dotproductavx.h dotproductsse.h
noinst_HEADERS += intsimdmatrix.h intsimdmatrixavx2.h intsimdmatrixsse.h
noinst_HEADERS += simddetect.h

noinst_LTLIBRARIES = libtesseract_avx.la libtesseract_avx2.la libtesseract_sse.la
noinst_LTLIBRARIES += libtesseract_arch.la

if AVX_OPT
libtesseract_avx_la_CXXFLAGS = -mavx
endif
if AVX2_OPT
libtesseract_avx2_la_CXXFLAGS = -mavx2
endif
if SSE41_OPT
libtesseract_sse_la_CXXFLAGS = -msse4.1
endif

libtesseract_arch_la_SOURCES = intsimdmatrix.cpp simddetect.cpp

libtesseract_avx_la_SOURCES = dotproductavx.cpp

libtesseract_avx2_la_SOURCES = intsimdmatrixavx2.cpp

libtesseract_sse_la_SOURCES = dotproductsse.cpp intsimdmatrixsse.cpp
