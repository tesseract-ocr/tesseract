CUTIL_HDR = \
            ../src/cutil/bitvec.h \
            ../src/cutil/callcpp.h \
            ../src/cutil/cutil_class.h \
            ../src/cutil/emalloc.h \
            ../src/cutil/oldlist.h \
            ../src/cutil/structures.h

CUTIL_SRC = \
            ../src/cutil/bitvec.cpp \
            ../src/cutil/callcpp.cpp \
            ../src/cutil/cutil_class.cpp \
            ../src/cutil/emalloc.cpp \
            ../src/cutil/oldlist.cpp \
            ../src/cutil/structures.cpp

CUTIL_OBJ = $(CUTIL_SRC:.cpp=.o)
$(CUTIL_OBJ): $(CUTIL_HDR)
