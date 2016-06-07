TRAINING_HDR = \
               ../training/boxchar.h \
               ../training/commandlineflags.h \
               ../training/commontraining.h \
               ../training/degradeimage.h \
               ../training/fileio.h \
               ../training/icuerrorcode.h \
               ../training/ligature_table.h \
               ../training/mergenf.h \
               ../training/normstrngs.h \
               ../training/pango_font_info.h \
               ../training/stringrenderer.h \
               ../training/tessopt.h \
               ../training/tlog.h \
               ../training/unicharset_training_utils.h \
               ../training/util.h

TRAINING_SRC = \
               ../training/boxchar.cpp \
               ../training/commandlineflags.cpp \
               ../training/commontraining.cpp \
               ../training/degradeimage.cpp \
               ../training/fileio.cpp \
               ../training/ligature_table.cpp \
               ../training/mergenf.cpp \
               ../training/normstrngs.cpp \
               ../training/pango_font_info.cpp \
               ../training/stringrenderer.cpp \
               ../training/tessopt.cpp \
               ../training/tlog.cpp \
               ../training/unicharset_training_utils.cpp

TRAINING_OBJ = $(TRAINING_SRC:.cpp=.o)
$(TRAINING_OBJ): $(TRAINING_HDR)

TRAINING_BINSRC = \
               ../training/ambiguous_words.cpp \
               ../training/classifier_tester.cpp \
               ../training/cntraining.cpp \
               ../training/combine_tessdata.cpp \
               ../training/dawg2wordlist.cpp \
               ../training/mftraining.cpp \
               ../training/set_unicharset_properties.cpp \
               ../training/shapeclustering.cpp \
               ../training/text2image.cpp \
               ../training/unicharset_extractor.cpp \
               ../training/wordlist2dawg.cpp

TRAINING_BIN = $(TRAINING_BINSRC:.cpp=)

TRAINING_EXTRA = \
               ../training/language-specific.sh \
               ../training/tesstrain.sh \
               ../training/tesstrain_utils.sh

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
