SUBDIRS =
AM_CPPFLAGS = \
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/neural_networks/runtime -I$(top_srcdir)/cube \
    -I$(top_srcdir)/textord

EXTRA_DIST = tessembedded.cpp

include_HEADERS = \
    control.h cube_reco_context.h \
    docqual.h fixspace.h \
    imgscale.h osdetect.h output.h \
    paramsd.h pgedit.h reject.h scaleimg.h \
    tessbox.h tessedit.h tessembedded.h tesseractclass.h \
    tesseract_cube_combiner.h \
    tessvars.h tfacep.h tfacepp.h thresholder.h \
    werdit.h

lib_LTLIBRARIES = libtesseract_main.la
libtesseract_main_la_SOURCES = \
    adaptions.cpp applybox.cpp \
    control.cpp cube_control.cpp cube_reco_context.cpp \
    docqual.cpp fixspace.cpp fixxht.cpp \
    imgscale.cpp osdetect.cpp output.cpp pagesegmain.cpp \
    pagewalk.cpp paramsd.cpp pgedit.cpp reject.cpp scaleimg.cpp \
    recogtraining.cpp tesseract_cube_combiner.cpp \
    tessbox.cpp tessedit.cpp tesseractclass.cpp tessvars.cpp \
    tfacepp.cpp thresholder.cpp \
    werdit.cpp
libtesseract_main_la_LIBADD = \
    ../wordrec/libtesseract_wordrec.la
libtesseract_main_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
