AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"\
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/textord -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil
    
if VISIBILITY
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
endif

include_HEADERS = apitypes.h baseapi.h capi.h
noinst_HEADERS =  tesseractmain.h
lib_LTLIBRARIES = 

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_api.la
else
lib_LTLIBRARIES += libtesseract_api.la
libtesseract_api_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_api_la_LIBADD = \
    ../ccmain/libtesseract_main.la \
    ../cube/libtesseract_cube.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../textord/libtesseract_textord.la \
    ../wordrec/libtesseract_wordrec.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la
endif
libtesseract_api_la_CPPFLAGS = $(AM_CPPFLAGS)
if VISIBILITY
libtesseract_api_la_CPPFLAGS += -DTESS_EXPORTS
endif
libtesseract_api_la_SOURCES = baseapi.cpp capi.cpp

lib_LTLIBRARIES += libtesseract.la
libtesseract_la_LDFLAGS = 
libtesseract_la_SOURCES =
# Dummy C++ source to cause C++ linking.
# see http://www.gnu.org/s/hello/manual/automake/Libtool-Convenience-Libraries.html#Libtool-Convenience-Libraries
nodist_EXTRA_libtesseract_la_SOURCES = dummy.cxx
libtesseract_la_LIBADD = \
    libtesseract_api.la \
    ../ccmain/libtesseract_main.la \
    ../cube/libtesseract_cube.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../textord/libtesseract_textord.la \
    ../wordrec/libtesseract_wordrec.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la

libtesseract_la_LDFLAGS += -version-info $(GENERIC_LIBRARY_VERSION)

bin_PROGRAMS = tesseract
tesseract_SOURCES = $(top_srcdir)/api/tesseractmain.cpp
tesseract_LDADD = libtesseract.la
tesseract_CPPFLAGS = $(AM_CPPFLAGS)

if MINGW
tesseract_LDADD += -lws2_32
libtesseract_la_LDFLAGS += -no-undefined  -Wl,--as-needed -lws2_32
endif

if VISIBILITY
tesseract_CPPFLAGS += -DTESS_IMPORTS
endif
