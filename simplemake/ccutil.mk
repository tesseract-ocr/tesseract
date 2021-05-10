CCUTIL_HDR = \
	../include/tesseract/export.h \
	../include/tesseract/publictypes.h \
	../include/tesseract/unichar.h \
	../src/ccutil/ccutil.h \
	../src/ccutil/clst.h \
	../src/ccutil/elst2.h \
	../src/ccutil/elst.h \
	../src/ccutil/errcode.h \
	../src/ccutil/fileerr.h \
	../src/ccutil/genericheap.h \
	../src/ccutil/genericvector.h \
	../src/ccutil/helpers.h \
	../src/ccutil/host.h \
	../src/ccutil/kdpair.h \
	../src/ccutil/lsterr.h \
	../src/ccutil/object_cache.h \
	../src/ccutil/params.h \
	../src/ccutil/qrsequence.h \
	../src/ccutil/scanutils.h \
	../src/ccutil/serialis.h \
	../src/ccutil/sorthelper.h \
	../src/ccutil/tessdatamanager.h \
	../src/ccutil/tprintf.h \
	../src/ccutil/unicharcompress.h \
	../src/ccutil/unicharmap.h \
	../src/ccutil/unicharset.h \
	../src/ccutil/unicity_table.h

CCUTIL_SRC = \
	../src/ccutil/ccutil.cpp \
	../src/ccutil/clst.cpp \
	../src/ccutil/elst2.cpp \
	../src/ccutil/elst.cpp \
	../src/ccutil/errcode.cpp \
	../src/ccutil/mainblk.cpp \
	../src/ccutil/params.cpp \
	../src/ccutil/scanutils.cpp \
	../src/ccutil/serialis.cpp \
	../src/ccutil/tessdatamanager.cpp \
	../src/ccutil/tprintf.cpp \
	../src/ccutil/unichar.cpp \
	../src/ccutil/unicharcompress.cpp \
	../src/ccutil/unicharmap.cpp \
	../src/ccutil/unicharset.cpp

CCUTIL_LEGACY_SRC = \
	../src/ccutil/ambigs.cpp \
	../src/ccutil/bitvector.cpp \
	../src/ccutil/indexmapbidi.cpp \
	../src/ccutil/universalambigs.cpp

CCUTIL_LEGACY_HDR = \
	../src/ccutil/ambigs.h \
	../src/ccutil/bitvector.h \
	../src/ccutil/indexmapbidi.h \
	../src/ccutil/universalambigs.h

CCUTIL_OBJ = $(CCUTIL_SRC:.cpp=.o)
$(CCUTIL_OBJ): $(CCUTIL_HDR)

CCUTIL_LEGACY_OBJ = $(CCUTIL_LEGACY_SRC:.cpp=.o)
$(CCUTIL_LEGACY_OBJ): $(CCUTIL_HDR) $(CCUTIL_LEGACY_HDR)
