AM_CPPFLAGS += \
    -DUSE_STD_NAMESPACE \
    -I$(top_srcdir)/cutil -I$(top_srcdir)/ccutil \
    -I$(top_srcdir)/ccstruct -I$(top_srcdir)/dict \
    -I$(top_srcdir)/ccmain -I$(top_srcdir)/classify \
    -I$(top_srcdir)/textord -I$(top_srcdir)/wordrec \
    -I$(top_srcdir)/neural_networks/runtime \
    -I$(top_srcdir)/image -I$(top_srcdir)/viewer
        
if VISIBILITY
AM_CPPFLAGS += -DTESS_EXPORTS \
    -fvisibility=hidden -fvisibility-inlines-hidden
endif

noinst_HEADERS = \
    altlist.h beam_search.h bmp_8.h cached_file.h \
    char_altlist.h char_bigrams.h char_samp.h char_samp_enum.h \
    char_samp_set.h char_set.h classifier_base.h classifier_factory.h \
    con_comp.h cube_const.h conv_net_classifier.h cube_line_object.h \
    cube_line_segmenter.h cube_object.h cube_search_object.h \
    cube_tuning_params.h cube_utils.h feature_base.h feature_bmp.h \
    feature_chebyshev.h feature_hybrid.h hybrid_neural_net_classifier.h \
    lang_mod_edge.h lang_model.h search_column.h search_node.h \
    search_object.h string_32.h tess_lang_mod_edge.h tess_lang_model.h \
    tuning_params.h word_altlist.h word_list_lang_model.h word_size_model.h \
    word_unigrams.h

if !USING_MULTIPLELIBS
noinst_LTLIBRARIES = libtesseract_cube.la
else
lib_LTLIBRARIES = libtesseract_cube.la
libtesseract_cube_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION)
libtesseract_cube_la_LIBADD = \
    ../ccstruct/libtesseract_ccstruct.la \
    ../ccutil/libtesseract_ccutil.la \
    ../neural_networks/runtime/libtesseract_neural.la \
    ../image/libtesseract_image.la \
    ../viewer/libtesseract_viewer.la \
    ../wordrec/libtesseract_wordrec.la \
    ../cutil/libtesseract_cutil.la \
    ../classify/libtesseract_classify.la \
    ../dict/libtesseract_dict.la 
endif

libtesseract_cube_la_SOURCES = \
    altlist.cpp beam_search.cpp bmp_8.cpp cached_file.cpp \
    char_altlist.cpp char_bigrams.cpp char_samp.cpp char_samp_enum.cpp \
    char_samp_set.cpp char_set.cpp classifier_factory.cpp \
    con_comp.cpp conv_net_classifier.cpp cube_line_object.cpp \
    cube_line_segmenter.cpp cube_object.cpp cube_search_object.cpp \
    cube_tuning_params.cpp cube_utils.cpp feature_bmp.cpp \
    feature_chebyshev.cpp feature_hybrid.cpp hybrid_neural_net_classifier.cpp \
    search_column.cpp search_node.cpp \
    tess_lang_mod_edge.cpp tess_lang_model.cpp \
    word_altlist.cpp word_list_lang_model.cpp word_size_model.cpp \
    word_unigrams.cpp
