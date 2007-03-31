SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/classify \
    -I$(top_srcdir)/image -I$(top_srcdir)/dict \
    -I$(top_srcdir)/viewer

include_HEADERS = \
    associate.h badwords.h bestfirst.h charsample.h chop.h \
    chopper.h closed.h djmenus.h drawfx.h findseam.h gradechop.h \
    heuristic.h makechop.h matchtab.h matrix.h measure.h metrics.h \
    mfvars.h msmenus.h olutil.h outlines.h pieces.h plotedges.h \
    plotseg.h render.h seam.h split.h tally.h tessinit.h tface.h \
    wordclass.h

lib_LIBRARIES = libtesseract_wordrec.a
libtesseract_wordrec_a_SOURCES = \
    associate.cpp badwords.cpp bestfirst.cpp chop.cpp chopper.cpp \
    closed.cpp djmenus.cpp drawfx.cpp findseam.cpp gradechop.cpp \
    heuristic.cpp makechop.cpp matchtab.cpp matrix.cpp metrics.cpp \
    mfvars.cpp msmenus.cpp olutil.cpp outlines.cpp pieces.cpp \
    plotedges.cpp plotseg.cpp render.cpp seam.cpp split.cpp \
    tally.cpp tessinit.cpp tface.cpp wordclass.cpp
