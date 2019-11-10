AM_CPPFLAGS += \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/cutil \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/wordrec \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/opencl
AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

pkginclude_HEADERS = publictypes.h
noinst_HEADERS = \
    blamer.h blobbox.h blobs.h blread.h boxread.h boxword.h \
    ccstruct.h coutln.h crakedge.h \
    debugpixa.h detlinefit.h dppoint.h \
    imagedata.h \
    linlsq.h matrix.h mod128.h normalis.h \
    ocrblock.h ocrpara.h ocrrow.h otsuthr.h \
    pageres.h \
    pdblock.h points.h polyaprx.h polyblk.h \
    quadlsq.h quadratc.h quspline.h ratngs.h rect.h rejctmap.h \
    seam.h split.h statistc.h stepblob.h werd.h

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += fontinfo.h
noinst_HEADERS += params_training_featdef.h
endif

noinst_LTLIBRARIES = libtesseract_ccstruct.la

libtesseract_ccstruct_la_SOURCES = \
    blamer.cpp blobbox.cpp blobs.cpp blread.cpp boxread.cpp boxword.cpp ccstruct.cpp coutln.cpp \
    detlinefit.cpp dppoint.cpp \
    imagedata.cpp \
    linlsq.cpp matrix.cpp mod128.cpp normalis.cpp \
    ocrblock.cpp ocrpara.cpp ocrrow.cpp otsuthr.cpp \
    pageres.cpp pdblock.cpp points.cpp polyaprx.cpp polyblk.cpp \
    publictypes.cpp \
    quadlsq.cpp quspline.cpp ratngs.cpp rect.cpp rejctmap.cpp \
    seam.cpp split.cpp statistc.cpp stepblob.cpp \
    werd.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_ccstruct_la_SOURCES += fontinfo.cpp
libtesseract_ccstruct_la_SOURCES += params_training_featdef.cpp
endif
