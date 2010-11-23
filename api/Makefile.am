SUBDIRS =
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"\
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord 

include_HEADERS = \
    apitypes.h baseapi.h pageiterator.h resultiterator.h tesseractmain.h

lib_LTLIBRARIES = libtesseract_api.la
libtesseract_api_la_SOURCES = baseapi.cpp pageiterator.cpp resultiterator.cpp
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

bin_PROGRAMS = tesseract
tesseract_SOURCES = tesseractmain.cpp
tesseract_LDADD = \
    libtesseract_api.la \
    ../ccmain/libtesseract_main.la \
    ../textord/libtesseract_textord.la \
    ../wordrec/libtesseract_wordrec.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccutil/libtesseract_ccutil.la

