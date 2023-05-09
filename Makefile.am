## run autogen.sh to create Makefile.in from this file
ACLOCAL_AMFLAGS = -I m4

.PHONY: doc html install-langs ScrollView.jar install-jars pdf training

CLEANFILES =

SUBDIRS = . tessdata

EXTRA_DIST = README.md LICENSE
EXTRA_DIST += aclocal.m4 config configure.ac autogen.sh
EXTRA_DIST += tesseract.pc.in doc
if !GRAPHICS_DISABLED
EXTRA_DIST += java
endif
EXTRA_DIST += CMakeLists.txt tesseract.pc.cmake cmake VERSION

DIST_SUBDIRS = $(SUBDIRS)

EXTRA_PROGRAMS =

uninstall-hook:
	rm -rf $(DESTDIR)$(pkgincludedir)

dist-hook:
# Need to remove .svn directories from directories
# added using EXTRA_DIST. $(distdir)/tessdata would in
# theory suffice.
	rm -rf `find $(distdir) -name .deps -type d`
	-rm -f $(distdir)/*/Makefile $(distdir)/*/*/Makefile
	rm -f `find $(distdir) -name '*~'`
	rm -rf $(distdir)/doc/html/* $(distdir)/doc/*.log

if !GRAPHICS_DISABLED
ScrollView.jar:
	@cd "$(top_builddir)/java" && $(MAKE) $@

install-jars:
	@cd "$(top_builddir)/java" && $(MAKE) $@
endif

doc:
	-srcdir="$(top_srcdir)" builddir="$(top_builddir)" \
	version="@PACKAGE_VERSION@" name="@PACKAGE_NAME@" \
	doxygen $(top_srcdir)/doc/Doxyfile

doc-pack: doc
	-chmod a+r $(top_builddir)/doc/html/*
	 @tar --create --directory=$(top_builddir)/doc/html --verbose --file=- . | gzip -c -9 > $(top_builddir)/@PACKAGE_NAME@-@PACKAGE_VERSION@-doc-html.tar.gz;

doc-clean:
	rm -rf $(top_builddir)/doc/html/*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tesseract.pc

pkginclude_HEADERS = $(top_builddir)/include/tesseract/version.h
pkginclude_HEADERS += include/tesseract/baseapi.h
pkginclude_HEADERS += include/tesseract/capi.h
pkginclude_HEADERS += include/tesseract/export.h
pkginclude_HEADERS += include/tesseract/ltrresultiterator.h
pkginclude_HEADERS += include/tesseract/ocrclass.h
pkginclude_HEADERS += include/tesseract/osdetect.h
pkginclude_HEADERS += include/tesseract/pageiterator.h
pkginclude_HEADERS += include/tesseract/publictypes.h
pkginclude_HEADERS += include/tesseract/renderer.h
pkginclude_HEADERS += include/tesseract/resultiterator.h
pkginclude_HEADERS += include/tesseract/unichar.h

# Rules for all subdirectories.

noinst_HEADERS =
noinst_LTLIBRARIES =

AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_builddir)/include
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -fPIC
endif
AM_CPPFLAGS += $(OPENCL_CPPFLAGS)

AM_CXXFLAGS = $(OPENMP_CXXFLAGS)

# Rules for src/api.

libtesseract_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_la_CPPFLAGS += -DTESS_COMMON_TRAINING_API=
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/arch
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/ccmain
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/classify
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/cutil
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/dict
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/lstm
if OPENCL
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/opencl
endif
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/textord
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/training/common
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/viewer
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/wordrec
libtesseract_la_CPPFLAGS += $(libcurl_CFLAGS)

lib_LTLIBRARIES = libtesseract.la
libtesseract_la_LDFLAGS = $(LEPTONICA_LIBS) $(OPENCL_LDFLAGS)
libtesseract_la_LDFLAGS += $(libarchive_LIBS)
libtesseract_la_LDFLAGS += $(libcurl_LIBS)
libtesseract_la_LDFLAGS += $(TENSORFLOW_LIBS)
if T_WIN
libtesseract_la_LDFLAGS += -no-undefined -lws2_32
else
libtesseract_la_LDFLAGS += $(NOUNDEFINED)
endif
libtesseract_la_LDFLAGS += -version-info $(GENERIC_LIBRARY_VERSION)

libtesseract_la_SOURCES = src/api/baseapi.cpp
libtesseract_la_SOURCES += src/api/altorenderer.cpp
libtesseract_la_SOURCES += src/api/capi.cpp
libtesseract_la_SOURCES += src/api/hocrrenderer.cpp
libtesseract_la_SOURCES += src/api/lstmboxrenderer.cpp
libtesseract_la_SOURCES += src/api/pdfrenderer.cpp
libtesseract_la_SOURCES += src/api/renderer.cpp
libtesseract_la_SOURCES += src/api/wordstrboxrenderer.cpp

libtesseract_la_LIBADD = libtesseract_ccutil.la
libtesseract_la_LIBADD += libtesseract_lstm.la
libtesseract_la_LIBADD += libtesseract_native.la
if OPENCL
libtesseract_la_LIBADD += libtesseract_opencl.la
endif

# Rules for src/arch.

noinst_HEADERS += src/arch/dotproduct.h
noinst_HEADERS += src/arch/intsimdmatrix.h
noinst_HEADERS += src/arch/simddetect.h

noinst_LTLIBRARIES += libtesseract_native.la

libtesseract_native_la_CXXFLAGS = -O3 -ffast-math
if OPENMP_SIMD
libtesseract_native_la_CXXFLAGS += -fopenmp-simd -DOPENMP_SIMD
endif
libtesseract_native_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_native_la_SOURCES = src/arch/dotproduct.cpp

if HAVE_AVX
libtesseract_avx_la_CXXFLAGS = -mavx
libtesseract_avx_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_avx_la_SOURCES = src/arch/dotproductavx.cpp
libtesseract_la_LIBADD += libtesseract_avx.la
noinst_LTLIBRARIES += libtesseract_avx.la
endif

if HAVE_AVX2
libtesseract_avx2_la_CXXFLAGS = -mavx2
libtesseract_avx2_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_avx2_la_SOURCES = src/arch/intsimdmatrixavx2.cpp
libtesseract_la_LIBADD += libtesseract_avx2.la
noinst_LTLIBRARIES += libtesseract_avx2.la
endif

if HAVE_AVX512F
libtesseract_avx512_la_CXXFLAGS = -mavx512f
libtesseract_avx512_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_avx512_la_SOURCES = src/arch/dotproductavx512.cpp
libtesseract_la_LIBADD += libtesseract_avx512.la
noinst_LTLIBRARIES += libtesseract_avx512.la
endif

if HAVE_FMA
libtesseract_fma_la_CXXFLAGS = -mfma
libtesseract_fma_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_fma_la_SOURCES = src/arch/dotproductfma.cpp
libtesseract_la_LIBADD += libtesseract_fma.la
noinst_LTLIBRARIES += libtesseract_fma.la
endif

if HAVE_SSE4_1
libtesseract_sse_la_CXXFLAGS = -msse4.1
libtesseract_sse_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_sse_la_SOURCES = src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp
libtesseract_la_LIBADD += libtesseract_sse.la
noinst_LTLIBRARIES += libtesseract_sse.la
endif

if HAVE_NEON
libtesseract_neon_la_CXXFLAGS = $(NEON_CXXFLAGS)
libtesseract_neon_la_CXXFLAGS += -O3
if OPENMP_SIMD
libtesseract_neon_la_CXXFLAGS += -fopenmp-simd -DOPENMP_SIMD
endif
libtesseract_neon_la_CXXFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_neon_la_SOURCES = src/arch/intsimdmatrixneon.cpp
libtesseract_neon_la_SOURCES += src/arch/dotproductneon.cpp
libtesseract_la_LIBADD += libtesseract_neon.la
noinst_LTLIBRARIES += libtesseract_neon.la
endif

libtesseract_la_SOURCES += src/arch/intsimdmatrix.cpp
libtesseract_la_SOURCES += src/arch/simddetect.cpp

# Rules for src/ccmain.

noinst_HEADERS += src/ccmain/control.h
noinst_HEADERS += src/ccmain/mutableiterator.h
noinst_HEADERS += src/ccmain/output.h
noinst_HEADERS += src/ccmain/paragraphs.h
noinst_HEADERS += src/ccmain/paragraphs_internal.h
noinst_HEADERS += src/ccmain/paramsd.h
noinst_HEADERS += src/ccmain/pgedit.h
noinst_HEADERS += src/ccmain/tesseractclass.h
noinst_HEADERS += src/ccmain/tessvars.h
noinst_HEADERS += src/ccmain/thresholder.h
noinst_HEADERS += src/ccmain/werdit.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/ccmain/docqual.h
noinst_HEADERS += src/ccmain/equationdetect.h
noinst_HEADERS += src/ccmain/fixspace.h
noinst_HEADERS += src/ccmain/reject.h
endif

libtesseract_la_SOURCES += src/ccmain/applybox.cpp
libtesseract_la_SOURCES += src/ccmain/control.cpp
libtesseract_la_SOURCES += src/ccmain/linerec.cpp
libtesseract_la_SOURCES += src/ccmain/ltrresultiterator.cpp
libtesseract_la_SOURCES += src/ccmain/mutableiterator.cpp
libtesseract_la_SOURCES += src/ccmain/output.cpp
libtesseract_la_SOURCES += src/ccmain/pageiterator.cpp
libtesseract_la_SOURCES += src/ccmain/pagesegmain.cpp
libtesseract_la_SOURCES += src/ccmain/pagewalk.cpp
libtesseract_la_SOURCES += src/ccmain/paragraphs.cpp
if !GRAPHICS_DISABLED
libtesseract_la_SOURCES += src/ccmain/paramsd.cpp
libtesseract_la_SOURCES += src/ccmain/pgedit.cpp
endif
libtesseract_la_SOURCES += src/ccmain/reject.cpp
libtesseract_la_SOURCES += src/ccmain/resultiterator.cpp
libtesseract_la_SOURCES += src/ccmain/tessedit.cpp
libtesseract_la_SOURCES += src/ccmain/tesseractclass.cpp
libtesseract_la_SOURCES += src/ccmain/tessvars.cpp
libtesseract_la_SOURCES += src/ccmain/thresholder.cpp
libtesseract_la_SOURCES += src/ccmain/werdit.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/ccmain/adaptions.cpp
libtesseract_la_SOURCES += src/ccmain/docqual.cpp
libtesseract_la_SOURCES += src/ccmain/equationdetect.cpp
libtesseract_la_SOURCES += src/ccmain/fixspace.cpp
libtesseract_la_SOURCES += src/ccmain/fixxht.cpp
libtesseract_la_SOURCES += src/ccmain/osdetect.cpp
libtesseract_la_SOURCES += src/ccmain/par_control.cpp
libtesseract_la_SOURCES += src/ccmain/recogtraining.cpp
libtesseract_la_SOURCES += src/ccmain/superscript.cpp
libtesseract_la_SOURCES += src/ccmain/tessbox.cpp
libtesseract_la_SOURCES += src/ccmain/tfacepp.cpp
endif

# Rules for src/ccstruct.

noinst_HEADERS += src/ccstruct/blamer.h
noinst_HEADERS += src/ccstruct/blobbox.h
noinst_HEADERS += src/ccstruct/blobs.h
noinst_HEADERS += src/ccstruct/blread.h
noinst_HEADERS += src/ccstruct/boxread.h
noinst_HEADERS += src/ccstruct/boxword.h
noinst_HEADERS += src/ccstruct/ccstruct.h
noinst_HEADERS += src/ccstruct/coutln.h
noinst_HEADERS += src/ccstruct/crakedge.h
noinst_HEADERS += src/ccstruct/debugpixa.h
noinst_HEADERS += src/ccstruct/detlinefit.h
noinst_HEADERS += src/ccstruct/dppoint.h
noinst_HEADERS += src/ccstruct/image.h
noinst_HEADERS += src/ccstruct/imagedata.h
noinst_HEADERS += src/ccstruct/linlsq.h
noinst_HEADERS += src/ccstruct/matrix.h
noinst_HEADERS += src/ccstruct/mod128.h
noinst_HEADERS += src/ccstruct/normalis.h
noinst_HEADERS += src/ccstruct/ocrblock.h
noinst_HEADERS += src/ccstruct/ocrpara.h
noinst_HEADERS += src/ccstruct/ocrrow.h
noinst_HEADERS += src/ccstruct/otsuthr.h
noinst_HEADERS += src/ccstruct/pageres.h
noinst_HEADERS += src/ccstruct/pdblock.h
noinst_HEADERS += src/ccstruct/points.h
noinst_HEADERS += src/ccstruct/polyaprx.h
noinst_HEADERS += src/ccstruct/polyblk.h
noinst_HEADERS += src/ccstruct/quadlsq.h
noinst_HEADERS += src/ccstruct/quadratc.h
noinst_HEADERS += src/ccstruct/quspline.h
noinst_HEADERS += src/ccstruct/ratngs.h
noinst_HEADERS += src/ccstruct/rect.h
noinst_HEADERS += src/ccstruct/rejctmap.h
noinst_HEADERS += src/ccstruct/seam.h
noinst_HEADERS += src/ccstruct/split.h
noinst_HEADERS += src/ccstruct/statistc.h
noinst_HEADERS += src/ccstruct/stepblob.h
noinst_HEADERS += src/ccstruct/werd.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/ccstruct/fontinfo.h
noinst_HEADERS += src/ccstruct/params_training_featdef.h
endif

libtesseract_la_SOURCES += src/ccstruct/blamer.cpp
libtesseract_la_SOURCES += src/ccstruct/blobbox.cpp
libtesseract_la_SOURCES += src/ccstruct/blobs.cpp
libtesseract_la_SOURCES += src/ccstruct/blread.cpp
libtesseract_la_SOURCES += src/ccstruct/boxread.cpp
libtesseract_la_SOURCES += src/ccstruct/boxword.cpp
libtesseract_la_SOURCES += src/ccstruct/ccstruct.cpp
libtesseract_la_SOURCES += src/ccstruct/coutln.cpp
libtesseract_la_SOURCES += src/ccstruct/detlinefit.cpp
libtesseract_la_SOURCES += src/ccstruct/dppoint.cpp
libtesseract_la_SOURCES += src/ccstruct/image.cpp
libtesseract_la_SOURCES += src/ccstruct/imagedata.cpp
libtesseract_la_SOURCES += src/ccstruct/linlsq.cpp
libtesseract_la_SOURCES += src/ccstruct/matrix.cpp
libtesseract_la_SOURCES += src/ccstruct/mod128.cpp
libtesseract_la_SOURCES += src/ccstruct/normalis.cpp
libtesseract_la_SOURCES += src/ccstruct/ocrblock.cpp
libtesseract_la_SOURCES += src/ccstruct/ocrpara.cpp
libtesseract_la_SOURCES += src/ccstruct/ocrrow.cpp
libtesseract_la_SOURCES += src/ccstruct/otsuthr.cpp
libtesseract_la_SOURCES += src/ccstruct/pageres.cpp
libtesseract_la_SOURCES += src/ccstruct/pdblock.cpp
libtesseract_la_SOURCES += src/ccstruct/points.cpp
libtesseract_la_SOURCES += src/ccstruct/polyaprx.cpp
libtesseract_la_SOURCES += src/ccstruct/polyblk.cpp
libtesseract_la_SOURCES += src/ccstruct/quadlsq.cpp
libtesseract_la_SOURCES += src/ccstruct/quspline.cpp
libtesseract_la_SOURCES += src/ccstruct/ratngs.cpp
libtesseract_la_SOURCES += src/ccstruct/rect.cpp
libtesseract_la_SOURCES += src/ccstruct/rejctmap.cpp
libtesseract_la_SOURCES += src/ccstruct/seam.cpp
libtesseract_la_SOURCES += src/ccstruct/split.cpp
libtesseract_la_SOURCES += src/ccstruct/statistc.cpp
libtesseract_la_SOURCES += src/ccstruct/stepblob.cpp
libtesseract_la_SOURCES += src/ccstruct/werd.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/ccstruct/fontinfo.cpp
libtesseract_la_SOURCES += src/ccstruct/params_training_featdef.cpp
endif

# Rules for src/ccutil

libtesseract_ccutil_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_ccutil_la_CPPFLAGS += $(libarchive_CFLAGS)
if !NO_TESSDATA_PREFIX
libtesseract_ccutil_la_CPPFLAGS += -DTESSDATA_PREFIX='"@datadir@"'
endif

noinst_HEADERS += src/ccutil/ccutil.h
noinst_HEADERS += src/ccutil/clst.h
noinst_HEADERS += src/ccutil/elst2.h
noinst_HEADERS += src/ccutil/elst.h
noinst_HEADERS += src/ccutil/errcode.h
noinst_HEADERS += src/ccutil/fileerr.h
noinst_HEADERS += src/ccutil/genericheap.h
noinst_HEADERS += src/ccutil/genericvector.h
noinst_HEADERS += src/ccutil/helpers.h
noinst_HEADERS += src/ccutil/host.h
noinst_HEADERS += src/ccutil/kdpair.h
noinst_HEADERS += src/ccutil/lsterr.h
noinst_HEADERS += src/ccutil/object_cache.h
noinst_HEADERS += src/ccutil/params.h
noinst_HEADERS += src/ccutil/qrsequence.h
noinst_HEADERS += src/ccutil/sorthelper.h
noinst_HEADERS += src/ccutil/scanutils.h
noinst_HEADERS += src/ccutil/serialis.h
noinst_HEADERS += src/ccutil/tessdatamanager.h
noinst_HEADERS += src/ccutil/tprintf.h
noinst_HEADERS += src/ccutil/unicharcompress.h
noinst_HEADERS += src/ccutil/unicharmap.h
noinst_HEADERS += src/ccutil/unicharset.h
noinst_HEADERS += src/ccutil/unicity_table.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/ccutil/ambigs.h
noinst_HEADERS += src/ccutil/bitvector.h
noinst_HEADERS += src/ccutil/indexmapbidi.h
noinst_HEADERS += src/ccutil/universalambigs.h
endif

noinst_LTLIBRARIES += libtesseract_ccutil.la

libtesseract_ccutil_la_SOURCES = src/ccutil/ccutil.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/clst.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/elst2.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/elst.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/errcode.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/serialis.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/scanutils.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/tessdatamanager.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/tprintf.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/unichar.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/unicharcompress.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/unicharmap.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/unicharset.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/params.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_ccutil_la_SOURCES += src/ccutil/ambigs.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/bitvector.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/indexmapbidi.cpp
endif

# Rules for src/classify.

noinst_HEADERS += src/classify/classify.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/classify/adaptive.h
noinst_HEADERS += src/classify/cluster.h
noinst_HEADERS += src/classify/clusttool.h
noinst_HEADERS += src/classify/featdefs.h
noinst_HEADERS += src/classify/float2int.h
noinst_HEADERS += src/classify/fpoint.h
noinst_HEADERS += src/classify/intfeaturespace.h
noinst_HEADERS += src/classify/intfx.h
noinst_HEADERS += src/classify/intmatcher.h
noinst_HEADERS += src/classify/intproto.h
noinst_HEADERS += src/classify/kdtree.h
noinst_HEADERS += src/classify/mf.h
noinst_HEADERS += src/classify/mfdefs.h
noinst_HEADERS += src/classify/mfoutline.h
noinst_HEADERS += src/classify/mfx.h
noinst_HEADERS += src/classify/normfeat.h
noinst_HEADERS += src/classify/normmatch.h
noinst_HEADERS += src/classify/ocrfeatures.h
noinst_HEADERS += src/classify/outfeat.h
noinst_HEADERS += src/classify/picofeat.h
noinst_HEADERS += src/classify/protos.h
noinst_HEADERS += src/classify/shapeclassifier.h
noinst_HEADERS += src/classify/shapetable.h
noinst_HEADERS += src/classify/tessclassifier.h
noinst_HEADERS += src/classify/trainingsample.h
endif

libtesseract_la_SOURCES += src/classify/classify.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/classify/adaptive.cpp
libtesseract_la_SOURCES += src/classify/adaptmatch.cpp
libtesseract_la_SOURCES += src/classify/blobclass.cpp
libtesseract_la_SOURCES += src/classify/cluster.cpp
libtesseract_la_SOURCES += src/classify/clusttool.cpp
libtesseract_la_SOURCES += src/classify/cutoffs.cpp
libtesseract_la_SOURCES += src/classify/featdefs.cpp
libtesseract_la_SOURCES += src/classify/float2int.cpp
libtesseract_la_SOURCES += src/classify/fpoint.cpp
libtesseract_la_SOURCES += src/classify/intfeaturespace.cpp
libtesseract_la_SOURCES += src/classify/intfx.cpp
libtesseract_la_SOURCES += src/classify/intmatcher.cpp
libtesseract_la_SOURCES += src/classify/intproto.cpp
libtesseract_la_SOURCES += src/classify/kdtree.cpp
libtesseract_la_SOURCES += src/classify/mf.cpp
libtesseract_la_SOURCES += src/classify/mfoutline.cpp
libtesseract_la_SOURCES += src/classify/mfx.cpp
libtesseract_la_SOURCES += src/classify/normfeat.cpp
libtesseract_la_SOURCES += src/classify/normmatch.cpp
libtesseract_la_SOURCES += src/classify/ocrfeatures.cpp
libtesseract_la_SOURCES += src/classify/outfeat.cpp
libtesseract_la_SOURCES += src/classify/picofeat.cpp
libtesseract_la_SOURCES += src/classify/protos.cpp
libtesseract_la_SOURCES += src/classify/shapeclassifier.cpp
libtesseract_la_SOURCES += src/classify/shapetable.cpp
libtesseract_la_SOURCES += src/classify/tessclassifier.cpp
libtesseract_la_SOURCES += src/classify/trainingsample.cpp
endif

# Rules for src/cutil.

if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/cutil/bitvec.h
noinst_HEADERS += src/cutil/oldlist.h
endif

if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/cutil/oldlist.cpp
endif

# Rules for src/dict.

noinst_HEADERS += src/dict/dawg.h
noinst_HEADERS += src/dict/dawg_cache.h
noinst_HEADERS += src/dict/dict.h
noinst_HEADERS += src/dict/matchdefs.h
noinst_HEADERS += src/dict/stopper.h
noinst_HEADERS += src/dict/trie.h

libtesseract_la_SOURCES += src/dict/context.cpp
libtesseract_la_SOURCES += src/dict/dawg.cpp
libtesseract_la_SOURCES += src/dict/dawg_cache.cpp
libtesseract_la_SOURCES += src/dict/dict.cpp
libtesseract_la_SOURCES += src/dict/stopper.cpp
libtesseract_la_SOURCES += src/dict/trie.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/dict/hyphen.cpp
libtesseract_la_SOURCES += src/dict/permdawg.cpp
endif

# Rules for src/lstm.

libtesseract_lstm_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/arch
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/classify
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/cutil
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/dict
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/lstm
libtesseract_lstm_la_CPPFLAGS += -I$(top_srcdir)/src/viewer
if TENSORFLOW
libtesseract_lstm_la_CPPFLAGS += -DINCLUDE_TENSORFLOW
libtesseract_lstm_la_CPPFLAGS += -I/usr/include/tensorflow
endif
if !NO_TESSDATA_PREFIX
libtesseract_lstm_la_CPPFLAGS += -DTESSDATA_PREFIX='"@datadir@"'
endif

noinst_HEADERS += src/lstm/convolve.h
noinst_HEADERS += src/lstm/fullyconnected.h
noinst_HEADERS += src/lstm/functions.h
noinst_HEADERS += src/lstm/input.h
noinst_HEADERS += src/lstm/lstm.h
noinst_HEADERS += src/lstm/lstmrecognizer.h
noinst_HEADERS += src/lstm/maxpool.h
noinst_HEADERS += src/lstm/network.h
noinst_HEADERS += src/lstm/networkio.h
noinst_HEADERS += src/lstm/networkscratch.h
noinst_HEADERS += src/lstm/parallel.h
noinst_HEADERS += src/lstm/plumbing.h
noinst_HEADERS += src/lstm/recodebeam.h
noinst_HEADERS += src/lstm/reconfig.h
noinst_HEADERS += src/lstm/reversed.h
noinst_HEADERS += src/lstm/series.h
noinst_HEADERS += src/lstm/static_shape.h
noinst_HEADERS += src/lstm/stridemap.h
noinst_HEADERS += src/lstm/tfnetwork.h
noinst_HEADERS += src/lstm/weightmatrix.h

noinst_LTLIBRARIES += libtesseract_lstm.la

libtesseract_lstm_la_SOURCES = src/lstm/convolve.cpp
libtesseract_lstm_la_SOURCES += src/lstm/fullyconnected.cpp
libtesseract_lstm_la_SOURCES += src/lstm/functions.cpp
libtesseract_lstm_la_SOURCES += src/lstm/input.cpp
libtesseract_lstm_la_SOURCES += src/lstm/lstm.cpp
libtesseract_lstm_la_SOURCES += src/lstm/lstmrecognizer.cpp
libtesseract_lstm_la_SOURCES += src/lstm/maxpool.cpp
libtesseract_lstm_la_SOURCES += src/lstm/network.cpp
libtesseract_lstm_la_SOURCES += src/lstm/networkio.cpp
libtesseract_lstm_la_SOURCES += src/lstm/parallel.cpp
libtesseract_lstm_la_SOURCES += src/lstm/plumbing.cpp
libtesseract_lstm_la_SOURCES += src/lstm/recodebeam.cpp
libtesseract_lstm_la_SOURCES += src/lstm/reconfig.cpp
libtesseract_lstm_la_SOURCES += src/lstm/reversed.cpp
libtesseract_lstm_la_SOURCES += src/lstm/series.cpp
libtesseract_lstm_la_SOURCES += src/lstm/stridemap.cpp
libtesseract_lstm_la_SOURCES += src/lstm/tfnetwork.cpp
libtesseract_lstm_la_SOURCES += src/lstm/weightmatrix.cpp
if TENSORFLOW
libtesseract_lstm_la_SOURCES += src/lstm/tfnetwork.pb.cc
endif

# Rules for src/opencl.

if OPENCL
libtesseract_opencl_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_opencl_la_CPPFLAGS += $(OPENCL_CFLAGS)
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccmain

noinst_HEADERS += src/opencl/openclwrapper.h
noinst_HEADERS += src/opencl/oclkernels.h

noinst_LTLIBRARIES += libtesseract_opencl.la

libtesseract_opencl_la_SOURCES = src/opencl/openclwrapper.cpp
endif

# Rules for src/textord.

noinst_HEADERS += src/textord/alignedblob.h
noinst_HEADERS += src/textord/baselinedetect.h
noinst_HEADERS += src/textord/bbgrid.h
noinst_HEADERS += src/textord/blkocc.h
noinst_HEADERS += src/textord/blobgrid.h
noinst_HEADERS += src/textord/ccnontextdetect.h
noinst_HEADERS += src/textord/cjkpitch.h
noinst_HEADERS += src/textord/colfind.h
noinst_HEADERS += src/textord/colpartition.h
noinst_HEADERS += src/textord/colpartitionset.h
noinst_HEADERS += src/textord/colpartitiongrid.h
noinst_HEADERS += src/textord/devanagari_processing.h
noinst_HEADERS += src/textord/drawtord.h
noinst_HEADERS += src/textord/edgblob.h
noinst_HEADERS += src/textord/edgloop.h
noinst_HEADERS += src/textord/fpchop.h
noinst_HEADERS += src/textord/gap_map.h
noinst_HEADERS += src/textord/imagefind.h
noinst_HEADERS += src/textord/linefind.h
noinst_HEADERS += src/textord/makerow.h
noinst_HEADERS += src/textord/oldbasel.h
noinst_HEADERS += src/textord/pithsync.h
noinst_HEADERS += src/textord/pitsync1.h
noinst_HEADERS += src/textord/scanedg.h
noinst_HEADERS += src/textord/sortflts.h
noinst_HEADERS += src/textord/strokewidth.h
noinst_HEADERS += src/textord/tabfind.h
noinst_HEADERS += src/textord/tablefind.h
noinst_HEADERS += src/textord/tabvector.h
noinst_HEADERS += src/textord/tablerecog.h
noinst_HEADERS += src/textord/textlineprojection.h
noinst_HEADERS += src/textord/textord.h
noinst_HEADERS += src/textord/topitch.h
noinst_HEADERS += src/textord/tordmain.h
noinst_HEADERS += src/textord/tovars.h
noinst_HEADERS += src/textord/underlin.h
noinst_HEADERS += src/textord/wordseg.h
noinst_HEADERS += src/textord/workingpartset.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/textord/equationdetectbase.h
endif

libtesseract_la_SOURCES += src/textord/alignedblob.cpp
libtesseract_la_SOURCES += src/textord/baselinedetect.cpp
libtesseract_la_SOURCES += src/textord/bbgrid.cpp
libtesseract_la_SOURCES += src/textord/blkocc.cpp
libtesseract_la_SOURCES += src/textord/blobgrid.cpp
libtesseract_la_SOURCES += src/textord/ccnontextdetect.cpp
libtesseract_la_SOURCES += src/textord/cjkpitch.cpp
libtesseract_la_SOURCES += src/textord/colfind.cpp
libtesseract_la_SOURCES += src/textord/colpartition.cpp
libtesseract_la_SOURCES += src/textord/colpartitionset.cpp
libtesseract_la_SOURCES += src/textord/colpartitiongrid.cpp
libtesseract_la_SOURCES += src/textord/devanagari_processing.cpp
libtesseract_la_SOURCES += src/textord/drawtord.cpp
libtesseract_la_SOURCES += src/textord/edgblob.cpp
libtesseract_la_SOURCES += src/textord/edgloop.cpp
libtesseract_la_SOURCES += src/textord/fpchop.cpp
libtesseract_la_SOURCES += src/textord/gap_map.cpp
libtesseract_la_SOURCES += src/textord/imagefind.cpp
libtesseract_la_SOURCES += src/textord/linefind.cpp
libtesseract_la_SOURCES += src/textord/makerow.cpp
libtesseract_la_SOURCES += src/textord/oldbasel.cpp
libtesseract_la_SOURCES += src/textord/pithsync.cpp
libtesseract_la_SOURCES += src/textord/pitsync1.cpp
libtesseract_la_SOURCES += src/textord/scanedg.cpp
libtesseract_la_SOURCES += src/textord/sortflts.cpp
libtesseract_la_SOURCES += src/textord/strokewidth.cpp
libtesseract_la_SOURCES += src/textord/tabfind.cpp
libtesseract_la_SOURCES += src/textord/tablefind.cpp
libtesseract_la_SOURCES += src/textord/tabvector.cpp
libtesseract_la_SOURCES += src/textord/tablerecog.cpp
libtesseract_la_SOURCES += src/textord/textlineprojection.cpp
libtesseract_la_SOURCES += src/textord/textord.cpp
libtesseract_la_SOURCES += src/textord/topitch.cpp
libtesseract_la_SOURCES += src/textord/tordmain.cpp
libtesseract_la_SOURCES += src/textord/tospace.cpp
libtesseract_la_SOURCES += src/textord/tovars.cpp
libtesseract_la_SOURCES += src/textord/underlin.cpp
libtesseract_la_SOURCES += src/textord/wordseg.cpp
libtesseract_la_SOURCES += src/textord/workingpartset.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/textord/equationdetectbase.cpp
endif

# Rules for src/viewer.

if !GRAPHICS_DISABLED
noinst_HEADERS += src/viewer/scrollview.h
noinst_HEADERS += src/viewer/svmnode.h
noinst_HEADERS += src/viewer/svutil.h

libtesseract_la_SOURCES += src/viewer/scrollview.cpp
libtesseract_la_SOURCES += src/viewer/svmnode.cpp
libtesseract_la_SOURCES += src/viewer/svutil.cpp

EXTRA_PROGRAMS += svpaint
svpaint_CPPFLAGS =
svpaint_CPPFLAGS += -I$(top_builddir)/include
svpaint_CPPFLAGS += -I$(top_srcdir)/include
svpaint_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
svpaint_CPPFLAGS += -I$(top_srcdir)/src/viewer
svpaint_SOURCES = src/svpaint.cpp
svpaint_LDADD = libtesseract.la
endif

# Rules for src/wordrec.

noinst_HEADERS += src/wordrec/wordrec.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/wordrec/associate.h
noinst_HEADERS += src/wordrec/chop.h
noinst_HEADERS += src/wordrec/drawfx.h
noinst_HEADERS += src/wordrec/findseam.h
noinst_HEADERS += src/wordrec/language_model.h
noinst_HEADERS += src/wordrec/lm_consistency.h
noinst_HEADERS += src/wordrec/lm_pain_points.h
noinst_HEADERS += src/wordrec/lm_state.h
noinst_HEADERS += src/wordrec/outlines.h
noinst_HEADERS += src/wordrec/params_model.h
noinst_HEADERS += src/wordrec/plotedges.h
noinst_HEADERS += src/wordrec/render.h
endif

libtesseract_la_SOURCES += src/wordrec/tface.cpp
libtesseract_la_SOURCES += src/wordrec/wordrec.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_la_SOURCES += src/wordrec/associate.cpp
libtesseract_la_SOURCES += src/wordrec/chop.cpp
libtesseract_la_SOURCES += src/wordrec/chopper.cpp
libtesseract_la_SOURCES += src/wordrec/drawfx.cpp
libtesseract_la_SOURCES += src/wordrec/findseam.cpp
libtesseract_la_SOURCES += src/wordrec/gradechop.cpp
libtesseract_la_SOURCES += src/wordrec/language_model.cpp
libtesseract_la_SOURCES += src/wordrec/lm_consistency.cpp
libtesseract_la_SOURCES += src/wordrec/lm_pain_points.cpp
libtesseract_la_SOURCES += src/wordrec/lm_state.cpp
libtesseract_la_SOURCES += src/wordrec/outlines.cpp
libtesseract_la_SOURCES += src/wordrec/params_model.cpp
libtesseract_la_SOURCES += src/wordrec/pieces.cpp
if !GRAPHICS_DISABLED
libtesseract_la_SOURCES += src/wordrec/plotedges.cpp
endif
libtesseract_la_SOURCES += src/wordrec/render.cpp
libtesseract_la_SOURCES += src/wordrec/segsearch.cpp
libtesseract_la_SOURCES += src/wordrec/wordclass.cpp
endif

# Rules for tesseract executable.

bin_PROGRAMS = tesseract
tesseract_SOURCES = src/tesseract.cpp
tesseract_CPPFLAGS =
tesseract_CPPFLAGS += -I$(top_srcdir)/src/arch
tesseract_CPPFLAGS += -I$(top_srcdir)/src/ccmain
tesseract_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
tesseract_CPPFLAGS += -I$(top_srcdir)/src/ccutil
tesseract_CPPFLAGS += -I$(top_srcdir)/src/classify
tesseract_CPPFLAGS += -I$(top_srcdir)/src/cutil
tesseract_CPPFLAGS += -I$(top_srcdir)/src/dict
tesseract_CPPFLAGS += -I$(top_srcdir)/src/textord
tesseract_CPPFLAGS += -I$(top_srcdir)/src/viewer
tesseract_CPPFLAGS += -I$(top_srcdir)/src/wordrec
if OPENCL
tesseract_CPPFLAGS += -I$(top_srcdir)/src/opencl
endif
tesseract_CPPFLAGS += $(AM_CPPFLAGS)
if VISIBILITY
tesseract_CPPFLAGS += -DTESS_IMPORTS
endif

tesseract_LDFLAGS = $(OPENCL_LDFLAGS)
tesseract_LDFLAGS += $(OPENMP_CXXFLAGS)

tesseract_LDADD = libtesseract.la
tesseract_LDADD += $(LEPTONICA_LIBS)
tesseract_LDADD += $(TENSORFLOW_LIBS)
tesseract_LDADD += $(libarchive_LIBS)
tesseract_LDADD += $(libcurl_LIBS)

if T_WIN
tesseract_LDADD += -ltiff
tesseract_LDADD += -lws2_32
endif
if ADD_RT
tesseract_LDADD += -lrt
endif

# Rules for training tools.

if ENABLE_TRAINING

training: $(trainingtools) | $(PROGRAMS)

training-install: $(trainingtools)
	mkdir -p $(DESTDIR)$(bindir)
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install \
	$(INSTALL) $(INSTALL_STRIP_FLAG) $(trainingtools) $(DESTDIR)$(bindir)

training-uninstall:

# Some unit tests use code from training.
check: libtesseract_training.la

# dawg_test runs dawg2wordlist and wordlist2dawg.
check: dawg2wordlist wordlist2dawg

else

training:
	@echo "Need to reconfigure project, so there are no errors"

endif

CLEANFILES += $(EXTRA_PROGRAMS)

training_CPPFLAGS := -DPANGO_ENABLE_ENGINE
training_CPPFLAGS += -DTESS_COMMON_TRAINING_API=
training_CPPFLAGS += -DTESS_PANGO_TRAINING_API=
training_CPPFLAGS += -DTESS_UNICHARSET_TRAINING_API=
training_CPPFLAGS += -I$(top_builddir)/include
training_CPPFLAGS += -I$(top_srcdir)/include
training_CPPFLAGS += -I$(top_srcdir)/src/training
training_CPPFLAGS += -I$(top_srcdir)/src/training/common
training_CPPFLAGS += -I$(top_srcdir)/src/training/pango
training_CPPFLAGS += -I$(top_srcdir)/src/training/unicharset
training_CPPFLAGS += -I$(top_srcdir)/src/api
training_CPPFLAGS += -I$(top_srcdir)/src/ccmain
training_CPPFLAGS += -I$(top_srcdir)/src/ccutil
training_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
training_CPPFLAGS += -I$(top_srcdir)/src/lstm
training_CPPFLAGS += -I$(top_srcdir)/src/arch
training_CPPFLAGS += -I$(top_srcdir)/src/viewer
training_CPPFLAGS += -I$(top_srcdir)/src/textord
training_CPPFLAGS += -I$(top_srcdir)/src/dict
training_CPPFLAGS += -I$(top_srcdir)/src/classify
training_CPPFLAGS += -I$(top_srcdir)/src/wordrec
training_CPPFLAGS += -I$(top_srcdir)/src/cutil
training_CPPFLAGS += $(ICU_UC_CFLAGS) $(ICU_I18N_CFLAGS)
training_CPPFLAGS += $(pango_CFLAGS)
training_CPPFLAGS += $(cairo_CFLAGS)

if DISABLED_LEGACY_ENGINE
training_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

# TODO: training programs cannot be linked to shared library created
# with -fvisibility
if VISIBILITY
AM_LDFLAGS += -all-static
endif

noinst_HEADERS += src/training/pango/boxchar.h
noinst_HEADERS += src/training/common/commandlineflags.h
noinst_HEADERS += src/training/common/commontraining.h
noinst_HEADERS += src/training/common/ctc.h
noinst_HEADERS += src/training/common/networkbuilder.h
noinst_HEADERS += src/training/degradeimage.h
noinst_HEADERS += src/training/pango/ligature_table.h
noinst_HEADERS += src/training/pango/pango_font_info.h
noinst_HEADERS += src/training/pango/stringrenderer.h
noinst_HEADERS += src/training/pango/tlog.h
noinst_HEADERS += src/training/unicharset/icuerrorcode.h
noinst_HEADERS += src/training/unicharset/fileio.h
noinst_HEADERS += src/training/unicharset/lang_model_helpers.h
noinst_HEADERS += src/training/unicharset/lstmtester.h
noinst_HEADERS += src/training/unicharset/lstmtrainer.h
noinst_HEADERS += src/training/unicharset/normstrngs.h
noinst_HEADERS += src/training/unicharset/unicharset_training_utils.h
noinst_HEADERS += src/training/unicharset/validate_grapheme.h
noinst_HEADERS += src/training/unicharset/validate_indic.h
noinst_HEADERS += src/training/unicharset/validate_javanese.h
noinst_HEADERS += src/training/unicharset/validate_khmer.h
noinst_HEADERS += src/training/unicharset/validate_myanmar.h
noinst_HEADERS += src/training/unicharset/validator.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/training/common/errorcounter.h
noinst_HEADERS += src/training/common/intfeaturedist.h
noinst_HEADERS += src/training/common/intfeaturemap.h
noinst_HEADERS += src/training/common/mastertrainer.h
noinst_HEADERS += src/training/common/sampleiterator.h
noinst_HEADERS += src/training/common/trainingsampleset.h
noinst_HEADERS += src/training/mergenf.h
endif

CLEANFILES += libtesseract_training.la

EXTRA_LTLIBRARIES = libtesseract_training.la

libtesseract_training_la_CPPFLAGS = $(training_CPPFLAGS)
libtesseract_training_la_SOURCES = src/training/pango/boxchar.cpp
libtesseract_training_la_SOURCES += src/training/common/commandlineflags.cpp
libtesseract_training_la_SOURCES += src/training/common/commontraining.cpp
libtesseract_training_la_SOURCES += src/training/common/ctc.cpp
libtesseract_training_la_SOURCES += src/training/common/networkbuilder.cpp
libtesseract_training_la_SOURCES += src/training/degradeimage.cpp
libtesseract_training_la_SOURCES += src/training/pango/ligature_table.cpp
libtesseract_training_la_SOURCES += src/training/pango/pango_font_info.cpp
libtesseract_training_la_SOURCES += src/training/pango/stringrenderer.cpp
libtesseract_training_la_SOURCES += src/training/pango/tlog.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/icuerrorcode.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/fileio.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/lang_model_helpers.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/lstmtester.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/lstmtrainer.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/normstrngs.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/unicharset_training_utils.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validate_grapheme.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validate_indic.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validate_javanese.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validate_khmer.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validate_myanmar.cpp
libtesseract_training_la_SOURCES += src/training/unicharset/validator.cpp
if !DISABLED_LEGACY_ENGINE
libtesseract_training_la_SOURCES += src/training/common/errorcounter.cpp
libtesseract_training_la_SOURCES += src/training/common/intfeaturedist.cpp
libtesseract_training_la_SOURCES += src/training/common/intfeaturemap.cpp
libtesseract_training_la_SOURCES += src/training/common/mastertrainer.cpp
libtesseract_training_la_SOURCES += src/training/common/sampleiterator.cpp
libtesseract_training_la_SOURCES += src/training/common/trainingsampleset.cpp
endif

trainingtools = combine_lang_model$(EXEEXT)
trainingtools += combine_tessdata$(EXEEXT)
trainingtools += dawg2wordlist$(EXEEXT)
trainingtools += lstmeval$(EXEEXT)
trainingtools += lstmtraining$(EXEEXT)
trainingtools += merge_unicharsets$(EXEEXT)
trainingtools += set_unicharset_properties$(EXEEXT)
trainingtools += text2image$(EXEEXT)
trainingtools += unicharset_extractor$(EXEEXT)
trainingtools += wordlist2dawg$(EXEEXT)
if !DISABLED_LEGACY_ENGINE
trainingtools += ambiguous_words$(EXEEXT)
trainingtools += classifier_tester$(EXEEXT)
trainingtools += cntraining$(EXEEXT)
trainingtools += mftraining$(EXEEXT)
trainingtools += shapeclustering$(EXEEXT)
endif

$(trainingtools): libtesseract.la

EXTRA_PROGRAMS += $(trainingtools)

extralib = libtesseract.la
extralib += $(libarchive_LIBS)
extralib += $(LEPTONICA_LIBS)
extralib += $(OPENCL_LDFLAGS)
extralib += $(TENSORFLOW_LIBS)
if T_WIN
extralib += -lws2_32
endif

if !DISABLED_LEGACY_ENGINE
ambiguous_words_CPPFLAGS = $(training_CPPFLAGS)
ambiguous_words_SOURCES = src/training/ambiguous_words.cpp
ambiguous_words_LDADD = libtesseract_training.la
ambiguous_words_LDADD += $(extralib)

classifier_tester_CPPFLAGS = $(training_CPPFLAGS)
classifier_tester_SOURCES = src/training/classifier_tester.cpp
classifier_tester_LDADD = libtesseract_training.la
classifier_tester_LDADD += $(extralib)

cntraining_CPPFLAGS = $(training_CPPFLAGS)
cntraining_SOURCES = src/training/cntraining.cpp
cntraining_LDADD = libtesseract_training.la
cntraining_LDADD += $(extralib)

mftraining_CPPFLAGS = $(training_CPPFLAGS)
mftraining_SOURCES = src/training/mftraining.cpp src/training/mergenf.cpp
mftraining_LDADD = libtesseract_training.la
mftraining_LDADD += $(ICU_UC_LIBS)
mftraining_LDADD += $(extralib)

shapeclustering_CPPFLAGS = $(training_CPPFLAGS)
shapeclustering_SOURCES = src/training/shapeclustering.cpp
shapeclustering_LDADD = libtesseract_training.la
shapeclustering_LDADD += $(extralib)
endif

combine_lang_model_CPPFLAGS = $(training_CPPFLAGS)
combine_lang_model_SOURCES = src/training/combine_lang_model.cpp
combine_lang_model_LDADD = libtesseract_training.la
combine_lang_model_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
combine_lang_model_LDADD += $(extralib)

combine_tessdata_CPPFLAGS = $(training_CPPFLAGS)
combine_tessdata_SOURCES = src/training/combine_tessdata.cpp
combine_tessdata_LDADD = $(extralib)

dawg2wordlist_CPPFLAGS = $(training_CPPFLAGS)
dawg2wordlist_SOURCES = src/training/dawg2wordlist.cpp
dawg2wordlist_LDADD = $(extralib)

lstmeval_CPPFLAGS = $(training_CPPFLAGS)
lstmeval_SOURCES = src/training/lstmeval.cpp
lstmeval_LDADD = libtesseract_training.la
lstmeval_LDADD += $(ICU_UC_LIBS)
lstmeval_LDADD += $(extralib)

lstmtraining_CPPFLAGS = $(training_CPPFLAGS)
lstmtraining_SOURCES = src/training/lstmtraining.cpp
lstmtraining_LDADD = libtesseract_training.la
lstmtraining_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
lstmtraining_LDADD += $(extralib)

merge_unicharsets_CPPFLAGS = $(training_CPPFLAGS)
merge_unicharsets_SOURCES = src/training/merge_unicharsets.cpp
merge_unicharsets_LDADD = $(extralib)

set_unicharset_properties_CPPFLAGS = $(training_CPPFLAGS)
set_unicharset_properties_SOURCES = src/training/set_unicharset_properties.cpp
set_unicharset_properties_LDADD = libtesseract_training.la
set_unicharset_properties_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
set_unicharset_properties_LDADD += $(extralib)

text2image_CPPFLAGS = $(training_CPPFLAGS)
text2image_SOURCES = src/training/text2image.cpp
text2image_LDADD = libtesseract_training.la
text2image_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
text2image_LDADD += $(extralib)
text2image_LDADD += $(ICU_UC_LIBS) $(cairo_LIBS)
text2image_LDADD += $(pango_LIBS) $(pangocairo_LIBS) $(pangoft2_LIBS)

unicharset_extractor_CPPFLAGS = $(training_CPPFLAGS)
unicharset_extractor_SOURCES = src/training/unicharset_extractor.cpp
unicharset_extractor_LDADD = libtesseract_training.la
unicharset_extractor_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
unicharset_extractor_LDADD += $(extralib)

wordlist2dawg_CPPFLAGS = $(training_CPPFLAGS)
wordlist2dawg_SOURCES = src/training/wordlist2dawg.cpp
wordlist2dawg_LDADD = $(extralib)

# fuzzer-api is used for fuzzing tests.
# They are run by OSS-Fuzz https://oss-fuzz.com/, but can also be run locally.
# Note: -fsanitize=fuzzer currently requires the clang++ compiler.

# LIB_FUZZING_ENGINE can be overridden by the caller.
# This is used by OSS-Fuzz.
LIB_FUZZING_ENGINE ?= -fsanitize=fuzzer

fuzzer-api: libtesseract.la
fuzzer-api: unittest/fuzzers/fuzzer-api.cpp
	$(CXX) $(CXXFLAGS) -g $(LIB_FUZZING_ENGINE) \
          -I $(top_srcdir)/include \
          -I $(builddir)/include \
          -I $(top_srcdir)/src/ccmain \
          -I $(top_srcdir)/src/ccstruct \
          -I $(top_srcdir)/src/ccutil \
          $(LEPTONICA_CFLAGS) \
          $(OPENMP_CXXFLAGS) \
          $< \
          $(builddir)/.libs/libtesseract.a \
          $(LEPTONICA_LIBS) \
          $(TENSORFLOW_LIBS) \
          $(libarchive_LIBS) \
          $(libcurl_LIBS) \
          -o $@

fuzzer-api-512x256: libtesseract.la
fuzzer-api-512x256: unittest/fuzzers/fuzzer-api.cpp
	$(CXX) $(CXXFLAGS) -g $(LIB_FUZZING_ENGINE) \
          -DTESSERACT_FUZZER_WIDTH=512 \
          -DTESSERACT_FUZZER_HEIGHT=256 \
          -I $(top_srcdir)/include \
          -I $(builddir)/include \
          -I $(top_srcdir)/src/ccmain \
          -I $(top_srcdir)/src/ccstruct \
          -I $(top_srcdir)/src/ccutil \
          $(LEPTONICA_CFLAGS) \
          $(OPENMP_CXXFLAGS) \
          $< \
          $(builddir)/.libs/libtesseract.a \
          $(LEPTONICA_LIBS) \
          $(TENSORFLOW_LIBS) \
          $(libarchive_LIBS) \
          $(libcurl_LIBS) \
          -o $@

CLEANFILES += fuzzer-api fuzzer-api-512x256

if ASCIIDOC

man_MANS = doc/combine_lang_model.1
man_MANS += doc/combine_tessdata.1
man_MANS += doc/dawg2wordlist.1
man_MANS += doc/lstmeval.1
man_MANS += doc/lstmtraining.1
man_MANS += doc/merge_unicharsets.1
man_MANS += doc/set_unicharset_properties.1
man_MANS += doc/tesseract.1
man_MANS += doc/text2image.1
man_MANS += doc/unicharset.5
man_MANS += doc/unicharset_extractor.1
man_MANS += doc/wordlist2dawg.1

if !DISABLED_LEGACY_ENGINE
man_MANS += doc/ambiguous_words.1
man_MANS += doc/classifier_tester.1
man_MANS += doc/cntraining.1
man_MANS += doc/mftraining.1
man_MANS += doc/shapeclustering.1
man_MANS += doc/unicharambigs.5
endif

man_xslt = http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

EXTRA_DIST += $(man_MANS) doc/Doxyfile

html: ${man_MANS:%=%.html}
pdf: ${man_MANS:%=%.pdf}

SUFFIXES = .asc .html .pdf

.asc:
if HAVE_XML_CATALOG_FILES
	asciidoc -b docbook -d manpage -o - $< | \
	XML_CATALOG_FILES=$(XML_CATALOG_FILES) xsltproc --nonet -o $@ $(man_xslt) -
else
	asciidoc -b docbook -d manpage -o - $< | \
	xsltproc --nonet -o $@ $(man_xslt) -
endif

.asc.html:
	asciidoc -b html5 -o $@ $<

.asc.pdf:
	asciidoc -b docbook -d manpage -o $*.dbk $<
	docbook2pdf -o doc $*.dbk

MAINTAINERCLEANFILES = $(man_MANS) Doxyfile

endif

# Absolute path of directory 'langdata'.
LANGDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/langdata_lstm

# Absolute path of directory 'tessdata' with traineddata files
# (must be on same level as top source directory).
TESSDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/tessdata

# Absolute path of directory 'testing' with test images and ground truth texts
# (using submodule test).
TESTING_DIR=$(shell cd $(top_srcdir) && pwd)/test/testing
# Absolute path of directory 'testdata' with test unicharset etc.
# (using submodule test).
TESTDATA_DIR=$(shell cd $(top_srcdir) && pwd)/test/testdata

# Suppress some memory leaks reported by LeakSanitizer.
export LSAN_OPTIONS=suppressions=$(top_srcdir)/unittest/tesseract_leaksanitizer.supp

unittest_CPPFLAGS =
unittest_CPPFLAGS += -DTESSBIN_DIR="\"$(abs_top_builddir)\""
unittest_CPPFLAGS += -DLANGDATA_DIR="\"$(LANGDATA_DIR)\""
unittest_CPPFLAGS += -DTESSDATA_DIR="\"$(TESSDATA_DIR)\""
unittest_CPPFLAGS += -DTESTING_DIR="\"$(TESTING_DIR)\""
unittest_CPPFLAGS += -DTESTDATA_DIR="\"$(TESTDATA_DIR)\""
unittest_CPPFLAGS += -DPANGO_ENABLE_ENGINE
if DISABLED_LEGACY_ENGINE
unittest_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif # DISABLED_LEGACY_ENGINE
unittest_CPPFLAGS += -DTESS_COMMON_TRAINING_API=
unittest_CPPFLAGS += -DTESS_PANGO_TRAINING_API=
unittest_CPPFLAGS += -DTESS_UNICHARSET_TRAINING_API=
unittest_CPPFLAGS += -I$(top_builddir)/include
unittest_CPPFLAGS += -I$(top_srcdir)/include
unittest_CPPFLAGS += -I$(top_srcdir)/src/arch
unittest_CPPFLAGS += -I$(top_srcdir)/src/ccmain
unittest_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
unittest_CPPFLAGS += -I$(top_srcdir)/src/ccutil
unittest_CPPFLAGS += -I$(top_srcdir)/src/classify
unittest_CPPFLAGS += -I$(top_srcdir)/src/cutil
unittest_CPPFLAGS += -I$(top_srcdir)/src/dict
unittest_CPPFLAGS += -I$(top_srcdir)/src/display
unittest_CPPFLAGS += -I$(top_srcdir)/src/lstm
unittest_CPPFLAGS += -I$(top_srcdir)/src/textord
unittest_CPPFLAGS += -I$(top_srcdir)/unittest/base
unittest_CPPFLAGS += -I$(top_srcdir)/unittest/util
unittest_CPPFLAGS += $(LEPTONICA_CFLAGS)
if ENABLE_TRAINING
unittest_CPPFLAGS += -I$(top_srcdir)/src/training
unittest_CPPFLAGS += -I$(top_srcdir)/src/training/common
unittest_CPPFLAGS += -I$(top_srcdir)/src/training/pango
unittest_CPPFLAGS += -I$(top_srcdir)/src/training/unicharset
unittest_CPPFLAGS += $(pangocairo_CFLAGS)
endif # ENABLE_TRAINING
unittest_CPPFLAGS += -I$(top_srcdir)/src/viewer
unittest_CPPFLAGS += -I$(top_srcdir)/src/wordrec
unittest_CPPFLAGS += -I$(top_srcdir)/unittest
if TENSORFLOW
unittest_CPPFLAGS += -DINCLUDE_TENSORFLOW
unittest_CPPFLAGS += -I/usr/include/tensorflow
endif # TENSORFLOW

# Build googletest:
check_LTLIBRARIES = libgtest.la libgtest_main.la libgmock.la libgmock_main.la
libgtest_la_SOURCES = unittest/third_party/googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/unittest/third_party/googletest/googletest/include
libgtest_la_CPPFLAGS += -I$(top_srcdir)/unittest/third_party/googletest/googletest
libgtest_la_CPPFLAGS += -pthread
libgtest_main_la_SOURCES = unittest/third_party/googletest/googletest/src/gtest_main.cc
libgtest_main_la_CPPFLAGS = $(libgtest_la_CPPFLAGS)

GMOCK_INCLUDES = -I$(top_srcdir)/unittest/third_party/googletest/googlemock/include \
                 -I$(top_srcdir)/unittest/third_party/googletest/googlemock \
                 -I$(top_srcdir)/unittest/third_party/googletest/googletest/include \
                 -I$(top_srcdir)/unittest/third_party/googletest/googletest

libgmock_la_SOURCES = unittest/third_party/googletest/googlemock/src/gmock-all.cc
libgmock_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                       -pthread
libgmock_main_la_SOURCES = unittest/third_party/googletest/googlemock/src/gmock_main.cc
libgmock_main_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                            -pthread

# Build unittests
GTEST_LIBS =  libgtest.la libgtest_main.la -lpthread
GMOCK_LIBS =  libgmock.la libgmock_main.la
TESS_LIBS = $(GTEST_LIBS)
TESS_LIBS += libtesseract.la $(libarchive_LIBS)
TESS_LIBS += $(TENSORFLOW_LIBS)
TRAINING_LIBS = libtesseract_training.la
TRAINING_LIBS += $(TESS_LIBS)
unittest_CPPFLAGS += -isystem $(top_srcdir)/unittest/third_party/googletest/googletest/include
unittest_CPPFLAGS += -isystem $(top_srcdir)/unittest/third_party/googletest/googlemock/include

check_PROGRAMS = apiexample_test
if ENABLE_TRAINING
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += applybox_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += baseapi_test
check_PROGRAMS += baseapi_thread_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += bitvector_test
endif # !DISABLED_LEGACY_ENGINE
endif # ENABLE_TRAINING
check_PROGRAMS += cleanapi_test
check_PROGRAMS += colpartition_test
if ENABLE_TRAINING
check_PROGRAMS += commandlineflags_test
check_PROGRAMS += dawg_test
endif # ENABLE_TRAINING
check_PROGRAMS += denorm_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += equationdetect_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += fileio_test
check_PROGRAMS += heap_test
check_PROGRAMS += imagedata_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += indexmapbidi_test
check_PROGRAMS += intfeaturemap_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += intsimdmatrix_test
check_PROGRAMS += lang_model_test
check_PROGRAMS += layout_test
check_PROGRAMS += ligature_table_test
check_PROGRAMS += linlsq_test
check_PROGRAMS += list_test
if ENABLE_TRAINING
check_PROGRAMS += lstm_recode_test
check_PROGRAMS += lstm_squashed_test
check_PROGRAMS += lstm_test
check_PROGRAMS += lstmtrainer_test
endif # ENABLE_TRAINING
check_PROGRAMS += loadlang_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += mastertrainer_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += matrix_test
check_PROGRAMS += networkio_test
if ENABLE_TRAINING
check_PROGRAMS += normstrngs_test
endif # ENABLE_TRAINING
check_PROGRAMS += nthitem_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += osd_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += pagesegmode_test
if ENABLE_TRAINING
check_PROGRAMS += pango_font_info_test
endif # ENABLE_TRAINING
check_PROGRAMS += paragraphs_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += params_model_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += progress_test
check_PROGRAMS += qrsequence_test
check_PROGRAMS += recodebeam_test
check_PROGRAMS += rect_test
check_PROGRAMS += resultiterator_test
check_PROGRAMS += scanutils_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += shapetable_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += stats_test
check_PROGRAMS += stridemap_test
check_PROGRAMS += stringrenderer_test
check_PROGRAMS += tablefind_test
check_PROGRAMS += tablerecog_test
check_PROGRAMS += tabvector_test
check_PROGRAMS += tatweel_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += textlineprojection_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += tfile_test
if ENABLE_TRAINING
check_PROGRAMS += unichar_test
check_PROGRAMS += unicharcompress_test
check_PROGRAMS += unicharset_test
check_PROGRAMS += validate_grapheme_test
check_PROGRAMS += validate_indic_test
check_PROGRAMS += validate_khmer_test
check_PROGRAMS += validate_myanmar_test
check_PROGRAMS += validator_test
endif # ENABLE_TRAINING

check_PROGRAMS: libtesseract.la libtesseract_training.la

TESTS = $(check_PROGRAMS)

# List of source files needed to build the executable:

apiexample_test_SOURCES = unittest/apiexample_test.cc
apiexample_test_CPPFLAGS = $(unittest_CPPFLAGS)
apiexample_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
apiexample_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

if !DISABLED_LEGACY_ENGINE
applybox_test_SOURCES = unittest/applybox_test.cc
applybox_test_CPPFLAGS = $(unittest_CPPFLAGS)
applybox_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)
endif # !DISABLED_LEGACY_ENGINE

baseapi_test_SOURCES = unittest/baseapi_test.cc
baseapi_test_CPPFLAGS = $(unittest_CPPFLAGS)
baseapi_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

baseapi_thread_test_SOURCES = unittest/baseapi_thread_test.cc
baseapi_thread_test_CPPFLAGS = $(unittest_CPPFLAGS)
baseapi_thread_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

if !DISABLED_LEGACY_ENGINE
bitvector_test_SOURCES = unittest/bitvector_test.cc
bitvector_test_CPPFLAGS = $(unittest_CPPFLAGS)
bitvector_test_LDADD = $(TRAINING_LIBS)
endif # !DISABLED_LEGACY_ENGINE

cleanapi_test_SOURCES = unittest/cleanapi_test.cc
cleanapi_test_CPPFLAGS = $(unittest_CPPFLAGS)
cleanapi_test_LDADD = $(TESS_LIBS)

colpartition_test_SOURCES = unittest/colpartition_test.cc
colpartition_test_CPPFLAGS = $(unittest_CPPFLAGS)
colpartition_test_LDADD = $(TESS_LIBS)

commandlineflags_test_SOURCES = unittest/commandlineflags_test.cc
commandlineflags_test_CPPFLAGS = $(unittest_CPPFLAGS)
commandlineflags_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

dawg_test_SOURCES = unittest/dawg_test.cc
dawg_test_CPPFLAGS = $(unittest_CPPFLAGS)
dawg_test_LDADD = $(TRAINING_LIBS)

denorm_test_SOURCES = unittest/denorm_test.cc
denorm_test_CPPFLAGS = $(unittest_CPPFLAGS)
denorm_test_LDADD = $(TESS_LIBS)

if !DISABLED_LEGACY_ENGINE
equationdetect_test_SOURCES = unittest/equationdetect_test.cc
equationdetect_test_CPPFLAGS = $(unittest_CPPFLAGS)
equationdetect_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)
endif # !DISABLED_LEGACY_ENGINE

fileio_test_SOURCES = unittest/fileio_test.cc
fileio_test_CPPFLAGS = $(unittest_CPPFLAGS)
fileio_test_LDADD = $(TRAINING_LIBS)

heap_test_SOURCES = unittest/heap_test.cc
heap_test_CPPFLAGS = $(unittest_CPPFLAGS)
heap_test_LDADD = $(TESS_LIBS)

imagedata_test_SOURCES = unittest/imagedata_test.cc
imagedata_test_CPPFLAGS = $(unittest_CPPFLAGS)
imagedata_test_LDADD = $(TRAINING_LIBS)

if !DISABLED_LEGACY_ENGINE
indexmapbidi_test_SOURCES = unittest/indexmapbidi_test.cc
indexmapbidi_test_CPPFLAGS = $(unittest_CPPFLAGS)
indexmapbidi_test_LDADD = $(TRAINING_LIBS)
endif # !DISABLED_LEGACY_ENGINE

if !DISABLED_LEGACY_ENGINE
intfeaturemap_test_SOURCES = unittest/intfeaturemap_test.cc
intfeaturemap_test_CPPFLAGS = $(unittest_CPPFLAGS)
intfeaturemap_test_LDADD = $(TRAINING_LIBS)
endif # !DISABLED_LEGACY_ENGINE

intsimdmatrix_test_SOURCES = unittest/intsimdmatrix_test.cc
intsimdmatrix_test_CPPFLAGS = $(unittest_CPPFLAGS)
if HAVE_AVX2
intsimdmatrix_test_CPPFLAGS += -DHAVE_AVX2
endif
if HAVE_SSE4_1
intsimdmatrix_test_CPPFLAGS += -DHAVE_SSE4_1
endif
intsimdmatrix_test_LDADD = $(TESS_LIBS)

lang_model_test_SOURCES = unittest/lang_model_test.cc
lang_model_test_CPPFLAGS = $(unittest_CPPFLAGS)
lang_model_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

layout_test_SOURCES = unittest/layout_test.cc
layout_test_CPPFLAGS = $(unittest_CPPFLAGS)
layout_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

ligature_table_test_SOURCES = unittest/ligature_table_test.cc
ligature_table_test_CPPFLAGS = $(unittest_CPPFLAGS)
ligature_table_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)
ligature_table_test_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
ligature_table_test_LDADD += $(pangocairo_LIBS) $(pangoft2_LIBS)
ligature_table_test_LDADD += $(cairo_LIBS) $(pango_LIBS)

linlsq_test_SOURCES = unittest/linlsq_test.cc
linlsq_test_CPPFLAGS = $(unittest_CPPFLAGS)
linlsq_test_LDADD = $(TESS_LIBS)

list_test_SOURCES = unittest/list_test.cc
list_test_CPPFLAGS = $(unittest_CPPFLAGS)
list_test_LDADD = $(TESS_LIBS)

loadlang_test_SOURCES = unittest/loadlang_test.cc
loadlang_test_CPPFLAGS = $(unittest_CPPFLAGS)
loadlang_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

lstm_recode_test_SOURCES = unittest/lstm_recode_test.cc
lstm_recode_test_CPPFLAGS = $(unittest_CPPFLAGS)
lstm_recode_test_LDADD = $(TRAINING_LIBS)

lstm_squashed_test_SOURCES = unittest/lstm_squashed_test.cc
lstm_squashed_test_CPPFLAGS = $(unittest_CPPFLAGS)
lstm_squashed_test_LDADD = $(TRAINING_LIBS)

lstm_test_SOURCES = unittest/lstm_test.cc
lstm_test_CPPFLAGS = $(unittest_CPPFLAGS)
lstm_test_LDADD = $(TRAINING_LIBS)

lstmtrainer_test_SOURCES = unittest/lstmtrainer_test.cc
lstmtrainer_test_CPPFLAGS = $(unittest_CPPFLAGS)
lstmtrainer_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

if !DISABLED_LEGACY_ENGINE
mastertrainer_test_SOURCES = unittest/mastertrainer_test.cc
mastertrainer_test_CPPFLAGS = $(unittest_CPPFLAGS)
mastertrainer_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)
endif # !DISABLED_LEGACY_ENGINE

matrix_test_SOURCES = unittest/matrix_test.cc
matrix_test_CPPFLAGS = $(unittest_CPPFLAGS)
matrix_test_LDADD = $(TESS_LIBS)

networkio_test_SOURCES = unittest/networkio_test.cc
networkio_test_CPPFLAGS = $(unittest_CPPFLAGS)
networkio_test_LDADD = $(TESS_LIBS)

normstrngs_test_SOURCES = unittest/normstrngs_test.cc
if TENSORFLOW
normstrngs_test_SOURCES += unittest/third_party/utf/rune.c
normstrngs_test_SOURCES += unittest/util/utf8/unilib.cc
endif # TENSORFLOW
normstrngs_test_CPPFLAGS = $(unittest_CPPFLAGS)
normstrngs_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

nthitem_test_SOURCES = unittest/nthitem_test.cc
nthitem_test_CPPFLAGS = $(unittest_CPPFLAGS)
nthitem_test_LDADD = $(TESS_LIBS)

if !DISABLED_LEGACY_ENGINE
osd_test_SOURCES = unittest/osd_test.cc
osd_test_CPPFLAGS = $(unittest_CPPFLAGS)
osd_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)
endif # !DISABLED_LEGACY_ENGINE

pagesegmode_test_SOURCES = unittest/pagesegmode_test.cc
pagesegmode_test_CPPFLAGS = $(unittest_CPPFLAGS)
pagesegmode_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

pango_font_info_test_SOURCES = unittest/pango_font_info_test.cc
if TENSORFLOW
pango_font_info_test_SOURCES += unittest/third_party/utf/rune.c
pango_font_info_test_SOURCES += unittest/util/utf8/unicodetext.cc
pango_font_info_test_SOURCES += unittest/util/utf8/unilib.cc
endif # TENSORFLOW
pango_font_info_test_CPPFLAGS = $(unittest_CPPFLAGS)
pango_font_info_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)
pango_font_info_test_LDADD += $(ICU_I18N_LIBS)
pango_font_info_test_LDADD += $(pangocairo_LIBS)
pango_font_info_test_LDADD += $(pangoft2_LIBS)

paragraphs_test_SOURCES = unittest/paragraphs_test.cc
paragraphs_test_CPPFLAGS = $(unittest_CPPFLAGS)
paragraphs_test_LDADD = $(TESS_LIBS)

if !DISABLED_LEGACY_ENGINE
params_model_test_SOURCES = unittest/params_model_test.cc
params_model_test_CPPFLAGS = $(unittest_CPPFLAGS)
params_model_test_LDADD = $(TRAINING_LIBS)
endif # !DISABLED_LEGACY_ENGINE

progress_test_SOURCES = unittest/progress_test.cc
progress_test_CPPFLAGS = $(unittest_CPPFLAGS)
progress_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
progress_test_LDADD = $(GTEST_LIBS) $(GMOCK_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

qrsequence_test_SOURCES = unittest/qrsequence_test.cc
qrsequence_test_CPPFLAGS = $(unittest_CPPFLAGS)
qrsequence_test_LDADD = $(TESS_LIBS)

recodebeam_test_SOURCES = unittest/recodebeam_test.cc
recodebeam_test_CPPFLAGS = $(unittest_CPPFLAGS)
recodebeam_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

rect_test_SOURCES = unittest/rect_test.cc
rect_test_CPPFLAGS = $(unittest_CPPFLAGS)
rect_test_LDADD = $(TESS_LIBS)

resultiterator_test_SOURCES = unittest/resultiterator_test.cc
resultiterator_test_CPPFLAGS = $(unittest_CPPFLAGS)
resultiterator_test_LDADD = $(TRAINING_LIBS)
resultiterator_test_LDADD += $(LEPTONICA_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

scanutils_test_SOURCES = unittest/scanutils_test.cc
scanutils_test_CPPFLAGS = $(unittest_CPPFLAGS)
scanutils_test_LDADD = $(TRAINING_LIBS)

if !DISABLED_LEGACY_ENGINE
shapetable_test_SOURCES = unittest/shapetable_test.cc
shapetable_test_CPPFLAGS = $(unittest_CPPFLAGS)
shapetable_test_LDADD = $(TRAINING_LIBS)
endif # !DISABLED_LEGACY_ENGINE

stats_test_SOURCES = unittest/stats_test.cc
stats_test_CPPFLAGS = $(unittest_CPPFLAGS)
stats_test_LDADD = $(TESS_LIBS)

stridemap_test_SOURCES = unittest/stridemap_test.cc
stridemap_test_CPPFLAGS = $(unittest_CPPFLAGS)
stridemap_test_LDADD = $(TESS_LIBS)

stringrenderer_test_SOURCES = unittest/stringrenderer_test.cc
stringrenderer_test_CPPFLAGS = $(unittest_CPPFLAGS)
stringrenderer_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)
stringrenderer_test_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
stringrenderer_test_LDADD += $(pangocairo_LIBS) $(pangoft2_LIBS)
stringrenderer_test_LDADD += $(cairo_LIBS) $(pango_LIBS)

tablefind_test_SOURCES = unittest/tablefind_test.cc
tablefind_test_CPPFLAGS = $(unittest_CPPFLAGS)
tablefind_test_LDADD = $(TESS_LIBS)

tablerecog_test_SOURCES = unittest/tablerecog_test.cc
tablerecog_test_CPPFLAGS = $(unittest_CPPFLAGS)
tablerecog_test_LDADD = $(TESS_LIBS)

tabvector_test_SOURCES = unittest/tabvector_test.cc
tabvector_test_CPPFLAGS = $(unittest_CPPFLAGS)
tabvector_test_LDADD = $(TESS_LIBS)

tatweel_test_SOURCES = unittest/tatweel_test.cc
tatweel_test_SOURCES += unittest/third_party/utf/rune.c
tatweel_test_SOURCES += unittest/util/utf8/unicodetext.cc
tatweel_test_SOURCES += unittest/util/utf8/unilib.cc
tatweel_test_CPPFLAGS = $(unittest_CPPFLAGS)
tatweel_test_LDADD = $(TRAINING_LIBS)

textlineprojection_test_SOURCES = unittest/textlineprojection_test.cc
textlineprojection_test_CPPFLAGS = $(unittest_CPPFLAGS)
textlineprojection_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

tfile_test_SOURCES = unittest/tfile_test.cc
tfile_test_CPPFLAGS = $(unittest_CPPFLAGS)
tfile_test_LDADD = $(TESS_LIBS)

unichar_test_SOURCES = unittest/unichar_test.cc
unichar_test_CPPFLAGS = $(unittest_CPPFLAGS)
unichar_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

unicharcompress_test_SOURCES = unittest/unicharcompress_test.cc
unicharcompress_test_CPPFLAGS = $(unittest_CPPFLAGS)
unicharcompress_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

unicharset_test_SOURCES = unittest/unicharset_test.cc
unicharset_test_CPPFLAGS = $(unittest_CPPFLAGS)
unicharset_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

validate_grapheme_test_SOURCES = unittest/validate_grapheme_test.cc
validate_grapheme_test_CPPFLAGS = $(unittest_CPPFLAGS)
validate_grapheme_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_indic_test_SOURCES = unittest/validate_indic_test.cc
validate_indic_test_CPPFLAGS = $(unittest_CPPFLAGS)
validate_indic_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_khmer_test_SOURCES = unittest/validate_khmer_test.cc
validate_khmer_test_CPPFLAGS = $(unittest_CPPFLAGS)
validate_khmer_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_myanmar_test_SOURCES = unittest/validate_myanmar_test.cc
validate_myanmar_test_CPPFLAGS = $(unittest_CPPFLAGS)
validate_myanmar_test_LDADD = $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validator_test_SOURCES = unittest/validator_test.cc
validator_test_CPPFLAGS = $(unittest_CPPFLAGS)
validator_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

# for windows
if T_WIN
apiexample_test_LDADD += -lws2_32
intsimdmatrix_test_LDADD += -lws2_32
matrix_test_LDADD += -lws2_32
if !DISABLED_LEGACY_ENGINE
osd_test_LDADD += -lws2_32
endif # !DISABLED_LEGACY_ENGINE
loadlang_test_LDADD += -lws2_32
endif

EXTRA_apiexample_test_DEPENDENCIES = $(abs_top_builddir)/test/testing/phototest.tif
EXTRA_apiexample_test_DEPENDENCIES += $(abs_top_builddir)/test/testing/phototest.txt

$(abs_top_builddir)/test/testing/phototest.tif:
	mkdir -p $(top_builddir)/test/testing
	ln -s $(TESTING_DIR)/phototest.tif $(top_builddir)/test/testing/phototest.tif

$(abs_top_builddir)/test/testing/phototest.txt:
	mkdir -p $(top_builddir)/test/testing
	ln -s $(TESTING_DIR)/phototest.txt $(top_builddir)/test/testing/phototest.txt

# Some tests require a local tmp directory.

$(check_PROGRAMS): | tmp

tmp:
	mkdir -p tmp

# Some tests require a well defined set of the following font files.

fonts = ae_Arab.ttf
fonts += Arial_Bold_Italic.ttf
fonts += DejaVuSans-ExtraLight.ttf
fonts += Lohit-Hindi.ttf
fonts += Times_New_Roman.ttf
fonts += UnBatang.ttf
fonts += Verdana.ttf

# These tests depend on installed model files and fonts:
#
# apiexample_test baseapi_test lang_model_test layout_test
# ligature_table_test loadlang_test lstm_recode_test lstm_squashed_test
# lstm_test lstmtrainer_test mastertrainer_test osd_test
# pagesegmode_test pango_font_info_test progress_test
# recodebeam_test resultiterator_test stringrenderer_test
# textlineprojection_test unicharcompress_test
#
# Instead of fine-tuned dependencies the following lines
# simply require those dependencies for all tests.
# That can be improved if necessary.

$(check_PROGRAMS): | $(LANGDATA_DIR)
$(check_PROGRAMS): | $(TESSDATA_DIR)
$(check_PROGRAMS): | $(TESSDATA_BEST_DIR)
$(check_PROGRAMS): | $(TESSDATA_FAST_DIR)
$(check_PROGRAMS): | $(fonts:%=$(TESTING_DIR)/%)

$(LANGDATA_DIR) $(TESSDATA_DIR) $(TESSDATA_BEST_DIR) $(TESSDATA_FAST_DIR):
	@echo "Some unit tests require $@."
	@echo "It can be installed manually by running this command:"
	@echo "  git clone https://github.com/tesseract-ocr/$$(basename $@).git $@"
	@exit 1

$(TESTING_DIR)/Arial_Bold_Italic.ttf:
	curl -sSL -o Arial.exe https://sourceforge.net/projects/corefonts/files/the%20fonts/final/arial32.exe/download
	cabextract -F Arialbi.TTF -q Arial.exe
	rm Arial.exe
	mv Arialbi.TTF $@

$(TESTING_DIR)/DejaVuSans-ExtraLight.ttf:
	curl -sSL http://sourceforge.net/projects/dejavu/files/dejavu/2.37/dejavu-fonts-ttf-2.37.tar.bz2 | \
	tar -xjO dejavu-fonts-ttf-2.37/ttf/DejaVuSans-ExtraLight.ttf >$@

$(TESTING_DIR)/Lohit-Hindi.ttf:
	curl -sSL https://releases.pagure.org/lohit/lohit-hindi-ttf-2.4.3.tar.gz | \
	tar -xzO lohit-hindi-ttf-2.4.3/Lohit-Hindi.ttf >$@

$(TESTING_DIR)/Times_New_Roman.ttf:
	curl -sSL -o Times.exe https://sourceforge.net/projects/corefonts/files/the%20fonts/final/times32.exe/download
	cabextract -F Times.TTF -q Times.exe
	rm Times.exe
	mv Times.TTF $@

$(TESTING_DIR)/UnBatang.ttf:
	curl -sSL -o $@ https://salsa.debian.org/fonts-team/fonts-unfonts-core/-/raw/master/UnBatang.ttf

$(TESTING_DIR)/Verdana.ttf:
	curl -sSL -o Verdana.exe https://sourceforge.net/projects/corefonts/files/the%20fonts/final/verdan32.exe/download
	cabextract -F Verdana.TTF -q Verdana.exe
	rm Verdana.exe
	mv Verdana.TTF $@

$(TESTING_DIR)/ae_Arab.ttf:
	curl -sSL -o $@ https://salsa.debian.org/fonts-team/fonts-arabeyes/-/raw/master/ae_Arab.ttf
