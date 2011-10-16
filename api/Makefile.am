AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"\
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/textord -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil

include_HEADERS = \
    apitypes.h baseapi.h pageiterator.h resultiterator.h tesseractmain.h
lib_LTLIBRARIES = 

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_api.la
else
lib_LTLIBRARIES += libtesseract_api.la
libtesseract_api_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_api_la_LIBADD = \
    $(top_srcdir)/ccmain/libtesseract_main.la \
    $(top_srcdir)/cube/libtesseract_cube.la \
    $(top_srcdir)/neural_networks/runtime/libtesseract_neural.la \
    $(top_srcdir)/textord/libtesseract_textord.la \
    $(top_srcdir)/wordrec/libtesseract_wordrec.la \
    $(top_srcdir)/classify/libtesseract_classify.la \
    $(top_srcdir)/dict/libtesseract_dict.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la \
    $(top_srcdir)/image/libtesseract_image.la \
    $(top_srcdir)/cutil/libtesseract_cutil.la \
    $(top_srcdir)/viewer/libtesseract_viewer.la \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
endif
libtesseract_api_la_SOURCES = baseapi.cpp pageiterator.cpp resultiterator.cpp

lib_LTLIBRARIES += libtesseract.la
libtesseract_la_LDFLAGS =
libtesseract_la_SOURCES =
# Dummy C++ source to cause C++ linking.
# see http://www.gnu.org/s/hello/manual/automake/Libtool-Convenience-Libraries.html#Libtool-Convenience-Libraries
nodist_EXTRA_libtesseract_la_SOURCES = dummy.cxx
libtesseract_la_LIBADD = \
    libtesseract_api.la \
    $(top_srcdir)/ccmain/libtesseract_main.la \
    $(top_srcdir)/cube/libtesseract_cube.la \
    $(top_srcdir)/neural_networks/runtime/libtesseract_neural.la \
    $(top_srcdir)/textord/libtesseract_textord.la \
    $(top_srcdir)/wordrec/libtesseract_wordrec.la \
    $(top_srcdir)/classify/libtesseract_classify.la \
    $(top_srcdir)/dict/libtesseract_dict.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la \
    $(top_srcdir)/image/libtesseract_image.la \
    $(top_srcdir)/cutil/libtesseract_cutil.la \
    $(top_srcdir)/viewer/libtesseract_viewer.la \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la

libtesseract_la_LDFLAGS += -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_la_LDFLAGS += -L./ -L../ -L../api -L../ccutil -L../viewer -L../cutil -L../image -L../ccstruct -L../dict -L../classify -L../wordrec -L../neural_networks/runtime -L../textord -L../cube -L../ccmain

bin_PROGRAMS = tesseract
tesseract_SOURCES = $(top_srcdir)/api/tesseractmain.cpp
tesseract_LDADD = libtesseract.la
