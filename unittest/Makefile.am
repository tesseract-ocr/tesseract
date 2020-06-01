# Makefile for Tesseract unit tests

# Absolute path of directory 'langdata'.
LANGDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/langdata_lstm

# Absolute path of directory 'tessdata' with traineddata files
# (must be on same level as top source directory).
TESSDATA_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/tessdata
TESSDATA_BEST_DIR=$(shell cd $(top_srcdir) && cd .. && pwd)/tessdata_best

# Absolute path of directory 'testing' with test images and ground truth texts
# (using submodule test).
TESTING_DIR=$(shell cd $(top_srcdir) && pwd)/test/testing
# Absolute path of directory 'testdata' with test unicharset etc.
# (using submodule test).
TESTDATA_DIR=$(shell cd $(top_srcdir) && pwd)/test/testdata

AM_CPPFLAGS += -DTESSBIN_DIR="\"$(abs_top_builddir)\""
AM_CPPFLAGS += -DLANGDATA_DIR="\"$(LANGDATA_DIR)\""
AM_CPPFLAGS += -DTESSDATA_DIR="\"$(TESSDATA_DIR)\""
AM_CPPFLAGS += -DTESSDATA_BEST_DIR="\"$(TESSDATA_BEST_DIR)\""
AM_CPPFLAGS += -DTESTING_DIR="\"$(TESTING_DIR)\""
AM_CPPFLAGS += -DTESTDATA_DIR="\"$(TESTDATA_DIR)\""
AM_CPPFLAGS += -DPANGO_ENABLE_ENGINE
if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif # DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -I$(top_builddir)/include
AM_CPPFLAGS += -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/arch
AM_CPPFLAGS += -I$(top_srcdir)/src/ccmain
AM_CPPFLAGS += -I$(top_srcdir)/src/ccstruct
AM_CPPFLAGS += -I$(top_srcdir)/src/ccutil
AM_CPPFLAGS += -I$(top_srcdir)/src/classify
AM_CPPFLAGS += -I$(top_srcdir)/src/cutil
AM_CPPFLAGS += -I$(top_srcdir)/src/dict
AM_CPPFLAGS += -I$(top_srcdir)/src/display
AM_CPPFLAGS += -I$(top_srcdir)/src/lstm
AM_CPPFLAGS += -I$(top_srcdir)/src/textord
AM_CPPFLAGS += -I$(top_srcdir)/unittest/base
AM_CPPFLAGS += -I$(top_srcdir)/unittest/util
if ENABLE_TRAINING
AM_CPPFLAGS += -I$(top_srcdir)/src/training
AM_CPPFLAGS += $(pangocairo_CFLAGS)
endif # ENABLE_TRAINING
AM_CPPFLAGS += -I$(top_srcdir)/src/viewer
AM_CPPFLAGS += -I$(top_srcdir)/src/wordrec
AM_CPPFLAGS += -I$(top_srcdir)/abseil
if TENSORFLOW
AM_CPPFLAGS += -DINCLUDE_TENSORFLOW
AM_CPPFLAGS += -I/usr/include/tensorflow
endif # TENSORFLOW

# Build googletest:
check_LTLIBRARIES = libgtest.la libgtest_main.la libgmock.la libgmock_main.la
libgtest_la_SOURCES = ../googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include -I$(top_srcdir)/googletest/googletest -pthread
libgtest_main_la_SOURCES = ../googletest/googletest/src/gtest_main.cc
## libgtest_main_la_LIBADD = libgtest.la

# Build Abseil (needed for some unit tests).
check_LTLIBRARIES += libabseil.la
libabseil_la_SOURCES =
libabseil_la_SOURCES += ../abseil/absl/base/internal/cycleclock.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/raw_logging.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/spinlock.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/spinlock_wait.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/sysinfo.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/throw_delegate.cc
libabseil_la_SOURCES += ../abseil/absl/base/internal/unscaledcycleclock.cc
libabseil_la_SOURCES += ../abseil/absl/numeric/int128.cc
libabseil_la_SOURCES += ../abseil/absl/strings/ascii.cc
libabseil_la_SOURCES += ../abseil/absl/strings/charconv.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/charconv_bigint.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/charconv_parse.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/memutil.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/arg.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/bind.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/extension.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/float_conversion.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/output.cc
libabseil_la_SOURCES += ../abseil/absl/strings/internal/str_format/parser.cc
libabseil_la_SOURCES += ../abseil/absl/strings/match.cc
libabseil_la_SOURCES += ../abseil/absl/strings/numbers.cc
libabseil_la_SOURCES += ../abseil/absl/strings/str_cat.cc
libabseil_la_SOURCES += ../abseil/absl/strings/str_split.cc
libabseil_la_SOURCES += ../abseil/absl/strings/string_view.cc
libabseil_la_SOURCES += ../abseil/absl/time/clock.cc
libabseil_la_SOURCES += ../abseil/absl/time/duration.cc

