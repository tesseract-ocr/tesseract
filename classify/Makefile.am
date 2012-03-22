AM_CPPFLAGS += \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/dict \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer
    
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    adaptive.h baseline.h blobclass.h chartoname.h \
    classify.h cluster.h clusttool.h cutoffs.h \
    errorcounter.h extern.h extract.h \
    featdefs.h flexfx.h float2int.h fpoint.h fxdefs.h \
    intfeaturedist.h intfeaturemap.h intfeaturespace.h \
    intfx.h intmatcher.h intproto.h kdtree.h \
    mastertrainer.h mf.h mfdefs.h mfoutline.h mfx.h \
    normfeat.h normmatch.h \
    ocrfeatures.h outfeat.h picofeat.h protos.h \
    sampleiterator.h shapeclassifier.h shapetable.h \
    speckle.h tessclassifier.h trainingsample.h trainingsampleset.h xform2d.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_classify.la
else
lib_LTLIBRARIES = libtesseract_classify.la
libtesseract_classify_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_classify_la_LIBADD = \
    ../ccutil/libtesseract_ccutil.la \
    ../cutil/libtesseract_cutil.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../dict/libtesseract_dict.la \
    ../image/libtesseract_image.la \
    ../viewer/libtesseract_viewer.la
endif

libtesseract_classify_la_SOURCES = \
    adaptive.cpp adaptmatch.cpp blobclass.cpp \
    chartoname.cpp classify.cpp cluster.cpp clusttool.cpp cutoffs.cpp \
    errorcounter.cpp extract.cpp \
    featdefs.cpp flexfx.cpp float2int.cpp fpoint.cpp fxdefs.cpp \
    intfeaturedist.cpp intfeaturemap.cpp intfeaturespace.cpp \
    intfx.cpp intmatcher.cpp intproto.cpp kdtree.cpp \
    mastertrainer.cpp mf.cpp mfdefs.cpp mfoutline.cpp mfx.cpp \
    normfeat.cpp normmatch.cpp \
    ocrfeatures.cpp outfeat.cpp picofeat.cpp protos.cpp \
    sampleiterator.cpp shapetable.cpp speckle.cpp \
    tessclassifier.cpp trainingsample.cpp trainingsampleset.cpp xform2d.cpp


