AM_CPPFLAGS += \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/opencl
AM_CPPFLAGS += $(OPENCL_CPPFLAGS)
    
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

include_HEADERS = publictypes.h
noinst_HEADERS = \
    blamer.h blckerr.h blobbox.h blobs.h blread.h boxread.h boxword.h ccstruct.h coutln.h crakedge.h \
    detlinefit.h dppoint.h fontinfo.h genblob.h hpdsizes.h \
    imagedata.h \
    ipoints.h \
    linlsq.h matrix.h mod128.h normalis.h \
    ocrblock.h ocrpara.h ocrrow.h otsuthr.h \
    pageres.h params_training_featdef.h \
    pdblock.h points.h polyaprx.h polyblk.h \
    quadlsq.h quadratc.h quspline.h ratngs.h rect.h rejctmap.h \
    seam.h split.h statistc.h stepblob.h vecfuncs.h werd.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_ccstruct.la
else
lib_LTLIBRARIES = libtesseract_ccstruct.la
libtesseract_ccstruct_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_ccstruct_la_LIBADD = \
    ../ccutil/libtesseract_ccutil.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../opencl/libtesseract_opencl.la 
endif

libtesseract_ccstruct_la_SOURCES = \
    blamer.cpp blobbox.cpp blobs.cpp blread.cpp boxread.cpp boxword.cpp ccstruct.cpp coutln.cpp \
    detlinefit.cpp dppoint.cpp fontinfo.cpp genblob.cpp \
    imagedata.cpp \
    linlsq.cpp matrix.cpp mod128.cpp normalis.cpp \
    ocrblock.cpp ocrpara.cpp ocrrow.cpp otsuthr.cpp \
    pageres.cpp pdblock.cpp points.cpp polyaprx.cpp polyblk.cpp \
    params_training_featdef.cpp publictypes.cpp \
    quadlsq.cpp quspline.cpp ratngs.cpp rect.cpp rejctmap.cpp \
    seam.cpp split.cpp statistc.cpp stepblob.cpp \
    vecfuncs.cpp werd.cpp
