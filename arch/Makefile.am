AM_CPPFLAGS += -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS =
AM_CXXFLAGS =

if VISIBILITY
AM_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
AM_CPPFLAGS += -DTESS_EXPORTS
endif

include_HEADERS = dotproductavx.h dotproductsse.h simddetect.h

noinst_HEADERS =

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_avx.la libtesseract_sse.la
noinst_LTLIBRARIES += libtesseract_arch.la
else
lib_LTLIBRARIES = libtesseract_avx.la libtesseract_sse.la
lib_LTLIBRARIES += libtesseract_arch.la
libtesseract_arch_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_avx_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_sse_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
endif

if AVX_OPT
libtesseract_avx_la_CXXFLAGS = -mavx
endif
if SSE41_OPT
libtesseract_sse_la_CXXFLAGS = -msse4.1
endif

libtesseract_arch_la_SOURCES = simddetect.cpp

libtesseract_avx_la_SOURCES = dotproductavx.cpp

libtesseract_sse_la_SOURCES = dotproductsse.cpp

