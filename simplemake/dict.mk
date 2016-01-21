DICT_HDR = \
           ../dict/dawg.h \
           ../dict/dawg_cache.h \
           ../dict/dict.h \
           ../dict/matchdefs.h \
           ../dict/stopper.h \
           ../dict/trie.h

DICT_SRC = \
           ../dict/context.cpp \
           ../dict/dawg.cpp \
           ../dict/dawg_cache.cpp \
           ../dict/dict.cpp \
           ../dict/hyphen.cpp \
           ../dict/permdawg.cpp \
           ../dict/stopper.cpp \
           ../dict/trie.cpp

DICT_OBJ = $(DICT_SRC:.cpp=.o)
$(DICT_OBJ): $(DICT_HDR)
