AM_CPPFLAGS += \
  -I$(top_builddir)/include \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/api \
  -I$(top_srcdir)/src/arch \
  -I$(top_srcdir)/src/classify \
  -I$(top_srcdir)/src/ccmain \
  -I$(top_srcdir)/src/ccstruct \
  -I$(top_srcdir)/src/ccutil \
  -I$(top_srcdir)/src/cutil \
  -I$(top_srcdir)/src/dict \
  -I$(top_srcdir)/src/lstm \
  -I$(top_srcdir)/src/opencl \
  -I$(top_srcdir)/src/textord \
  -I$(top_srcdir)/src/viewer \
  -I$(top_srcdir)/src/wordrec

if !NO_TESSDATA_PREFIX
AM_CPPFLAGS += -DTESSDATA_PREFIX=@datadir@
endif

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
endif


pkginclude_HEADERS = \
  apitypes.h \
  baseapi.h \
  capi.h renderer.h \
  tess_version.h

pkginclude_HEADERS += \
  thresholder.h \
  osdetect.h \
  ltrresultiterator.h \
  pageiterator.h \
  resultiterator.h

pkginclude_HEADERS += publictypes.h

pkginclude_HEADERS += \
  genericvector.h \
  helpers.h \
  ocrclass.h \
  platform.h \
  serialis.h \
  strngs.h \
  unichar.h

