SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/viewer -I$(top_srcdir)/image

EXTRA_DIST = dict.vcproj

include_HEADERS = \
    choicearr.h choices.h context.h conversion.h \
    dawg.h dict.h matchdefs.h \
    permdawg.h permngram.h permute.h states.h stopper.h trie.h

lib_LIBRARIES = libtesseract_dict.a
libtesseract_dict_a_SOURCES = \
    choices.cpp context.cpp conversion.cpp \
    dawg.cpp dict.cpp hyphen.cpp \
    permdawg.cpp permngram.cpp permute.cpp states.cpp stopper.cpp trie.cpp
