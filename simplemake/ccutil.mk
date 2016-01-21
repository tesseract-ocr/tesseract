CCUTIL_HDR = \
             ../ccutil/ambigs.h \
             ../ccutil/bits16.h \
             ../ccutil/bitvector.h \
             ../ccutil/ccutil.h \
             ../ccutil/clst.h \
             ../ccutil/doubleptr.h \
             ../ccutil/elst2.h \
             ../ccutil/elst.h \
             ../ccutil/genericheap.h \
             ../ccutil/globaloc.h \
             ../ccutil/hashfn.h \
             ../ccutil/indexmapbidi.h \
             ../ccutil/kdpair.h \
             ../ccutil/lsterr.h \
             ../ccutil/nwmain.h \
             ../ccutil/object_cache.h \
             ../ccutil/qrsequence.h \
             ../ccutil/scanutils.h \
             ../ccutil/sorthelper.h \
             ../ccutil/stderr.h \
             ../ccutil/tessdatamanager.h \
             ../ccutil/tprintf.h \
             ../ccutil/unicity_table.h \
             ../ccutil/unicodes.h \
             ../ccutil/universalambigs.h

CCUTIL_INSTHDR = \
                 ../ccutil/basedir.h \
                 ../ccutil/errcode.h \
                 ../ccutil/fileerr.h \
                 ../ccutil/genericvector.h \
                 ../ccutil/helpers.h \
                 ../ccutil/host.h \
                 ../ccutil/memry.h \
                 ../ccutil/ndminx.h \
                 ../ccutil/params.h \
                 ../ccutil/ocrclass.h \
                 ../ccutil/platform.h \
                 ../ccutil/serialis.h \
                 ../ccutil/strngs.h \
                 ../ccutil/tesscallback.h \
                 ../ccutil/unichar.h \
                 ../ccutil/unicharmap.h \
                 ../ccutil/unicharset.h

CCUTIL_SRC = \
             ../ccutil/ambigs.cpp \
             ../ccutil/basedir.cpp \
             ../ccutil/bits16.cpp \
             ../ccutil/bitvector.cpp \
             ../ccutil/ccutil.cpp \
             ../ccutil/clst.cpp \
             ../ccutil/elst2.cpp \
             ../ccutil/elst.cpp \
             ../ccutil/errcode.cpp \
             ../ccutil/globaloc.cpp \
             ../ccutil/indexmapbidi.cpp \
             ../ccutil/mainblk.cpp \
             ../ccutil/params.cpp \
             ../ccutil/memry.cpp \
             ../ccutil/scanutils.cpp \
             ../ccutil/serialis.cpp \
             ../ccutil/strngs.cpp \
             ../ccutil/tessdatamanager.cpp \
             ../ccutil/tprintf.cpp \
             ../ccutil/unichar.cpp \
             ../ccutil/unicharmap.cpp \
             ../ccutil/unicharset.cpp \
             ../ccutil/unicodes.cpp \
             ../ccutil/universalambigs.cpp

CCUTIL_OBJ = $(CCUTIL_SRC:.cpp=.o)
$(CCUTIL_OBJ): $(CCUTIL_HDR) $(CCUTIL_INSTHDR)
