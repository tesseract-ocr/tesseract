
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS += \
    -DUSE_STD_NAMESPACE \
    -DPANGO_ENABLE_ENGINE\
    -I$(top_srcdir)/api \
    -I$(top_srcdir)/arch \
    -I$(top_srcdir)/ccmain \
    -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/classify \
    -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/dict \
    -I$(top_srcdir)/lstm \
    -I$(top_srcdir)/textord \
    -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/wordrec


EXTRA_DIST = \
    language-specific.sh \
    tesstrain.sh \
    tesstrain_utils.sh


# TODO: training programs can not be linked to shared library created 
# with -fvisibility 
if VISIBILITY
AM_LDFLAGS += -all-static
endif


noinst_LTLIBRARIES = \
    libtesseract_training.la \
    libtesseract_tessopt.la

libtesseract_training_la_LIBADD = ../libtesseract.la 


libtesseract_training_la_SOURCES = \
    boxchar.h \
    commandlineflags.h \
    commontraining.h \
    degradeimage.h \
    fileio.h \
    icuerrorcode.h \
    ligature_table.h \
    lstmtester.h \
    normstrngs.h \
    mergenf.h \
    pango_font_info.h \
   stringrenderer.h \
    tlog.h \
    unicharset_training_utils.h \
    util.h

libtesseract_training_la_SOURCES += \
    boxchar.cpp \
    commandlineflags.cpp \
    commontraining.cpp \
    degradeimage.cpp \
    fileio.cpp \
    ligature_table.cpp \
     lstmtester.cpp \
    normstrngs.cpp \
    pango_font_info.cpp \
    stringrenderer.cpp \
    tlog.cpp \
    unicharset_training_utils.cpp


libtesseract_tessopt_la_SOURCES = \
    tessopt.h

libtesseract_tessopt_la_SOURCES += \
    tessopt.cpp


bin_PROGRAMS = \
    ambiguous_words \
    classifier_tester \
    cntraining \
    combine_tessdata \
    dawg2wordlist \
    lstmeval \
    lstmtraining \
    mftraining \
    set_unicharset_properties \
    shapeclustering \
    text2image \
    unicharset_extractor \
    wordlist2dawg


ambiguous_words_SOURCES = ambiguous_words.cpp

ambiguous_words_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la


classifier_tester_SOURCES = classifier_tester.cpp

classifier_tester_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la


combine_tessdata_SOURCES = combine_tessdata.cpp

combine_tessdata_LDADD = ../libtesseract.la


cntraining_SOURCES = cntraining.cpp

cntraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la


dawg2wordlist_SOURCES = dawg2wordlist.cpp

dawg2wordlist_LDADD = \
    libtesseract_tessopt.la \
    ../libtesseract.la


lstmeval_SOURCES = lstmeval.cpp

lstmeval_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la \
    $(ICU_UC_LIBS)


lstmtraining_SOURCES = lstmtraining.cpp

lstmtraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)


mftraining_SOURCES = \
    mftraining.cpp \
    mergenf.cpp

mftraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la \
    $(ICU_UC_LIBS)


set_unicharset_properties_SOURCES = set_unicharset_properties.cpp

set_unicharset_properties_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS)


shapeclustering_SOURCES = shapeclustering.cpp

shapeclustering_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la


text2image_SOURCES = text2image.cpp

text2image_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../libtesseract.la \
    $(ICU_I18N_LIBS) $(ICU_UC_LIBS) \
    -lgobject-2.0 \
    -lglib-2.0 \
    -lcairo \
    -lfontconfig \
    -lpango-1.0 \
    -lpangocairo-1.0 \
    -lpangoft2-1.0


unicharset_extractor_SOURCES = unicharset_extractor.cpp

unicharset_extractor_LDADD = \
    libtesseract_tessopt.la \
    ../libtesseract.la


wordlist2dawg_SOURCES = wordlist2dawg.cpp

wordlist2dawg_LDADD = \
    libtesseract_tessopt.la \
    ../libtesseract.la


if T_WIN
ambiguous_words_LDADD += -lws2_32
classifier_tester_LDADD += -lws2_32
cntraining_LDADD += -lws2_32
combine_tessdata_LDADD += -lws2_32
dawg2wordlist_LDADD += -lws2_32
mftraining_LDADD += -lws2_32
set_unicharset_properties_LDADD += -lws2_32
shapeclustering_LDADD += -lws2_32
unicharset_extractor_LDADD += -lws2_32
text2image_LDADD += -lws2_32
wordlist2dawg_LDADD += -lws2_32

AM_CPPFLAGS += -I$(top_srcdir)/vs2010/port

endif  #  T_WIN


ambiguous_words_LDFLAGS = $(OPENCL_LDFLAGS)
classifier_tester_LDFLAGS = $(OPENCL_LDFLAGS)
cntraining_LDFLAGS = $(OPENCL_LDFLAGS)
combine_tessdata_LDFLAGS = $(OPENCL_LDFLAGS)
dawg2wordlist_LDFLAGS = $(OPENCL_LDFLAGS)
mftraining_LDFLAGS = $(OPENCL_LDFLAGS)
set_unicharset_properties_LDFLAGS = $(OPENCL_LDFLAGS)
shapeclustering_LDFLAGS = $(OPENCL_LDFLAGS)
text2image_LDFLAGS = $(OPENCL_LDFLAGS)
unicharset_extractor_LDFLAGS = $(OPENCL_LDFLAGS)
wordlist2dawg_LDFLAGS = $(OPENCL_LDFLAGS)

ambiguous_words_LDADD += $(LEPTONICA_LIBS)
classifier_tester_LDADD += $(LEPTONICA_LIBS)
cntraining_LDADD += $(LEPTONICA_LIBS)
combine_tessdata_LDADD += $(LEPTONICA_LIBS)
dawg2wordlist_LDADD += $(LEPTONICA_LIBS)
lstmeval_LDADD += $(LEPTONICA_LIBS)
lstmtraining_LDADD += $(LEPTONICA_LIBS)
mftraining_LDADD += $(LEPTONICA_LIBS)
set_unicharset_properties_LDADD += $(LEPTONICA_LIBS)
shapeclustering_LDADD += $(LEPTONICA_LIBS)
text2image_LDADD += $(LEPTONICA_LIBS)
unicharset_extractor_LDFLAGS += $(LEPTONICA_LIBS) 
wordlist2dawg_LDADD += $(LEPTONICA_LIBS)

