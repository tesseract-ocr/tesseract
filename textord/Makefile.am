SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccmain -I$(top_srcdir)/wordrec -I$(top_srcdir)/api \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/classify -I$(top_srcdir)/dict

EXTRA_DIST = textord.vcproj

include_HEADERS = \
    alignedblob.h bbgrid.h blkocc.h blobcmpl.h \
    colfind.h colpartition.h colpartitionset.h \
    drawedg.h drawtord.h edgblob.h edgloop.h \
    fpchop.h gap_map.h imagefind.h linefind.h makerow.h oldbasel.h \
    pithsync.h pitsync1.h scanedg.h sortflts.h strokewidth.h \
    tabfind.h tablefind.h tabvector.h tessout.h \
    topitch.h tordmain.h tospace.h tovars.h \
    underlin.h wordseg.h workingpartset.h

lib_LTLIBRARIES = libtesseract_textord.la
libtesseract_textord_la_SOURCES = \
    alignedblob.cpp bbgrid.cpp blkocc.cpp \
    colfind.cpp colpartition.cpp colpartitionset.cpp \
    drawedg.cpp drawtord.cpp edgblob.cpp edgloop.cpp \
    fpchop.cpp gap_map.cpp imagefind.cpp linefind.cpp makerow.cpp oldbasel.cpp \
    pagesegmain.cpp pithsync.cpp pitsync1.cpp scanedg.cpp sortflts.cpp strokewidth.cpp \
    tabfind.cpp tablefind.cpp tabvector.cpp \
    topitch.cpp tordmain.cpp tospace.cpp tovars.cpp \
    underlin.cpp wordseg.cpp workingpartset.cpp

libtesseract_textord_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
