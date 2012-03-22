AM_CPPFLAGS += -I$(top_srcdir)/ccutil -I$(top_srcdir)/viewer
    
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    image.h img.h imgerrs.h imgs.h \
    imgtiff.h imgunpk.h svshowim.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_image.la
else
lib_LTLIBRARIES = libtesseract_image.la
libtesseract_image_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_image_la_LIBADD = \
    ../ccutil/libtesseract_ccutil.la \
    ../viewer/libtesseract_viewer.la
endif

libtesseract_image_la_SOURCES = \
    image.cpp imgs.cpp imgtiff.cpp svshowim.cpp

