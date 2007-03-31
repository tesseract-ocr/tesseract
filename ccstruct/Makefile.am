SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer

include_HEADERS = \
    blckerr.h blobbox.h blobs.h blread.h coutln.h crakedge.h \
    genblob.h hpddef.h hpdsizes.h ipoints.h labls.h linlsq.h \
    lmedsq.h mod128.h normalis.h ocrblock.h ocrrow.h pageblk.h \
    pageres.h pdblock.h pdclass.h points.h polyaprx.h polyblk.h \
    polyblob.h polyvert.h poutline.h quadlsq.h quadratc.h \
    quspline.h ratngs.h rect.h rejctmap.h rwpoly.h statistc.h \
    stepblob.h txtregn.h vecfuncs.h werd.h

lib_LIBRARIES = libtesseract_ccstruct.a
libtesseract_ccstruct_a_SOURCES = \
    blobbox.cpp blobs.cpp blread.cpp callcpp.cpp \
    coutln.cpp genblob.cpp labls.cpp linlsq.cpp \
    lmedsq.cpp mod128.cpp normalis.cpp ocrblock.cpp \
    ocrrow.cpp pageblk.cpp pageres.cpp pdblock.cpp \
    points.cpp polyaprx.cpp polyblk.cpp polyblob.cpp \
    polyvert.cpp poutline.cpp quadlsq.cpp quadratc.cpp \
    quspline.cpp ratngs.cpp rect.cpp rejctmap.cpp \
    rwpoly.cpp statistc.cpp stepblob.cpp txtregn.cpp \
    vecfuncs.cpp werd.cpp
