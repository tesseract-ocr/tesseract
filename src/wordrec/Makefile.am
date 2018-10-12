AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/classify \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/viewer

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    wordrec.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += \
    associate.h \
    chop.h \
    chopper.h \
    drawfx.h \
    findseam.h \
    language_model.h \
    lm_consistency.h \
    lm_pain_points.h \
    lm_state.h \
    measure.h \
    outlines.h \
    params_model.h \
    plotedges.h \
    render.h
endif

noinst_LTLIBRARIES = libtesseract_wordrec.la

libtesseract_wordrec_la_SOURCES = \
    tface.cpp \
    wordrec.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_wordrec_la_SOURCES += \
    associate.cpp \
    chop.cpp \
    chopper.cpp \
    drawfx.cpp \
    findseam.cpp \
    gradechop.cpp \
    language_model.cpp \
    lm_consistency.cpp \
    lm_pain_points.cpp \
    lm_state.cpp \
    outlines.cpp \
    params_model.cpp \
    pieces.cpp \
    plotedges.cpp \
    render.cpp \
    segsearch.cpp \
    wordclass.cpp
endif
