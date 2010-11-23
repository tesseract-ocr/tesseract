SUBDIRS =
AM_CPPFLAGS = \
    -I$(top_srcdir)/ccutil -I$(top_srcdir)/ccstruct \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer \
    -I$(top_srcdir)/ccops -I$(top_srcdir)/dict \
    -I$(top_srcdir)/classify -I$(top_srcdir)/display \
    -I$(top_srcdir)/wordrec -I$(top_srcdir)/cutil \
    -I$(top_srcdir)/textord

include_HEADERS = \
    commontraining.h mergenf.h tessopt.h

lib_LTLIBRARIES = libtesseract_training.la libtesseract_tessopt.la
libtesseract_training_la_SOURCES = \
    commontraining.cpp
libtesseract_tessopt_la_SOURCES = \
    tessopt.cpp

libtesseract_training_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)

libtesseract_tessopt_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)

bin_PROGRAMS = cntraining combine_tessdata mftraining unicharset_extractor wordlist2dawg
combine_tessdata_SOURCES = combine_tessdata.cpp
combine_tessdata_LDADD = \
    ../ccutil/libtesseract_ccutil.la

cntraining_SOURCES = cntraining.cpp
cntraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../textord/libtesseract_textord.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccmain/libtesseract_main.la \
    ../cube/libtesseract_cube.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../wordrec/libtesseract_wordrec.la \
    ../ccutil/libtesseract_ccutil.la

mftraining_SOURCES = mftraining.cpp mergenf.cpp
mftraining_LDADD = \
    libtesseract_training.la \
    libtesseract_tessopt.la \
    ../textord/libtesseract_textord.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccmain/libtesseract_main.la \
    ../cube/libtesseract_cube.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../wordrec/libtesseract_wordrec.la \
    ../ccutil/libtesseract_ccutil.la

unicharset_extractor_SOURCES = unicharset_extractor.cpp
unicharset_extractor_LDADD = \
    libtesseract_tessopt.la \
     ../ccutil/libtesseract_ccutil.la

wordlist2dawg_SOURCES = wordlist2dawg.cpp
wordlist2dawg_LDADD = \
    libtesseract_tessopt.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la \
    ../ccstruct/libtesseract_ccstruct.la \
    ../image/libtesseract_image.la \
    ../cutil/libtesseract_cutil.la \
    ../viewer/libtesseract_viewer.la \
    ../ccmain/libtesseract_main.la \
    ../cube/libtesseract_cube.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../wordrec/libtesseract_wordrec.la \
    ../textord/libtesseract_textord.la \
    ../ccutil/libtesseract_ccutil.la
