AM_CPPFLAGS += \
    -DPANGO_ENABLE_ENGINE \
    -I$(top_builddir)/include \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/src/api \
    -I$(top_srcdir)/src/ccmain \
    -I$(top_srcdir)/src/ccutil \
    -I$(top_srcdir)/src/ccstruct \
    -I$(top_srcdir)/src/lstm \
    -I$(top_srcdir)/src/arch \
    -I$(top_srcdir)/src/viewer \
    -I$(top_srcdir)/src/textord \
    -I$(top_srcdir)/src/dict \
    -I$(top_srcdir)/src/classify \
    -I$(top_srcdir)/src/wordrec \
    -I$(top_srcdir)/src/cutil

AM_CPPFLAGS += $(ICU_UC_CFLAGS) $(ICU_I18N_CFLAGS)
AM_CPPFLAGS += $(pango_CFLAGS)
AM_CPPFLAGS += $(cairo_CFLAGS)

bin_SCRIPTS = language-specific.sh tesstrain.sh
scripts_DATA = tesstrain_utils.sh
scriptsdir = $(bindir)

if DISABLED_LEGACY_ENGINE
AM_CPPFLAGS += -DDISABLED_LEGACY_ENGINE
endif

# TODO: training programs can not be linked to shared library created
# with -fvisibility
if VISIBILITY
AM_LDFLAGS += -all-static
endif

noinst_HEADERS = \
    boxchar.h \
    commandlineflags.h \
    commontraining.h \
    ctc.h \
    degradeimage.h \
    icuerrorcode.h \
    fileio.h \
    lang_model_helpers.h \
    ligature_table.h \
    lstmtester.h \
    lstmtrainer.h \
    networkbuilder.h \
    normstrngs.h \
    pango_font_info.h \
    stringrenderer.h \
    tessopt.h \
    tlog.h \
    unicharset_training_utils.h \
    util.h \
    validate_grapheme.h \
    validate_indic.h \
    validate_javanese.h \
    validate_khmer.h \
    validate_myanmar.h \
    validator.h

if !DISABLED_LEGACY_ENGINE
    noinst_HEADERS += errorcounter.h
    noinst_HEADERS += intfeaturedist.h
    noinst_HEADERS += intfeaturemap.h
    noinst_HEADERS += mastertrainer.h
    noinst_HEADERS += mergenf.h
    noinst_HEADERS += sampleiterator.h
    noinst_HEADERS += trainingsampleset.h
endif


noinst_LTLIBRARIES = libtesseract_training.la libtesseract_tessopt.la

libtesseract_training_la_SOURCES = \
    boxchar.cpp \
    commandlineflags.cpp \
    commontraining.cpp \
    ctc.cpp \
    degradeimage.cpp \
    icuerrorcode.cpp \
    fileio.cpp \
    lang_model_helpers.cpp \
    ligature_table.cpp \
    lstmtester.cpp \
    lstmtrainer.cpp \
    networkbuilder.cpp \
    normstrngs.cpp \
    pango_font_info.cpp \
    stringrenderer.cpp \
    tlog.cpp \
    unicharset_training_utils.cpp \
    validate_grapheme.cpp \
    validate_indic.cpp \
    validate_javanese.cpp \
    validate_khmer.cpp \
    validate_myanmar.cpp \
    validator.cpp

if !DISABLED_LEGACY_ENGINE
libtesseract_training_la_SOURCES += errorcounter.cpp
libtesseract_training_la_SOURCES += intfeaturedist.cpp
libtesseract_training_la_SOURCES += intfeaturemap.cpp
libtesseract_training_la_SOURCES += mastertrainer.cpp
libtesseract_training_la_SOURCES += sampleiterator.cpp
libtesseract_training_la_SOURCES += trainingsampleset.cpp
endif

libtesseract_tessopt_la_SOURCES = \
    tessopt.cpp

bin_PROGRAMS = \
    combine_lang_model \
    combine_tessdata \
    dawg2wordlist \
    lstmeval \
    lstmtraining \
    merge_unicharsets \
    set_unicharset_properties \
    text2image \
    unicharset_extractor \
    wordlist2dawg

if !DISABLED_LEGACY_ENGINE
bin_PROGRAMS += \
   ambiguous_words \
   classifier_tester \
   cntraining \
   mftraining \
   shapeclustering
endif

if !DISABLED_LEGACY_ENGINE
ambiguous_words_SOURCES = ambiguous_words.cpp
ambiguous_words_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
ambiguous_words_LDADD += \
    ../../libtesseract.la

classifier_tester_SOURCES = classifier_tester.cpp
#classifier_tester_LDFLAGS = -static
classifier_tester_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
classifier_tester_LDADD += \
    ../../libtesseract.la

cntraining_SOURCES = cntraining.cpp
#cntraining_LDFLAGS = -static
cntraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
cntraining_LDADD += \
    ../../libtesseract.la

mftraining_SOURCES = mftraining.cpp mergenf.cpp
#mftraining_LDFLAGS = -static
mftraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_UC_LIBS)
mftraining_LDADD += \
    ../../libtesseract.la

shapeclustering_SOURCES = shapeclustering.cpp
#shapeclustering_LDFLAGS = -static
shapeclustering_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
shapeclustering_LDADD += \
    ../../libtesseract.la
endif

combine_lang_model_SOURCES = combine_lang_model.cpp
#combine_lang_model_LDFLAGS = -static
combine_lang_model_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
combine_lang_model_LDADD += \
    ../../libtesseract.la

combine_tessdata_SOURCES = combine_tessdata.cpp
#combine_tessdata_LDFLAGS = -static
combine_tessdata_LDADD = \
    ../../libtesseract.la

