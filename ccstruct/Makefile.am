SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer

EXTRA_DIST = ccstruct.vcproj

include_HEADERS = \
    blckerr.h blobbox.h blobs.h blread.h ccstruct.h coutln.h crakedge.h \
    detlinefit.h genblob.h hpddef.h hpdsizes.h ipoints.h \
    labls.h linlsq.h lmedsq.h mod128.h normalis.h \
    ocrblock.h ocrrow.h otsuthr.h \
    pageres.h pdblock.h points.h polyaprx.h polyblk.h \
    polyblob.h polyvert.h poutline.h \
    quadlsq.h quadratc.h quspline.h ratngs.h rect.h rejctmap.h \
    statistc.h stepblob.h vecfuncs.h werd.h

lib_LTLIBRARIES = libtesseract_ccstruct.la
libtesseract_ccstruct_la_SOURCES = \
    blobbox.cpp blobs.cpp blread.cpp callcpp.cpp ccstruct.cpp coutln.cpp \
    detlinefit.cpp genblob.cpp \
    labls.cpp linlsq.cpp lmedsq.cpp mod128.cpp normalis.cpp \
    ocrblock.cpp ocrrow.cpp otsuthr.cpp \
    pageres.cpp pdblock.cpp points.cpp polyaprx.cpp polyblk.cpp \
    polyblob.cpp polyvert.cpp poutline.cpp \
    quadlsq.cpp quadratc.cpp quspline.cpp ratngs.cpp rect.cpp rejctmap.cpp \
    statistc.cpp stepblob.cpp \
    vecfuncs.cpp werd.cpp

libtesseract_ccstruct_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