GMOCK_INCLUDES = -I$(top_srcdir)/googletest/googlemock/include \
                 -I$(top_srcdir)/googletest/googlemock \
                 -I$(top_srcdir)/googletest/googletest/include \
                 -I$(top_srcdir)/googletest/googletest

libgmock_la_SOURCES = ../googletest/googlemock/src/gmock-all.cc
libgmock_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                       -pthread
libgmock_main_la_SOURCES = ../googletest/googlemock/src/gmock_main.cc
libgmock_main_la_CPPFLAGS = $(GMOCK_INCLUDES) \
                            -pthread

# Build unittests
ABSEIL_LIBS = libabseil.la
GTEST_LIBS =  libgtest.la libgtest_main.la -lpthread
GMOCK_LIBS =  libgmock.la libgmock_main.la
TESS_LIBS = $(GTEST_LIBS)
TESS_LIBS += $(top_builddir)/libtesseract.la $(libarchive_LIBS)
TESS_LIBS += $(TENSORFLOW_LIBS)
TRAINING_LIBS = $(top_builddir)/src/training/libtesseract_training.la
TRAINING_LIBS += $(top_builddir)/src/training/libtesseract_tessopt.la
TRAINING_LIBS += $(TESS_LIBS)
AM_CPPFLAGS +=   -isystem $(top_srcdir)/googletest/googletest/include \
                 -isystem $(top_srcdir)/googletest/googlemock/include

check_PROGRAMS = apiexample_test
if ENABLE_TRAINING
check_PROGRAMS += applybox_test
check_PROGRAMS += baseapi_test
if TENSORFLOW
check_PROGRAMS += baseapi_thread_test
endif # TENSORFLOW
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
if TENSORFLOW
check_PROGRAMS += equationdetect_test
endif # TENSORFLOW
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += fileio_test
check_PROGRAMS += heap_test
check_PROGRAMS += imagedata_test
if !DISABLED_LEGACY_ENGINE
check_PROGRAMS += indexmapbidi_test
endif # !DISABLED_LEGACY_ENGINE
check_PROGRAMS += intfeaturemap_test
check_PROGRAMS += intsimdmatrix_test
check_PROGRAMS += lang_model_test
check_PROGRAMS += layout_test
check_PROGRAMS += ligature_table_test
check_PROGRAMS += linlsq_test
if ENABLE_TRAINING
check_PROGRAMS += lstm_recode_test
check_PROGRAMS += lstm_squashed_test
check_PROGRAMS += lstm_test
check_PROGRAMS += lstmtrainer_test
endif # ENABLE_TRAINING
check_PROGRAMS += loadlang_test
check_PROGRAMS += mastertrainer_test
check_PROGRAMS += matrix_test
if TENSORFLOW
check_PROGRAMS += networkio_test
endif # TENSORFLOW
if ENABLE_TRAINING
check_PROGRAMS += normstrngs_test
endif # ENABLE_TRAINING
check_PROGRAMS += nthitem_test
check_PROGRAMS += osd_test
check_PROGRAMS += pagesegmode_test
if ENABLE_TRAINING
if TENSORFLOW
check_PROGRAMS += pango_font_info_test
endif # TENSORFLOW
endif # ENABLE_TRAINING
check_PROGRAMS += paragraphs_test
check_PROGRAMS += params_model_test
check_PROGRAMS += progress_test
check_PROGRAMS += qrsequence_test
check_PROGRAMS += recodebeam_test
check_PROGRAMS += rect_test
check_PROGRAMS += resultiterator_test
check_PROGRAMS += scanutils_test
check_PROGRAMS += shapetable_test
check_PROGRAMS += stats_test
if TENSORFLOW
check_PROGRAMS += stridemap_test
endif # TENSORFLOW
check_PROGRAMS += stringrenderer_test
check_PROGRAMS += tablefind_test
check_PROGRAMS += tablerecog_test
check_PROGRAMS += tabvector_test
if TENSORFLOW
check_PROGRAMS += tatweel_test
endif # TENSORFLOW
check_PROGRAMS += textlineprojection_test
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

TESTS = $(check_PROGRAMS)

.PHONY: all

all: tmp

