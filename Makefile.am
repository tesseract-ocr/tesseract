## run autogen.sh to create Makefile.in from this file

# Be less noisy by default. Can be overridden with `make V=1`.
V = 0

ACLOCAL_AMFLAGS = -I m4

.PHONY: doc install-langs ScrollView.jar install-jars training

SUBDIRS = . tessdata doc unittest

EXTRA_DIST = README.md LICENSE
EXTRA_DIST += aclocal.m4 config configure.ac autogen.sh
EXTRA_DIST += tesseract.pc.in java doc
EXTRA_DIST += CMakeLists.txt tesseract.pc.cmake cmake VERSION src/vs2010

DIST_SUBDIRS = $(SUBDIRS)

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

ScrollView.jar:
	@cd "$(top_builddir)/java" && $(MAKE) $@

install-jars:
	@cd "$(top_builddir)/java" && $(MAKE) $@

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
pkginclude_HEADERS += include/tesseract/thresholder.h
pkginclude_HEADERS += include/tesseract/unichar.h

# Rules for all subdirectories.

noinst_HEADERS =
noinst_LTLIBRARIES =

AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_builddir)/include
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS
AM_CPPFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
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
libtesseract_la_CPPFLAGS += -I$(top_srcdir)/src/opencl
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
libtesseract_la_LDFLAGS += -no-undefined -Wl,--as-needed -lws2_32
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
libtesseract_la_LIBADD += libtesseract_opencl.la

# Rules for src/arch.

noinst_HEADERS += src/arch/dotproduct.h
noinst_HEADERS += src/arch/intsimdmatrix.h
noinst_HEADERS += src/arch/simddetect.h

noinst_LTLIBRARIES += libtesseract_native.la

libtesseract_native_la_CXXFLAGS = -O3 -ffast-math
if MARCH_NATIVE_OPT
libtesseract_native_la_CXXFLAGS += -march=native -mtune=native
endif
libtesseract_native_la_SOURCES = src/arch/dotproduct.cpp

if HAVE_AVX
libtesseract_avx_la_CXXFLAGS = -mavx
libtesseract_avx_la_SOURCES = src/arch/dotproductavx.cpp
libtesseract_la_LIBADD += libtesseract_avx.la
noinst_LTLIBRARIES += libtesseract_avx.la
endif

if HAVE_AVX2
libtesseract_avx2_la_CXXFLAGS = -mavx2
libtesseract_avx2_la_SOURCES = src/arch/intsimdmatrixavx2.cpp
libtesseract_la_LIBADD += libtesseract_avx2.la
noinst_LTLIBRARIES += libtesseract_avx2.la
endif

if HAVE_FMA
libtesseract_fma_la_CXXFLAGS = -mfma
libtesseract_fma_la_SOURCES = src/arch/dotproductfma.cpp
libtesseract_la_LIBADD += libtesseract_fma.la
noinst_LTLIBRARIES += libtesseract_fma.la
endif

if HAVE_SSE4_1
libtesseract_sse_la_CXXFLAGS = -msse4.1
libtesseract_sse_la_SOURCES = src/arch/dotproductsse.cpp src/arch/intsimdmatrixsse.cpp
libtesseract_la_LIBADD += libtesseract_sse.la
noinst_LTLIBRARIES += libtesseract_sse.la
endif

if HAVE_NEON
libtesseract_neon_la_CXXFLAGS = $(NEON_CXXFLAGS)
libtesseract_neon_la_SOURCES = src/arch/intsimdmatrixneon.cpp
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
libtesseract_la_SOURCES += src/ccmain/paramsd.cpp
libtesseract_la_SOURCES += src/ccmain/pgedit.cpp
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
libtesseract_ccutil_la_CPPFLAGS += -DTESSDATA_PREFIX=@datadir@
endif

noinst_HEADERS += src/ccutil/bits16.h
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
noinst_HEADERS += src/ccutil/strngs.h
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
libtesseract_ccutil_la_SOURCES += src/ccutil/mainblk.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/serialis.cpp
libtesseract_ccutil_la_SOURCES += src/ccutil/strngs.cpp
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
libtesseract_ccutil_la_SOURCES += src/ccutil/universalambigs.cpp
endif

