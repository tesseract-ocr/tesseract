CUTIL_LEGACY_HDR = \
	../src/cutil/bitvec.h \
	../src/cutil/oldlist.h

CUTIL_LEGACY_SRC = \
	../src/cutil/oldlist.cpp

CUTIL_LEGACY_OBJ = $(CUTIL_LEGACY_SRC:.cpp=.o)
$(CUTIL_LEGACY_OBJ): $(CUTIL_LEGACY_HDR)
