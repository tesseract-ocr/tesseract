CLASSIFY_HDR = \
	../src/classify/classify.h

CLASSIFY_LEGACY_HDR = \
	../src/classify/adaptive.h \
	../src/classify/cluster.h \
	../src/classify/clusttool.h \
	../src/classify/featdefs.h \
	../src/classify/float2int.h \
	../src/classify/fpoint.h \
	../src/classify/intfeaturespace.h \
	../src/classify/intfx.h \
	../src/classify/intmatcher.h \
	../src/classify/intproto.h \
	../src/classify/kdtree.h \
	../src/classify/mf.h \
	../src/classify/mfdefs.h \
	../src/classify/mfoutline.h \
	../src/classify/mfx.h \
	../src/classify/normfeat.h \
	../src/classify/normmatch.h \
	../src/classify/ocrfeatures.h \
	../src/classify/outfeat.h \
	../src/classify/picofeat.h \
	../src/classify/protos.h \
	../src/classify/shapeclassifier.h \
	../src/classify/shapetable.h \
	../src/classify/tessclassifier.h \
	../src/classify/trainingsample.h

CLASSIFY_SRC = \
	../src/classify/classify.cpp

CLASSIFY_LEGACY_SRC = \
	../src/classify/adaptive.cpp \
	../src/classify/adaptmatch.cpp \
	../src/classify/blobclass.cpp \
	../src/classify/cluster.cpp \
	../src/classify/clusttool.cpp \
	../src/classify/cutoffs.cpp \
	../src/classify/featdefs.cpp \
	../src/classify/float2int.cpp \
	../src/classify/fpoint.cpp \
	../src/classify/intfeaturespace.cpp \
	../src/classify/intfx.cpp \
	../src/classify/intmatcher.cpp \
	../src/classify/intproto.cpp \
	../src/classify/kdtree.cpp \
	../src/classify/mf.cpp \
	../src/classify/mfdefs.cpp \
	../src/classify/mfoutline.cpp \
	../src/classify/mfx.cpp \
	../src/classify/normfeat.cpp \
	../src/classify/normmatch.cpp \
	../src/classify/ocrfeatures.cpp \
	../src/classify/outfeat.cpp \
	../src/classify/picofeat.cpp \
	../src/classify/protos.cpp \
	../src/classify/shapeclassifier.cpp \
	../src/classify/shapetable.cpp \
	../src/classify/tessclassifier.cpp \
	../src/classify/trainingsample.cpp

CLASSIFY_OBJ = $(CLASSIFY_SRC:.cpp=.o)
$(CLASSIFY_OBJ): $(CLASSIFY_HDR)

CLASSIFY_LEGACY_OBJ = $(CLASSIFY_LEGACY_SRC:.cpp=.o)
$(CLASSIFY_LEGACY_OBJ): $(CLASSIFY_HDR) $(CLASSIFY_LEGACY_HDR)
