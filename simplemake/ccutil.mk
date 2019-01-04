CCUTIL_HDR = \
             ../src/ccutil/ambigs.h \
             ../src/ccutil/basedir.h \
             ../src/ccutil/bits16.h \
             ../src/ccutil/bitvector.h \
             ../src/ccutil/ccutil.h \
             ../src/ccutil/clst.h \
             ../src/ccutil/doubleptr.h \
             ../src/ccutil/elst2.h \
             ../src/ccutil/elst.h \
             ../src/ccutil/errcode.h \
             ../src/ccutil/fileerr.h \
             ../src/ccutil/fileio.h \
             ../src/ccutil/genericheap.h \
             ../src/ccutil/globaloc.h \
             ../src/ccutil/indexmapbidi.h \
             ../src/ccutil/kdpair.h \
             ../src/ccutil/lsterr.h \
             ../src/ccutil/object_cache.h \
             ../src/ccutil/params.h \
             ../src/ccutil/qrsequence.h \
             ../src/ccutil/scanutils.h \
             ../src/ccutil/sorthelper.h \
             ../src/ccutil/tessdatamanager.h \
             ../src/ccutil/tprintf.h \
             ../src/ccutil/unicharcompress.h \
             ../src/ccutil/unicharmap.h \
             ../src/ccutil/unicharset.h
             ../src/ccutil/unicity_table.h \
             ../src/ccutil/unicodes.h \
             ../src/ccutil/universalambigs.h

CCUTIL_INSTHDR = \
                 ../src/ccutil/genericvector.h \
                 ../src/ccutil/helpers.h \
                 ../src/ccutil/host.h \
                 ../src/ccutil/ocrclass.h \
                 ../src/ccutil/platform.h \
                 ../src/ccutil/serialis.h \
                 ../src/ccutil/strngs.h \
                 ../src/ccutil/tesscallback.h \
                 ../src/ccutil/unichar.h

CCUTIL_SRC = \
             ../src/ccutil/ambigs.cpp \
             ../src/ccutil/basedir.cpp \
             ../src/ccutil/bitvector.cpp \
             ../src/ccutil/ccutil.cpp \
             ../src/ccutil/clst.cpp \
             ../src/ccutil/elst2.cpp \
             ../src/ccutil/elst.cpp \
             ../src/ccutil/errcode.cpp \
             ../src/ccutil/fileio.cpp \
             ../src/ccutil/globaloc.cpp \
             ../src/ccutil/indexmapbidi.cpp \
             ../src/ccutil/mainblk.cpp \
             ../src/ccutil/params.cpp \
             ../src/ccutil/scanutils.cpp \
             ../src/ccutil/serialis.cpp \
             ../src/ccutil/strngs.cpp \
             ../src/ccutil/tessdatamanager.cpp \
             ../src/ccutil/tprintf.cpp \
             ../src/ccutil/unichar.cpp \
             ../src/ccutil/unicharcompress.cpp \
             ../src/ccutil/unicharmap.cpp \
             ../src/ccutil/unicharset.cpp \
             ../src/ccutil/unicodes.cpp \
             ../src/ccutil/universalambigs.cpp

CCUTIL_OBJ = $(CCUTIL_SRC:.cpp=.o)
$(CCUTIL_OBJ): $(CCUTIL_HDR) $(CCUTIL_INSTHDR)
