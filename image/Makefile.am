SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer

include_HEADERS = \
    image.h img.h imgerrs.h imgs.h \
    imgtiff.h imgunpk.h svshowim.h

lib_LTLIBRARIES = libtesseract_image.la
libtesseract_image_la_SOURCES = \
    image.cpp imgs.cpp imgtiff.cpp svshowim.cpp

libtesseract_image_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
