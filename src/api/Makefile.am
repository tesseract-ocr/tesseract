AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\" \
    -I$(top_srcdir)/src/arch \
    -I$(top_srcdir)/src/lstm \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/textord \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/classify \
    -I$(top_srcdir)/src/ccmain \
    -I$(top_srcdir)/src/wordrec \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/opencl \
    -I$(top_builddir)/api

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
endif

pkginclude_HEADERS = apitypes.h baseapi.h capi.h renderer.h tess_version.h
lib_LTLIBRARIES =

noinst_LTLIBRARIES = libtesseract_api.la

libtesseract_api_la_CPPFLAGS = $(AM_CPPFLAGS)
if VISIBILITY
libtesseract_api_la_CPPFLAGS += -DTESS_EXPORTS
endif
if HAVE_LIBCURL
libtesseract_api_la_CPPFLAGS += $(libcurl_CFLAGS) -DHAVE_LIBCURL
endif
libtesseract_api_la_SOURCES = baseapi.cpp capi.cpp
libtesseract_api_la_SOURCES += altorenderer.cpp
libtesseract_api_la_SOURCES += hocrrenderer.cpp
libtesseract_api_la_SOURCES += lstmboxrenderer.cpp
libtesseract_api_la_SOURCES += pdfrenderer.cpp
libtesseract_api_la_SOURCES += wordstrboxrenderer.cpp
libtesseract_api_la_SOURCES += renderer.cpp

lib_LTLIBRARIES += libtesseract.la
libtesseract_la_LDFLAGS = $(LEPTONICA_LIBS) $(OPENCL_LDFLAGS) $(libarchive_LIBS)
libtesseract_la_LDFLAGS += $(libcurl_LIBS)
libtesseract_la_LDFLAGS += $(TENSORFLOW_LIBS)
libtesseract_la_SOURCES =
# Dummy C++ source to cause C++ linking.
# see http://www.gnu.org/s/hello/manual/automake/Libtool-Convenience-Libraries.html#Libtool-Convenience-Libraries
nodist_EXTRA_libtesseract_la_SOURCES = dummy.cxx
libtesseract_la_LIBADD = \
    libtesseract_api.la \
    ../ccmain/libtesseract_main.la \
    ../textord/libtesseract_textord.la \
    ../wordrec/libtesseract_wordrec.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../arch/libtesseract_arch.la \
    ../arch/libtesseract_native.la \
    ../lstm/libtesseract_lstm.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la \
    ../opencl/libtesseract_opencl.la

if AVX_OPT
libtesseract_la_LIBADD += ../arch/libtesseract_avx.la
endif
if AVX2_OPT
libtesseract_la_LIBADD += ../arch/libtesseract_avx2.la
endif
if FMA_OPT
libtesseract_la_LIBADD += ../arch/libtesseract_fma.la
endif
if SSE41_OPT
libtesseract_la_LIBADD += ../arch/libtesseract_sse.la
endif

libtesseract_la_LDFLAGS += -version-info $(GENERIC_LIBRARY_VERSION) $(NOUNDEFINED)

bin_PROGRAMS = tesseract
tesseract_SOURCES = tesseractmain.cpp
tesseract_CPPFLAGS = $(AM_CPPFLAGS)
if VISIBILITY
tesseract_CPPFLAGS += -DTESS_IMPORTS
endif

tesseract_LDADD = libtesseract.la


tesseract_LDFLAGS = $(OPENCL_LDFLAGS)

tesseract_LDADD += $(LEPTONICA_LIBS)
tesseract_LDADD += $(OPENMP_CXXFLAGS)
tesseract_LDADD += $(TENSORFLOW_LIBS)
tesseract_LDADD += $(libarchive_LIBS)
tesseract_LDADD += $(libcurl_LIBS)

if T_WIN
tesseract_LDADD += -ltiff
tesseract_LDADD += -lws2_32
libtesseract_la_LDFLAGS += -no-undefined -Wl,--as-needed -lws2_32
endif
if ADD_RT
tesseract_LDADD += -lrt
endif
