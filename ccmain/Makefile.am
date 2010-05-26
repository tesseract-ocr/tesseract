SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord

EXTRA_DIST = tessembedded.cpp ccmain.vcproj

include_HEADERS = \
    adaptions.h applybox.h blobcmp.h \
    callnet.h charcut.h charsample.h control.h \
    docqual.h expandblob.h fixspace.h fixxht.h \
    imgscale.h matmatch.h osdetect.h output.h \
    pagewalk.h paircmp.h pgedit.h reject.h scaleimg.h \
    tessbox.h tessedit.h tessembedded.h tesseractclass.h \
    tessio.h tessvars.h tfacep.h tfacepp.h thresholder.h tstruct.h \
    varabled.h werdit.h

lib_LTLIBRARIES = libtesseract_main.la
libtesseract_main_la_SOURCES = \
    adaptions.cpp ambigsrecog.cpp applybox.cpp \
    blobcmp.cpp \
    callnet.cpp charcut.cpp charsample.cpp control.cpp \
    docqual.cpp expandblob.cpp fixspace.cpp fixxht.cpp \
    imgscale.cpp matmatch.cpp osdetect.cpp output.cpp \
    pagewalk.cpp paircmp.cpp pgedit.cpp reject.cpp scaleimg.cpp \
    tessbox.cpp tessedit.cpp tesseractclass.cpp tessvars.cpp \
    tfacepp.cpp thresholder.cpp tstruct.cpp \
    varabled.cpp werdit.cpp
libtesseract_main_la_LIBADD = \
    ../wordrec/libtesseract_wordrec.la
libtesseract_main_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
