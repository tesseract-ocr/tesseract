TRAINING_HDR = \
	../src/training/common/commandlineflags.h \
	../src/training/common/commontraining.h \
	../src/training/common/ctc.h \
	../src/training/common/networkbuilder.h \
	../src/training/pango/boxchar.h \
	../src/training/pango/ligature_table.h \
	../src/training/pango/pango_font_info.h \
	../src/training/pango/stringrenderer.h \
	../src/training/pango/tlog.h \
	../src/training/unicharset/fileio.h \
	../src/training/unicharset/icuerrorcode.h \
	../src/training/unicharset/lang_model_helpers.h \
	../src/training/unicharset/lstmtester.h \
	../src/training/unicharset/lstmtrainer.h \
	../src/training/unicharset/normstrngs.h \
	../src/training/unicharset/unicharset_training_utils.h \
	../src/training/unicharset/validate_grapheme.h \
	../src/training/unicharset/validate_indic.h \
	../src/training/unicharset/validate_javanese.h \
	../src/training/unicharset/validate_khmer.h \
	../src/training/unicharset/validate_myanmar.h \
	../src/training/unicharset/validator.h \
	../src/training/degradeimage.h

TRAINING_LEGACY_HDR = \
	../src/training/common/errorcounter.h \
	../src/training/common/intfeaturedist.h \
	../src/training/common/intfeaturemap.h \
	../src/training/common/mastertrainer.h \
	../src/training/common/sampleiterator.h \
	../src/training/common/trainingsampleset.h \
	../src/training/mergenf.h

TRAINING_SRC = \
	../src/training/common/commandlineflags.cpp \
	../src/training/common/commontraining.cpp \
	../src/training/common/ctc.cpp \
	../src/training/common/networkbuilder.cpp \
	../src/training/pango/boxchar.cpp \
	../src/training/pango/ligature_table.cpp \
	../src/training/pango/pango_font_info.cpp \
	../src/training/pango/stringrenderer.cpp \
	../src/training/pango/tlog.cpp \
	../src/training/unicharset/fileio.cpp \
	../src/training/unicharset/icuerrorcode.cpp \
	../src/training/unicharset/lang_model_helpers.cpp \
	../src/training/unicharset/lstmtester.cpp \
	../src/training/unicharset/lstmtrainer.cpp \
	../src/training/unicharset/normstrngs.cpp \
	../src/training/unicharset/unicharset_training_utils.cpp \
	../src/training/unicharset/validate_grapheme.cpp \
	../src/training/unicharset/validate_indic.cpp \
	../src/training/unicharset/validate_javanese.cpp \
	../src/training/unicharset/validate_khmer.cpp \
	../src/training/unicharset/validate_myanmar.cpp \
	../src/training/unicharset/validator.cpp \
	../src/training/degradeimage.cpp

TRAINING_LEGACY_SRC = \
	../src/training/common/errorcounter.cpp \
	../src/training/common/intfeaturedist.cpp \
	../src/training/common/intfeaturemap.cpp \
	../src/training/common/mastertrainer.cpp \
	../src/training/common/sampleiterator.cpp \
	../src/training/common/trainingsampleset.cpp

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
