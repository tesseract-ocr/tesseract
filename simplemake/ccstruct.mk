CCSTRUCT_HDR = \
               ../ccstruct/blamer.h \
               ../ccstruct/blckerr.h \
               ../ccstruct/blobbox.h \
               ../ccstruct/blobs.h \
               ../ccstruct/blread.h \
               ../ccstruct/boxread.h \
               ../ccstruct/boxword.h \
               ../ccstruct/ccstruct.h \
               ../ccstruct/coutln.h \
               ../ccstruct/crakedge.h \
               ../ccstruct/detlinefit.h \
               ../ccstruct/dppoint.h \
               ../ccstruct/fontinfo.h \
               ../ccstruct/genblob.h \
               ../ccstruct/hpdsizes.h \
               ../ccstruct/imagedata.h \
               ../ccstruct/ipoints.h \
               ../ccstruct/linlsq.h \
               ../ccstruct/matrix.h \
               ../ccstruct/mod128.h \
               ../ccstruct/normalis.h \
               ../ccstruct/ocrblock.h \
               ../ccstruct/ocrpara.h \
               ../ccstruct/ocrrow.h \
               ../ccstruct/otsuthr.h \
               ../ccstruct/pageres.h \
               ../ccstruct/params_training_featdef.h \
               ../ccstruct/pdblock.h \
               ../ccstruct/points.h \
               ../ccstruct/polyaprx.h \
               ../ccstruct/polyblk.h \
               ../ccstruct/quadlsq.h \
               ../ccstruct/quadratc.h \
               ../ccstruct/quspline.h \
               ../ccstruct/ratngs.h \
               ../ccstruct/rect.h \
               ../ccstruct/rejctmap.h \
               ../ccstruct/seam.h \
               ../ccstruct/split.h \
               ../ccstruct/statistc.h \
               ../ccstruct/stepblob.h \
               ../ccstruct/vecfuncs.h \
               ../ccstruct/werd.h

CCSTRUCT_INSTHDR = ../ccstruct/publictypes.h

CCSTRUCT_SRC = \
               ../ccstruct/blamer.cpp \
               ../ccstruct/blobbox.cpp \
               ../ccstruct/blobs.cpp \
               ../ccstruct/blread.cpp \
               ../ccstruct/boxread.cpp \
               ../ccstruct/boxword.cpp \
               ../ccstruct/ccstruct.cpp \
               ../ccstruct/coutln.cpp \
               ../ccstruct/detlinefit.cpp \
               ../ccstruct/dppoint.cpp \
               ../ccstruct/fontinfo.cpp \
               ../ccstruct/genblob.cpp \
               ../ccstruct/imagedata.cpp \
               ../ccstruct/linlsq.cpp \
               ../ccstruct/matrix.cpp \
               ../ccstruct/mod128.cpp \
               ../ccstruct/normalis.cpp \
               ../ccstruct/ocrblock.cpp \
               ../ccstruct/ocrpara.cpp \
               ../ccstruct/ocrrow.cpp \
               ../ccstruct/otsuthr.cpp \
               ../ccstruct/pageres.cpp \
               ../ccstruct/pdblock.cpp \
               ../ccstruct/points.cpp \
               ../ccstruct/polyaprx.cpp \
               ../ccstruct/polyblk.cpp \
               ../ccstruct/params_training_featdef.cpp \
               ../ccstruct/publictypes.cpp \
               ../ccstruct/quadlsq.cpp \
               ../ccstruct/quspline.cpp \
               ../ccstruct/ratngs.cpp \
               ../ccstruct/rect.cpp \
               ../ccstruct/rejctmap.cpp \
               ../ccstruct/seam.cpp \
               ../ccstruct/split.cpp \
               ../ccstruct/statistc.cpp \
               ../ccstruct/stepblob.cpp \
               ../ccstruct/vecfuncs.cpp \
               ../ccstruct/werd.cpp

CCSTRUCT_OBJ = $(CCSTRUCT_SRC:.cpp=.o)
$(CCSTRUCT_OBJ): $(CCSTRUCT_HDR) $(CCSTRUCT_INSTHDR)
