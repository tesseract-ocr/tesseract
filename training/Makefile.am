SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/textord -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/display \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil

include_HEADERS = \
    commontraining.h tessopt.h mergenf.h

noinst_LTLIBRARIES = libtesseract_training.la libtesseract_tessopt.la

libtesseract_training_la_SOURCES = \
    commontraining.cpp
libtesseract_training_la_LDFLAGS = -static
libtesseract_tessopt_la_SOURCES = \
    tessopt.cpp
libtesseract_tessopt_la_LDFLAGS = -static

bin_PROGRAMS = cntraining combine_tessdata mftraining unicharset_extractor wordlist2dawg
combine_tessdata_SOURCES = combine_tessdata.cpp
if USING_MULTIPLELIBS
combine_tessdata_LDADD = \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
else
combine_tessdata_LDADD = \
    $(top_srcdir)/api/libtesseract.la
endif

cntraining_SOURCES = cntraining.cpp
cntraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
if USING_MULTIPLELIBS
cntraining_LDADD += \
    $(top_srcdir)/textord/libtesseract_textord.la \
    $(top_srcdir)/classify/libtesseract_classify.la \
    $(top_srcdir)/dict/libtesseract_dict.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la \
    $(top_srcdir)/image/libtesseract_image.la \
    $(top_srcdir)/cutil/libtesseract_cutil.la \
    $(top_srcdir)/viewer/libtesseract_viewer.la \
    $(top_srcdir)/ccmain/libtesseract_main.la \
    $(top_srcdir)/cube/libtesseract_cube.la \
    $(top_srcdir)/neural_networks/runtime/libtesseract_neural.la \
    $(top_srcdir)/wordrec/libtesseract_wordrec.la \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
else
cntraining_LDADD += \
    $(top_srcdir)/api/libtesseract.la
endif

mftraining_SOURCES = mftraining.cpp mergenf.cpp
mftraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la
if USING_MULTIPLELIBS
mftraining_LDADD += \
    $(top_srcdir)/textord/libtesseract_textord.la \
    $(top_srcdir)/classify/libtesseract_classify.la \
    $(top_srcdir)/dict/libtesseract_dict.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la \
    $(top_srcdir)/image/libtesseract_image.la \
    $(top_srcdir)/cutil/libtesseract_cutil.la \
    $(top_srcdir)/viewer/libtesseract_viewer.la \
    $(top_srcdir)/ccmain/libtesseract_main.la \
    $(top_srcdir)/cube/libtesseract_cube.la \
    $(top_srcdir)/neural_networks/runtime/libtesseract_neural.la \
    $(top_srcdir)/wordrec/libtesseract_wordrec.la \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
else
mftraining_LDADD += \
    $(top_srcdir)/api/libtesseract.la
endif

unicharset_extractor_SOURCES = unicharset_extractor.cpp
unicharset_extractor_LDADD = \
    libtesseract_tessopt.la
if USING_MULTIPLELIBS
unicharset_extractor_LDADD += \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
else
unicharset_extractor_LDADD += \
    $(top_srcdir)/api/libtesseract.la
endif

wordlist2dawg_SOURCES = wordlist2dawg.cpp
wordlist2dawg_LDADD = \
    libtesseract_tessopt.la
if USING_MULTIPLELIBS
wordlist2dawg_LDADD += \
    $(top_srcdir)/classify/libtesseract_classify.la \
    $(top_srcdir)/dict/libtesseract_dict.la \
    $(top_srcdir)/ccstruct/libtesseract_ccstruct.la \
    $(top_srcdir)/image/libtesseract_image.la \
    $(top_srcdir)/cutil/libtesseract_cutil.la \
    $(top_srcdir)/viewer/libtesseract_viewer.la \
    $(top_srcdir)/ccmain/libtesseract_main.la \
    $(top_srcdir)/cube/libtesseract_cube.la \
    $(top_srcdir)/neural_networks/runtime/libtesseract_neural.la \
    $(top_srcdir)/wordrec/libtesseract_wordrec.la \
    $(top_srcdir)/textord/libtesseract_textord.la \
    $(top_srcdir)/ccutil/libtesseract_ccutil.la
else
wordlist2dawg_LDADD += \
    $(top_srcdir)/api/libtesseract.la
endif
