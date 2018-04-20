#AM_CPPFLAGS += -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
#    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/viewer

# HFST BEGIN
AM_CPPFLAGS += -g -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/viewer -std=c++0x
# HFST END

    
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

# ORIGINAL:
#noinst_HEADERS = \
#    dawg.h dawg_cache.h dict.h matchdefs.h \
#    stopper.h trie.h 

# HFST BEGIN
noinst_HEADERS = \
    dawg.h dawg_cache.h dict.h matchdefs.h \
    stopper.h trie.h hfst_word_model.h \
    hfst_ol_driver.h hfst_io_utils.h \
    hfst_size_defs.h
# HFST END

noinst_LTLIBRARIES = libtesseract_dict.la

# ORIGINAL:
#libtesseract_dict_la_SOURCES = \
#    context.cpp \
#    dawg.cpp dawg_cache.cpp dict.cpp hyphen.cpp \
#    permdawg.cpp stopper.cpp trie.cpp

# HFST BEGIN
libtesseract_dict_la_SOURCES = \
    context.cpp \
    dawg.cpp dawg_cache.cpp dict.cpp hyphen.cpp \
    permdawg.cpp stopper.cpp trie.cpp \
    hfst_word_model.cpp hfst_ol_driver.cpp \
    hfst_io_utils.cpp
# HFST END
