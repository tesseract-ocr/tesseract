AM_CPPFLAGS += -I$(top_srcdir)/src/ccstruct -I$(top_srcdir)/src/ccutil -I$(top_srcdir)/src/viewer

AM_CXXFLAGS =

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

pkginclude_HEADERS =

noinst_HEADERS = dotproduct.h
noinst_HEADERS += intsimdmatrix.h
noinst_HEADERS += simddetect.h

noinst_LTLIBRARIES = libtesseract_native.la
if AVX_OPT
noinst_LTLIBRARIES += libtesseract_avx.la
endif
if AVX2_OPT
noinst_LTLIBRARIES += libtesseract_avx2.la
endif
if FMA_OPT
noinst_LTLIBRARIES += libtesseract_fma.la
endif
if SSE41_OPT
noinst_LTLIBRARIES += libtesseract_sse.la
endif
noinst_LTLIBRARIES += libtesseract_arch.la

libtesseract_arch_la_CPPFLAGS = $(AM_CPPFLAGS)
if AVX_OPT
libtesseract_arch_la_CPPFLAGS += -DAVX
libtesseract_avx_la_CXXFLAGS = -mavx
endif
if AVX2_OPT
libtesseract_arch_la_CPPFLAGS += -DAVX2
libtesseract_avx2_la_CXXFLAGS = -mavx2
endif
if FMA_OPT
libtesseract_arch_la_CPPFLAGS += -DFMA
libtesseract_fma_la_CXXFLAGS = -mfma
endif
if SSE41_OPT
libtesseract_arch_la_CPPFLAGS += -DSSE4_1
libtesseract_sse_la_CXXFLAGS = -msse4.1
endif

libtesseract_native_la_CXXFLAGS = -O3 -ffast-math
if MARCH_NATIVE_OPT
libtesseract_native_la_CXXFLAGS += -march=native -mtune=native
endif
libtesseract_native_la_SOURCES = dotproduct.cpp

libtesseract_arch_la_SOURCES = intsimdmatrix.cpp simddetect.cpp

if AVX_OPT
libtesseract_avx_la_SOURCES = dotproductavx.cpp
endif

if AVX2_OPT
libtesseract_avx2_la_SOURCES = intsimdmatrixavx2.cpp
endif

if FMA_OPT
libtesseract_fma_la_SOURCES = dotproductfma.cpp
endif

if SSE41_OPT
libtesseract_sse_la_SOURCES = dotproductsse.cpp intsimdmatrixsse.cpp
endif
