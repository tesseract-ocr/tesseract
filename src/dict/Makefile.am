AM_CPPFLAGS += \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/viewer

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

noinst_HEADERS = \
    dawg.h dawg_cache.h dict.h matchdefs.h \
    stopper.h trie.h

noinst_LTLIBRARIES = libtesseract_dict.la

libtesseract_dict_la_SOURCES = \
    context.cpp \
    dawg.cpp dawg_cache.cpp dict.cpp \
    permdawg.cpp stopper.cpp trie.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_dict_la_SOURCES += hyphen.cpp
endif
