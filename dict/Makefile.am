SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/viewer -I$(top_srcdir)/image

include_HEADERS = \
    dawg.h dict.h matchdefs.h \
    permute.h states.h stopper.h trie.h

lib_LTLIBRARIES = libtesseract_dict.la
libtesseract_dict_la_SOURCES = \
    context.cpp \
    dawg.cpp dict.cpp hyphen.cpp \
    permdawg.cpp permute.cpp states.cpp stopper.cpp trie.cpp

libtesseract_dict_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
