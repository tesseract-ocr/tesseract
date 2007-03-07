SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/display \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord

EXTRA_DIST = \
    adaptions.h applybox.h baseapi.h blobcmp.h \
    callnet.h charcut.h \
    control.h docqual.h expandblob.h fixspace.h fixxht.h \
    imgscale.h matmatch.h output.h paircmp.h reject.h scaleimg.h \
    tessbox.h tessedit.h tesseractmain.h tessvars.h tfacep.h \
    tessembedded.h tfacepp.h tstruct.h werdit.h

noinst_LIBRARIES = libtesseract_main.a
libtesseract_main_a_SOURCES = \
    tessedit.cpp adaptions.cpp applybox.cpp \
    baseapi.cpp blobcmp.cpp \
    callnet.cpp charcut.cpp charsample.cpp control.cpp \
    docqual.cpp expandblob.cpp fixspace.cpp fixxht.cpp \
    imgscale.cpp matmatch.cpp output.cpp paircmp.cpp \
    reject.cpp scaleimg.cpp tessbox.cpp tessvars.cpp \
    tfacepp.cpp tstruct.cpp werdit.cpp

bin_PROGRAMS = tesseract
tesseract_SOURCES = tesseractmain.cpp
tesseract_LDADD = \
    libtesseract_main.a \
    ../display/libtesseract_display.a \
    ../textord/libtesseract_textord.a \
    ../wordrec/libtesseract_wordrec.a \
    ../classify/libtesseract_classify.a \
    ../dict/libtesseract_dict.a \
    ../viewer/libtesseract_viewer.a \
    ../image/libtesseract_image.a \
    ../cutil/libtesseract_cutil.a \
    ../ccstruct/libtesseract_ccstruct.a \
    ../ccutil/libtesseract_ccutil.a
