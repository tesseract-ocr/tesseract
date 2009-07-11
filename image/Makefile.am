SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer

EXTRA_DIST = image.vcproj

include_HEADERS = \
    bitstrm.h image.h img.h imgbmp.h imgerrs.h imgio.h imgs.h \
    imgtiff.h imgunpk.h svshowim.h

lib_LIBRARIES = libtesseract_image.a
libtesseract_image_a_SOURCES = \
    bitstrm.cpp image.cpp imgbmp.cpp imgio.cpp imgs.cpp imgtiff.cpp svshowim.cpp
