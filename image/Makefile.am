SUBDIRS =
AM_CPPFLAGS = -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer

include_HEADERS = \
    bitstrm.h imgbmp.h imgerrs.h img.h imgio.h imgs.h \
    imgtiff.h imgunpk.h svshowim.h

lib_LIBRARIES = libtesseract_image.a
libtesseract_image_a_SOURCES = \
    imgbmp.cpp imgio.cpp imgs.cpp imgtiff.cpp bitstrm.cpp svshowim.cpp
