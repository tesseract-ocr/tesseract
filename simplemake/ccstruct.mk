CCSTRUCT_HDR = \
	../include/tesseract/publictypes.h \
	../include/tesseract/unichar.h \
	../src/ccstruct/blamer.h \
	../src/ccstruct/blobbox.h \
	../src/ccstruct/blobs.h \
	../src/ccstruct/blread.h \
	../src/ccstruct/boxread.h \
	../src/ccstruct/boxword.h \
	../src/ccstruct/ccstruct.h \
	../src/ccstruct/coutln.h \
	../src/ccstruct/crakedge.h \
	../src/ccstruct/debugpixa.h \
	../src/ccstruct/detlinefit.h \
	../src/ccstruct/dppoint.h \
	../src/ccstruct/image.h \
	../src/ccstruct/imagedata.h \
	../src/ccstruct/linlsq.h \
	../src/ccstruct/matrix.h \
	../src/ccstruct/mod128.h \
	../src/ccstruct/normalis.h \
	../src/ccstruct/ocrblock.h \
	../src/ccstruct/ocrpara.h \
	../src/ccstruct/ocrrow.h \
	../src/ccstruct/otsuthr.h \
	../src/ccstruct/pageres.h \
	../src/ccstruct/pdblock.h \
	../src/ccstruct/points.h \
	../src/ccstruct/polyaprx.h \
	../src/ccstruct/polyblk.h \
	../src/ccstruct/quadlsq.h \
	../src/ccstruct/quadratc.h \
	../src/ccstruct/quspline.h \
	../src/ccstruct/ratngs.h \
	../src/ccstruct/rect.h \
	../src/ccstruct/rejctmap.h \
	../src/ccstruct/seam.h \
	../src/ccstruct/split.h \
	../src/ccstruct/statistc.h \
	../src/ccstruct/stepblob.h \
	../src/ccstruct/tabletransfer.h \
	../src/ccstruct/werd.h

CCSTRUCT_SRC = \
	../src/ccstruct/blamer.cpp \
	../src/ccstruct/blobbox.cpp \
	../src/ccstruct/blobs.cpp \
	../src/ccstruct/blread.cpp \
	../src/ccstruct/boxread.cpp \
	../src/ccstruct/boxword.cpp \
	../src/ccstruct/ccstruct.cpp \
	../src/ccstruct/coutln.cpp \
	../src/ccstruct/detlinefit.cpp \
	../src/ccstruct/dppoint.cpp \
	../src/ccstruct/image.cpp \
	../src/ccstruct/imagedata.cpp \
	../src/ccstruct/linlsq.cpp \
	../src/ccstruct/matrix.cpp \
	../src/ccstruct/mod128.cpp \
	../src/ccstruct/normalis.cpp \
	../src/ccstruct/ocrblock.cpp \
	../src/ccstruct/ocrpara.cpp \
	../src/ccstruct/ocrrow.cpp \
	../src/ccstruct/otsuthr.cpp \
	../src/ccstruct/pageres.cpp \
	../src/ccstruct/pdblock.cpp \
	../src/ccstruct/points.cpp \
	../src/ccstruct/polyaprx.cpp \
	../src/ccstruct/polyblk.cpp \
	../src/ccstruct/quadlsq.cpp \
	../src/ccstruct/quspline.cpp \
	../src/ccstruct/ratngs.cpp \
	../src/ccstruct/rect.cpp \
	../src/ccstruct/rejctmap.cpp \
	../src/ccstruct/seam.cpp \
	../src/ccstruct/split.cpp \
	../src/ccstruct/statistc.cpp \
	../src/ccstruct/stepblob.cpp \
	../src/ccstruct/werd.cpp

CCSTRUCT_LEGACY_SRC = \
	../src/ccstruct/fontinfo.cpp \
	../src/ccstruct/params_training_featdef.cpp

CCSTRUCT_LEGACY_HDR = \
	../src/ccstruct/fontinfo.h \
	../src/ccstruct/params_training_featdef.h

CCSTRUCT_OBJ = $(CCSTRUCT_SRC:.cpp=.o)
$(CCSTRUCT_OBJ): $(CCSTRUCT_HDR)

CCSTRUCT_LEGACY_OBJ = $(CCSTRUCT_LEGACY_SRC:.cpp=.o)
$(CCSTRUCT_LEGACY_OBJ): $(CCSTRUCT_HDR) $(CCSTRUCT_LEGACY_HDR)
