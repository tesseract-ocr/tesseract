AM_CPPFLAGS += -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/viewer -I$(top_srcdir)/image
    
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    dawg.h dict.h matchdefs.h \
    permute.h states.h stopper.h trie.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_dict.la
else
lib_LTLIBRARIES = libtesseract_dict.la
libtesseract_dict_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_dict_la_LIBADD = \
    ../ccutil/libtesseract_ccutil.la \
    ../cutil/libtesseract_cutil.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../viewer/libtesseract_viewer.la
endif

libtesseract_dict_la_SOURCES = \
    context.cpp \
    dawg.cpp dict.cpp hyphen.cpp \
    permdawg.cpp permute.cpp states.cpp stopper.cpp trie.cpp


