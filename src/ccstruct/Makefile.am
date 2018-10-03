AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/opencl
AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

pkginclude_HEADERS = publictypes.h
noinst_HEADERS = \
    blamer.h blobbox.h blobs.h blread.h boxread.h boxword.h \
    ccstruct.h coutln.h crakedge.h \
    debugpixa.h detlinefit.h dppoint.h fontinfo.h \
    imagedata.h \
    linlsq.h matrix.h mod128.h normalis.h \
    ocrblock.h ocrpara.h ocrrow.h otsuthr.h \
    pageres.h params_training_featdef.h \
    pdblock.h points.h polyaprx.h polyblk.h \
    quadlsq.h quadratc.h quspline.h ratngs.h rect.h rejctmap.h \
    seam.h split.h statistc.h stepblob.h vecfuncs.h werd.h

noinst_LTLIBRARIES = libtesseract_ccstruct.la

libtesseract_ccstruct_la_SOURCES = \
    blamer.cpp blobbox.cpp blobs.cpp blread.cpp boxread.cpp boxword.cpp ccstruct.cpp coutln.cpp \
    detlinefit.cpp dppoint.cpp fontinfo.cpp \
    imagedata.cpp \
    linlsq.cpp matrix.cpp mod128.cpp normalis.cpp \
    ocrblock.cpp ocrpara.cpp ocrrow.cpp otsuthr.cpp \
    pageres.cpp pdblock.cpp points.cpp polyaprx.cpp polyblk.cpp \
    params_training_featdef.cpp publictypes.cpp \
    quadlsq.cpp quspline.cpp ratngs.cpp rect.cpp rejctmap.cpp \
    seam.cpp split.cpp statistc.cpp stepblob.cpp \
    vecfuncs.cpp werd.cpp
