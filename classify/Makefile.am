SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/dict \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer

EXTRA_DIST = classify.vcproj

include_HEADERS = \
    adaptive.h adaptmatch.h baseline.h blobclass.h chartoname.h \
    classify.h cluster.h clusttool.h cutoffs.h \
    extern.h extract.h \
    featdefs.h flexfx.h float2int.h fpoint.h fxdefs.h fxid.h \
    hideedge.h intfx.h intmatcher.h intproto.h kdtree.h \
    mf.h mfdefs.h mfoutline.h mfx.h \
    normfeat.h normmatch.h \
    ocrfeatures.h outfeat.h picofeat.h protos.h \
    speckle.h xform2d.h

lib_LTLIBRARIES = libtesseract_classify.la
libtesseract_classify_la_SOURCES = \
    adaptive.cpp adaptmatch.cpp baseline.cpp blobclass.cpp \
    chartoname.cpp classify.cpp cluster.cpp clusttool.cpp cutoffs.cpp \
    extract.cpp \
    featdefs.cpp flexfx.cpp float2int.cpp fpoint.cpp fxdefs.cpp \
    hideedge.cpp intfx.cpp intmatcher.cpp intproto.cpp kdtree.cpp \
    mf.cpp mfdefs.cpp mfoutline.cpp mfx.cpp \
    normfeat.cpp normmatch.cpp \
    ocrfeatures.cpp outfeat.cpp picofeat.cpp protos.cpp \
    speckle.cpp xform2d.cpp

libtesseract_classify_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
