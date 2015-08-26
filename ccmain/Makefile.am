AM_CPPFLAGS += \
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/classify  -I$(top_srcdir)/dict \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord -I$(top_srcdir)/opencl

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

include_HEADERS = \
    thresholder.h ltrresultiterator.h pageiterator.h resultiterator.h \
    osdetect.h
noinst_HEADERS = \
    control.h docqual.h equationdetect.h fixspace.h mutableiterator.h \
    output.h paragraphs.h paragraphs_internal.h paramsd.h pgedit.h \
    reject.h tessbox.h tessedit.h tesseractclass.h tessvars.h werdit.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_main.la
else
lib_LTLIBRARIES = libtesseract_main.la
libtesseract_main_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_main_la_LIBADD = \
    ../wordrec/libtesseract_wordrec.la \
    ../textord/libtesseract_textord.la \
    ../ccutil/libtesseract_ccutil.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../viewer/libtesseract_viewer.la \
    ../dict/libtesseract_dict.la \
    ../classify/libtesseract_classify.la \
    ../cutil/libtesseract_cutil.la \
    ../opencl/libtesseract_opencl.la
    if !NO_CUBE_BUILD
        libtesseract_main_la_LIBADD += ../cube/libtesseract_cube.la
    endif
endif

libtesseract_main_la_SOURCES = \
    adaptions.cpp applybox.cpp control.cpp  \
    docqual.cpp equationdetect.cpp fixspace.cpp fixxht.cpp \
    ltrresultiterator.cpp \
    osdetect.cpp output.cpp pageiterator.cpp pagesegmain.cpp \
    pagewalk.cpp par_control.cpp paragraphs.cpp paramsd.cpp pgedit.cpp recogtraining.cpp \
    reject.cpp resultiterator.cpp superscript.cpp \
    tessbox.cpp tessedit.cpp tesseractclass.cpp tessvars.cpp \
    tfacepp.cpp thresholder.cpp \
    werdit.cpp

if !NO_CUBE_BUILD
AM_CPPFLAGS += \
    -I$(top_srcdir)/neural_networks/runtime -I$(top_srcdir)/cube
noinst_HEADERS += \
    cube_reco_context.h cubeclassifier.h tesseract_cube_combiner.h
libtesseract_main_la_SOURCES += \
    cube_control.cpp cube_reco_context.cpp cubeclassifier.cpp \
    tesseract_cube_combiner.cpp
endif
