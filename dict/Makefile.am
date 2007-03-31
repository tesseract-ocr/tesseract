SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil

include_HEADERS = \
    choicearr.h choices.h context.h dawg.h hyphen.h matchdefs.h \
    permdawg.h permnum.h permute.h states.h stopper.h trie.h

lib_LIBRARIES = libtesseract_dict.a
libtesseract_dict_a_SOURCES = \
    choices.cpp context.cpp dawg.cpp hyphen.cpp permdawg.cpp \
    permnum.cpp permute.cpp states.cpp stopper.cpp trie.cpp
