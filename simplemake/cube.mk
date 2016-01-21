CUBE_HDR = \
           ../cube/altlist.h \
           ../cube/beam_search.h \
           ../cube/bmp_8.h \
           ../cube/cached_file.h \
           ../cube/char_altlist.h \
           ../cube/char_bigrams.h \
           ../cube/char_samp.h \
           ../cube/char_samp_enum.h \
           ../cube/char_samp_set.h \
           ../cube/char_set.h \
           ../cube/classifier_base.h \
           ../cube/classifier_factory.h \
           ../cube/con_comp.h \
           ../cube/conv_net_classifier.h \
           ../cube/cube_const.h \
           ../cube/cube_line_object.h \
           ../cube/cube_line_segmenter.h \
           ../cube/cube_object.h \
           ../cube/cube_search_object.h \
           ../cube/cube_tuning_params.h \
           ../cube/cube_utils.h \
           ../cube/feature_base.h \
           ../cube/feature_bmp.h \
           ../cube/feature_chebyshev.h \
           ../cube/feature_hybrid.h \
           ../cube/hybrid_neural_net_classifier.h \
           ../cube/lang_mod_edge.h \
           ../cube/lang_model.h \
           ../cube/search_column.h \
           ../cube/search_node.h \
           ../cube/search_object.h \
           ../cube/string_32.h \
           ../cube/tess_lang_mod_edge.h \
           ../cube/tess_lang_model.h \
           ../cube/tuning_params.h \
           ../cube/word_altlist.h \
           ../cube/word_list_lang_model.h \
           ../cube/word_size_model.h \
           ../cube/word_unigrams.h

CUBE_SRC = \
           ../cube/altlist.cpp \
           ../cube/beam_search.cpp \
           ../cube/bmp_8.cpp \
           ../cube/cached_file.cpp \
           ../cube/char_altlist.cpp \
           ../cube/char_bigrams.cpp \
           ../cube/char_samp.cpp \
           ../cube/char_samp_enum.cpp \
           ../cube/char_samp_set.cpp \
           ../cube/char_set.cpp \
           ../cube/classifier_factory.cpp \
           ../cube/con_comp.cpp \
           ../cube/conv_net_classifier.cpp \
           ../cube/cube_line_object.cpp \
           ../cube/cube_line_segmenter.cpp \
           ../cube/cube_object.cpp \
           ../cube/cube_search_object.cpp \
           ../cube/cube_tuning_params.cpp \
           ../cube/cube_utils.cpp \
           ../cube/feature_bmp.cpp \
           ../cube/feature_chebyshev.cpp \
           ../cube/feature_hybrid.cpp \
           ../cube/hybrid_neural_net_classifier.cpp \
           ../cube/search_column.cpp \
           ../cube/search_node.cpp \
           ../cube/tess_lang_mod_edge.cpp \
           ../cube/tess_lang_model.cpp \
           ../cube/word_altlist.cpp \
           ../cube/word_list_lang_model.cpp \
           ../cube/word_size_model.cpp \
           ../cube/word_unigrams.cpp

CUBE_OBJ = $(CUBE_SRC:.cpp=.o)
$(CUBE_OBJ): $(CUBE_HDR)
