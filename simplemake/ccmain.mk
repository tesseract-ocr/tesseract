CCMAIN_HDR = \
             ../src/ccmain/control.h \
             ../src/ccmain/docqual.h \
             ../src/ccmain/equationdetect.h \
             ../src/ccmain/fixspace.h \
             ../src/ccmain/mutableiterator.h \
             ../src/ccmain/output.h \
             ../src/ccmain/paragraphs.h \
             ../src/ccmain/paragraphs_internal.h \
             ../src/ccmain/paramsd.h \
             ../src/ccmain/pgedit.h \
             ../src/ccmain/reject.h \
             ../src/ccmain/tessedit.h \
             ../src/ccmain/tesseractclass.h \
             ../src/ccmain/tessvars.h \
             ../src/ccmain/werdit.h

CCMAIN_INSTHDR = \
                 ../src/ccmain/ltrresultiterator.h \
                 ../src/ccmain/pageiterator.h \
                 ../src/ccmain/resultiterator.h \
                 ../src/ccmain/osdetect.h \
                 ../src/ccmain/thresholder.h

CCMAIN_SRC = \
             ../src/ccmain/applybox.cpp \
             ../src/ccmain/control.cpp \
             ../src/ccmain/fixxht.cpp \
             ../src/ccmain/linerec.cpp \
             ../src/ccmain/ltrresultiterator.cpp \
             ../src/ccmain/mutableiterator.cpp \
             ../src/ccmain/output.cpp \
             ../src/ccmain/pageiterator.cpp \
             ../src/ccmain/pagesegmain.cpp \
             ../src/ccmain/pagewalk.cpp \
             ../src/ccmain/paragraphs.cpp \
             ../src/ccmain/paramsd.cpp \
             ../src/ccmain/pgedit.cpp \
             ../src/ccmain/recogtraining.cpp \
             ../src/ccmain/reject.cpp \
             ../src/ccmain/resultiterator.cpp \
             ../src/ccmain/tessedit.cpp \
             ../src/ccmain/tesseractclass.cpp \
             ../src/ccmain/tessvars.cpp \
             ../src/ccmain/thresholder.cpp \
             ../src/ccmain/werdit.cpp

CCMAIN_LEGACY_SRC = \
             ../src/ccmain/adaptions.cpp \
             ../src/ccmain/docqual.cpp \
             ../src/ccmain/equationdetect.cpp \
             ../src/ccmain/fixspace.cpp \
             ../src/ccmain/osdetect.cpp \
             ../src/ccmain/par_control.cpp \
             ../src/ccmain/superscript.cpp \
             ../src/ccmain/tessbox.cpp \
             ../src/ccmain/tfacepp.cpp

CCMAIN_OBJ = $(CCMAIN_SRC:.cpp=.o)
$(CCMAIN_OBJ): $(CCMAIN_HDR) $(CCMAIN_INSTHDR)

CCMAIN_LEGACY_OBJ = $(CCMAIN_LEGACY_SRC:.cpp=.o)
$(CCMAIN_LEGACY_OBJ): $(CCMAIN_HDR) $(CCMAIN_INSTHDR)