tmp:
	mkdir -p tmp

# List of source files needed to build the executable:

apiexample_test_SOURCES = apiexample_test.cc
apiexample_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
apiexample_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

applybox_test_SOURCES = applybox_test.cc
applybox_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

baseapi_test_SOURCES = baseapi_test.cc
baseapi_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)

if TENSORFLOW
baseapi_thread_test_SOURCES = baseapi_thread_test.cc
baseapi_thread_test_LDADD = $(ABSEIL_LIBS)
baseapi_thread_test_LDADD += $(TESS_LIBS) $(LEPTONICA_LIBS)
endif # TENSORFLOW

bitvector_test_SOURCES = bitvector_test.cc
bitvector_test_LDADD = $(TRAINING_LIBS)

cleanapi_test_SOURCES = cleanapi_test.cc
cleanapi_test_LDADD = $(TESS_LIBS)

colpartition_test_SOURCES = colpartition_test.cc
colpartition_test_LDADD = $(TESS_LIBS)

commandlineflags_test_SOURCES = commandlineflags_test.cc
commandlineflags_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

dawg_test_SOURCES = dawg_test.cc
dawg_test_LDADD = $(TRAINING_LIBS)

denorm_test_SOURCES = denorm_test.cc
denorm_test_LDADD = $(TESS_LIBS)

if TENSORFLOW
equationdetect_test_SOURCES = equationdetect_test.cc
equationdetect_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)
endif # TENSORFLOW

fileio_test_SOURCES = fileio_test.cc
fileio_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

heap_test_SOURCES = heap_test.cc
heap_test_LDADD = $(TESS_LIBS)

imagedata_test_SOURCES = imagedata_test.cc
imagedata_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

indexmapbidi_test_SOURCES = indexmapbidi_test.cc
indexmapbidi_test_LDADD = $(TRAINING_LIBS)

intfeaturemap_test_SOURCES = intfeaturemap_test.cc
intfeaturemap_test_LDADD = $(TRAINING_LIBS)

intsimdmatrix_test_SOURCES = intsimdmatrix_test.cc
intsimdmatrix_test_LDADD = $(TESS_LIBS)
intsimdmatrix_test_CPPFLAGS = $(AM_CPPFLAGS)
if HAVE_AVX2
intsimdmatrix_test_CPPFLAGS += -DHAVE_AVX2
endif
if HAVE_SSE4_1
intsimdmatrix_test_CPPFLAGS += -DHAVE_SSE4_1
endif

lang_model_test_SOURCES = lang_model_test.cc
lang_model_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

layout_test_SOURCES = layout_test.cc
layout_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

ligature_table_test_SOURCES = ligature_table_test.cc
ligature_table_test_LDADD = $(TRAINING_LIBS)
ligature_table_test_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS) -lfontconfig
ligature_table_test_LDADD += -lpangocairo-1.0 -lpangoft2-1.0
ligature_table_test_LDADD += $(cairo_LIBS) $(pango_LIBS)

linlsq_test_SOURCES = linlsq_test.cc
linlsq_test_LDADD = $(TESS_LIBS)

loadlang_test_SOURCES = loadlang_test.cc
loadlang_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

lstm_recode_test_SOURCES = lstm_recode_test.cc
lstm_recode_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

lstm_squashed_test_SOURCES = lstm_squashed_test.cc
lstm_squashed_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

lstm_test_SOURCES = lstm_test.cc
lstm_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

lstmtrainer_test_SOURCES = lstmtrainer_test.cc
lstmtrainer_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)

mastertrainer_test_SOURCES = mastertrainer_test.cc
mastertrainer_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)

matrix_test_SOURCES = matrix_test.cc
matrix_test_LDADD = $(TESS_LIBS)

if TENSORFLOW
networkio_test_SOURCES = networkio_test.cc
networkio_test_LDADD = $(TESS_LIBS)
endif # TENSORFLOW

normstrngs_test_SOURCES = normstrngs_test.cc
if TENSORFLOW
normstrngs_test_SOURCES += third_party/utf/rune.c util/utf8/unilib.cc
endif # TENSORFLOW
normstrngs_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

nthitem_test_SOURCES = nthitem_test.cc
nthitem_test_LDADD = $(TESS_LIBS)

osd_test_SOURCES = osd_test.cc
osd_test_LDADD = $(TESS_LIBS) $(LEPTONICA_LIBS)

pagesegmode_test_SOURCES = pagesegmode_test.cc
pagesegmode_test_LDADD = $(TRAINING_LIBS) $(LEPTONICA_LIBS)

