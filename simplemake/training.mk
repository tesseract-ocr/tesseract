TRAINING_HDR = \
               ../src/training/boxchar.h \
               ../src/training/commandlineflags.h \
               ../src/training/commontraining.h \
               ../src/training/degradeimage.h \
               ../src/training/icuerrorcode.h \
               ../src/training/lang_model_helpers.h \
               ../src/training/ligature_table.h \
               ../src/training/lstmtester.h \
               ../src/training/mergenf.h \
               ../src/training/normstrngs.h \
               ../src/training/pango_font_info.h \
               ../src/training/stringrenderer.h \
               ../src/training/tessopt.h \
               ../src/training/tlog.h \
               ../src/training/unicharset_training_utils.h \
               ../src/training/util.h \
               ../src/training/validate_grapheme.h \
               ../src/training/validate_indic.h \
               ../src/training/validate_javanese.h \
               ../src/training/validate_khmer.h \
               ../src/training/validate_myanmar.h \
               ../src/training/validator.h

TRAINING_SRC = \
               ../src/training/boxchar.cpp \
               ../src/training/commandlineflags.cpp \
               ../src/training/commontraining.cpp \
               ../src/training/degradeimage.cpp \
               ../src/training/icuerrorcode.cpp \
               ../src/training/lang_model_helpers.cpp \
               ../src/training/ligature_table.cpp \
               ../src/training/lstmtester.cpp \
               ../src/training/mergenf.cpp
               ../src/training/normstrngs.cpp \
               ../src/training/pango_font_info.cpp \
               ../src/training/stringrenderer.cpp \
               ../src/training/tessopt.cpp \
               ../src/training/tlog.cpp \
               ../src/training/unicharset_training_utils.cpp \
               ../src/training/validate_grapheme.cpp \
               ../src/training/validate_indic.cpp \
               ../src/training/validate_javanese.cpp \
               ../src/training/validate_khmer.cpp \
               ../src/training/validate_myanmar.cpp \
               ../src/training/validator.cpp

TRAINING_OBJ = $(TRAINING_SRC:.cpp=.o)
$(TRAINING_OBJ): $(TRAINING_HDR)

TRAINING_BINSRC = \
               ../src/training/combine_lang_model.cpp \
               ../src/training/combine_tessdata.cpp \
               ../src/training/dawg2wordlist.cpp \
               ../src/training/lstmeval.cpp \
               ../src/training/lstmtraining.cpp \
               ../src/training/merge_unicharsets.cpp \
               ../src/training/set_unicharset_properties.cpp \
               ../src/training/text2image.cpp \
               ../src/training/unicharset_extractor.cpp \
               ../src/training/wordlist2dawg.cpp

TRAINING_BIN = $(TRAINING_BINSRC:.cpp=)

TRAINING_LEGACY_BINSRC = \
               ../src/training/ambiguous_words.cpp \
               ../src/training/classifier_tester.cpp \
               ../src/training/cntraining.cpp \
               ../src/training/mftraining.cpp \
               ../src/training/shapeclustering.cpp \

TRAINING_LEGACY_BIN = $(TRAINING_LEGACY_BINSRC:.cpp=)

TRAINING_EXTRA = \
               ../src/training/language-specific.sh \
               ../src/training/tesstrain.sh \
               ../src/training/tesstrain_utils.sh

TRAINING_MAN = \
               ../doc/ambiguous_words.1 \
               ../doc/cntraining.1 \
               ../doc/combine_tessdata.1 \
               ../doc/dawg2wordlist.1 \
               ../doc/mftraining.1 \
               ../doc/shapeclustering.1 \
               ../doc/unicharset_extractor.1 \
               ../doc/wordlist2dawg.1

TRAINING_MAN5 = \
                ../doc/unicharambigs.5 \
                ../doc/unicharset.5