# Rules for src/classify.

noinst_HEADERS += src/classify/classify.h
if !DISABLED_LEGACY_ENGINE
noinst_HEADERS += src/classify/adaptive.h
noinst_HEADERS += src/classify/blobclass.h
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
libtesseract_la_SOURCES += src/classify/mfdefs.cpp
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
libtesseract_lstm_la_CPPFLAGS += -DTESSDATA_PREFIX=@datadir@
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

libtesseract_opencl_la_CPPFLAGS = $(AM_CPPFLAGS)
libtesseract_opencl_la_CPPFLAGS += $(OPENCL_CFLAGS)
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccutil
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
libtesseract_opencl_la_CPPFLAGS += -I$(top_srcdir)/src/ccmain

noinst_HEADERS += src/opencl/openclwrapper.h
noinst_HEADERS += src/opencl/oclkernels.h

noinst_LTLIBRARIES += libtesseract_opencl.la

libtesseract_opencl_la_SOURCES = src/opencl/openclwrapper.cpp

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

noinst_HEADERS += src/viewer/scrollview.h
noinst_HEADERS += src/viewer/svmnode.h
noinst_HEADERS += src/viewer/svutil.h

libtesseract_la_SOURCES += src/viewer/scrollview.cpp
libtesseract_la_SOURCES += src/viewer/svmnode.cpp
libtesseract_la_SOURCES += src/viewer/svutil.cpp

# TODO: Add rule to generate svpaint from svpaint.cpp.

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
noinst_HEADERS += src/wordrec/measure.h
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
libtesseract_la_SOURCES += src/wordrec/plotedges.cpp
libtesseract_la_SOURCES += src/wordrec/render.cpp
libtesseract_la_SOURCES += src/wordrec/segsearch.cpp
libtesseract_la_SOURCES += src/wordrec/wordclass.cpp
endif

# Rules for tesseract executable.

bin_PROGRAMS = tesseract
tesseract_SOURCES = src/api/tesseractmain.cpp
tesseract_CPPFLAGS =
tesseract_CPPFLAGS += -I$(top_srcdir)/src/arch
tesseract_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
tesseract_CPPFLAGS += -I$(top_srcdir)/src/ccutil
tesseract_CPPFLAGS += -I$(top_srcdir)/src/dict
tesseract_CPPFLAGS += -I$(top_srcdir)/src/viewer
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

training: $(EXTRA_PROGRAMS) | $(PROGRAMS)

training-install: $(EXTRA_PROGRAMS)
	mkdir -p $(DESTDIR)$(bindir)
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=install \
	$(INSTALL) $(INSTALL_STRIP_FLAG) $(EXTRA_PROGRAMS) $(DESTDIR)$(bindir)

training-uninstall:

# Some unit tests use code from training.
check: libtesseract_training.la

# dawg_test runs dawg2wordlist and wordlist2dawg.
check: dawg2wordlist wordlist2dawg

CLEANFILES = $(EXTRA_PROGRAMS)

else

training:
	@echo "Need to reconfigure project, so there are no errors"

endif

$(EXTRA_PROGRAMS): libtesseract.la

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

bin_SCRIPTS = src/training/language-specific.sh src/training/tesstrain.sh
scripts_DATA = src/training/tesstrain_utils.sh
scriptsdir = $(bindir)

if DISABLED_LEGACY_ENGINE
training_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

# TODO: training programs can not be linked to shared library created
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

trainingtools = combine_lang_model
trainingtools += combine_tessdata
trainingtools += dawg2wordlist
trainingtools += lstmeval
trainingtools += lstmtraining
trainingtools += merge_unicharsets
trainingtools += set_unicharset_properties
trainingtools += text2image
trainingtools += unicharset_extractor
trainingtools += wordlist2dawg
if !DISABLED_LEGACY_ENGINE
trainingtools += ambiguous_words
trainingtools += classifier_tester
trainingtools += cntraining
trainingtools += mftraining
trainingtools += shapeclustering
endif

EXTRA_PROGRAMS = $(trainingtools)

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
fuzzer-api: $(top_srcdir)/unittest/fuzzers/fuzzer-api.cpp
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