if TENSORFLOW
pango_font_info_test_SOURCES = pango_font_info_test.cc
pango_font_info_test_SOURCES += third_party/utf/rune.c
pango_font_info_test_SOURCES += util/utf8/unicodetext.cc util/utf8/unilib.cc
pango_font_info_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)
pango_font_info_test_LDADD += $(ICU_I18N_LIBS)
pango_font_info_test_LDADD += $(pangocairo_LIBS)
pango_font_info_test_LDADD += $(pangoft2_LIBS)
endif # TENSORFLOW

paragraphs_test_SOURCES = paragraphs_test.cc
paragraphs_test_LDADD = $(ABSEIL_LIBS) $(TESS_LIBS)

params_model_test_SOURCES = params_model_test.cc
params_model_test_LDADD = $(TRAINING_LIBS)

progress_test_SOURCES = progress_test.cc
progress_test_LDFLAGS = $(OPENCL_LDFLAGS) $(LEPTONICA_LIBS)
progress_test_LDADD = $(GTEST_LIBS) $(GMOCK_LIBS) $(TESS_LIBS) $(LEPTONICA_LIBS)

qrsequence_test_SOURCES = qrsequence_test.cc
qrsequence_test_LDADD = $(ABSEIL_LIBS) $(TESS_LIBS)

recodebeam_test_SOURCES = recodebeam_test.cc
recodebeam_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

rect_test_SOURCES = rect_test.cc
rect_test_LDADD = $(TESS_LIBS)

resultiterator_test_SOURCES = resultiterator_test.cc
resultiterator_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)
resultiterator_test_LDADD += $(LEPTONICA_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

scanutils_test_SOURCES = scanutils_test.cc
scanutils_test_LDADD = $(TRAINING_LIBS)

shapetable_test_SOURCES = shapetable_test.cc
shapetable_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS)

stats_test_SOURCES = stats_test.cc
stats_test_LDADD = $(TESS_LIBS)

if TENSORFLOW
stridemap_test_SOURCES = stridemap_test.cc
stridemap_test_LDADD = $(TESS_LIBS)
endif # TENSORFLOW

stringrenderer_test_SOURCES = stringrenderer_test.cc
stringrenderer_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)
stringrenderer_test_LDADD += $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
stringrenderer_test_LDADD += -lfontconfig -lpangocairo-1.0 -lpangoft2-1.0
stringrenderer_test_LDADD += $(cairo_LIBS) $(pango_LIBS)

tablefind_test_SOURCES = tablefind_test.cc
tablefind_test_LDADD = $(TESS_LIBS)

tablerecog_test_SOURCES = tablerecog_test.cc
tablerecog_test_LDADD = $(TESS_LIBS)

tabvector_test_SOURCES = tabvector_test.cc
tabvector_test_LDADD = $(TESS_LIBS)

if TENSORFLOW
tatweel_test_SOURCES = tatweel_test.cc
tatweel_test_SOURCES += third_party/utf/rune.c
tatweel_test_SOURCES += util/utf8/unicodetext.cc util/utf8/unilib.cc
tatweel_test_LDADD = $(TRAINING_LIBS)
endif # TENSORFLOW

textlineprojection_test_SOURCES = textlineprojection_test.cc
textlineprojection_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(LEPTONICA_LIBS)

tfile_test_SOURCES = tfile_test.cc
tfile_test_LDADD = $(TESS_LIBS)

unichar_test_SOURCES = unichar_test.cc
unichar_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

unicharcompress_test_SOURCES = unicharcompress_test.cc
unicharcompress_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_UC_LIBS)

unicharset_test_SOURCES = unicharset_test.cc
unicharset_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

validate_grapheme_test_SOURCES = validate_grapheme_test.cc
validate_grapheme_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_indic_test_SOURCES = validate_indic_test.cc
validate_indic_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_khmer_test_SOURCES = validate_khmer_test.cc
validate_khmer_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validate_myanmar_test_SOURCES = validate_myanmar_test.cc
validate_myanmar_test_LDADD = $(ABSEIL_LIBS) $(TRAINING_LIBS) $(ICU_I18N_LIBS) $(ICU_UC_LIBS)

validator_test_SOURCES = validator_test.cc
validator_test_LDADD = $(TRAINING_LIBS) $(ICU_UC_LIBS)

# for windows
if T_WIN
apiexample_test_LDADD += -lws2_32
intsimdmatrix_test_LDADD += -lws2_32
matrix_test_LDADD += -lws2_32
osd_test_LDADD += -lws2_32
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
