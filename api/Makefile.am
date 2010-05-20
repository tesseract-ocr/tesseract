SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord

include_HEADERS = \
    baseapi.h tesseractmain.h

lib_LIBRARIES = libtesseract_api.a
libtesseract_api_a_SOURCES = baseapi.cpp
libtesseract_api.o: baseapi.o \
    ../ccmain/libtesseract_main.a \
    ../textord/libtesseract_textord.a \
    ../wordrec/libtesseract_wordrec.a \
    ../classify/libtesseract_classify.a \
    ../dict/libtesseract_dict.a \
    ../ccstruct/libtesseract_ccstruct.a \
    ../image/libtesseract_image.a \
    ../cutil/libtesseract_cutil.a \
    ../viewer/libtesseract_viewer.a \
    ../ccutil/libtesseract_ccutil.a
	ld -r -o libtesseract_api.o baseapi.o \
    ../ccmain/libtesseract_main.a \
    ../textord/libtesseract_textord.a \
    ../wordrec/libtesseract_wordrec.a \
    ../classify/libtesseract_classify.a \
    ../dict/libtesseract_dict.a \
    ../ccstruct/libtesseract_ccstruct.a \
    ../image/libtesseract_image.a \
    ../cutil/libtesseract_cutil.a \
    ../viewer/libtesseract_viewer.a \
    ../ccutil/libtesseract_ccutil.a

libtesseract_api.a : libtesseract_api.o
	ar cru libtesseract_api.a libtesseract_api.o ; $(RANLIB) libtesseract_api.a

bin_PROGRAMS = tesseract
tesseract_SOURCES = tesseractmain.cpp
tesseract_LDADD = \
    libtesseract_api.a
