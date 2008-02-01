SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/pageseg

include_HEADERS = \
    blkocc.h blobcmpl.h drawedg.h drawtord.h edgblob.h \
    edgloop.h fpchop.h gap_map.h makerow.h oldbasel.h \
    pithsync.h pitsync1.h scanedg.h sortflts.h tessout.h \
    topitch.h tordmain.h tospace.h tovars.h underlin.h \
    wordseg.h

lib_LIBRARIES = libtesseract_textord.a
libtesseract_textord_a_SOURCES = \
    blkocc.cpp drawedg.cpp drawtord.cpp edgblob.cpp \
    edgloop.cpp fpchop.cpp gap_map.cpp makerow.cpp oldbasel.cpp \
    pithsync.cpp pitsync1.cpp scanedg.cpp sortflts.cpp \
    topitch.cpp tordmain.cpp tospace.cpp tovars.cpp underlin.cpp \
    wordseg.cpp
