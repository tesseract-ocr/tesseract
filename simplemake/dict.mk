DICT_HDR = \
           ../src/dict/dawg.h \
           ../src/dict/dawg_cache.h \
           ../src/dict/dict.h \
           ../src/dict/matchdefs.h \
           ../src/dict/stopper.h \
           ../src/dict/trie.h

DICT_SRC = \
           ../src/dict/context.cpp \
           ../src/dict/dawg.cpp \
           ../src/dict/dawg_cache.cpp \
           ../src/dict/dict.cpp \
           ../src/dict/hyphen.cpp \
           ../src/dict/permdawg.cpp \
           ../src/dict/stopper.cpp \
           ../src/dict/trie.cpp

DICT_OBJ = $(DICT_SRC:.cpp=.o)
$(DICT_OBJ): $(DICT_HDR)
