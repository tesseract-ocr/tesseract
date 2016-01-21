CLASSIFY_HDR = \
               ../classify/adaptive.h \
               ../classify/blobclass.h \
               ../classify/classify.h \
               ../classify/cluster.h \
               ../classify/clusttool.h \
               ../classify/cutoffs.h \
               ../classify/errorcounter.h \
               ../classify/featdefs.h \
               ../classify/float2int.h \
               ../classify/fpoint.h \
               ../classify/intfeaturedist.h \
               ../classify/intfeaturemap.h \
               ../classify/intfeaturespace.h \
               ../classify/intfx.h \
               ../classify/intmatcher.h \
               ../classify/intproto.h \
               ../classify/kdtree.h \
               ../classify/mastertrainer.h \
               ../classify/mf.h \
               ../classify/mfdefs.h \
               ../classify/mfoutline.h \
               ../classify/mfx.h \
               ../classify/normfeat.h \
               ../classify/normmatch.h \
               ../classify/ocrfeatures.h \
               ../classify/outfeat.h \
               ../classify/picofeat.h \
               ../classify/protos.h \
               ../classify/sampleiterator.h \
               ../classify/shapeclassifier.h \
               ../classify/shapetable.h \
               ../classify/tessclassifier.h \
               ../classify/trainingsample.h \
               ../classify/trainingsampleset.h

CLASSIFY_SRC = \
               ../classify/adaptive.cpp \
               ../classify/adaptmatch.cpp \
               ../classify/blobclass.cpp \
               ../classify/classify.cpp \
               ../classify/cluster.cpp \
               ../classify/clusttool.cpp \
               ../classify/cutoffs.cpp \
               ../classify/errorcounter.cpp \
               ../classify/featdefs.cpp \
               ../classify/float2int.cpp \
               ../classify/fpoint.cpp \
               ../classify/intfeaturedist.cpp \
               ../classify/intfeaturemap.cpp \
               ../classify/intfeaturespace.cpp \
               ../classify/intfx.cpp \
               ../classify/intmatcher.cpp \
               ../classify/intproto.cpp \
               ../classify/kdtree.cpp \
               ../classify/mastertrainer.cpp \
               ../classify/mf.cpp \
               ../classify/mfdefs.cpp \
               ../classify/mfoutline.cpp \
               ../classify/mfx.cpp \
               ../classify/normfeat.cpp \
               ../classify/normmatch.cpp \
               ../classify/ocrfeatures.cpp \
               ../classify/outfeat.cpp \
               ../classify/picofeat.cpp \
               ../classify/protos.cpp \
               ../classify/sampleiterator.cpp \
               ../classify/shapeclassifier.cpp \
               ../classify/shapetable.cpp \
               ../classify/tessclassifier.cpp \
               ../classify/trainingsample.cpp \
               ../classify/trainingsampleset.cpp

CLASSIFY_OBJ = $(CLASSIFY_SRC:.cpp=.o)
$(CLASSIFY_OBJ): $(CLASSIFY_HDR)
