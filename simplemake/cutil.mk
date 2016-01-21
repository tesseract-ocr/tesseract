CUTIL_HDR = \
            ../cutil/bitvec.h \
            ../cutil/callcpp.h \
            ../cutil/const.h \
            ../cutil/cutil.h \
            ../cutil/cutil_class.h \
            ../cutil/danerror.h \
            ../cutil/efio.h \
            ../cutil/emalloc.h \
            ../cutil/freelist.h \
            ../cutil/globals.h \
            ../cutil/listio.h \
            ../cutil/oldlist.h \
            ../cutil/structures.h

CUTIL_SRC = \
            ../cutil/bitvec.cpp \
            ../cutil/callcpp.cpp \
            ../cutil/cutil.cpp \
            ../cutil/cutil_class.cpp \
            ../cutil/danerror.cpp \
            ../cutil/efio.cpp \
            ../cutil/emalloc.cpp \
            ../cutil/freelist.cpp \
            ../cutil/listio.cpp \
            ../cutil/oldlist.cpp \
            ../cutil/structures.cpp

CUTIL_OBJ = $(CUTIL_SRC:.cpp=.o)
$(CUTIL_OBJ): $(CUTIL_HDR)
