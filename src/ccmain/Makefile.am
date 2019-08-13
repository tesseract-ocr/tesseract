AM_CPPFLAGS += \
  -I$(top_srcdir)/src/ccutil \
  -I$(top_srcdir)/src/ccstruct \
  -I$(top_srcdir)/src/arch \
  -I$(top_srcdir)/src/lstm \
  -I$(top_srcdir)/src/viewer \
  -I$(top_srcdir)/src/classify \
  -I$(top_srcdir)/src/dict \
  -I$(top_srcdir)/src/wordrec \
  -I$(top_srcdir)/src/cutil \
  -I$(top_srcdir)/src/textord \
  -I$(top_srcdir)/src/opencl

AM_CPPFLAGS += $(OPENCL_CPPFLAGS)
AM_CPPFLAGS += $(OPENMP_CXXFLAGS)

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

pkginclude_HEADERS = \
    thresholder.h \
    osdetect.h \
    ltrresultiterator.h \
    pageiterator.h \
    resultiterator.h

noinst_HEADERS = \
    control.h \
    equationdetect.h \
    mutableiterator.h \
    output.h \
    paragraphs.h \
    paragraphs_internal.h \
    paramsd.h \
    pgedit.h \
    tesseractclass.h \
    tessvars.h \
    werdit.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += \
    docqual.h \
    fixspace.h \
    reject.h
endif

noinst_LTLIBRARIES = libtesseract_main.la

libtesseract_main_la_SOURCES = \
  applybox.cpp \
  control.cpp \
  linerec.cpp \
  ltrresultiterator.cpp \
  mutableiterator.cpp \
  output.cpp \
  pageiterator.cpp \
  pagesegmain.cpp \
  pagewalk.cpp \
  paragraphs.cpp \
  paramsd.cpp \
  pgedit.cpp \
  reject.cpp \
  resultiterator.cpp \
  tessedit.cpp \
  tesseractclass.cpp \
  tessvars.cpp \
  thresholder.cpp \
  werdit.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_main_la_SOURCES += adaptions.cpp
libtesseract_main_la_SOURCES += docqual.cpp
libtesseract_main_la_SOURCES += equationdetect.cpp
libtesseract_main_la_SOURCES += fixspace.cpp
libtesseract_main_la_SOURCES += fixxht.cpp
libtesseract_main_la_SOURCES += osdetect.cpp
libtesseract_main_la_SOURCES += par_control.cpp
libtesseract_main_la_SOURCES += recogtraining.cpp
libtesseract_main_la_SOURCES += superscript.cpp
libtesseract_main_la_SOURCES += tessbox.cpp
libtesseract_main_la_SOURCES += tfacepp.cpp
endif
