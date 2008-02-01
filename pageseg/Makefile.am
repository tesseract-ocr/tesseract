SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/textord -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccmain

include_HEADERS = \
    leptonica_pageseg.h leptonica_pageseg_interface.h \
    pageseg.h

lib_LIBRARIES = libtesseract_pageseg.a
libtesseract_pageseg_a_SOURCES = \
    leptonica_pageseg.cpp leptonica_pageseg_interface.cpp \
    pageseg.cpp
