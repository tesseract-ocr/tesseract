AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"\
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct -I$(top_srcdir)/cube \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/textord -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/opencl

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if VISIBILITY
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
endif

include_HEADERS = apitypes.h baseapi.h capi.h renderer.h
lib_LTLIBRARIES = 

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_api.la
else
lib_LTLIBRARIES += libtesseract_api.la
libtesseract_api_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_api_la_LIBADD = \
    ../ccmain/libtesseract_main.la \
    ../textord/libtesseract_textord.la \
    ../wordrec/libtesseract_wordrec.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la \
    ../opencl/libtesseract_opencl.la 
    if !NO_CUBE_BUILD
        libtesseract_api_la_LIBADD += ../cube/libtesseract_cube.la \
            ../neural_networks/runtime/libtesseract_neural.la \
    endif
endif

libtesseract_api_la_CPPFLAGS = $(AM_CPPFLAGS)
if VISIBILITY
libtesseract_api_la_CPPFLAGS += -DTESS_EXPORTS
endif
libtesseract_api_la_SOURCES = baseapi.cpp capi.cpp renderer.cpp pdfrenderer.cpp

lib_LTLIBRARIES += libtesseract.la
libtesseract_la_LDFLAGS = $(LEPTONICA_LIBS) $(OPENCL_LDFLAGS)
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
    ../ccstruct/libtesseract_ccstruct.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la \
    ../opencl/libtesseract_opencl.la 
if !NO_CUBE_BUILD
libtesseract_la_LIBADD += ../cube/libtesseract_cube.la \
        ../neural_networks/runtime/libtesseract_neural.la
endif

libtesseract_la_LDFLAGS += -version-info $(GENERIC_LIBRARY_VERSION) -no-undefined

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

if T_WIN
tesseract_LDADD += -lws2_32 -ltiff
libtesseract_la_LDFLAGS += -no-undefined -Wl,--as-needed -lws2_32
endif
if ADD_RT
tesseract_LDADD += -lrt
endif
