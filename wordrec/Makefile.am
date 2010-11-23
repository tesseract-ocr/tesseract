SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/classify \
    -I$(top_srcdir)/image -I$(top_srcdir)/dict \
    -I$(top_srcdir)/viewer

include_HEADERS = \
    associate.h bestfirst.h chop.h \
    chopper.h closed.h drawfx.h findseam.h gradechop.h \
    language_model.h makechop.h matchtab.h measure.h \
    olutil.h outlines.h pieces.h plotedges.h \
    plotseg.h render.h tally.h \
    wordclass.h wordrec.h

lib_LTLIBRARIES = libtesseract_wordrec.la
libtesseract_wordrec_la_SOURCES = \
    associate.cpp bestfirst.cpp chop.cpp chopper.cpp \
    closed.cpp drawfx.cpp findseam.cpp gradechop.cpp \
    heuristic.cpp language_model.cpp makechop.cpp matchtab.cpp \
    olutil.cpp outlines.cpp pieces.cpp \
    plotedges.cpp plotseg.cpp render.cpp segsearch.cpp \
    tally.cpp tface.cpp wordclass.cpp wordrec.cpp
libtesseract_wordrec_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
