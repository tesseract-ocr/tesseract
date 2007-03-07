SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/image -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/viewer -I$(top_srcdir)/textord \
    -I$(top_srcdir)/ccutil

EXTRA_DIST = \
    cmndwin.h pagewalk.h pgedit.h pgeditx.h sbdmenu.h submen.h \
    tessio.h varabled.h varblmen.h varblwin.h

noinst_LIBRARIES = libtesseract_display.a
libtesseract_display_a_SOURCES = \
    cmndwin.cpp pagewalk.cpp pgedit.cpp sbdmenu.cpp varabled.cpp \
    varblmen.cpp varblwin.cpp
