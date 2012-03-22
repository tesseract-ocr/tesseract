AM_CPPFLAGS += \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/classify \
    -I$(top_srcdir)/image -I$(top_srcdir)/dict \
    -I$(top_srcdir)/viewer

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    associate.h bestfirst.h chop.h \
    chopper.h closed.h drawfx.h findseam.h gradechop.h \
    language_model.h makechop.h matchtab.h measure.h \
    olutil.h outlines.h plotedges.h \
    plotseg.h render.h \
    wordclass.h wordrec.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_wordrec.la
else
lib_LTLIBRARIES = libtesseract_wordrec.la
libtesseract_wordrec_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_wordrec_la_LIBADD = \
    ../ccstruct/libtesseract_ccstruct.la \
    ../ccutil/libtesseract_ccutil.la \
    ../cutil/libtesseract_cutil.la \
    ../classify/libtesseract_classify.la \
    ../image/libtesseract_image.la \
    ../dict/libtesseract_dict.la \
    ../viewer/libtesseract_viewer.la
endif

libtesseract_wordrec_la_SOURCES = \
    associate.cpp bestfirst.cpp chop.cpp chopper.cpp \
    closed.cpp drawfx.cpp findseam.cpp gradechop.cpp \
    heuristic.cpp language_model.cpp makechop.cpp matchtab.cpp \
    olutil.cpp outlines.cpp pieces.cpp \
    plotedges.cpp plotseg.cpp render.cpp segsearch.cpp \
    tface.cpp wordclass.cpp wordrec.cpp