dawg2wordlist_SOURCES = dawg2wordlist.cpp
#dawg2wordlist_LDFLAGS = -static
dawg2wordlist_LDADD = \
    libtesseract_tessopt.la
dawg2wordlist_LDADD += \
    ../../libtesseract.la

lstmeval_SOURCES = lstmeval.cpp
#lstmeval_LDFLAGS = -static
lstmeval_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_UC_LIBS)
lstmeval_LDADD += \
    ../../libtesseract.la

lstmtraining_SOURCES = lstmtraining.cpp
#lstmtraining_LDFLAGS = -static
lstmtraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
lstmtraining_LDADD += \
    ../../libtesseract.la

merge_unicharsets_SOURCES = merge_unicharsets.cpp
#merge_unicharsets_LDFLAGS = -static
merge_unicharsets_LDADD = \
    libtesseract_tessopt.la
merge_unicharsets_LDADD += \
    ../../libtesseract.la

set_unicharset_properties_SOURCES = set_unicharset_properties.cpp
set_unicharset_properties_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
set_unicharset_properties_LDADD += \
    ../../libtesseract.la

text2image_SOURCES = text2image.cpp
#text2image_LDFLAGS = -static
text2image_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
text2image_LDADD += \
    ../../libtesseract.la
text2image_LDADD += $(ICU_UC_LIBS) -lpango-1.0 -lpangocairo-1.0
text2image_LDADD += -lgobject-2.0 -lglib-2.0 -lcairo -lpangoft2-1.0 -lfontconfig

unicharset_extractor_SOURCES = unicharset_extractor.cpp
#unicharset_extractor_LDFLAGS = -static
unicharset_extractor_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)
unicharset_extractor_LDADD += \
    ../../libtesseract.la

wordlist2dawg_SOURCES = wordlist2dawg.cpp
#wordlist2dawg_LDFLAGS = -static
wordlist2dawg_LDADD = \
    libtesseract_tessopt.la
wordlist2dawg_LDADD += \
    ../../libtesseract.la

if T_WIN
if !DISABLED_LEGACY_ENGINE
ambiguous_words_LDADD += -lws2_32
classifier_tester_LDADD += -lws2_32
cntraining_LDADD += -lws2_32
mftraining_LDADD += -lws2_32
shapeclustering_LDADD += -lws2_32
endif

combine_tessdata_LDADD += -lws2_32
dawg2wordlist_LDADD += -lws2_32
merge_unicharsets_LDADD += -lws2_32
set_unicharset_properties_LDADD += -lws2_32
unicharset_extractor_LDADD += -lws2_32
text2image_LDADD += -lws2_32
wordlist2dawg_LDADD += -lws2_32
endif

if !DISABLED_LEGACY_ENGINE
ambiguous_words_LDFLAGS = $(OPENCL_LDFLAGS)
classifier_tester_LDFLAGS = $(OPENCL_LDFLAGS)
cntraining_LDFLAGS = $(OPENCL_LDFLAGS)
mftraining_LDFLAGS = $(OPENCL_LDFLAGS)
shapeclustering_LDFLAGS = $(OPENCL_LDFLAGS)
endif

combine_tessdata_LDFLAGS = $(OPENCL_LDFLAGS)
dawg2wordlist_LDFLAGS = $(OPENCL_LDFLAGS)
merge_unicharsets_LDFLAGS = $(OPENCL_LDFLAGS)
set_unicharset_properties_LDFLAGS = $(OPENCL_LDFLAGS)
text2image_LDFLAGS = $(OPENCL_LDFLAGS)
unicharset_extractor_LDFLAGS = $(OPENCL_LDFLAGS)
wordlist2dawg_LDFLAGS = $(OPENCL_LDFLAGS)

if !DISABLED_LEGACY_ENGINE
ambiguous_words_LDADD += $(LEPTONICA_LIBS)
classifier_tester_LDADD += $(LEPTONICA_LIBS)
cntraining_LDADD += $(LEPTONICA_LIBS)
mftraining_LDADD += $(LEPTONICA_LIBS)
shapeclustering_LDADD += $(LEPTONICA_LIBS)
endif

combine_tessdata_LDADD += $(LEPTONICA_LIBS)
dawg2wordlist_LDADD += $(LEPTONICA_LIBS)
lstmeval_LDADD += $(LEPTONICA_LIBS)
lstmtraining_LDADD += $(LEPTONICA_LIBS)
set_unicharset_properties_LDADD += $(LEPTONICA_LIBS)
text2image_LDADD += $(LEPTONICA_LIBS)
unicharset_extractor_LDADD += $(LEPTONICA_LIBS)
wordlist2dawg_LDADD += $(LEPTONICA_LIBS)

extralib = $(libarchive_LIBS)
extralib += $(TENSORFLOW_LIBS)

if !DISABLED_LEGACY_ENGINE
ambiguous_words_LDADD += $(extralib)
classifier_tester_LDADD += $(extralib)
cntraining_LDADD += $(extralib)
mftraining_LDADD += $(extralib)
shapeclustering_LDADD += $(extralib)
endif
combine_lang_model_LDADD += $(extralib)
combine_tessdata_LDADD += $(extralib)
dawg2wordlist_LDADD += $(extralib)
lstmeval_LDADD += $(extralib)
lstmtraining_LDADD += $(extralib)
merge_unicharsets_LDADD += $(extralib)
set_unicharset_properties_LDADD += $(extralib)
text2image_LDADD += $(extralib)
unicharset_extractor_LDADD += $(extralib)
wordlist2dawg_LDADD += $(extralib)
