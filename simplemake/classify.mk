CLASSIFY_HDR = \
               ../src/classify/adaptive.h \
               ../src/classify/blobclass.h \
               ../src/classify/classify.h \
               ../src/classify/cluster.h \
               ../src/classify/clusttool.h \
               ../src/classify/cutoffs.h \
               ../src/classify/errorcounter.h \
               ../src/classify/featdefs.h \
               ../src/classify/float2int.h \
               ../src/classify/fpoint.h \
               ../src/classify/intfeaturedist.h \
               ../src/classify/intfeaturemap.h \
               ../src/classify/intfeaturespace.h \
               ../src/classify/intfx.h \
               ../src/classify/intmatcher.h \
               ../src/classify/intproto.h \
               ../src/classify/kdtree.h \
               ../src/classify/mastertrainer.h \
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
               ../src/classify/sampleiterator.h \
               ../src/classify/shapeclassifier.h \
               ../src/classify/shapetable.h \
               ../src/classify/tessclassifier.h \
               ../src/classify/trainingsample.h \
               ../src/classify/trainingsampleset.h

CLASSIFY_SRC = \
               ../src/classify/blobclass.cpp \
               ../src/classify/classify.cpp \
               ../src/classify/featdefs.cpp \
               ../src/classify/ocrfeatures.cpp \
               ../src/classify/protos.cpp

CLASSIFY_LEGACY_SRC = \
               ../src/classify/adaptive.cpp \
               ../src/classify/adaptmatch.cpp \
               ../src/classify/cluster.cpp \
               ../src/classify/clusttool.cpp \
               ../src/classify/cutoffs.cpp \
               ../src/classify/errorcounter.cpp \
               ../src/classify/float2int.cpp \
               ../src/classify/fpoint.cpp \
               ../src/classify/intfeaturedist.cpp \
               ../src/classify/intfeaturemap.cpp \
               ../src/classify/intfeaturespace.cpp \
               ../src/classify/intfx.cpp \
               ../src/classify/intmatcher.cpp \
               ../src/classify/intproto.cpp \
               ../src/classify/kdtree.cpp \
               ../src/classify/mastertrainer.cpp \
               ../src/classify/mf.cpp \
               ../src/classify/mfdefs.cpp \
               ../src/classify/mfoutline.cpp \
               ../src/classify/mfx.cpp \
               ../src/classify/normfeat.cpp \
               ../src/classify/normmatch.cpp \
               ../src/classify/outfeat.cpp \
               ../src/classify/picofeat.cpp \
               ../src/classify/sampleiterator.cpp \
               ../src/classify/shapeclassifier.cpp \
               ../src/classify/shapetable.cpp \
               ../src/classify/tessclassifier.cpp \
               ../src/classify/trainingsample.cpp \
               ../src/classify/trainingsampleset.cpp

CLASSIFY_OBJ = $(CLASSIFY_SRC:.cpp=.o)
$(CLASSIFY_OBJ): $(CLASSIFY_HDR)

CLASSIFY_LEGACY_OBJ = $(CLASSIFY_LEGACY_SRC:.cpp=.o)
$(CLASSIFY_LEGACY_OBJ): $(CLASSIFY_HDR)
