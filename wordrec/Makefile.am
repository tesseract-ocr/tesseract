SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/classify \
    -I$(top_srcdir)/image -I$(top_srcdir)/dict \
    -I$(top_srcdir)/viewer

EXTRA_DIST = wordrec.vcproj

include_HEADERS = \
    associate.h badwords.h bestfirst.h chop.h \
    chopper.h closed.h drawfx.h findseam.h gradechop.h \
    heuristic.h makechop.h matchtab.h matrix.h measure.h metrics.h \
    mfvars.h olutil.h outlines.h pieces.h plotedges.h \
    plotseg.h render.h seam.h split.h tally.h tessinit.h tface.h \
    wordclass.h wordrec.h

lib_LTLIBRARIES = libtesseract_wordrec.la
libtesseract_wordrec_la_SOURCES = \
    associate.cpp badwords.cpp bestfirst.cpp chop.cpp chopper.cpp \
    closed.cpp drawfx.cpp findseam.cpp gradechop.cpp \
    heuristic.cpp makechop.cpp matchtab.cpp matrix.cpp metrics.cpp \
    mfvars.cpp olutil.cpp outlines.cpp pieces.cpp \
    plotedges.cpp plotseg.cpp render.cpp seam.cpp split.cpp \
    tally.cpp tessinit.cpp tface.cpp wordclass.cpp wordrec.cpp
libtesseract_wordrec_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
